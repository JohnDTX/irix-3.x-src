/*
 * $Source: /d2/3.7/src/sys/vm/RCS/vm_sched.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:36:04 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/seg.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/text.h"
#include "../vm/vm.h"
#include "../h/map.h"
#include "../h/conf.h"
#include "../h/cmap.h"

/* insure non-zero */
#define	nz(x)	(x != 0 ? x : 1)

/*
 * The following parameters control operation of the page replacement
 * algorithm.  They are initialized to 0, and then computed at boot time
 * based on the size of the system.  If they are patched non-zero in
 * a loaded vmunix they are left alone and may thus be changed per system
 * using adb on the loaded system.
 */
int	minfree;
int	desfree;
int	lotsfree;
int	klin = KLIN;
int	klseql = KLSEQL;
int	klsdist = KLSDIST;
int	kltxt = KLTXT;
int	klout = KLOUT;
int	samplespersec = 400;
#define	RATETOSCHEDPAGING	4

/*
 * Setup the paging constants for the clock algorithm.
 * Called after the system is initialized and the amount of memory
 * and number of paging devices is known.
 *
 * Threshold constants are defined in ../machine/vmparam.h.
 */
int	slowscan = 10;
int	fastscan;
setupclock()
{

	/*
	 * Setup thresholds for paging:
	 *	lotsfree	is threshold where paging daemon turns on
	 *	desfree		is amount of memory desired free.  if less
	 *			than this for extended period, do swapping
	 *	minfree		is minimal amount of free memory which is
	 *			tolerable.
	 */
	if (lotsfree == 0) {
		lotsfree = LOTSFREE / NBPG;
		if (lotsfree > LOOPPAGES / LOTSFREEFRACT)
			lotsfree = LOOPPAGES / LOTSFREEFRACT;
		if (lotsfree == 0)
			lotsfree = 1;
	}
	if (desfree == 0) {
		desfree = DESFREE / NBPG;
		if (desfree > LOOPPAGES / DESFREEFRACT)
			desfree = LOOPPAGES / DESFREEFRACT;
	}
	if (minfree == 0) {
		minfree = MINFREE / NBPG;
		if (minfree > desfree / MINFREEFRACT)
			minfree = desfree / MINFREEFRACT;
	}
	/*
	 * Strategy of 2/9/86:
	 *
	 * Fastscan is the number of seconds it takes to scan the entire
	 * loop, when no pages are available.
	 * A minimum of 2 seconds is allowed.
	 * Slowscan is the number of seconds it takes to scan the entire
	 * loop, when lotsfree pages are available.  We force this to
	 * be 10 seconds, because its probably a good rate given low
	 * demands on memory.  If the amount of memory avaialbe is quite
	 * large, then it is possible that fastscan could be longer than
	 * slowscan, so we insure that slowscan is at least twice fastscan.
	 */
	fastscan = (LOOPPAGES) / samplespersec;
	if (fastscan < 2)
		fastscan = 2;
	if (fastscan >= (slowscan / 2))
		slowscan = fastscan * 2;
	/*
	 * Figure out maximum number of pushes per pageout() invocation.
	 * We use a rough 2/3 of the DISKRPM and then divide by the
	 * number of times the pager runs per second.
	 */
	vm_maxpushes = ((DISKRPM * 2) / 3) / RATETOSCHEDPAGING;
#ifdef	notdef
printf("maxfree=%d minfree=%d desfree=%d lotsfree=%d slowscan=%d fastscan=%d\n",
		   maxfree, minfree, desfree, lotsfree, slowscan, fastscan);
#endif	notdef
}

/*
 * Schedule rate for paging.
 * Rate is linear interpolation between
 * slowscan with lotsfree and fastscan when out of memory.
 */
schedpaging()
{
	register int vavail, scanrate;

	nscan = 0;
	desscan = 0;
	vavail = freemem - deficit;
	if (vavail < 0)
		vavail = 0;
	if (freemem < lotsfree) {
		scanrate =
			(slowscan * vavail + fastscan * (lotsfree - vavail)) /
				lotsfree;
		desscan = (LOOPPAGES / nz(scanrate)) / RATETOSCHEDPAGING;
		wakeup((caddr_t)&proc[2]);
	}
	timeout(schedpaging, (caddr_t)0, (short) hz / RATETOSCHEDPAGING);
}

/*
 * Scheduler process.  The scheduler process on the IRIS is responsible for
 * 3 things.
 *
 *	(1) syncing the various caches (buffer, inode, superblock, etc).
 *	(2) swapping processes out when memory gets really tight
 *	(3) swapping processes in that need to run
 *
 * The only real purpose to swapping processes out is to get rid of the per
 * process data that can be paged that the pager couldn't page out on its
 * own.  Thus we let the pager scrape the processes address space clean
 * on its own, and only toss out processes that have no memory.  Because
 * the net gain on this swapping is very small, we only do it when the
 * process has not run in a while.  Anything that hasn't run recently won't
 * free up memory.
 *
 * To swap a process in, all we need to do is bring in its page tables and
 * udot from disk.  The process can demand fill the rest.
 */
char	sched_sleeping;
char	sched_bdsync;
struct	proc *lastProc;

#define	swappable(p) \
 (((p)->p_flag&(SSYS | SLOCK | SULOCK | SLOAD | SPAGE | SKEEP | \
		     SWEXIT | SPHYSIO | SGR | STLOCK | SDLOCK))==SLOAD)

/* maximum number of p_slptime ticks that clock will give to a process */
#define	SLPTIME_PEGGED	127

sched()
{
	register struct proc *p;
	register struct proc *inProc, *outProc;
	register int inPriority, outPriority;
	int minToSwap;

	/*
	 * Start kicking processes out when the pager is letting us get
	 * ``too'' close to desfree.  minToSwap defines this threshold.
	 */
	minToSwap = desfree + ((lotsfree - desfree) / 2);

	lastProc = procNPROC;
	for (;;) {
		spl6();
		sched_sleeping = 1;
		sleep(&sched_sleeping, PSWP);
		sched_sleeping = 0;
		spl0();

		/*
		 * Sync things up
		 */
again:
		if (sched_bdsync) {
			sched_bdsync = 0;
			bdsync();
		}

		/*
		 * Look for processes to get rid of, or bring in.
		 */
		outProc = NULL;
		outPriority = -5000;
		inProc = NULL;
		inPriority = 5000;
		for (p = proc; p < lastProc; p++) {
			switch (p->p_stat) {
			  case SRUN:
				/*
				 * Pick a process to swap in.  If the process
				 * is not in memory, has no data/stack pages
				 * being pushed, has no text pages being
				 * pushed, and it has the best priority seen
				 * so far, make it a candidate for swapping in.
				 */
				if (((p->p_flag & SLOAD) == 0) &&
				    (p->p_poip == 0) &&
				    ((p->p_textp == NULL) ||
				     (p->p_textp->x_poip == 0)) &&
				    (p->p_pri < inPriority)) {
					inPriority = p->p_pri;
					inProc = p;
				}
				break;
			  case SSLEEP:
			  case SSTOP:
				/*
				 * Pick a process to swap out.  If the process
				 * has no data+stack pages, has been sleeping
				 * for a good while, is swappable, and has the
				 * worst priority seen so far, make it a
				 * candidate for swapping out.
				 */
				if ((p->p_rssize == 0) &&
				    (p->p_slptime >= MAXSLP) && swappable(p) &&
				    (p->p_pri > outPriority)) {
					outPriority = p->p_pri;
					outProc = p;
				}
				break;
			}
		}

		/*
		 * Bring a process in, if we can fit its udot and page table
		 * in memory, and we can fit its likely usage of memory
		 * in as well.  We wait to bring a process in until we can
		 * provide enough memory for its average working set.  Note
		 * that its average working set will decay towards zero as
		 * the process lingers on swap, thus we can be certain that
		 * the process will get in at some point.
		 */
		if ((inProc != NULL) &&
		    (freemem > UPAGES + inProc->p_szpt + inProc->p_avgrss) &&
		    swapin(inProc)) {
			/*
			 * We were able to swap a process in.  See if there
			 * are more processes ready to come in.  We have
			 * to start our process table scan over again, because
			 * we may have slept during the swapin.
			 */
			goto again;
		}
		if ((outProc != NULL) &&
		    ((freemem < minToSwap) || kmap_wanted ||
		     (outProc->p_slptime == SLPTIME_PEGGED))) {
			outProc->p_flag &= ~SLOAD;
			if (swapout(outProc, outProc->p_dsize,
					     outProc->p_ssize)) {
				/*
				 * We were able to swap a process out.  Look
				 * for more processes to fiddle with, in case
				 * memory is still tight. As with a swapin, we
				 * have to restart our process table scan
				 * because we may have slept.
				 */
				goto again;
			}
		}
	}
}

vmmeter()
{
	register unsigned *cp, *rp, *sp;

	deficit -= MIN(deficit,
	    MAX(deficit / 10, ((KLIN) / 2) * DISKRPM / 2));
	ave(avefree, freemem, 5);
	ave(avefree30, freemem, 30);
	/* v_pgin is maintained by clock.c */
	cp = &cnt.v_first; rp = &rate.v_first; sp = &sum.v_first;
	while (cp <= &cnt.v_last) {
		ave(*rp, *cp, 5);
		rp++;
		*sp += *cp;
		sp++;
		*cp++ = 0;
	}
	if (time % 5 == 0) {
		vmtotal();
		rate.v_swpin = cnt.v_swpin;
		sum.v_swpin += cnt.v_swpin;
		cnt.v_swpin = 0;
		rate.v_swpout = cnt.v_swpout;
		sum.v_swpout += cnt.v_swpout;
		cnt.v_swpout = 0;
	}
}

/*
 * Compute a tenex style load average of a quantity on
 * 1, 5 and 15 minute intervals.
 * (Using 'fixed-point' with 3 decimal digits to right)
 */
long	avenrun[3];

/*
 * Constants for averages over 1, 5, and 15 minutes
 * when sampling at 5 second intervals.
 * (Using 'fixed-point' with 3 decimal digits to right)
 */
#define	CEXP_0	957		/* (int)( exp(-1/15) * 1024) */
#define	CEXP_1	1010		/* (int)( exp(-1/75) * 1024) */
#define	CEXP_2	1019		/* (int)( exp(-1/225) * 1024) */

vmtotal()
{
	register struct vmtotal *tp;
	register struct proc *p;
	register struct text *xp;
	register long *avg = avenrun;
	register int nrun = 0;

	tp = &total;
	tp->t_vmtxt = 0;
	tp->t_avmtxt = 0;
	tp->t_rmtxt = 0;
	tp->t_armtxt = 0;
	for (xp = text; xp < textNTEXT; xp++) {
		if (xp->x_iptr && !(xp->x_flag & XSAVE)) {
			tp->t_vmtxt += xp->x_size;
			tp->t_rmtxt += xp->x_rssize;
			for (p = xp->x_caddr; p; p = p->p_xlink)
			switch (p->p_stat) {

			case SSTOP:
			case SSLEEP:
				if (p->p_slptime >= MAXSLP)
					continue;
				/* fall into... */

			case SRUN:
			case SIDL:
				tp->t_avmtxt += xp->x_size;
				tp->t_armtxt += xp->x_rssize;
				goto next;
			}
next:
			;
		}
	}
	tp->t_vm = 0;
	tp->t_avm = 0;
	tp->t_rm = 0;
	tp->t_arm = 0;
	tp->t_rq = 0;
	tp->t_dw = 0;
	tp->t_pw = 0;
	tp->t_sl = 0;
	tp->t_sw = 0;
	for (p = proc + 1; p < procNPROC; p++) {
		if (p->p_stat) {
			if (p->p_flag & SSYS)
				continue;
			tp->t_vm += p->p_dsize + p->p_ssize;
			tp->t_rm += p->p_rssize;
			switch (p->p_stat) {

			case SSLEEP:
			case SSTOP:
				if (p->p_pri <= PZERO)
					nrun++;
				if (p->p_flag & SPAGE)
					tp->t_pw++;
				else if (p->p_flag & SLOAD) {
					if (p->p_pri <= PZERO)
						tp->t_dw++;
					else if (p->p_slptime < MAXSLP)
						tp->t_sl++;
				} else if (p->p_slptime < MAXSLP)
					tp->t_sw++;
				if (p->p_slptime < MAXSLP)
					goto active;
				break;

			case SRUN:
			case SIDL:
				nrun++;
				if (p->p_flag & SLOAD)
					tp->t_rq++;
				else
					tp->t_sw++;
active:
				tp->t_avm += p->p_dsize + p->p_ssize;
				tp->t_arm += p->p_rssize;
				break;
			}
		}
	}
	tp->t_vm += tp->t_vmtxt;
	tp->t_avm += tp->t_avmtxt;
	tp->t_rm += tp->t_rmtxt;
	tp->t_arm += tp->t_armtxt;
	tp->t_free = avefree;

	avg[0] = (CEXP_0 * avg[0] +
	    (((short)nrun * (short)(1024 - CEXP_0)) << 10)) >> 10;
	avg[1] = (CEXP_1 * avg[1] +
	    (((short)nrun * (short)(1024 - CEXP_1)) << 10)) >> 10;
	avg[2] = (CEXP_2 * avg[2] +
	    (((short)nrun * (short)(1024 - CEXP_2)) << 10)) >> 10;
}
