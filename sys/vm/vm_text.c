/*
 * $Source: /d2/3.7/src/sys/vm/RCS/vm_text.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:36:08 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/map.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/text.h"
#include "../h/conf.h"
#include "../h/fstyp.h"
#include "../h/inode.h"
#include "../h/seg.h"
#include "../h/buf.h"
#include "../vm/vm.h"
#include "../h/cmap.h"
#include "../h/sysinfo.h"
#include "machine/pte.h"

/*
 * Notes on differences between traditional text code and this stuff:
 *	- this code supports a concept of a 'saved text', which is nothing
 *	  more than leaving the pages of text around in memory in case
 *	  they might be used in the near future
 *	- we use the new IXSAVED bit in the inode flag to indicate a text is
 *	  being saved (and not in use) so that writei() and other file system
 *	  routines can force us to flush the stale text pages
 *	- the outofmem() function used by the memory allocators, knows
 *	  to call xflush() to pick up free pages (so does pageout())
 *	- ptrace() turns off the ITEXT bit for the users text structure
 *	  if it writes on the text area (breakpoint setting, for instance).
 *	  This causes xfree() to flush out a possible saved text because
 *	  it no longer correctly represents the inode
 */

/* release a text inode without sleeping waiting to lock it */
#define	IUNUSE(ip) \
	if ((ip)->i_count == 1) { \
		ASSERT(((ip)->i_flag & ILOCKED) == 0); \
		iunuse(ip); \
	} else \
		--(ip)->i_count

/*
 * xfree:
 *	- relinquish use of the shared text segment of a process
 *	- attempt to save the text pages for later re-use when the
 *	  last refrence frees the text segment
 */
xfree()
{
	register struct text *xp;
	register struct inode *ip;
	register int s;

	if ((xp = u.u_procp->p_textp) == NULL)
		return;
	xlock(xp);				/* lock it */
	ip = xp->x_iptr;
	if (--xp->x_count == 0) {
		/*
		 * wait for any pageouts to complete
		 */
		s = spl6();
		while (xp->x_poip)
			sleep((caddr_t)&xp->x_poip, PSWP+1);
		splx(s);

		/*
		 * We unlink the process from the text structure after
		 * waiting for the pageouts to complete because cleanup()
		 * needs to have a process to attribute the pageout to, and
		 * uses x_caddr to find it.
		 */
		xunlink(u.u_procp);		/* safe to do now */
		vsxfree(xp, (long)xp->x_size);

		/*
		 * Try to save the text segment for a later re-reference.
		 */
		if (!xsave(xp)) {
			ip->i_flag &= ~ITEXT;
			IUNUSE(ip);
			xp->x_iptr = NULL;
		}
		xp->x_flag &= ~XLOCK;
	} else {
		xp->x_flag &= ~XLOCK;
		xccdec(xp, u.u_procp);
	}
	u.u_procp->p_textp = NULL;
}

/*
 * Attach to a shared text segment.
 * If there is no shared text, just return.
 * If there is, hook up to it:
 * if it is not currently being used, it has to be read
 * in from the inode (ip); the written bit is set to force it
 * to be written out as appropriate.
 * If it is being used, but is not currently in core,
 * a swap has to be done to get it back.
 */
xalloc(ip)
	register struct inode *ip;
{
	register struct text *xp;
	register size_t ts;
	register struct text *xp1;

	if(u.u_exdata.ux_tsize == 0)
		return;
again:
	xp1 = NULL;
	for (xp = text; xp < textNTEXT; xp++) {
		if(xp->x_iptr == NULL) {
			if(xp1 == NULL)
				xp1 = xp;
			continue;
		}
		if ((xp->x_count > 0 || (xp->x_flag&XSAVE)) &&
		    (xp->x_iptr == ip)) {
			if (xp->x_flag&XLOCK) {
				xwait(xp);
				goto again;
			}
			xlock(xp);
			xp->x_count++;
			u.u_procp->p_textp = xp;
			xlink(u.u_procp);
			xunlock(xp);
			return;
		}
	}
	if((xp=xp1) == NULL) {
		if (!xflush((struct inode *)NULL)) {
			u.u_error = ENOEXEC;
			syserr.textovf++;
			return;
		}
		goto again;
	}

	xp->x_flag = XLOAD|XLOCK;
	ts = btoc(u.u_exdata.ux_tsize);
	xp->x_size = ts;
	if (vsxalloc(xp) == NULL) {
		swkill(u.u_procp, "xalloc");
		return;
	}
	xp->x_count = 1;
	xp->x_ccount = 0;
	xp->x_rssize = 0;
	xp->x_iptr = ip;
	ip->i_flag |= ITEXT;
	ip->i_flag &= ~IXSAVED;		/* force off until save */
	iuse(ip);			/* bump reference count */
	u.u_procp->p_textp = xp;
	xlink(u.u_procp);
	settprot(RO);
	xp->x_flag |= XWRIT;
	xp->x_flag &= ~XLOAD;
	xunlock(xp);
}

/*
 * Lock and unlock a text segment from swapping
 */
xlock(xp)
register struct text *xp;
{
	while(xp->x_flag&XLOCK) {
		xp->x_flag |= XWANT;
		sleep((caddr_t)xp, PSWP);
	}
	xp->x_flag |= XLOCK;
}

/*
 * Wait for xp to be unlocked if it is currently locked.
 */
xwait(xp)
register struct text *xp;
{
	xlock(xp);
	xunlock(xp);
}

xunlock(xp)
	register struct text *xp;
{
	if (xp->x_flag&XWANT)
		wakeup((caddr_t)xp);
	xp->x_flag &= ~(XLOCK|XWANT);
}

/*
 * Decrement the in-core usage count of a shared text segment.
 * When it drops to zero, free the core space.
 */
xccdec(xp, p)
	register struct text *xp;
	register struct proc *p;
{
	if (xp==NULL || xp->x_ccount==0)
		return;

	xlock(xp);
	if (--xp->x_ccount == 0) {
		if (xp->x_flag & XWRIT) {
			ASSERT(xp->x_ptdaddr);
			vsswap(p, tptopte(p, 0), CTEXT, 0, xp->x_size,
				  (struct dmap *)0);
			swap(p, xp->x_ptdaddr, (caddr_t)tptopte(p, 0),
				xp->x_size * sizeof (struct pte),
				B_WRITE, B_PAGET, swapdev, 0);
			xp->x_flag &= ~XWRIT;
		} else
			xp->x_rssize -= vmemfree(tptopte(p, 0), xp->x_size);
		ASSERT(xp->x_rssize == 0);
	}
	xunlink(p);
	xunlock(xp);
}

/*
 * free the swap image of all unused saved-text text segments
 * which are from device mp (used by umount system call).
 */
xumount(mp)
	register struct mount *mp;
{
	register struct text *xp;

	for (xp = text; xp < textNTEXT; xp++) 
		if (xp->x_iptr!=NULL && mp==xp->x_iptr->i_mntdev)
			xuntext(xp);
}

/*
 * remove text image from the text table.
 * the use count must be zero.
 */
xuntext(xp)
register struct text *xp;
{
	register struct inode *ip;

	xlock(xp);
	if (xp->x_count) {
		xunlock(xp);
		return;
	}
	xp->x_flag &= ~XLOCK;
	if (xp->x_flag & XSAVE)
		xdestroy(xp);
	else {
		ip = xp->x_iptr;
		xp->x_iptr = NULL;
		vsxfree(xp, (long)xp->x_size);
		ip->i_flag &= ~ITEXT;
		IUNUSE(ip);
	}
}

/*
 * Add a process to those sharing a text segment by
 * getting the page tables and then linking to x_caddr.
 */
xlink(p)
	register struct proc *p;
{
	register struct text *xp = p->p_textp;

	if (xp == 0)
		return;
	if (xp->x_flag & XSAVE)
		xrestore(p, xp);
	else
		vinitpt(p);
	p->p_xlink = xp->x_caddr;
	xp->x_caddr = p;
	xp->x_ccount++;
}

xunlink(p)
	register struct proc *p;
{
	register struct text *xp = p->p_textp;
	register struct proc *q;

	if (xp == 0)
		return;
	if (xp->x_caddr == p) {
		xp->x_caddr = p->p_xlink;
		p->p_xlink = 0;
		return;
	}
	for (q = xp->x_caddr; q->p_xlink; q = q->p_xlink)
		if (q->p_xlink == p) {
			q->p_xlink = p->p_xlink;
			p->p_xlink = 0;
			return;
		}
	panic("lost text");
}

/*
 * xflush:
 *	- used to flush out a saved text during a write to the inode
 *	  which previously contained the saved text, or, during a memory
 *	  shortage
 */
xflush(ip)
	register struct inode *ip;
{
	register struct text *xp;

	for (xp = text; xp < textNTEXT; xp++) {
		if (xp->x_iptr && (xp->x_flag & XSAVE) &&
			       !(xp->x_flag & XLOCK)) {
			if (ip && (xp->x_iptr != ip))
				continue;
			xdestroy(xp);
			return (1);
		}
	}
	return (0);
}

/*
 * xsave:
 *	- save a given text structure in the text table for possible
 *	  refrence.  This implements an in memory sticky bit...
 *	- the caller of this procedure must lock and unlock the text
 */
xsave(xp)
	register struct text *xp;
{
	register struct pte *pte;
	register struct cmap *c;
	register int i;
	register short head, tail, new;

	/* don't bother saving a dirty text (from ptrace()) */
	pte = tptopte(u.u_procp, 0);
	if (!(xp->x_iptr->i_flag & ITEXT)) {
		(void) vmemfree(pte, u.u_tsize);
		return (0);			/* failed save.. */
	}

	/* chain together the real pages into a list */
	xp->x_rssize = 0;
	head = 0;
#ifdef lint
	tail = 0;
#endif
	for (i = xp->x_size; -- i >= 0; pte++) {
		/*
		 * Disqualify any pages which are either fill-on-demand,
		 * or have been paged out.  These pages will have to
		 * be re-filled from the inode upon reclaiming of this
		 * saved text.
		 */
		if (pte->pg_fod)
			continue;
		if (!pte->pg_v && (pte->pg_pfnum == 0))
			continue;
		new = pgtocm(pte->pg_pfnum);
		c = &cmap[new];
		ASSERT(!c->c_lock && !c->c_intrans);

		/* get rid of half freed pages */
		if (c->c_free)
			(void) vmemfree(pte, 1);
		else {
			ASSERT(!c->c_gone);
			/* link page onto chain */
			if (head == 0)
				head = new;
			else
				cmap[tail].c_next = new;
			tail = new;
			c->c_next = 0;

			/* count text page saves */
			vm_savedpages++;
			xp->x_rssize++;
			cnt.v_xsfrec++;
		}
	}
#ifdef	OS_DEBUG
	if (((head == 0) && xp->x_rssize) ||
	    (xp->x_rssize > xp->x_size)) {
		printf("xp=%x head=0, rssize=%d size=%d\n",
			      xp, xp->x_rssize, xp->x_size);
		panic("xfree");
	}
#endif
	if (xp->x_rssize == 0)
		return (0);		/* nothing saved */

	/* set state info for later xrestore'ing or xdestroy'ing */
	xp->x_iptr->i_flag |= IXSAVED;	/* force this on */
	xp->x_cmx = head;
	xp->x_flag |= XSAVE;
	xp->x_flag &= ~XWRIT;
	return (1);
}

/*
 * xrestore:
 *	- initialize text portion of page table for a saved text upon
 *	  the creation of a new refrence
 *	- because the text segment is XLOCK'd, we don't have to worry
 *	  about the pageout daemon stealing pages from us in the middle
 *	  of our loop...
 */
xrestore(p, xp)
	struct proc *p;
	register struct text *xp;
{
	register struct pte *pte;
	register struct cmap *c;
	register int i;
	register short cmx;
	struct pte proto;

	if (vsxalloc(xp) == NULL) {
		swkill(u.u_procp, "xalloc");
		return;
	}

	/* initialize text page tables as inode fill on demand */
	pte = tptopte(p, 0);
	*(int *)&proto = PG_URKR | PG_FOD;
	for (i = 0; i < xp->x_size; i++)
		*pte++ = proto;

	/* find pages in cmap and initialize page table */
	pte = tptopte(p, 0);
	cmx = xp->x_cmx;
	for (i = xp->x_rssize; --i >= 0;) {
		c = &cmap[cmx];
		initpte(pte + c->c_page, c->c_pfnum, PG_V | PG_M | PG_URKR);
		cmx = c->c_next;
		cnt.v_xifrec++;			/* count inode save reclaims */
		vm_savedpages--;		/* no longer saved... */
		ASSERT(vm_savedpages >= 0);
	}
	ASSERT(cmx == 0);
	xp->x_cmx = 0;
	xp->x_flag &= ~XSAVE;
	/*
	 * Turn on XWRIT because we have to write the page table to disk
	 * again, given that we are now using a new slice of disk.
	 */
	xp->x_flag |= XWRIT;
	xp->x_iptr->i_flag &= ~IXSAVED;		/* no longer being saved... */
	xp->x_ccount = 0;
}

/*
 * xdestroy:
 *	- destroy a saved text image, freeing any remaining saved pages
 */
xdestroy(xp)
	register struct text *xp;
{
	register struct inode *ip;
	register short cmx;
	struct pte apte;

	/* release text pages still available */
	xlock(xp);
	if (xp->x_rssize) {
		*(long *)&apte = 0;
		cmx = xp->x_cmx;
		while (--xp->x_rssize >= 0) {
			apte.pg_pfnum = cmap[cmx].c_pfnum;
			cmx = cmap[cmx].c_next;
			memfree(&apte, 1, 1);
			vm_savedpages--;	/* bye bye */
			ASSERT(vm_savedpages >= 0);
		}
		ASSERT(cmx == 0);
	}

	/* first release the text structure (which won't sleep) */
	ip = xp->x_iptr;
	xp->x_iptr = NULL;
	xp->x_flag &= ~XSAVE;
	xunlock(xp);

	/* now release the inode */
	ip->i_flag &= (~ITEXT & ~IXSAVED);
	IUNUSE(ip);
}
