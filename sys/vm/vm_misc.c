/*
 * Miscellaneous vm code.
 *
 * $Source: /d2/3.7/src/sys/vm/RCS/vm_misc.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:59 $
 */
#include "../h/param.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/ipc.h"
#include "../h/shm.h"
#include "../h/cmap.h"
#include "../vm/vm.h"
#include "../h/text.h"
#include "../h/vadvise.h"
#include "machine/pte.h"

/*
 * ckoverlap:
 *	- insure that none of the processes segments overlap
 *	- return 1 if they overlap, 0 if they don't
 */
int
ckoverlap()
{	
	register struct shmid_ds **slot0;
	register struct shmpt_ds *map0;
	register struct proc *p;
	register int caddr0, caddr1;
	register int caddr0end;
	register int limit;
	register int i;
	register int j;

	p = u.u_procp;
	limit = p->p_loadc + p->p_tsize + p->p_dsize;

#ifdef	pmII
	/*
	 * Check that phys segments don't intersect
	 *	1. the text, data, bss segment
	 *	2. each other
	 *	3. the shared memory segments
	 */
	if (u.u_pcb.pcb_physused) {
	    register struct phys *ph0, *ph1;

	    for (ph0 = &u.u_pcb.pcb_phys[0]; ph0 < &u.u_pcb.pcb_phys[NPHYS];
		     ph0++) {
		if (ph0->p_phsize) {
		    caddr0 = btoc(ph0->p_phladdr);
		    caddr0end = caddr0 + ph0->p_phsize;
		    /*
		     * Test phys segment against the text & data region
		     */
		    if (caddr0 < limit)
			    return (1);
		    /*
		     * Test phys segment against each of the other
		     * remaining phys segments
		     */
		    for (ph1 = ph0 + 1; ph1 < &u.u_pcb.pcb_phys[NPHYS]; ph1++) {
			if (ph1->p_phsize) {
			    caddr1 = btoc(ph1->p_phladdr);
			    /*
			     * See if this segment completely non-overlaps
			     * the other segment by checking to see
			     * if it ends on the lower size of caddr0, or
			     * begins on the higher side of the end of
			     * caddr0 (caddr0end).
			     */
			    if (!((caddr1 + ph1->p_phsize <= caddr0) ||
				  (caddr1 >= caddr0end)))
				return (1);
			}
		    }
		    /*
		     * Test phys segment against all of the shared memory
		     * segments
		     */
		    if (u.u_procp->p_flag & SSHMEM) {
			i = u.u_procp->p_ndx * shminfo.shmseg;
			slot0 = &shm_shmem[i];
			map0 = &shm_ptbl[i];
			for (i = shminfo.shmseg; --i >= 0; slot0++, map0++) {
			    if (*slot0) {
				caddr1 = map0->shm_segbeg;
				if (!((caddr1 + btoc((*slot0)->shm_segsz) <=
					      caddr0) ||
				      (caddr1 >= caddr0end)))
				    return (1);
			    }
			}
		    }
		}
	    }
	}
#endif	pmII

	/*
	 * Check that each shared memory segment doesn't overlap either
	 * the text/data region or any other shared memory segment.
	 * The above code has already checked that the shared memory
	 * segments don't overlap any phys segments.
	 */
	if (u.u_procp->p_flag & SSHMEM) {
	    register struct shmid_ds **slot1;
	    register struct shmpt_ds *map1;

	    i = u.u_procp->p_ndx * shminfo.shmseg;
	    slot0 = &shm_shmem[i];
	    map0 = &shm_ptbl[i];
	    for (i = shminfo.shmseg; --i >= 0; ) {
		if (*slot0) {
		    caddr0 = map0->shm_segbeg;
		    caddr0end = caddr0 + btoc((*slot0)->shm_segsz);
		    /*
		     * Test shmem segment against the text & data region
		     */
		    if (caddr0 < limit)
			    return (1);
		    /*
		     * Test shmem segment against each of the other
		     * remaining shmem segments
		     */
		    slot1 = slot0 + 1;
		    map1 = map0 + 1;
		    for (j = i; --j >= 0; slot1++, map1++) {
			if (*slot1) {
			    caddr1 = map1->shm_segbeg;
			    if (!((caddr1 + btoc((*slot1)->shm_segsz) <=
					  caddr0) ||
				  (caddr1 >= caddr0end)))
				return (1);
			}
		    }
		}
	    }
	}

	return (0);
}

vadvise()
{
	register struct a {
		int	anom;
	} *uap;
	register struct proc *rp = u.u_procp;
	int oanom = rp->p_flag & SUANOM;
	register struct pte *pte;
	register struct cmap *c;
	register size_t i;

	uap = (struct a *)u.u_ap;
	rp->p_flag &= ~(SSEQL|SUANOM);
	switch (uap->anom) {

	case VA_ANOM:
		rp->p_flag |= SUANOM;
		break;

	case VA_SEQL:
		rp->p_flag |= SSEQL;
		break;
	}

	/*
	 * Ditch hardware mapping, as we are about to invalidate it anyway.
	 */
	cxput(rp, 1);

	if ((oanom && (rp->p_flag & SUANOM) == 0) || uap->anom == VA_FLUSH) {
		for (i = 0; i < rp->p_dsize; i++) {
			pte = dptopte(rp, i);
			if (pte->pg_v) {
				c = &cmap[pgtocm(pte->pg_pfnum)];
				if (c->c_lock)
					continue;
				pte->pg_v = 0;
			}
		}
	}
	if (uap->anom == VA_FLUSH) {	/* invalidate all pages */
		for (i = 1; i < rp->p_ssize; i++) {
			pte = sptopte(rp, i);
			if (pte->pg_v) {
				c = &cmap[pgtocm(pte->pg_pfnum)];
				if (c->c_lock)
					continue;
				pte->pg_v = 0;
			}
		}
		for (i = 0; i < rp->p_tsize; i++) {
			pte = tptopte(rp, i);
			if (pte->pg_v) {
				c = &cmap[pgtocm(pte->pg_pfnum)];
				if (c->c_lock)
					continue;
				pte->pg_v = 0;
				distpte(rp->p_textp, i, pte);
			}
		}
	}
	rp->p_flag |= SPTECHG;
}
