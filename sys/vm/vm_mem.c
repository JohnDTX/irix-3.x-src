/*
 * $Source: /d2/3.7/src/sys/vm/RCS/vm_mem.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:57 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/cmap.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/text.h"
#include "../h/file.h"
#include "../h/inode.h"
#include "../h/map.h"
#include "../vm/vm.h"
#include "../h/buf.h"
#include "machine/pte.h"

/*
 * Allocate memory, and always succeed
 * by jolting page-out daemon
 * so as to obtain page frames.
 * To be used in conjunction with vmemfree().
 * This really returns real pages!
 */
vmemall(pte, size, p, type)
	register struct pte *pte;
	int size;
	struct proc *p;
{
	register int m;

	ASSERT((size > 0) && (size <= maxmem));
	while (size > 0) {
		if (freemem < desfree)
			outofmem();
		while (freemem == 0)
			sleep((caddr_t)&freemem, PSWP+2);
		m = MIN(size, freemem);
		(void) memall(pte, m, p, type);
		size -= m;
		pte += m;
	}
	if (freemem < desfree)
		outofmem();
	/*
	 * Always succeeds, but return success for
	 * vgetu and vgetpt (e.g.) which call either
	 * memall or vmemall depending on context.
	 */
	return (1);
}

/*
 * Free valid and reclaimable page frames belonging to the
 * count pages starting at pte.  If a page is valid
 * or reclaimable and locked (but not a system page), then
 * we simply mark the page as c_gone and let the pageout
 * daemon free the page when it is through with it.
 * If a page is reclaimable, and already in the free list, then
 * we mark the page as c_gone, and (of course) don't free it.
 *
 * Determines the largest contiguous cluster of
 * valid pages and frees them in one call to memfree.
 */
vmemfree(pte, count)
	register struct pte *pte;
	register int count;
{
	register struct cmap *c;
	register struct pte *spte;
	int size, pcnt;

	for (size = 0, pcnt = 0; count > 0; pte++, count--) {
		if (pte->pg_fod == 0 && pte->pg_pfnum) {
			c = &cmap[pgtocm(pte->pg_pfnum)];
#ifdef	OS_DEBUG
			if (pte->pg_pfnum != c->c_pfnum) {
			    printf("pte=%x *pte=%x c=%x cmap=%x ecmap=%x\n",
					   pte, *(long *)pte, c, cmap,
					   ecmap);
			    printf("c_pfnum=%x c_type=%d c_page=%d\n",
					       c->c_pfnum, c->c_type,
					       c->c_page);
			}
#endif
			ASSERT(pte->pg_pfnum == c->c_pfnum);
			ASSERT(c->c_iolocks == 0);
			pcnt++;
			if (c->c_lock && c->c_type != CSYS) {
				*(long *)pte &= PG_PROT;
				c->c_gone = 1;
				goto free;
			}
			if (c->c_free) {
				pcnt--;
				*(long *)pte &= PG_PROT;
				if (c->c_type == CTEXT)
					distpte(&text[c->c_ndx], (int)c->c_page, pte);
				c->c_gone = 1;
				goto free;
			}
			if (size == 0)
				spte = pte;
			size++;
			continue;
		}
#ifdef notdef
		/* Don't do anything with mapped ptes */
		if (pte->pg_fod && pte->pg_v)
			goto free;
#endif
		if (pte->pg_fod)
			*(long *)pte &= PG_PROT;
free:
		if (size) {
			memfree(spte, size, 1);
			size = 0;
		}
	}
	if (size)
		memfree(spte, size, 1);
	return (pcnt);
}

/*
 * Unlink a page frame from the free list -
 *
 * Performed if the page being reclaimed
 * is in the free list.
 */
munlink(pf)
	unsigned pf;
{
	register int next, prev;
	register struct cmap *c;

	c = &cmap[(short) pgtocm(pf)];
	next = c->c_next;
	prev = c->c_prev;
	cmap[prev].c_next = next;
	cmap[next].c_prev = prev;
	c->c_free = 0;
	/*
	 * Kick pager if that put us under its threshold.  Note that we
	 * DONT'T call outofmem here, because we can't afford to sleep(),
	 * as pagein (which calls us) assumes that.
	 */
	freemem--;
	if (freemem < minfree)
		wakeup((caddr_t)&proc[2]);
}

/*
 * Allocate memory -
 *
 * The free list appears as a doubly linked list
 * in the core map with cmap[0] serving as a header.
 */
memall(pte, size, p, type)
	register struct pte *pte;
	int size;
	struct proc *p;
{
	register struct cmap *c;
	register struct pte *rpte;
	register struct proc *rp;
	register int i;
	register short next;
	register unsigned curpos;
	register int s;

	s = spl6();
	if (size > freemem) {
		splx(s);
		return (0);
	}
	for (i = size; i > 0; i--) {
		curpos = cmap[CMHEAD].c_next;
		c = &cmap[curpos];
		freemem--;
		ASSERT(freemem >= 0);
		next = c->c_next;
		cmap[CMHEAD].c_next = next;
		cmap[next].c_prev = CMHEAD;
		ASSERT(c->c_free);
		ASSERT(!((c->c_type != CSYS) && c->c_lock));
		if (c->c_gone == 0 && c->c_type != CSYS) {
			if (c->c_type == CTEXT)
				rp = text[c->c_ndx].x_caddr;
			else
				rp = &proc[c->c_ndx];
			switch (c->c_type) {

			case CTEXT:
				rpte = tptopte(rp, c->c_page);
				break;

			case CDATA:
				rpte = dptopte(rp, c->c_page);
				break;

			case CSTACK:
				rpte = sptopte(rp, c->c_page);
				break;
			}
			/*
			 * Clear out the pagetable entry of the previous owner
			 * of this page
			 */
			rpte->pg_pfnum = 0;
			if (c->c_type == CTEXT)
				distpte(&text[c->c_ndx], (int)c->c_page, rpte);
		}
		switch (type) {

		case CSYS:
			c->c_ndx = 0;		/* XXX is this right? */
			break;

		case CTEXT:
			c->c_page = vtotp(p, ptetov(p, pte));
			c->c_ndx = p->p_textp - &text[0];
			break;

		case CDATA:
			c->c_page = vtodp(p, ptetov(p, pte));
			c->c_ndx = p->p_ndx;
			break;

		case CSTACK:
			c->c_page = vtosp(p, ptetov(p, pte));
			c->c_ndx = p->p_ndx;
			break;
		}
		initpte(pte, c->c_pfnum, 0);
		pte++;
		c->c_free = 0;
		c->c_gone = 0;
		ASSERT(!c->c_intrans && !c->c_want);
		c->c_lock = 1;
		c->c_type = type;
	}
	splx(s);
	return (size);
}

/*
 * Free memory -
 *
 * The page frames being returned are inserted
 * to the head/tail of the free list depending
 * on whether there is any possible future use of them.
 *
 * If the freemem count had been zero,
 * the processes sleeping for memory
 * are awakened.
 */
memfree(pte, size, detach)
	register struct pte *pte;
	register int size;
{
	register unsigned i, prev, next;
	register struct cmap *c;
	register int s;
	
	if (freemem < KLMAX)
		wakeup((caddr_t)&freemem);

	while (size > 0) {
		size--;
		i = pgtocm(pte->pg_pfnum);
		c = &cmap[(short) i];
#ifdef	OS_DEBUG
		if (pte->pg_pfnum != c->c_pfnum) {
		    printf("pte=%x *pte=%x c=%x cmap=%x ecmap=%x\n",
				   pte, *(long *)pte, c, cmap, ecmap);
		    printf("c_pfnum=%x c_type=%d c_page=%d\n",
				       c->c_pfnum, c->c_type, c->c_page);
		}
#endif
		ASSERT(pte->pg_pfnum == c->c_pfnum);
		ASSERT(!c->c_free);
		if (detach && c->c_type != CSYS) {
			*(long *)pte &= PG_PROT;
			c->c_gone = 1;
		}
		if (c->c_type == CSYS)
			c->c_lock = 0;
		s = spl6();
		if (detach) {
			next = cmap[CMHEAD].c_next;
			cmap[next].c_prev = i;
			c->c_prev = CMHEAD;
			c->c_next = next;
			cmap[CMHEAD].c_next = i;
		} else {
			prev = cmap[CMHEAD].c_prev;
			cmap[prev].c_next = i;
			c->c_next = CMHEAD;
			c->c_prev = prev;
			cmap[CMHEAD].c_prev = i;
		}
		c->c_free = 1;
		ASSERT(c->c_iolocks == 0);
		freemem++;
		ASSERT(freemem <= maxmem);
		splx(s);
		pte++;
	}
}

/*
 * Lock a page frame (keep pageout/pagein from messing with it).
 *
 * THIS ROUTINE SHOULD TAKE A CMAP STRUCTURE AS ARGUMENT.
 */
mlock(pf)
	unsigned pf;
{
	register struct cmap *c = &cmap[pgtocm(pf)];

	while (c->c_lock) {
		c->c_want = 1;
		sleep((caddr_t)c, PSWP+1);
	}
	c->c_lock = 1;
}

/*
 * Unlock a page frame.
 *
 * THIS ROUTINE SHOULD TAKE A CMAP STRUCTURE AS ARGUMENT.
 */
munlock(pf)
	unsigned pf;
{
	register struct cmap *c;

	c = &cmap[pgtocm(pf)];
	ASSERT(pf && (c->c_pfnum == pf) && c->c_lock && (c->c_iolocks >= 0));

	/*
	 * If the page still has some i/o locks on it, leave it c_lock'd until
	 * the last iolock is removed.
	 */
	if (c->c_iolocks) {
		if (--c->c_iolocks == 0)
			c->c_lock = 0;
	} else
		c->c_lock = 0;
	/*
	 * If page is finally unlocked, wakeup anybody waiting for the page
	 */
	if (!c->c_lock && c->c_want) {
		wakeup((caddr_t)c);
		c->c_want = 0;
	}
}

/* 
 * Lock a virtual segment for i/o.
 *
 * For each cluster of pages, if the cluster is not valid,
 * touch it to fault it in, otherwise just lock page frame.
 * Called from physio to ensure that the pages 
 * participating in raw i/o are valid and locked.
 */
vslock(base, count)
	caddr_t base;
{
	register unsigned v;
	register int npf;
	register struct pte *pte;
	register struct cmap *c;

	v = btop(base);
	pte = vtopte(u.u_procp, v);
	npf = btoc(count + ((int)base & PGOFSET));
	while (npf > 0) {
		if (pte->pg_v) {
			c = &cmap[pgtocm(pte->pg_pfnum)];

			/*
			 * If page has some iolocks but no lock, then somebody
			 * blew it.
			 */
			ASSERT(!(c->c_iolocks && !c->c_lock));
			/*
			 * If page is locked and has no i/o locks on it, then
			 * the pager must have it.  Get lock.  If the page
			 * is not locked, then just lock it.
			 */
			if (c->c_lock && (c->c_iolocks == 0)) {
				c->c_want = 1;
				sleep((caddr_t)c, PSWP+1);
				/*
				 * Try this page again, from the top, since
				 * during our sleep, anything could have
				 * happened.
				 */
				continue;
			}
			c->c_lock = 1;
		} else {
			if (pagein((long) ctob(v), 1))
				panic("vslock pagein");	/* return it locked */
			c = &cmap[pgtocm(pte->pg_pfnum)];
		}
		/*
		 * Advance number of i/o locks on the page.
		 */
		c->c_iolocks++;
		pte++;
		v++;
		npf--;
	}
}

/* 
 * Unlock a virtual segment.
 */
vsunlock(base, count, rw)
	caddr_t base;
	int count;
	int rw;
{
	register struct pte *pte;
	register int npf;

	pte = vtopte(u.u_procp, btop(base));
	npf = btoc(count + ((int)base & PGOFSET));
	while (npf > 0) {
		ASSERT(pte->pg_pfnum && pte->pg_v && !pte->pg_fod);
		munlock(pte->pg_pfnum);
		if (rw == B_READ)	/* Reading from device writes memory */
			pte->pg_m = 1;
		pte++;
		npf--;
	}
}

/*
 * outofmem:
 *	- this is called when free memory runs real low
 */
outofmem()
{
	/*
	 * Push out saved texts first.
	 */
	if (vm_savedpages) {
		while (freemem < desfree) {
			if (!xflush((struct inode *)NULL))
				break;
		}
	}

	/*
	 * If we still need some space, squeeze some from the
	 * buffer cache.
	 */
	if (freemem < minfree) {
		(void) bio_uncache_mem(PGTOBB(minfree - freemem));
		wakeup((caddr_t)&proc[2]);
	}
}
