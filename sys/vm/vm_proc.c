/*
 * $Source: /d2/3.7/src/sys/vm/RCS/vm_proc.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:36:02 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/map.h"
#include "../h/cmap.h"
#include "../h/text.h"
#include "../vm/vm.h"
#include "machine/pte.h"

/*
 * Get virtual memory resources for a new process.
 * Called after page tables are allocated, but before they
 * are initialized, we initialize the memory management registers,
 * and then expand the page tables for the data and stack segments
 * creating zero fill pte's there.  Text pte's are set up elsewhere.
 *
 * SHOULD FREE EXTRA PAGE TABLE PAGES HERE OR SOMEWHERE.
 */
vgetvm(ts, ds, ss)
	size_t ts, ds, ss;
{
	u.u_pcb.pcb_p0lr = AST_NONE;
	setp0lr(ts);
	setp1lr(P1PAGES - HIGHPAGES);
	u.u_procp->p_tsize = ts;
	u.u_tsize = ts;
	expand((int)ss, 1, 0);
	expand((int)ds, 0, ts);
}

/*
 * Release the virtual memory resources (memory
 * pages, and swap area) associated with the current process.
 * Caller must not be swappable.  Used at exit or execl.
 */
vrelvm()
{
	register struct proc *p = u.u_procp;
	register int s;
	
	/*
	 * Release memory; text first, then data and stack pages.
	 */
	xfree();
	p->p_rssize -= vmemfree(dptopte(p, 0), p->p_dsize);
	p->p_rssize -= vmemfree(sptopte(p, p->p_ssize - 1), p->p_ssize);
	ASSERT(p->p_rssize == 0);
	/*
	 * Wait for all page outs to complete, then
	 * release swap space.
	 */
	p->p_swrss = 0;
	s = spl6();
	while (p->p_poip)
		sleep((caddr_t)&p->p_poip, PSWP+1);
	splx(s);
	(void) vsexpand((size_t)0, &u.u_dmap, 1);
	(void) vsexpand((size_t)0, &u.u_smap, 1);
	p->p_tsize = 0;
	p->p_dsize = 0;
	p->p_ssize = 0;
	u.u_tsize = 0;
	u.u_dsize = 0;
	u.u_ssize = 0;
}

/*
 * Change the size of the data+stack regions of the process.
 * If the size is shrinking, it's easy-- just release virtual memory.
 * If it's growing, initalize new page table entries as either
 * 'zero fill on demand' pages (if fod is 0), or 'inode fill on demand'
 * pages (if fod is non zero).
 */
expand(change, region, fod)
	int change, region;
	int fod;
{
	register struct proc *p;
	register struct pte *base, *p0, *p1;
	struct pte proto;
	int p0lr, p1lr;
	size_t ods, oss;

	p = u.u_procp;
	if (change == 0)
		return;

	/*
	 * We get rid of our context here, because we might be growing
	 * past our current size.  While we are at it, gather up the dirty
	 * bits.
	 */
	cxput(p, 1);

#ifdef PGINPROF
	vmsizmon();
#endif

	/*
	 * Update the sizes to reflect the change.  Note that we may
	 * swap as a result of a ptexpand, but this will work, because
	 * the routines which swap out will get the current text and data
	 * sizes from the arguments they are passed, and when the process
	 * resumes the lengths in the proc structure are used to 
	 * build the new page tables.
	 */
	ods = u.u_dsize;
	oss = u.u_ssize;
	if (region == 0) {
		p->p_dsize += change;
		u.u_dsize += change;
	} else {
		p->p_ssize += change;
		u.u_ssize += change;
	}

	/*
	 * Compute the end of the text+data regions and the beginning
	 * of the stack region in the page tables,
	 * and expand the page tables if necessary.
	 */
	p0 = u.u_pcb.pcb_p0br + (u.u_pcb.pcb_p0lr&~AST_CLR);
	p1 = u.u_pcb.pcb_p1br + (u.u_pcb.pcb_p1lr&~PME_CLR);
	if (p0 + change >= p1)
		ptexpand(ctopt(change - (p1 - p0)), ods, oss);
	/* PTEXPAND SHOULD GIVE BACK EXCESS PAGE TABLE PAGES */

	/*
	 * Compute the base of the allocated/freed region.
	 */
	p0lr = u.u_pcb.pcb_p0lr&~AST_CLR;
	p1lr = u.u_pcb.pcb_p1lr&~PME_CLR;
	if (region == 0)
		base = u.u_pcb.pcb_p0br + p0lr + (change > 0 ? 0 : change);
	else
		base = u.u_pcb.pcb_p1br + p1lr - (change > 0 ? change : 0);
	/*
	 * If we shrunk, give back the virtual memory.
	 */
	if (change < 0)
		p->p_rssize -= vmemfree(base, -change);

	/*
	 * Update the processor length registers and copies in the pcb.
	 */
	if (region == 0)
		setp0lr(p0lr + change);
	else
		setp1lr(p1lr - change);

	/*
	 * If shrinking, clear pte's, otherwise
	 * initialize zero fill on demand pte's.
	 */
	initpte(&proto, 0, PG_UW);
	if (change < 0)
		change = -change;
	else {
		proto.pg_fod = 1;
		if (!fod) {
			((struct fpte *)&proto)->pg_zfod = 1;
			cnt.v_nzfod += change;
		} else
			cnt.v_nexfod += change;
	}
	while (--change >= 0)
		*base++ = proto;
}

/*
 * Create a duplicate copy of the current process
 * in process slot p, which has been partially initialized
 * by newproc().
 *
 * Could deadlock here if two large proc's get page tables
 * and then each gets part of his UPAGES if they then have
 * consumed all the available memory.  This can only happen when
 *	USRPTSIZE + UPAGES * NPROC > maxmem
 * which is impossible except on systems with tiny real memories,
 * when large procs stupidly fork() instead of vfork().
 */
procdup(cp)
	register struct proc *cp;
{
	/*
	 * Allocate page tables for new process, waiting
	 * for memory to be free.
	 */
	while (vgetpt(cp, vmemall) == 0) {
		kmap_wanted = 1;
		sleep((caddr_t)&kmap_wanted, PSWP+4);
	}

	/*
	 * Snapshot the current u. area pcb and get a u.
	 * for the new process, a copy of our u.
	 */
	(void) vgetu(cp, vmemall, Forkmap, &forkutl, &u);

	/*
	 * A real fork; clear vm statistics of new process
	 * and link into the new text segment.
	 */
	bcopy((caddr_t)&u.u_cdmap, (caddr_t)&forkutl.u_dmap,
	      sizeof(forkutl.u_dmap));
	bcopy((caddr_t)&u.u_csmap, (caddr_t)&forkutl.u_smap,
	      sizeof(forkutl.u_smap));
	forkutl.u_outime = 0;
	if (cp->p_textp) {
		cp->p_textp->x_count++;
		xlink(cp);
	}

	/*
	 * Duplicate data and stack space of current process
	 * in the new process.
	 */
	vmdup(cp, dptopte(cp, 0), dptov(cp, 0), cp->p_dsize, CDATA);
	vmdup(cp, sptopte(cp, cp->p_ssize - 1), sptov(cp, cp->p_ssize - 1),
		 cp->p_ssize, CSTACK);
	cp->p_flag |= SPTECHG;

	/*
	 * Return 0 in parent.
	 */
	return (0);
}

vmdup(cp, pte, v, count, type)
	struct proc *cp;
	register struct pte *pte;
	register unsigned v;
	register size_t count;
	int type;
{
	register struct pte *opte = vtopte(u.u_procp, v);

	while (count != 0) {
		count--;
		if (opte->pg_fod) {
			*(int *)pte++ = *(int *)opte++;
			v++;
			continue;
		}
		ASSERT(!(opte->pg_pfnum && cmap[pgtocm(opte->pg_pfnum)].c_intrans));
		(void) vmemall(pte, 1, cp, type);
		ASSERT(pte->pg_pfnum && !pte->pg_fod);
		copyseg((caddr_t)ctob(v), pte->pg_pfnum);
		initpte(pte, pte->pg_pfnum, PG_V | PG_M | PG_UW);
		munlock(pte->pg_pfnum);

		cp->p_rssize++;
		opte++;
		pte++;
		v++;
	}
}
