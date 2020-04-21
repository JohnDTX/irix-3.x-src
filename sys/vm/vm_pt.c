/*
 * $Source: /d2/3.7/src/sys/vm/RCS/vm_pt.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:36:02 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/map.h"
#include "../h/cmap.h"
#include "../h/buf.h"
#include "../vm/vm.h"
#include "../h/text.h"
#include "../h/inode.h"
#include "machine/pte.h"

#include "sgigsc.h"
#if NSGIGSC > 0
#include "../h/sgigsc.h"
#endif

/*
 * Allocate space from kernel map.  Optionally wait for the space if there
 * isn't enough available.
 */
long
kmap_alloc(size, canwait)
	long size;
	int canwait;
{
	long addr;
	int s;

	s = spl6();
	while ((addr = rmalloc(kernelmap, size)) == 0) {
		if (bio_uncache_kmap(PGTOBB(size)))
			continue;
		if (!canwait)
			return ((long)0);
		kmap_wanted = 1;
		sleep((caddr_t)&kmap_wanted, PSWP+4);
	}
	splx(s);
	return (addr);
}

/* XXX kernel internal compatability stub */
long
kmalloc(size)
	long size;
{
	return (kmap_alloc(size, 1));
}

/*
 * Free something in the kernel map
 */
kmfree(size, addr)
	long size;
	long addr;
{
	int s;

	s = spl6();
	rmfree(kernelmap, size, addr);
	if (kmap_wanted) {
		kmap_wanted = 0;
		wakeup((caddr_t)&kmap_wanted);
	}
	splx(s);
}

/*
 * Get page tables for process p.  Allocator
 * for memory is argument; process must be locked
 * from swapping if vmemall is used; if memall is
 * used, call will return w/o waiting for memory.
 * In any case an error return results if no user
 * page table space is available.
 */
int
vgetpt(p, pmemall)
	register struct proc *p;
	int (*pmemall)();
{
	register long a;
	register int i;

	ASSERT(p->p_szpt);
	/*
	 * Allocate space in the kernel map for this process.
	 * Then allocate page table pages, and initialize the
	 * process' p0br and addr pointer to be the kernel
	 * virtual addresses of the base of the page tables and
	 * the pte for the process pcb (at the base of the u.).
	 */
	a = kmalloc((long)p->p_szpt);
	if ((*pmemall)(&Usrptmap[a], p->p_szpt, p, CSYS) == 0) {
		kmfree((long) p->p_szpt, a);
		return (0);
	}
	p->p_p0br = kmxtob(a);
	p->p_addr = uaddr(p);
	/*
	 * Now validate the system page table entries for the
	 * user page table pages, flushing old translations
	 * for these kernel virtual addresses.  Clear the new
	 * page table pages for clean post-mortems.
	 */
	vmaccess(&Usrptmap[a], (caddr_t)p->p_p0br, p->p_szpt);
	for (i = 0; i < p->p_szpt; i++)
		clearseg(Usrptmap[a + i].pg_pfnum);
	return (1);
}

/*
 * Initialize text portion of page table.
 */
vinitpt(p)
	struct proc *p;
{
	register struct text *xp;
	register struct proc *q;
	register struct pte *pte;
	register struct pte *frompte;
	register int i;
	struct pte proto;

	xp = p->p_textp;
	if (xp == 0)
		return;

	pte = tptopte(p, 0);
	/*
	 * If there is another instance of same text in core
	 * then just copy page tables from other process.
	 * If the page is intransit and valid, then it means that some
	 * other process using the page is in the middle of filling the
	 * page from inode space.  Tag the page is refrencing the same
	 * page frame, but not valid so that this process will take a
	 * fault and hang on c_intrans in pagein() until the i/o completes.
	 */
	if (q = xp->x_caddr) {
		frompte = tptopte(q, 0);
		for (i = 0; i < xp->x_size; i++) {
			if (frompte->pg_v &&
			    cmap[pgtocm(frompte->pg_pfnum)].c_intrans) {
				*pte = *frompte++;
				pte->pg_v = 0;
				pte++;
			} else
				*pte++ = *frompte++;
		}
		return;
	}

	/*
	 * Initialize text page tables, fod if the text is
	 * loading right now.  If text is not being loaded
	 * at this moment (XLOAD off) then get page tables
	 * from swap space.
	 */
	if (xp->x_flag & XLOAD) {
		*(long *)&proto = PG_URKR | PG_FOD;
		cnt.v_nexfod += xp->x_size;
		for (i = 0; i < xp->x_size; i++)
			*pte++ = proto;
	} else {
		ASSERT(xp->x_ptdaddr);
		/*
		 * Retrieve page tables from swap area.
		 */
		swap(p, xp->x_ptdaddr, (caddr_t)pte,
			xp->x_size * sizeof (struct pte), B_READ,
			B_PAGET, swapdev, 0);
#ifdef	OS_DEBUG
		for (i = 0; i < xp->x_size; i++, pte++) {
			if (pte->pg_m || !(pte->pg_fod || (pte->pg_v == 0))) {
				printf("pte=%x, *pte=%x\n", pte, *(long *)pte);
				panic("vinitpt");
			}
		}
#endif
	}
}

/*
 * Update the page tables of all processes linked
 * to a particular text segment, by distributing
 * dpte to the the text page at virtual frame v.
 *
 * Note that invalidation in the translation buffer for
 * the current process is the responsibility of the caller.
 */
distpte(xp, tp, dpte)
	struct text *xp;
	register size_t tp;
	register struct pte *dpte;
{
	register struct proc *p;
	register struct pte *pte;

	for (p = xp->x_caddr; p; p = p->p_xlink) {
		pte = tptopte(p, tp);
		sethwpte(p, (int)tptov(p, tp), CTEXT, dpte);
		if (pte != dpte)
			*pte = *dpte;
	}
}

/*
 * Release page tables of process p.
 */
vrelpt(p)
	register struct proc *p;
{
	register int a;

	if (p->p_szpt == 0)
		return;
	a = btokmx(p->p_p0br);
	(void) vmemfree(&Usrptmap[a], p->p_szpt);
	kmfree((long)p->p_szpt, (long)a);
}

/*
 * Compute number of pages to be allocated to the u. area
 * and data and stack area page tables, which are stored on the
 * disk immediately after the u. area.
 */
/*ARGSUSED*/
vusize(p, utl)
	register struct proc *p;
	struct user *utl;
{
	register int tsz = p->p_tsize / NPTEPG;

	/*
	 * We do not need page table space on the disk for page
	 * table pages wholly containing text.  This is well
	 * understood in the code in vmswap.c.
	 */
	return UPAGES + ctopt(p->p_tsize+p->p_dsize+p->p_ssize+UPAGES) - tsz;
}

/*
 * Get u area for process p.  If a old u area is given,
 * then copy the new area from the old, else
 * swap in as specified in the proc structure.
 *
 * Since argument map/newu is potentially shared
 * when an old u. is provided we have to be careful not
 * to block after beginning to use them in this case.
 * (This is not true when called from swapin() with no old u.)
 */
vgetu(p, palloc, map, newu, oldu)
	register struct proc *p;
	int (*palloc)();
	register struct pte *map;
	register struct user *newu;
	struct user *oldu;
{
	register int i;

	if ((*palloc)(p->p_addr, UPAGES, p, CSYS) == 0)
		return (0);
	/*
	 * New u. pages are to be accessible in map/newu as well
	 * as in process p's virtual memory.
	 */
	for (i = 0; i < UPAGES; i++) {
		map[i] = p->p_addr[i];
		initpte(p->p_addr + i, (p->p_addr + i)->pg_pfnum, PG_V | PG_KW);
	}
	setredzone(p->p_addr, (caddr_t)0);
	vmaccess(map, (caddr_t)newu, UPAGES);

	/*
	 * New u.'s come from forking or inswap.
	 */
	if (oldu) {
		bcopy((caddr_t)oldu, (caddr_t)newu, UPAGES * NBPG);
		newu->u_procp = p;
	} else {
		swap(p, p->p_swaddr, (caddr_t)0, ctob(UPAGES),
			B_READ, B_UAREA, swapdev, 0);
		if ((newu->u_tsize != p->p_tsize) ||
		    (newu->u_dsize != p->p_dsize) ||
		    (newu->u_ssize != p->p_ssize) || (newu->u_procp != p))
			panic("vgetu");
	}

	/*
	 * Initialize the pcb copies of the p0 and p1 region bases and
	 * software page table size from the information in the proc structure.
	 */
	newu->u_pcb.pcb_p0br = p->p_p0br;
	newu->u_pcb.pcb_p1br = initp1br(p->p_p0br + p->p_szpt * NPTEPG);
	newu->u_pcb.pcb_szpt = p->p_szpt;
	return (1);
}

/*
 * Release swap space for a u. area.
 */
vrelswu(p, utl)
	struct proc *p;
	struct user *utl;
{

	rmfree(swapmap, (long)ctod(vusize(p, utl)), p->p_swaddr);
	/* p->p_swaddr = 0; */	/* leave for post-mortems */
}

/*
 * Get swap space for a u. area.
 */
vgetswu(p, utl)
	struct proc *p;
	struct user *utl;
{

	p->p_swaddr = rmalloc(swapmap, (long)ctod(vusize(p, utl)));
	return (p->p_swaddr);
}

/*
 * Release u. area, swapping it out if desired.
 *
 * Note: we run on the old u. after it is released into swtch(),
 * and are safe because nothing can happen at interrupt time.
 */
vrelu(p, swapu)
	register struct proc *p;
{
	register int i;
	struct pte uu[UPAGES];

	if (swapu)
		swap(p, p->p_swaddr, (caddr_t)0, ctob(UPAGES),
		    B_WRITE, B_UAREA, swapdev, 0);
	for (i = 0; i < UPAGES; i++)
		uu[i] = p->p_addr[i];
	(void) vmemfree(uu, UPAGES);
}

/*
 * Expand a page table, assigning new kernel virtual
 * space and copying the page table entries over both
 * in the system map and as necessary in the user page table space.
 */
/* ARGSUSED */
ptexpand(change, ods, oss)
	register int change;
	size_t ods, oss;
{
	register struct pte *p1, *p2;
	register int i;
	register int spages, ss = P1PAGES - u.u_pcb.pcb_p1lr;
	register long kold = btokmx(u.u_pcb.pcb_p0br);
	long knew;
	int tdpages;
	int szpt = u.u_pcb.pcb_szpt;
	int s;

	ASSERT(change > 0);
	/*
	 * Change is the number of new page table pages needed.
	 * Kold is the old index in the kernel-map of the page tables.
	 * Allocate a new kernel map segment of size szpt+change for
	 * the page tables, and the new page table pages in the
	 * middle of this new region.
	 */
top:
	knew = kmalloc((long) (szpt + change));
	spages = ss/NPTEPG;
	tdpages = szpt - spages;
	if (memall(&Usrptmap[knew+tdpages], change, u.u_procp, CSYS) == 0) {
		kmfree((long)(szpt+change), (long)knew);
		goto bad;
	}

	/*
	 * Spages pages of u.+stack page tables go over unchanged.
	 * Tdpages of text+data page table may contain a few stack
	 * pages which need to go in one of the newly allocated pages;
	 * this is a rough cut.
	 */
	kmcopy(knew, kold, tdpages);
	kmcopy(knew+tdpages+change, kold+tdpages, spages);

	/*
	 * Validate and clear the newly allocated page table pages in the
	 * center of the new region of the kernel-map.
	 */
	i = knew + tdpages;
	p1 = &Usrptmap[i];
	p2 = p1 + change;
	while (p1 < p2) {
/* XXX is this comment right??? */
		/* tptov BELOW WORKS ONLY FOR VAX */
		mapin(p1, tptov(u.u_procp, i), p1->pg_pfnum, 1,
			  (int)(PG_V|PG_KW));
		clearseg(p1->pg_pfnum);
		p1++;
		i++;
	}
	tlbzap(u.u_procp);
#ifdef	sgi
	ptaccess(&Usrptmap[knew], kmxtob(knew), u.u_procp->p_szpt + change);
#endif

	/*
	 * Move the stack and u. pte's which are before the newly
	 * allocated pages into the last of the newly allocated pages.
	 * They are taken from the end of the current p1 region,
	 * and moved to the end of the new p1 region.
	 */
	p1 = u.u_pcb.pcb_p1br + u.u_pcb.pcb_p1lr;
	p2 = initp1br(kmxtob(knew+szpt+change)) + u.u_pcb.pcb_p1lr;
	for (i = kmxtob(kold+szpt) - p1; i != 0; i--)
		*p2++ = *p1++;

	/*
	 * Now switch to the new page tables.
	 */
	tlbzap(u.u_procp);	/* paranoid */

	s = spl7();	/* conservative */
	u.u_procp->p_p0br = kmxtob(knew);
	setp0br(u.u_procp->p_p0br);
	u.u_pcb.pcb_p1br = initp1br(kmxtob(knew+szpt+change));
	setp1br(u.u_pcb.pcb_p1br);
	u.u_pcb.pcb_szpt += change;
	u.u_procp->p_szpt += change;
	u.u_procp->p_addr = uaddr(u.u_procp);

	tlbzap(u.u_procp);
	splx(s);

	/*
	 * Finally, free old kernel-map.
	 */
	if (szpt)
		kmfree((long)szpt, (long)kold);
	return;

bad:
#if NSGIGSC > 0
	if (u.u_procp == sgstate.wman) {
		/*
		 * Can't swap out the window manager.  Nuke it.
		 * Sorry.
		 */
		psignal(u.u_procp, SIGKILL);
		runrun++;
		return;
	}
#endif
	/*
	 * Swap out the process so that the unavailable 
	 * resource will be allocated upon swapin.
	 *
	 * When resume is executed for the process, 
	 * here is where it will resume.
	 */
	if (save(u.u_ssave))
		return;
	if (swapout(u.u_procp, ods, oss) == 0) {
		/*
		 * No space to swap... it is inconvenient to try
		 * to exit, so just wait a bit and hope something
		 * turns up.  Could deadlock here.
		 *
		 * SOMEDAY REFLECT ERROR BACK THROUGH expand TO CALLERS
		 * (grow, sbreak) SO CAN'T DEADLOCK HERE.
		 */
		sleep((caddr_t)&lbolt, PRIBIO);
		goto top;
	}
	/*
	 * Set SSWAP bit, so that when process is swapped back in
	 * swapin will set u.u_pcb.pcb_sswap to u_sswap and force a
	 * return from the setjmp() above.
	 */
	u.u_procp->p_flag |= SSWAP;
	setrq(u.u_procp);
	swtch();
	/* no return */
}

kmcopy(to, from, count)
	register long to;
	long from;
	register int count;
{
	register struct pte *tp = &Usrptmap[to];
	register struct pte *fp = &Usrptmap[from];

	while (count != 0) {
		mapin(tp, tptov(u.u_procp, to), fp->pg_pfnum, 1,
			  (int)(*((int *)fp) & (PG_V|PG_PROT)));
		tp++;
		fp++;
		to++;
		count--;
	}
}
