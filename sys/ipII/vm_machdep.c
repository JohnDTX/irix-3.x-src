/*
 * Machine dependent portions of the paging code
 *
 * Written by : Kipp Hickman
 * Modified by: John Lindquist (IP2)
 *
 * $Source: /d2/3.7/src/sys/ipII/RCS/vm_machdep.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:31:06 $
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/cmap.h"
#include "../h/mount.h"
#include "../vm/vm.h"
#include "../h/text.h"
#include "../h/seg.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/ipc.h"
#include "../h/shm.h"
#include "../ipII/cx.h"
#include "../ipII/pte.h"
#include "../ipII/cpureg.h"
#include "../multibus/mbvar.h"

#ifdef	GL1
# include "../gl1/kgl.h"
#endif

/*
 * Convert a process virtual page number to its index into the hardware
 * pagemap.  For stack segment only.
 */
#define	VSTOCXI(p, v) \
	(((p)->p_cxsnum << PPCXLOG2) + \
	 ((PTEPCX << (p)->p_cxssize) - HIGHPAGES - 1 - vtosp(p, v)))

/*
 * Convert a virtual page number to a pte address.
 * This assumes that text+data are page wise contiguous
 */
struct pte *
vtopte(p, v)
	register struct proc *p;
	register unsigned v;
{
	if (isassv(p, v))			/* in stack */
		return (sptopte(p, vtosp(p, v)));
	return (p->p_p0br + v - p->p_loadc);
}

/*
 * glean:
 *	- glean the dirty/used bits off of the hardware page map, into
 *	  the software page map, when switching from one proc to another
 *	- this is sort of a reverse sureg, except we write the dirty info
 *	  into the software pte's
 *	- only used when a process loses its context in the page map, or
 *	  when a process changes its address space in a drastic manner
 */
glean(p)
	register struct proc *p;
{
	register struct pte *pte;
	register u_long *pagemap;
	register short npages;
	register long base;

	/*
	 * If process has no context, then the dirty bits have already
	 * been gathered up.
	 */
	if (p->p_flag & SLOSTCX)
		return;
	ASSERT((p->p_flag & (HAVTDCX|HAVSCX)) == (HAVTDCX|HAVSCX));

	/* figure out where first data page is */
	base = p->p_loadc;
	if (p->p_textp) {
		ASSERT(p->p_tsize);
		base += p->p_tsize;
	}
	base += (p->p_cxtdnum << PPCXLOG2);

	/* glean info from the data region */
	pte = dptopte(p, 0);
	pagemap = ((u_long *)PTMAP_BASE) + base;
	npages = p->p_dsize;
	while (--npages >= 0) {
		if (*pagemap & PG_M)
			pte->pg_m = 1;
		pagemap++;
		pte++;
	}

	/* glean info from the stack region */
	pte = sptopte(p, 0);
	base = (p->p_cxsnum << PPCXLOG2) + (PTEPCX << p->p_cxssize) -
		HIGHPAGES - 1;
	pagemap = ((u_long *)PTMAP_BASE) + base;
	npages = p->p_ssize;
	while (--npages >= 0) {
		if (*pagemap & PG_M)
			pte->pg_m = 1;
		pagemap--;
		pte--;
	}
}

/*
 * gleanpte:
 *	- we gather up the modified bit into the software pte structure
 *	  so that the pageout daemon will have a correct model of the
 *	  process state
 */
gleanpte(p, v, pte)
	register struct proc *p;
	register int v;
	register struct pte *pte;
{
	if (p->p_flag & SLOSTCX)
		return;
	ASSERT((p->p_flag & (HAVTDCX|HAVSCX)) == (HAVTDCX|HAVSCX));

	if ( isassv(p, v ) ) {
		ASSERT((ctob(v) & SEG_MSK) == SEG_STK);
		v = VSTOCXI(p, v);
	} else {
		ASSERT((ctob(v) & SEG_MSK) == SEG_TD);
		v += p->p_cxtdnum << PPCXLOG2;
	}

	if (*(((u_long *)PTMAP_BASE) + v) & PG_M)
		pte->pg_m = 1;
}

/*
 * sureg:
 *	- set up the users address space, given the software page table
 *	  info, and the context to load it with
 *	- ASSUMES UPAGES == 1
 *	- note that the user page tables DON'T contain any info about the
 *	  pages below u.u_loadaddr!
 *	- this code is sloppy; btop and btoc are used interchangeably
 */
sureg(doglean)
	short doglean;
{
	register struct pte *pte;
	register u_long *pagemap;
	register short npages;
	register struct proc *p;
	register long vaddr;
	register long scx;
	short cxsize;

	/* get context info */
	p = u.u_procp;
	if (p->p_flag & SLOSTCX)
		cxget(p);
	else
	if (doglean)
		glean(p);
	p->p_flag &= ~(SPTECHG | SLOSTCX);
	cx.cx_tduser = p->p_cxtdnum;
	cx.cx_suser = p->p_cxsnum;
	cx.cx_tdsize = p->p_cxbsize;
	cx.cx_ssize = p->p_cxssize;

	scx = p->p_cxtdnum << PPCXLOG2;
	cxsize = PTEPCX << p->p_cxbsize;	/* min size is 4 pages */

	/* now map text+data region */
	vaddr =  p->p_loadc + scx;
	pte = (struct pte *)u.u_pcb.pcb_p0br;
	pagemap = ((u_long *)PTMAP_BASE) + vaddr;
	npages = p->p_tsize + p->p_dsize;
	while (--npages >= 0) {
		if (!((struct pte *)pte)->pg_v) {
			*pagemap++ = 0;
		} else {
			*pagemap++ = *(u_long *)pte;
		}
		pte++;
	}

	/* clear out unused text+data regions */
	if (!cx.cx_zeroinuse) {
		/* map invalid unused part of context in text+data region */
		npages = cxsize - p->p_tsize - p->p_dsize - p->p_loadc;
		while (--npages >= 0)
			*pagemap++ = 0;

		/* clear the unused part of the stack */
		scx = p->p_cxsnum << PPCXLOG2;
		cxsize = PTEPCX << p->p_cxssize;
		if (cxsize > MAXSSIZ)
			cxsize = MAXSSIZ;
		pagemap = ((u_long *)PTMAP_BASE) + scx;
		npages = cxsize - p->p_ssize - HIGHPAGES;
	} else {
		npages = MAXTDSIZ - p->p_tsize - p->p_dsize - p->p_loadc;
		while (--npages >= 0)
			*pagemap++ = 0;

		/* bump pagemap pointer to over the kernel area (4mb)
		 * so 1024 is the number of ptes the the kernel takes up
		 *
		 * calc the number of pages to clear in the stack. The
		 * total area is 8mb, number of pages in 8mb is 2048
		 */
		pagemap += 1024;
		npages = 2048 - p->p_ssize - HIGHPAGES;
	}
	/* clear the unused part of the stack */
	while (--npages >= 0)
		*pagemap++ = 0;

	/* map the used part of the stack */
	pte = (struct pte *)(u.u_pcb.pcb_p1br + u.u_pcb.pcb_p1lr);
	npages = p->p_ssize;
	while (--npages >= 0) {
		if (!pte->pg_v) {
			*pagemap++ = 0;
		} else {
			*pagemap++ = *(u_long *)pte;
		}
		pte++;
	}

	if (p->p_flag & SGR) {
#ifndef KOPT_NOGL
		*pagemap++ = gr_getshmempa(p, 0) | PG_UW;
#else
		*pagemap++ = 0;
#endif
		*pagemap++ = 0;
	}
	else
	{
		npages = HIGHPAGES;
		while ( --npages >= 0 )
			*pagemap++ = 0;
	}

	/* set up shared memory regions, if user has any */
	if (p->p_flag & SSHMEM) {
		register struct shmid_ds **slot;
		register struct shmpt_ds *map;
		register struct shmid_ds *sp;
		register int i;
		register long prot;

		i = p->p_ndx * shminfo.shmseg;
		slot = &shm_shmem[i];
		map = &shm_ptbl[i];
		for (i = shminfo.shmseg; --i >= 0; slot++, map++) {
			if (sp = *slot) {
				pagemap = ((u_long *)PTMAP_BASE) + 
					    (p->p_cxtdnum << PPCXLOG2) +
					    map->shm_segbeg;
				npages = btoc(sp->shm_segsz);
				pte = sp->shm_ptbl;
				if (map->shm_sflg == RO)
					prot = PTE_RACC;
				else
					prot = PTE_RWACC;
				while (--npages >= 0) {
					*pagemap++ = pte->pg_pfnum | prot;
					pte++;
				}
			}
		}
	}
}

/*
 * Check for valid program size
 */
chksize(ts, ds, ss)
	register unsigned ts, ds, ss;
{
	register struct proc *p;
	static int maxdmap = 0;
	int savets, saveds, savess;
	int rv;

	if ((ts + ds + ss > MAXTDSIZ) || (ss > (MAXSSIZ - HIGHPAGES))) {
		u.u_error = ENOMEM;
		return (1);
	}
	/* check for swap map overflow */
	if (maxdmap == 0) {
		register int i, blk;

		blk = DMMIN;
		for (i = 0; i < NDMAP; i++) {
			maxdmap += blk;
			if (blk < DMMAX)
				blk *= 2;
		}
	}
	if (ctod(ts) > NXDAD * DMTEXT ||
	    ctod(ds) > maxdmap || ctod(ss) > maxdmap) {
		u.u_error = ENOMEM;
		return (1);
	}
	/*
	 * Make sure the process isn't bigger than our
	 * virtual memory limit.
	 *
	 * THERE SHOULD BE A CONSTANT FOR THIS.
	 */
	p = u.u_procp;
	if (ts + ds + ss + p->p_loadc > btoc(USRSTACK)) {
		u.u_error = ENOMEM;
		return (1);
	}

	/*
	 * Lastly, insure that the new allocation doesn't cause any
	 * segments to overlap.
	 */
	savets = p->p_tsize;
	saveds = p->p_dsize;
	savess = p->p_ssize;
	p->p_tsize = ts;
	p->p_dsize = ds;
	p->p_ssize = ss;
	if (rv = ckoverlap())
		u.u_error = ENOMEM;
	p->p_tsize = savets;
	p->p_dsize = saveds;
	p->p_ssize = savess;
	return (rv);
}

/*
 * ptaccess:
 *	- update hardware map to reflect new mapping of kernel virtual
 *	  space (usrpt area, utl area)
 *	- called in places where vmaccess is called, to update the
 *	  hardware map after the software map is updated
 * XXX get rid of this - put in mapin
 */
ptaccess(swpte, vaddr, size)
	register struct pte *swpte;
	register struct pte *vaddr;
	register int size;
{
	register u_long *pagemap;

	vaddr = (struct pte *)(btop((long)vaddr & ~SEG_MSK) + (KCX << PPCXLOG2));
	pagemap = ((u_long *)PTMAP_BASE) + (long)vaddr;
	while (size--) {
		*pagemap++ = swpte->pg_pfnum | PG_KW;
		swpte++;
	}
}

/*
 * Change protection codes of text segment.
 * Have to flush translation buffer since this
 * affect virtual memory mapping of current process.
 */
chgprot(addr, tprot)
	caddr_t addr;
	int tprot;
{
	unsigned v;
	int tp;
	register struct pte *pte;
	register struct proc *p;

	p = u.u_procp;
	v = btop(addr);
	if (!isatsv(p, v)) {
		u.u_error = EFAULT;
		return (0);
	}
	tp = vtotp(p, v);
	pte = tptopte(p, tp);
	*(long *)pte &= ~PG_PROT;
	if (tprot == RO)
		*(long *)pte |= PG_URKR;
	else
		*(long *)pte |= PG_UWKW;
	if (p->p_flag & SLOSTCX)
		panic("chgprot with no cx");
	sethwpte(p, v, CTEXT, pte);
	return (1);
}

/*
 * settprot:
 *	- change a text segments protection to tprot
 */
settprot(tprot)
	register int tprot;
{
	register struct pte *pte;
	register size_t i;

	pte = u.u_pcb.pcb_p0br;
	if (tprot == RW)
		tprot = PG_UWKW;
	else
		tprot = PG_URKR;
	for (i = 0; i < u.u_tsize; i++, pte++) {
		*(long *)pte &= ~PG_PROT;
		*(long *)pte |= tprot;
	}
}

/*
 * mapkernel:
 *	- map a range of the hardware page and protection maps within the
 *	  kernel's address space
 */
mapkernel(vaddr, npages, pfnum, prot)
	register long vaddr;
	register int npages;
	register int pfnum, prot;
{
	register u_long *pagemap;

	vaddr = btop(vaddr & ~SEG_MSK) + (KCX << PPCXLOG2);
	pagemap = ((u_long *)PTMAP_BASE) + vaddr;
	while (npages--) {
		*pagemap++ = pfnum | prot;
		pfnum++;
	}
}

sethwpte(p, v, type, pte)
	register struct proc *p;
	register int v;
	int type;
	register struct pte *pte;
{
	if (p->p_flag & SLOSTCX)
		return;
	ASSERT((p->p_flag & (HAVTDCX|HAVSCX)) == (HAVTDCX|HAVSCX));

	if (type == CSTACK) {
		ASSERT((ctob(v) & SEG_MSK) == SEG_STK);
		v = VSTOCXI(p, v);
	} else {
		ASSERT((ctob(v) & SEG_MSK) == SEG_TD);
		v += (p->p_cxtdnum << PPCXLOG2);
	}

	if (pte->pg_fod || !pte->pg_v)
		*(((u_long *)PTMAP_BASE) + v) = 0;
	else
		*(((u_long *)PTMAP_BASE) + v) = *(u_long *)pte;
}

#ifdef	OS_DEBUG
cleanpte(p, v, type, pte)
	register struct proc *p;
	register int v;
	int type;
	register struct pte *pte;
{
	register int ov;

	if (p->p_flag & SLOSTCX)
		return;
	ASSERT((p->p_flag & (HAVTDCX|HAVSCX)) == (HAVTDCX|HAVSCX));

	ov = v;
	if (type == CSTACK) {
		ASSERT((ctob(v) & SEG_MSK) == SEG_STK);
		v = VSTOCXI(p, v);
	} else {
		ASSERT((ctob(v) & SEG_MSK) == SEG_TD);
		v += (p->p_cxtdnum << PPCXLOG2);
	}

	if (*(((u_long *)PTMAP_BASE) + v) & PG_M) {
		printf("pid=%d v=%d ov=%d type=%d pte=%x *pte=%x *hw=%x\n",
			       p->p_pid, v, ov, type, pte, *(long *)pte,
			       *(((u_long *)PTMAP_BASE) + v));
		panic("cleanpte");
	}
}
#endif

/* XXX get rid of these */
/*
 * getpte:
 *	- read the hardware page and protection mapping information and
 *	  translate it into a software pte
 */
getpte(vaddr, cx, pte)
	register long vaddr;
	register short cx;
	register struct pte *pte;
{
	register short page;

	page = btop(vaddr & ~SEG_MSK) + (cx << PPCXLOG2);
	*(u_long *)pte = *(((u_long *)PTMAP_BASE) + page) & PTE_GOODBITS;
	pte->pg_v = 1;
}

/*
 * setpte:
 *	- translate a software pte into the hardware page and protection
 *	  information and load it into the page and protection maps
 *	- this is for the text/data context for the KERNEL only
 */
setpte(vaddr, cx, pte)
	register long vaddr;
	register short cx;
	struct pte *pte;
{
	register int page;

	page = btop(vaddr & ~SEG_MSK) + (cx << PPCXLOG2);
	if (pte->pg_fod || !pte->pg_v) {
		*(((u_long *)PTMAP_BASE) + page) = 0;
	} else {
		*(((u_long *)PTMAP_BASE) + page) = *(u_long *)pte;
	}
}
