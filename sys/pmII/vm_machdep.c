/*
 * Machine dependent portions of the paging code
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/sys/pmII/RCS/vm_machdep.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:54 $
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
#include "../pmII/cx.h"
#include "../pmII/pte.h"
#include "../pmII/cpureg.h"
#include "../multibus/mbvar.h"

#ifdef	GL1
# include "../gl1/kgl.h"
#endif

/*
 * Convert a virtual page number to a pte address.
 * This assumes that text+data are page wise contiguous
 */
struct pte *
vtopte(p, v)
	register struct proc *p;
	register int v;
{
	if (isassv(p, v))
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
	register u_short *protmap;
	register short npages;
	register long vaddr;

	/*
	 * If process has no context, then the dirty bits have already
	 * been gathered up.
	 */
	if (p->p_flag & SLOSTCX)
		return;

	/* figure out where first data page is */
	vaddr = p->p_loadc;
	if (p->p_textp) {
		if (p->p_tsize)
			vaddr += p->p_tsize;
#ifdef	ASSERT
		else {
			printf("pid=%d\n", p->p_pid);
			panic("glean: xp !ts");			/* XXX debug */
		}
#endif	ASSERT
	}

	/* glean info from the data region */
	pte = dptopte(p, 0);
	vaddr = vaddr ^ (p->p_cxnum << 4);
	protmap = ((u_short *)PROTBASE) + vaddr;
	npages = p->p_dsize;
	while (--npages >= 0) {
		if (*protmap & PR_DIRTY)
			pte->pg_m = 1;
		protmap++;
		pte++;
	}

	/* glean info from the stack region */
	pte = sptopte(p, 0);
	vaddr = btop(USRSTACK - 1) ^ (p->p_cxnum << 4);
	protmap = ((u_short *)PROTBASE) + vaddr;
	npages = p->p_ssize;
	while (--npages >= 0) {
		if (*protmap & PR_DIRTY)
			pte->pg_m = 1;
		protmap--;
		pte--;
	}
}

/*
 * gleanpte:
 *	- we gather up the modified bit into the software pte structure
 *	  so that the pageout daemon will have a correct model of the
 *	  process state
 */
gleanpte(p, vpage, pte)
	register struct proc *p;
	int vpage;
	register struct pte *pte;
{
	register u_short *protmap;

	if (p->p_flag & SLOSTCX)
		return;
	protmap = ((u_short *)PROTBASE) + (vpage ^ (p->p_cxnum << 4));
	if (*protmap & PR_DIRTY)
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
	register struct grotpte *pte;
	register u_short *pagemap, *protmap;
	register short npages;
	register struct proc *p;
	register long vaddr;
	register struct phys *ph;
	register short pf, prot;
	register long scx;
	struct pte apte;
	short cxsize;

    /* get context info */
	p = u.u_procp;
	if (p->p_flag & SLOSTCX)
		cxget(p);
	else
	if (doglean)
		glean(p);
	p->p_flag &= ~(SPTECHG | SLOSTCX);
	cx.cx_user = p->p_cxnum;
	scx = p->p_cxnum << 16;
	cxsize = 0x10 << p->p_cxbsize;		/* min size is 16 pages */

	/* set up graphics pages, if using graphics */
	vaddr = btop(USRSTACK ^ scx);
	pagemap = ((u_short *)PAGEBASE) + vaddr;
	protmap = ((u_short *)PROTBASE) + vaddr;
#ifndef	KOPT_NOGL
	if (p->p_flag & SGR) {
		prot = ((PG_AS_MBIO | PG_UW) >> 16) | cx.cx_user;
		for (npages = 1; npages < 8; npages++) {
			*pagemap++ = npages;
			*protmap++ = prot;
		}
		prot = ((PG_AS_OBRAM | PG_UW) >> 16) | cx.cx_user;
#ifdef	GL1
		*pagemap++ = shmem_pa;
		*protmap++ = prot;
		*pagemap++ = shmem_pa1;
		*protmap++ = prot;
#else
		*pagemap++ = gr_getshmempa(p, 0);
		*protmap++ = prot;
		*pagemap++ = 0;
		*protmap++ = PR_INVALID;
#endif	GL1
	} else
#endif	KOPT_NOGL
	{
		npages = 9;
		while (--npages >= 0) {
			*pagemap++ = 0;
			*protmap++ = PR_INVALID;
		}
	}

	/* map in the interrupt vectors and udot in users virtual space */
	*(long *)&apte = 0 | PG_NOACC | PG_AS_OBRAM | PG_V;
	setpte((long)0, cx.cx_user, &apte);
	*(long *)&apte = 0 | PG_KR | PG_AS_OBRAM | PG_V;
	setpte((long)IVEC_VBASE, cx.cx_user, &apte);
	*(long *)&apte = p->p_addr->pg_pfnum | PG_KW | PG_AS_OBRAM | PG_V;
	setpte((long)UDOT_VBASE, cx.cx_user, &apte);

	/* if load address is 0x2000, then map sky board into user space */
	if (p->p_loadc == 2) {
		vaddr = btop(0x1000 ^ scx);
		*(((u_short *)PAGEBASE) + vaddr) = 0;
		*(((u_short *)PROTBASE) + vaddr) =
			((PG_UWKW | PG_AS_MBIO) >> 16) | cx.cx_user;
	}

	/* now map text+data region */
	vaddr = btop(u.u_loadaddr ^ scx);
	pte = (struct grotpte *)u.u_pcb.pcb_p0br;
	pagemap = ((u_short *)PAGEBASE) + vaddr;
	protmap = ((u_short *)PROTBASE) + vaddr;
	npages = p->p_tsize + p->p_dsize;
	prot = cx.cx_user | (PG_AS_OBRAM >> 16);
	while (--npages >= 0) {
		if (!((struct pte *)pte)->pg_v) {
			*pagemap++ = 0;
			*protmap++ = PR_INVALID;
		} else {
			*pagemap++ = pte->pg_pfnum;
			*protmap++ = (pte->pg_protbits & 0x3F00) | prot;
		}
		pte++;
	}

	/* clear out unused text+data and stack regions */
	if (!cx.cx_zeroinuse) {
		/* map invalid unused part of context in text+data region */
		npages = cxsize - p->p_tsize - p->p_dsize - p->p_loadc;
		while (--npages >= 0) {
			*pagemap++ = 0;
			*protmap++ = PR_INVALID;
		}

		/* clear the unused part of the stack */
		if (cxsize > MAXSSIZ)
			cxsize = MAXSSIZ;
		vaddr = btop((USRSTACK - ctob(cxsize) + ctob(HIGHPAGES)) ^ scx);
		pagemap = ((u_short *)PAGEBASE) + vaddr;
		protmap = ((u_short *)PROTBASE) + vaddr;
		npages = cxsize - p->p_ssize - HIGHPAGES;
	} else
		npages = MAXTDSIZ - p->p_tsize - p->p_dsize - p->p_loadc -
			p->p_ssize - HIGHPAGES;

	/* clear unused region */
	while (--npages >= 0) {
		*pagemap++ = 0;
		*protmap++ = PR_INVALID;
	}

	/* map the used part of the stack */
	pte = (struct grotpte *)(u.u_pcb.pcb_p1br + u.u_pcb.pcb_p1lr);
	npages = p->p_ssize;
	prot = cx.cx_user | (PG_AS_OBRAM >> 16);
	while (--npages >= 0) {
		if (!((struct pte *)pte)->pg_v) {
			*pagemap++ = 0;
			*protmap++ = PR_INVALID;
		} else {
			*pagemap++ = pte->pg_pfnum;
			*protmap++ = (pte->pg_protbits & 0x3F00) | prot;
		}
		pte++;
	}

	/* set up phys() regions, if user has one */
	if (u.u_pcb.pcb_physused) {
		for (ph = &u.u_pcb.pcb_phys[0]; ph < &u.u_pcb.pcb_phys[NPHYS];
			ph++) {
			if (ph->p_phsize) {
				vaddr = btop(ph->p_phladdr ^ scx);
				pagemap = ((u_short *)PAGEBASE) + vaddr;
				protmap = ((u_short *)PROTBASE) + vaddr;
				npages = ph->p_phsize;
				pf = btop(ph->p_phpaddr);
				prot = (pf & 0x3000) | (PG_UWKW >> 16) | cx.cx_user;
				pf &= 0x0FFF;
				while (--npages >= 0) {
					*pagemap++ = pf++;
					*protmap++ = prot;
				}
			}
		}
	}

	/* set up shared memory regions, if user has any */
	if (p->p_flag & SSHMEM) {
		register struct shmid_ds **slot;
		register struct shmpt_ds *map;
		register struct shmid_ds *sp;
		register int i;

		i = p->p_ndx * shminfo.shmseg;
		slot = &shm_shmem[i];
		map = &shm_ptbl[i];
		for (i = shminfo.shmseg; --i >= 0; slot++, map++) {
			if (sp = *slot) {
				vaddr = btop(ctob(map->shm_segbeg) ^ scx);
				pagemap = ((u_short *)PAGEBASE) + vaddr;
				protmap = ((u_short *)PROTBASE) + vaddr;
				npages = btoc(sp->shm_segsz);
				pte = (struct grotpte *) sp->shm_ptbl;
				if (map->shm_sflg == RO)
					prot = PR_URKW | PR_ASOBRAM;
				else
					prot = PR_UWKW | PR_ASOBRAM;
				prot |= cx.cx_user;
				while (--npages >= 0) {
					*pagemap++ = pte->pg_pfnum;
					*protmap++ = prot;
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
	register u_short *pagemap, *protmap;

	vaddr = (struct pte *)btop((long)vaddr ^ (KCX << 16));
	pagemap = ((u_short *)PAGEBASE) + (long)vaddr;
	protmap = ((u_short *)PROTBASE) + (long)vaddr;
	while (size--) {
		*pagemap++ = swpte->pg_pfnum;
		*protmap++ = ((PG_AS_OBRAM | PG_KW) >> 16) | KCX;
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
	register int v;
	register int tp;
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
		*(long *)pte |= PG_URKW;
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
		tprot = PG_URKW;
	for (i = 0; i < u.u_tsize; i++, pte++) {
		*(long *)pte &= ~PG_PROT;
		*(long *)pte |= tprot;
	}
}

#ifdef	NOTDEF
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
	register u_short *pagemap, *protmap;

	vaddr = btop(vaddr ^ (KCX << 16));
	pagemap = ((u_short *)PAGEBASE) + vaddr;
	protmap = ((u_short *)PROTBASE) + vaddr;
	prot |= KCX;
	while (npages--) {
		*pagemap++ = pfnum++;
		*protmap++ = prot;
	}
}
#endif

/*
 * sethwpte:
 *	- translate a software pte into the hardware page and protection
 *	  information and load it into the page and protection maps
 *	- don't bother if the process has lost its pagemap resource
 */
/* ARGSUSED */
sethwpte(p, vpage, type, pte)
	register struct proc *p;
	register int vpage;
	int type;
	register struct pte *pte;
{
	if (p->p_flag & SLOSTCX)
		return;
	vpage = vpage ^ (p->p_cxnum << 4);
	if (pte->pg_fod || !pte->pg_v) {
		*(((u_short *)PAGEBASE) + vpage) = 0;
		*(((u_short *)PROTBASE) + vpage) = PR_INVALID;
	} else {
		*(((u_short *)PAGEBASE) + vpage) =
			((struct grotpte *)pte)->pg_pfnum;
		*(((u_short *)PROTBASE) + vpage) =
			(((struct grotpte *)pte)->pg_protbits & 0x3F00) |
				p->p_cxnum;
#ifdef	ASSERT
		if ((*(long *)pte & PG_AS) != PG_AS_OBRAM) {
			printf("pte=%x *pte=%x\n", pte, *(long *)pte);
			panic("sethwpte not correct PG_AS");
		}
#endif	ASSERT
	}
}

#ifdef	ASSERT
/*
 * Insure that the given pte is clean - that is, the modified bit in the
 * hardware map MUST be zero.
 */
/* ARGSUSED */
cleanpte(p, vpage, type, pte)
	register struct proc *p;
	register int vpage;
	int type;
	register struct pte *pte;
{
	if (p->p_flag & SLOSTCX)
		return;

	if (*(((u_short *)PROTBASE) + (vpage ^ (p->p_cxnum << 4))) & PR_DIRTY) {
		printf("pid=%d v=%d type=%d pte=%x *pte=%x\n",
			       p->p_pid, vpage, type, pte, *(long *)pte);
		debug("cleanpte");
	}
}
#endif	ASSERT

/* XXX remove these */

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

	page = btop(vaddr ^ (cx << 16));
	((struct grotpte *)pte)->pg_pfnum = *(((u_short *)PAGEBASE) + page);
	((struct grotpte *)pte)->pg_protbits =
		*(((u_short *)PROTBASE) + page) & 0xFF00;
	pte->pg_v = 1;
}

/*
 * setpte:
 *	- translate a software pte into the hardware page and protection
 *	  information and load it into the page and protection maps
 *	- this is for the KERNEL only
 */
setpte(vaddr, cx, pte)
	register long vaddr;
	short cx;
	register struct pte *pte;
{
	register short vpage;

	vpage = btop(vaddr);
	vpage = vpage ^ (cx << 4);
	if (pte->pg_fod || !pte->pg_v) {
		*(((u_short *)PAGEBASE) + vpage) = 0;
		*(((u_short *)PROTBASE) + vpage) = PR_INVALID;
	} else {
		*(((u_short *)PAGEBASE) + vpage) =
			((struct grotpte *)pte)->pg_pfnum;
		*(((u_short *)PROTBASE) + vpage) =
			(((struct grotpte *)pte)->pg_protbits & 0x3F00) | cx;
	}
}

/* XXX end */

#undef	GOSH

#ifdef	GOSH
/*
 * Icky checksuming stuff to insure that paged-out data matches paged-in
 * data.
 */
#define	NSUMS	12000
short	savedsums[NSUMS];

short
compute_cksum(pf)
	int pf;
{
	register short checksum;
	register long *lp;
	register long len;
	struct pte savepte, apte;

	getpte(SCRPG3_VBASE, KCX, &savepte);
	*(long *)&apte = pf | PG_V | PG_KR | PG_AS_OBRAM;
	setpte(SCRPG3_VBASE, KCX, &apte);

	lp = (long *)SCRPG3_VBASE;
	len = NBPG / sizeof(long);
	checksum = 0;
	while (--len >= 0) {
		checksum ^= *lp++;
		checksum <<= 1;
	}

	setpte(SCRPG3_VBASE, KCX, &savepte);
	return (checksum);
}

/*
 * Record the checksum for sector bn
 */
set_cksum(pf, bn, where)
	int pf;
	daddr_t bn;
	char *where;
{
	if ((bn < 0) || (bn >= NSUMS))
		return;
	savedsums[bn] = compute_cksum(pf);
}

/*
 * Check the checksum for sector bn
 */
check_cksum(pf, bn, where)
	int pf;
	daddr_t bn;
	char *where;
{
	short sum;

	if ((bn < 0) || (bn >= NSUMS))
		return;
	sum = compute_cksum(pf);
	if (sum != savedsums[bn]) {
		trace("sum=%x bn=%d savedsum=%x MISMATCH\n", sum, bn,
			      savedsums[bn]);
		debug("check_cksum");
	}
}
#endif	GOSH
