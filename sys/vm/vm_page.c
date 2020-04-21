/*
 * $Source: /d2/3.7/src/sys/vm/RCS/vm_page.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:36:00 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/inode.h"
#include "../h/conf.h"
#include "../h/fstyp.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/text.h"
#include "../h/cmap.h"
#include "../h/buf.h"
#include "../vm/vm.h"
#include "machine/reg.h"
#include "machine/pte.h"
#include "machine/cx.h"
#include "machine/cpureg.h"

#ifdef	notdef
dirty(pte)
	register struct pte *pte;
{
	if (pte->pg_fod) {
		printf("pte=%x *pte=%x\n", pte, *(long *)pte);
		panic("dirty0");
		return (0);
	}
	if (pte->pg_pfnum == 0) {
		printf("pte=%x *pte=%x\n", pte, *(long *)pte);
		panic("dirty1");
		return (0);
	}
	if (pte->pg_m == 0)
		return (0);
	return (1);
}
#endif

swblk_t	vtod();

int	preptofree = 1;		/* send pre-paged pages to free list */
/*
 * Handle a page fault.
 *
 * Basic outline
 *	If page is allocated, but just not valid:
 *		Wait if intransit, else just revalidate
 *		Done
 *	Compute <dev,bn> from which page operation would take place
 *	If page is text page, and filling from file system or swap space:
 *		If in free list cache, reattach it and then done
 *	Allocate memory for page in
 *		If block here, restart because we could have swapped, etc.
 *	Lock process from swapping for duration
 *	Update pte's to reflect that page is intransit.
 *	If page is zero fill on demand:
 *		Clear pages and flush free list cache of stale cacheing
 *		for this swap page (e.g. before initializing again due
 *		to 407/410 exec).
 *	If page is fill from file and in buffer cache:
 *		Copy the page from the buffer cache.
 *	If not a fill on demand:
 *		Determine swap address and cluster to page in
 *	Do the swap to bring the page in
 *	Instrument the pagein
 *	After swap validate the required new page
 *	Leave prepaged pages reclaimable (not valid)
 *	Update shared copies of text page tables
 *	Complete bookkeeping on pages brought in:
 *		No longer intransit
 *		Hash text pages into core hash structure
 *		Unlock pages (modulo raw i/o requirements)
 *		Flush translation buffer
 *	Process pagein is done
 */
int
pagein(virtaddr, dlyu)
	long virtaddr;
	int dlyu;
{
	register struct proc *p;
	register struct pte *pte;
	register struct cmap *c;
	register int v;
	register int i;
	register int type;
	register unsigned pf;
	register unsigned vsave;
	int klsize;
	int si;
	struct pte opte;

	cnt.v_faults++;
	/*
	 * Classify faulted page into a segment and get a pte
	 * for the faulted page.
	 */
	vsave = v = btop(virtaddr);
	p = u.u_procp;
	if (isatsv(p, v)) {
		type = CTEXT;
		pte = tptopte(p, vtotp(p, v));
	} else if (isadsv(p, v)) {
		type = CDATA;
		pte = dptopte(p, vtodp(p, v));
	} else {
		type = CSTACK;
		pte = sptopte(p, vtosp(p, v));
	}
	if (pte->pg_v)
		return (1);

	/*
	 * If page is reclaimable, reclaim it.
	 * If page is text and intransit, sleep while it is intransit,
	 * If it is valid after the sleep, we are done.
	 * Otherwise we have to start checking again, since page could
	 * even be reclaimable now (we may have swapped for a long time).
	 */
restart:
	if (pte->pg_fod == 0 && pte->pg_pfnum) {
		if (type == CTEXT && cmap[pgtocm(pte->pg_pfnum)].c_intrans) {
			sleep((caddr_t)p->p_textp, PSWP+1);
			pte = vtopte(p, v);
			if (pte->pg_v) {
valid:
				if (dlyu) {
					c = &cmap[pgtocm(pte->pg_pfnum)];
					if (c->c_lock) {
						c->c_want = 1;
						sleep((caddr_t)c, PSWP+1);
						goto restart;
					}
					c->c_lock = 1;
				}
				sethwpte(p, v, CTEXT, pte);
				cnt.v_intrans++;
				return (0);
			}
			goto restart;
		}
		/*
		 * If page is in the free list, then take
		 * it back into the resident set, updating
		 * the size recorded for the resident set.
		 */
		si = spl6();
		pf = pte->pg_pfnum;
		c = &cmap[pgtocm(pf)];
		if (c->c_free) {
			munlink(pf);
			cnt.v_pgfrec++;
			if (type == CTEXT)
				p->p_textp->x_rssize++;
			else
				p->p_rssize++;
		}
		splx(si);
		pte->pg_v = 1;
		if (type == CTEXT)
			distpte(p->p_textp, vtotp(p, v), pte);
		cnt.v_pgrec++;
		if (dlyu) {
			if (c->c_lock) {
				c->c_want = 1;
				sleep((caddr_t)c, PSWP+1);
				goto restart;
			}
			c->c_lock = 1;
		}
		sethwpte(p, v, type, pte);
		return (0);
	}

	/*
	 * Wasn't reclaimable or reattachable.
	 * Have to prepare to bring the page in.
	 * We allocate the page before locking so we will
	 * be swappable if there is no free memory.
	 * If we block we have to start over, since anything
	 * could have happened.
	 */
	si = spl6();
	if (freemem < KLMAX) {
		while (freemem < KLMAX)
			sleep((caddr_t)&freemem, PSWP+2);
		splx(si);
		pte = vtopte(p, v);
		if (pte->pg_v)
			goto valid;
		goto restart;
	}

	/*
	 * Now can get memory and committed to bringing in the page.
	 * Lock this process, get a page,
	 * construct the new pte, and increment
	 * the (process or text) resident set size.
	 */
	klsize = 1;
	opte = *pte;
	p->p_flag |= SPAGE;
	(void) memall(pte, 1, p, type);
	pte->pg_prot = opte.pg_prot;
	pf = pte->pg_pfnum;
	cmap[pgtocm(pf)].c_intrans = 1;
	ASSERT(!pte->pg_v);
	if (type == CTEXT) {
		p->p_textp->x_rssize++;
		distpte(p->p_textp, vtotp(p, v), pte);
	} else
		p->p_rssize++;

	/*
	 * Two cases: either fill on demand (zero, or from file or text)
	 * or from swap space.
	 */
	if (opte.pg_fod) {
		pte->pg_m = 1;
		/*
		 * If zero fill, short-circuit hard work
		 * by just clearing pages.
		 */
		if (((struct fpte *)&opte)->pg_zfod) {
			splx(si);
			clearseg(pf);
			cnt.v_zfod++;
			goto skipswap;
		} else {
			splx(si);
			fod_pagein(v, type, pte);
		}
	} else {
		if (opte.pg_pfnum)
			panic("pagein pfnum");
		/*
		 * Fill from swap area.  Try to find adjacent
		 * pages to bring in also.
		 */
		if (!(p->p_flag & SUANOM)) {
			v = kluster(p, v, pte, B_READ, &klsize,
				       (type == CTEXT) ? kltxt :
				       ((p->p_flag & SSEQL) ? klseql : klin));
		}
		splx(si);

		/* read in page from swapdev */
		swap(p, vtod(p, v, &u.u_dmap, &u.u_smap), ptob(v),
			ctob(klsize), B_READ, B_PGIN, swapdev, 0); 
	}

	/*
	 * Instrumentation.
	 */
	cnt.v_pgin++;
	cnt.v_pgpgin += klsize;

skipswap:
	/*
	 * Fix page table entries.
	 *
	 * Only page requested in is validated, and rest of pages
	 * can be ``reclaimed''.  This allows system to reclaim prepaged pages
	 * quickly if they are not used and memory is tight.
	 */
	pte = vtopte(p, vsave);
	pte->pg_v = 1;
	if (type == CTEXT) {
		distpte(p->p_textp, vtotp(p, vsave), pte);
		if (opte.pg_fod)
			p->p_textp->x_flag |= XWRIT;
		/* XXX could avoid wakeup with a status bit XXX */
		wakeup((caddr_t)p->p_textp);
	}

	/*
	 * Memall returned page(s) locked.  Unlock all
	 * pages in cluster.  If locking pages for raw i/o
	 * leave the page which was required to be paged in locked,
	 * but still unlock others.
	 * If text pages, hash into the cmap situation table.
	 */
	pte = vtopte(p, v);
	for (i = klsize; --i >= 0; ) {
		c = &cmap[pgtocm(pte->pg_pfnum)];
		c->c_intrans = 0;
		if (v != vsave || !dlyu)
			munlock(pte->pg_pfnum);
		ASSERT(c->c_type == type);
		if (v != vsave && type != CTEXT && preptofree &&
		    opte.pg_fod == 0) {
			/*
			 * Throw pre-paged data/stack pages at the
			 * bottom of the free list.
			 */
			p->p_rssize--;
			memfree(pte, 1, 0);
		}
		sethwpte(p, v, (int)c->c_type, pte);
		v++;
		pte++;
	}

	/*
	 * All done.
	 */
	p->p_flag &= ~SPAGE;

	/*
	 * If process is declared fifo, memory is tight,
	 * and this was a data page-in, free memory
	 * klsdist pagein clusters away from the current fault.
	 */
	if (((p->p_flag & SDLOCK) == 0) && (p->p_flag&SSEQL) &&
	    (freemem < lotsfree) && (type == CDATA)) {
		int k = vtodp(p, vsave) / klseql;
		dpageout(p, (k - klsdist) * klseql, klout);
		dpageout(p, (k + klsdist) * klseql, klout);
	}
	return (0);
}

/*
 * Take away n pages of data space
 * starting at data page dp.
 * Used to take pages away from sequential processes.
 * Mimics pieces of code in pageout() below.
 */
dpageout(p, dp, n)
	register struct proc *p;
	register int dp, n;
{
	register struct cmap *c;
	register int i;
	register struct pte *pte;
	register int v;
	int klsize;

	if (dp < 0) {
		n += dp;
		dp = 0;
	}
	if (dp + n > p->p_dsize)
		n = p->p_dsize - dp;

	for (i = n; --i >= 0; dp++) {
		pte = dptopte(p, dp);
		if (pte->pg_fod || pte->pg_pfnum == 0)
			continue;
		c = &cmap[pgtocm(pte->pg_pfnum)];
		if (c->c_lock || c->c_free)
			continue;

		v = dptov(p, dp);
		gleanpte(p, v, pte);
		pte->pg_v = 0;
		sethwpte(p, v, (int) c->c_type, pte);
		if (dirty(pte)) {
			if (bswlist.av_forw == NULL)
				continue;
			mlock(pte->pg_pfnum);
			pte->pg_m = 0;
#ifdef	OS_DEBUG
			cleanpte(p, v, (int)c->c_type, pte);
#endif
			p->p_poip++;
			v = kluster(p, v, pte, B_WRITE, &klsize, klout);
			/* THIS ASSUMES THAT p == u.u_procp */
			swap(p, vtod(p, v, &u.u_dmap, &u.u_smap),
				ptob(v), ctob(klsize),
				B_WRITE, B_DIRTY, swapdev, pte->pg_pfnum);
		} else {
			if (c->c_gone == 0)
				p->p_rssize--;
			memfree(pte, 1, 0);
			cnt.v_seqfree++;
		}
	}
}

/*
 * fod_pagein:
 *	- initiate FS_READI() to read in data from executable
 *	- we have to save a bunch of the udot variables clobbered
 *	  by FS_READI() since this pagein may occur during a user i/o
 *	  operation...No recursion, please!
 *	- only allows 0410/0413 executables
 *	- note that this procedure gets called to fill in zero-fill pages
 *	  from a processes bss region (only during initital setup, not during
 *	  sbreak'ing)
 */
fod_pagein(v, type, pte)
	int v;
	int type;
	register struct pte *pte;
{
	register off_t offset;
	register u_int count;
	register int segflg;
	register long vpage;
	register struct cmap *c;
	struct userio save_io;
	char save_error;

	/* stuff saved in the user struct */
	offset = sizeof(u.u_exdata);
	if (u.u_exdata.ux_mag == 0413)
		offset = NBPG;
	count = NBPG;
	ASSERT((type == CTEXT) || (type == CDATA));
	if (type == CTEXT) {
		segflg = 2;
		vpage = vtotp(u.u_procp, v);
		if (vpage >= btoc(u.u_exdata.ux_tsize)) {
			printf("txsize=%d vpage=%d ts=%d\n",
					  btoc(u.u_exdata.ux_tsize),
					  vpage, u.u_procp->p_tsize);
			panic("fod pagein bogus text vpage");
		}
		/*
		 * check for runt page at end of text segment
		 */
		if ((long)ptob(vpage) + NBPG > u.u_exdata.ux_tsize) {
			count = u.u_exdata.ux_tsize - (long)ptob(vpage);
			clearseg(pte->pg_pfnum);
		}
	} else {
		segflg = 0;
		offset += u.u_exdata.ux_tsize;
		vpage = vtodp(u.u_procp, v);
		/*
		 * During process creation, expand() will setup the data
		 * AND bss pages of a process to be fod text.  This means
		 * that we will get faults for fill-on-demand-zero pages
		 * here.
		 */
		if (vpage >= btoc(u.u_exdata.ux_dsize)) {	/* bss */
			clearseg(pte->pg_pfnum);
			cnt.v_zfod++;
			return;
		}
		/*
		 * check for runt page at end of data segment
		 */
		if ((long)ptob(vpage) + NBPG > u.u_exdata.ux_dsize) {
			count = u.u_exdata.ux_dsize - (long)ptob(vpage);
			clearseg(pte->pg_pfnum);
		}
	}
	offset += (long)ptob(vpage);

	/* save u. variables */
	save_io = u.u_io;
	save_error = u.u_error;

	/*
	 * Tag page as having an i/o lock on it to keep the pageout code
	 * from mucking with it.  This also allows copyout() to use the page.
	 */
	c = &cmap[pgtocm(pte->pg_pfnum)];
	c->c_iolocks++;
	pte->pg_v = 1;

	u.u_base = ptob(v);
	u.u_count = count;
	u.u_offset = offset;
	u.u_segflg = segflg;
	u.u_error = 0;				/* start cleanly */
	FS_READI(u.u_procp->p_textp->x_iptr);

	c->c_iolocks--;				/* reduce count */
	if (c->c_iolocks)
		panic("fod_pagein iolocks");
	gleanpte(u.u_procp, v, pte);
	pte->pg_v = 0;
	cnt.v_exfod++;

	/* restore u. variables */
	u.u_io = save_io;
	u.u_error = save_error;
}

/*
 * There are two clock hands, initially separated by HANDSPREAD bytes
 * (but at most all of user memory).  The amount of time to reclaim
 * a page once the pageout process examines it increases with this
 * distance and decreases as the scan rate rises.
 */
#define	HANDSPREAD	(2 * 1024 * 1024)

int	pushes;

#define	FRONT	1
#define	BACK	2

/*
 * The page out daemon, which runs as process 2.
 *
 * As long as there are at least lotsfree pages,
 * this process is not run.  When the number of free
 * pages stays in the range desfree to lotsfree,
 * this daemon runs through the pages in the loop
 * at a rate determined in vmsched().  Pageout manages
 * two hands on the clock.  The front hand moves through
 * memory, clearing the valid bit (simulating a reference bit),
 * and stealing pages from procs that are over maxrss.
 * The back hand travels a distance behind the front hand,
 * freeing the pages that have not been referenced in the time
 * since the front hand passed.  If modified, they are pushed to
 * swap before being freed.
 */
pageout()
{
	register int count;
	register int fronthand, backhand;

	/*
	 * Set the two clock hands to be separated by a reasonable amount,
	 * but no more than 360 degrees apart.
	 */
	backhand = 1;
	fronthand = HANDSPREAD / NBPG;
	if (fronthand >= ncmap)
		fronthand = ncmap - 1;

loop:
	/*
	 * Before sleeping, look to see if there are any swap I/O headers
	 * in the ``cleaned'' list that correspond to dirty
	 * pages that have been pushed asynchronously. If so,
	 * empty the list by calling cleanup().
	 *
	 * N.B.: We guarantee never to block while the cleaned list is nonempty.
	 */
	(void) spl6();
  	if (bclnlist != NULL) {
 		(void) spl0();
  		cleanup();
 		goto loop;
  	}
	pager_sleeping = 1;
  	sleep((caddr_t)&proc[2], PSWP+1);
	pager_sleeping = 0;
	(void) spl0();

	count = 0;
	pushes = 0;
	while (nscan < desscan && freemem < lotsfree) {
		/*
		 * If checkpage manages to add a page to the free list,
		 * we give ourselves another couple of trips around the loop.
		 */
		if (checkpage(fronthand, FRONT))
			count = 0;
		if (checkpage(backhand, BACK))
			count = 0;
		cnt.v_scan++;
		nscan++;
		if (++fronthand >= ncmap) {
			fronthand = 1;
			cnt.v_rev++;
			if (count > 2) {
				/*
				 * Extremely unlikely, but we went around
				 * the loop twice and didn't get anywhere.
				 * Don't cycle, stop till the next clock tick.
				 */
				goto loop;
			}
			count++;
		}
		if (++backhand >= ncmap)
			backhand = 1;
	}
	goto loop;
}

/*
 * An iteration of the clock pointer (hand) around the loop.
 * Look at the page at hand.  If it is a
 * locked (for physical i/o e.g.), system (u., page table)
 * or free, then leave it alone.
 * Otherwise, if we are running the front hand,
 * invalidate the page for simulation of the reference bit.
 * If the proc is over maxrss, we take it.
 * If running the back hand, check whether the page
 * has been reclaimed.  If not, free the page,
 * pushing it to disk first if necessary.
 */
checkpage(hand, whichhand)
	int hand, whichhand;
{
	register struct proc *rp;
	register struct text *xp;
	register struct cmap *c;
	register struct pte *pte;
	register int type;
	swblk_t daddr;
	int v;
	int klsize;

top:
	/*
	 * Find a process and text pointer for the
	 * page, and a virtual page number in either the
	 * process or the text image.
	 */
	c = &cmap[hand];
	if (c->c_lock || c->c_free || c->c_intrans)
		return (0);
	ASSERT(c->c_iolocks == 0);
	switch (type = c->c_type) {

	case CSYS:
		return (0);

	case CTEXT:
		xp = &text[c->c_ndx];
		/*
		 * This check gets rid of many race problems with
		 * xfree and the pageout code (cleanup too)...
		 */
		if (xp->x_flag & (XLOCK|XSAVE))
			return (0);
		/*
		 * Scan list of processes using this text segment.  If any
		 * one of them has their text segment locked, don't page
		 * out anything.
		 */
		rp = xp->x_caddr;
		while (rp) {
			if (rp->p_flag & STLOCK)
				return (0);
			rp = rp->p_xlink;
		}
		rp = xp->x_caddr;
		v = tptov(rp, c->c_page);
		pte = tptopte(rp, c->c_page);
		break;

	case CDATA:
	case CSTACK:
		rp = &proc[c->c_ndx];
		if (rp->p_flag & SDLOCK)
			return (0);
		xp = rp->p_textp;
		if (c->c_type == CDATA) {
			v = dptov(rp, c->c_page);
			pte = dptopte(rp, c->c_page);
		} else {
			v = sptov(rp, c->c_page);
			pte = sptopte(rp, c->c_page);
		}
		break;
	}

	if (pte->pg_pfnum != cmtopg(hand))
		panic("bad c_page");

	/*
	 * If page is valid; make invalid but reclaimable.
	 * If this pte is not valid, then it must be reclaimable
	 * and we can add it to the free list.
	 */
	if (pte->pg_v) {
		if (whichhand == BACK)
			return(0);
		gleanpte(rp, v, pte);
		pte->pg_v = 0;
		if (type == CTEXT)
			distpte(xp, vtotp(rp, v), pte);
		else
			sethwpte(rp, v, type, pte);
#ifdef	OS_DEBUG
		cleanpte(rp, v, type, pte);
#endif
		if ((rp->p_flag & (SSEQL|SUANOM)) == 0 &&
		    rp->p_rssize <= rp->p_maxrss)
			return (0);
	}
	if (type != CTEXT) {
		/*
		 * Guarantee a minimal investment in data
		 * space for jobs in balance set.
		 */
		if (rp->p_rssize < SAFERSS - rp->p_slptime)
			return (0);
	}

	/*
	 * If the page is currently dirty, we
	 * have to arrange to have it cleaned before it
	 * can be freed.  We mark it clean immediately.
	 * If it is reclaimed while being pushed, then modified
	 * again, we are assured of the correct order of 
	 * writes because we lock the page during the write.  
	 * This guarantees that a swap() of this process (and
	 * thus this page), initiated in parallel, will,
	 * in fact, push the page after us.
	 *
	 * The most general worst case here would be for
	 * a reclaim, a modify and a swapout to occur
	 * all before the single page transfer completes.
	 */
	if (dirty(pte)) {
		/*
		 * If the process is being swapped out
		 * or about to exit, do not bother with its
		 * dirty pages
		 */
		if (rp->p_flag & (SLOCK|SWEXIT))
			return (0);
		/*
		 * Limit pushes to avoid saturating
		 * pageout device.
		 */
		if (pushes > vm_maxpushes)
			return (0);
		pushes++;

		/*
		 * Now carefully make sure that there will
		 * be a header available for the push so that
		 * we will not block waiting for a header in
		 * swap().  The reason this is important is
		 * that we (proc[2]) are the one who cleans
		 * dirty swap headers and we could otherwise
		 * deadlock waiting for ourselves to clean
		 * swap headers.  The sleep here on &proc[2]
		 * is actually (effectively) a sleep on both
		 * ourselves and &bswlist, and this is known
		 * to swdone and swap in vm_swp.c.  That is,
		 * &proc[2] will be awakened both when dirty
		 * headers show up and also to get the pageout
		 * daemon moving.
		 */
loop2:
		(void) spl6();
		if (bclnlist != NULL) {
			(void) spl0();
			cleanup();
			goto loop2;
		}
		if (bswlist.av_forw == NULL) {
			bswlist.b_flags |= B_WANTED;
			pager_sleeping = 1;
			sleep((caddr_t)&proc[2], PSWP+2);
			pager_sleeping = 0;
			(void) spl0();
			/*
			 * Page disposition may have changed
			 * since process may have exec'ed,
			 * forked, exited or just about
			 * anything else... try this page
			 * frame again, from the top.
			 */
			goto top;
		}
		(void) spl0();

		mlock((unsigned)cmtopg(hand));
		uaccess(rp, Pushmap, &pushutl);
		/*
		 * Now committed to pushing the page...
		 */
		pte->pg_m = 0;
		ASSERT(!pte->pg_v);
#ifdef	OS_DEBUG
		cleanpte(rp, v, type, pte);
#endif
		if (type == CTEXT)  {
			xp->x_poip++;
			distpte(xp, vtotp(rp, v), pte);
		} else
			rp->p_poip++;
		v = kluster(rp, v, pte, B_WRITE, &klsize, klout);
		if (klsize == 0)
			panic("pageout klsize");
		daddr = vtod(rp, v, &pushutl.u_dmap, &pushutl.u_smap);
		swap(rp, daddr, ptob(v), ctob(klsize),
			 B_WRITE, B_DIRTY, swapdev, pte->pg_pfnum);
		/*
		 * The cleaning of this page will be
		 * completed later, in cleanup() called
		 * (synchronously) by us (proc[2]).  In
		 * the meantime, the page frame is locked
		 * so no havoc can result.
		 */
		return (1);	/* well, it'll be free soon */
	}

	/*
	 * Decrement the resident set size of the current
	 * text object/process, and put the page in the
	 * free list. Note that we don't give memfree the
	 * pte as its argument, since we don't want to destroy
	 * the pte.  If it hasn't already been discarded
	 * it may yet have a chance to be reclaimed from
	 * the free list.
	 */
	if (c->c_gone == 0)
		if (type == CTEXT)
			xp->x_rssize--;
		else
			rp->p_rssize--;
	memfree(pte, 1, 0);
	cnt.v_dfree++;
	return (1);		/* freed a page! */
}

/*
 * Process the ``cleaned'' list.
 *
 * Scan through the linked list of swap I/O headers
 * and free the corresponding pages that have been
 * cleaned by being written back to the paging area.
 * If the page has been reclaimed during this time,
 * we do not free the page.  As they are processed,
 * the swap I/O headers are removed from the cleaned
 * list and inserted into the free list.
 */
cleanup()
{
	register struct buf *bp;
	register struct cmap *c;
	register struct pte *pte;
	register struct proc *rp;
	register struct text *xp;
	register unsigned pf;
	register int i;
	register int s, center;
	struct pte *upte;

	for (;;) {
		s = spl6();
		if ((bp = bclnlist) == 0) {
			splx(s);
			break;
		}
		bclnlist = bp->av_forw;
		splx(s);

		pte = vtopte(&proc[2], btop(bp->b_un.b_addr));
		center = 0;
		for (i = 0; i < bp->b_bcount; i += NBPG) {
			ASSERT(pte->pg_pfnum && !pte->pg_fod);
			pf = pte->pg_pfnum;
			c = &cmap[pgtocm(pf)];
			munlock(pte->pg_pfnum);
			if (pf != bp->b_pfcent) {
				if (c->c_gone) {
					memfree(pte, 1, 0);
					cnt.v_dfree++;
				}
				goto skip;
			}
			center++;
			switch (c->c_type) {
#ifdef	OS_DEBUG
			  case CSYS:
				panic("cleanup CSYS");
#endif
			  case CTEXT:
				xp = &text[c->c_ndx];
				xp->x_poip--;
				if (xp->x_poip == 0)
					wakeup((caddr_t)&xp->x_poip);
				break;
			  case CDATA:
			  case CSTACK:
				rp = &proc[c->c_ndx];
				rp->p_poip--;
				if (rp->p_poip == 0)
					wakeup((caddr_t)&rp->p_poip);
				break;
			}
			if (c->c_gone == 0) {
				switch (c->c_type) {
				  case CTEXT:
					upte = tptopte(xp->x_caddr, c->c_page);
					break;
				  case CDATA:
					upte = dptopte(rp, c->c_page);
					break;
				  case CSTACK:
					upte = sptopte(rp, c->c_page);
					break;
				}
				if (upte->pg_v)
					goto skip;
				if (c->c_type == CTEXT)
					xp->x_rssize--;
				else
					rp->p_rssize--;
			}
			memfree(pte, 1, 0);
			cnt.v_dfree++;
skip:
			pte++;
		}
#ifdef	OS_DEBUG
		pte = vtopte(&proc[2], btop(bp->b_un.b_addr));
		for (i = 0; i < KLMAX; i++)
			*(long *)pte++ = 0;
#endif
		if (center != 1)
			panic("cleanup center");
		bp->b_flags = 0;
		bp->av_forw = bswlist.av_forw;
		bswlist.av_forw = bp;
		if (bswlist.b_flags & B_WANTED) {
			bswlist.b_flags &= ~B_WANTED;
			wakeup((caddr_t)&bswlist);
		}
	}
}

/*
 * Kluster locates pages adjacent to the argument pages
 * that are immediately available to include in the pagein/pageout,
 * and given the availability of memory includes them.
 * It knows that the process image is contiguous in chunks;
 * an assumption here is that KLMAX is a divisor of dmmin,
 * so that by looking at KLMAX chunks of pages, all such will
 * necessarily be mapped swap contiguous.
 *
 * Note that text klustering done here is reading the text from
 * the swap space, not the filesystem space.  The first reference
 * to a text page is from filesystem space, and is always done with
 * cooked i/o in fod_pagein().  After that, kluster'ing can be done
 * from the swap space.
 */
int	klicnt[KLMAX];
int	klocnt[KLMAX];

int
kluster(p, v, pte0, rw, pkl, klsize)
	register struct proc *p;
	int v;
	struct pte *pte0;
	int rw;
	register int *pkl;
	int klsize;
{
	register struct pte *pte;
	register int i, type, kloff, k, cl;
	int clmax;
	int klback, klforw;
	int klmax;
	unsigned v0;

	if (rw == B_READ)
		klicnt[0]++;
	else
		klocnt[0]++;
	*pkl = 1;
	if ((klsize <= 1) || (klsize > KLMAX) || (klsize & (klsize - 1)))
		return (v);
	if (rw == B_READ && freemem < KLMAX)
		return (v);
	if (isassv(p, v)) {
		type = CSTACK;
		cl = vtosp(p, v);
		clmax = p->p_ssize;
	} else
	if (isadsv(p, v)) {
		type = CDATA;
		cl = vtodp(p, v);
		clmax = p->p_dsize;
	} else {
		type = CTEXT;
		cl = vtotp(p, v);
		clmax = p->p_textp->x_size;
	}
	kloff = cl & (klsize - 1);
	pte = pte0;
	for (k = kloff; --k >= 0;) {
		if (type == CSTACK)
			pte++;
		else
			pte--;
		if (!klok(p, pte, rw))
			break;
	}
	klback = (kloff - k) - 1;
	pte = pte0;
	if ((cl - kloff) + klsize > clmax)
		klmax = clmax - (cl - kloff);
	else
		klmax = klsize;
	for (k = kloff; ++k < klmax;) {
		if (type == CSTACK)
			pte--;
		else
			pte++;
		if (!klok(p, pte, rw))
			break;
	}
	klforw = (k - kloff) - 1;
	if (klforw + klback == 0)
		return (v);
	pte = pte0;
	if (type == CSTACK) {
		pte -= klforw;
		v -= klforw;
	} else {
		pte -= klback;
		v -= klback;
	}
	*pkl = klforw + klback + 1;
	if (rw == B_READ)
		klicnt[0]--, klicnt[*pkl - 1]++;
	else
		klocnt[0]--, klocnt[*pkl - 1]++;

	v0 = v;
	for (i = 0; i < *pkl; i++) {
		if (pte == pte0)
			goto cont;
		if (rw == B_WRITE) {
			mlock(pte->pg_pfnum);
			pte->pg_m = 0;
			if (type == CTEXT)
				distpte(p->p_textp, vtotp(p, v), pte);
			else
				sethwpte(p, v, type, pte);
#ifdef	OS_DEBUG
			cleanpte(p, v, type, pte);
#endif
		} else {
			struct pte opte;
			int pf;

			opte = *pte;
			if (memall(pte, 1, p, type) == 0)
				panic("kluster");
			pte->pg_prot = opte.pg_prot;
			pf = pte->pg_pfnum;
			cmap[pgtocm(pf)].c_intrans = 1;
			ASSERT(!pte->pg_v);
			if (type == CTEXT) {
				p->p_textp->x_rssize++;
				distpte(p->p_textp, vtotp(p, v), pte);
			} else {
				p->p_rssize++;
				sethwpte(p, v, type, pte);
			}
		}
cont:
		pte++;
		v++;
	}
	return (v0);
}

klok(p, pte, rw)
	struct proc *p;
	register struct pte *pte;
	int rw;
{
	register struct cmap *c;

	if (pte->pg_fod)
		return (0);
	if (rw == B_WRITE) {
		if (pte->pg_pfnum == 0)
			return (0);
		c = &cmap[pgtocm(pte->pg_pfnum)];
		if (c->c_lock || c->c_intrans)
			return (0);
		ASSERT(c->c_iolocks == 0);
		gleanpte(p, ptetov(p, pte), pte);
		if (!dirty(pte))
			return (0);
		ASSERT(!c->c_free);
	} else {
		if (pte->pg_pfnum)
			return (0);
	}
	return (1);
}
