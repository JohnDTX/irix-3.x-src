/*
 * $Source: /d2/3.7/src/sys/vm/RCS/vm_swap.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:36:06 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/text.h"
#include "../h/map.h"
#include "../h/cmap.h"
#include "../vm/vm.h"
#include "../h/buf.h"
#include "../h/sysinfo.h"
#include "machine/pte.h"

/*
 * Swap a process in.
 */
swapin(p)
	register struct proc *p;
{
	register struct text *xp;
	register int s;

	ASSERT(!(p->p_flag & SLOAD));
	if (xp = p->p_textp) 
		xlock(xp);
	p->p_szpt = ctopt(p->p_ssize+p->p_dsize+p->p_tsize+UPAGES);
	if (vgetpt(p, memall) == 0)
		goto nomem;
	if (vgetu(p, memall, Swapmap, &swaputl, (struct user *)0) == 0) {
		vrelpt(p);
		goto nomem;
	}

	syswait.swap++;
	swdspt(p, &swaputl, B_READ);
#ifdef	OS_DEBUG
{
	register int i;
	/*
	 * Make sure swdspt didn't smash u. pte's
	 */
	for (i = 0; i < UPAGES; i++) {
		if (Swapmap[i].pg_pfnum != p->p_addr[i].pg_pfnum)
			panic("swapin");
	}
}
#endif
	vrelswu(p, &swaputl);
	if (xp) {
		xlink(p);
		xunlock(xp);
	}
	p->p_rssize = 0;
	s = spl6();
	p->p_flag |= SLOAD;
	splx(s);

	p->p_time = 0;
	cnt.v_swpin++;
	syswait.swap--;
	return (1);

nomem:
	if (xp)
		xunlock(xp);
	return (0);
}

#define	notswappable(p) \
	((p)->p_flag&(SSYS|SLOCK|SULOCK|SPAGE|SKEEP|SWEXIT|SPHYSIO|SGR))

int	xswapwant, xswaplock;
/*
 * Swap out process p.
 * ds and ss are the old data size and the stack size
 * of the process, and are supplied during page table
 * expansion swaps.
 */
swapout(p, ds, ss)
	register struct proc *p;
	size_t ds, ss;
{
	register struct pte *map;
	register struct user *utl;
	register int a;
	int s;
	int rc = 1;

	ASSERT(!notswappable(p));
	s = 1;
	map = Xswapmap;
	utl = &xswaputl;
	if (xswaplock & s)
		if ((xswaplock & 2) == 0) {
			s = 2;
			map = Xswap2map;
			utl = &xswap2utl;
		}
	a = spl6();
	while (xswaplock & s) {
		xswapwant |= s;
		sleep((caddr_t)map, PSWP);
	}
	xswaplock |= s;
	splx(a);
	uaccess(p, map, utl);
	if (vgetswu(p, utl) == 0) {
		swkill(p, "swapout");
		rc = 0;
		goto out;
	}

	utl->u_odsize = ds;
	utl->u_ossize = ss;
	p->p_flag |= SLOCK;

	syswait.swap++;
	cxput(p, 1);
	p->p_flag &= ~SPTECHG;

	if (p->p_textp) {
		if (p->p_textp->x_ccount == 1)
			p->p_textp->x_swrss = p->p_textp->x_rssize;
		xccdec(p->p_textp, p);
	}
	p->p_swrss = p->p_rssize;
	vsswap(p, dptopte(p, 0), CDATA, 0, ds, &utl->u_dmap);
	vsswap(p, sptopte(p, 0), CSTACK, 0, ss, &utl->u_smap);
	ASSERT(p->p_rssize == 0);

	swdspt(p, utl, B_WRITE);
	(void) spl6();		/* hack memory interlock XXX */
	vrelu(p, 1);
	if ((p->p_flag & SLOAD) && (p->p_stat != SRUN || p != u.u_procp))
		panic("swapout");
	p->p_flag &= ~SLOAD;
	vrelpt(p);
	p->p_flag &= ~SLOCK;
	p->p_time = 0;

	cnt.v_swpout++;
	syswait.swap--;

out:
	xswaplock &= ~s;
	if (xswapwant & s) {
		xswapwant &= ~s;
		wakeup((caddr_t)map);
	}
	if (rc == 0) {
		a = spl6();
		p->p_flag |= SLOAD;
		splx(a);
	}
	return (rc);
}

/*
 * Swap the data and stack page tables in or out.
 * Only hard thing is swapping out when new pt size is different than old.
 * If we are growing new pt pages, then we must spread pages with 2 swaps.
 * If we are shrinking pt pages, then we must merge stack pte's into last
 * data page so as not to lose them (and also do two swaps).
 */
swdspt(p, utl, rdwri)
	register struct proc *p;
	register struct user *utl;
{
	register int szpt, tsz, ssz;
	int tdlast, slast, tdsz;
#ifdef	OS_DEBUG
	register struct pte *pte;
	register int i;
#endif

	szpt = ctopt(p->p_tsize+p->p_dsize+p->p_ssize+UPAGES);
	tsz = p->p_tsize / NPTEPG;
	if (szpt == p->p_szpt) {
		swptstat.pteasy++;
		swpt(rdwri, p, 0, tsz,
		    (p->p_szpt - tsz) * NBPG - UPAGES * sizeof (struct pte));
#ifdef	OS_DEBUG
		goto check;
#else
		return;
#endif
	}
	if (szpt < p->p_szpt)
		swptstat.ptshrink++;
	else
		swptstat.ptexpand++;
	ssz = ctopt(utl->u_ossize+UPAGES);
	if (szpt < p->p_szpt && utl->u_odsize && (utl->u_ossize+UPAGES)) {
		/*
		 * Page tables shrinking... see if last text+data and
		 * last stack page must be merged... if so, copy
		 * stack pte's from last stack page to end of last
		 * data page, and decrease size of stack pt to be swapped.
		 */
		tdlast = (p->p_tsize + utl->u_odsize) % (NPTEPG);
		slast = (utl->u_ossize + UPAGES) % (NPTEPG);
		if (tdlast && slast && tdlast + slast <= (NPTEPG)) {
			swptstat.ptpack++;
			tdsz = ctopt(p->p_tsize + utl->u_odsize);
			bcopy((caddr_t)sptopte(p, utl->u_ossize - 1),
			      (caddr_t)&p->p_p0br[tdsz * NPTEPG - slast],
			      (slast * sizeof (struct pte)));
			ssz--;
		}
	}
	if (ssz)
		swpt(rdwri, p, szpt - ssz - tsz, p->p_szpt - ssz, ssz * NBPG);
	if (utl->u_odsize)
		swpt(rdwri, p, 0, tsz,
		    (ctopt(p->p_tsize + utl->u_odsize) - tsz) * NBPG);

#ifdef	OS_DEBUG
check:
	for (i = 0; i < utl->u_odsize; i++) {
		pte = dptopte(p, i);
		if (pte->pg_v || pte->pg_fod == 0 && (pte->pg_pfnum||pte->pg_m))
		{
			printf("pte=%x *pte=%x pid=%d\n",
				       pte, *(long *)pte, p->p_pid);
			panic("swdspt data");
		}
	}
	for (i = 0; i < utl->u_ossize; i++) {
		pte = sptopte(p, i);
		if (pte->pg_v || pte->pg_fod == 0 && (pte->pg_pfnum||pte->pg_m))
		{
			printf("pte=%x *pte=%x pid=%d\n",
				       pte, *(long *)pte, p->p_pid);
			panic("swdspt stack");
		}
	}
#endif
}

swpt(rdwri, p, doff, a, n)
	int rdwri;
	struct proc *p;
	int doff, a, n;
{
	if (n > 0) {
		swap(p, p->p_swaddr + ctod(UPAGES) + ctod(doff),
			(caddr_t)&p->p_p0br[a * NPTEPG], n, rdwri, B_PAGET,
			swapdev, 0);
	}
}
