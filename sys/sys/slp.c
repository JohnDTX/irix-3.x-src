/*
 * $Source: /d2/3.7/src/sys/sys/RCS/slp.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:32 $
 */
#include "../h/param.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/text.h"
#include "../h/systm.h"
#include "../h/file.h"
#include "../h/inode.h"
#include "../vm/vm.h"
#include "../h/sysinfo.h"
#include "machine/pte.h"

short	slice_size = 2;
extern	char sched_sleeping;

#define	DEBUG

#define	NHSQUE	64	/* must be power of 2 */
#define	sqhash(X)	(&hsque[(short) (((int)(X) >> 3) & (NHSQUE-1))])
struct	proc *hsque[NHSQUE];

char	runrun, curpri;
struct	proc *runq;				/* run queue */

/* put process p on the run queue */
#define	SETRQ(p)	(p)->p_link = runq; runq = (p)
	
/*
 * Give up the processor till a wakeup occurs
 * on chan, at which time the process
 * enters the scheduling queue at priority pri.
 * The most important effect of pri is that when
 * pri<=PZERO a signal cannot disturb the sleep;
 * if pri>PZERO signals will be processed.
 * Callers of this routine must be prepared for
 * premature return, and check that the reason for
 * sleeping has gone away.
 */
#define	TZERO	10
sleep(chan, disp)
caddr_t chan;
{
	register struct proc *rp = u.u_procp;
	register struct proc **q = sqhash(chan);
	register s;
	extern short panicing;

	s = spl7();
	if (panicing) {
		splx(s);
		return(0);
	}

	ASSERT(chan);
	rp->p_stat = SSLEEP;
	rp->p_wchan = chan;
	rp->p_link = *q;
	rp->p_slptime = 0;
	*q = rp;
	if (rp->p_time > TZERO)
		rp->p_time = TZERO;
	if ((rp->p_pri = (disp&PMASK)) > PZERO) {
		if (rp->p_sig && issig()) {
			rp->p_wchan = 0;
			rp->p_stat = SRUN;
			*q = rp->p_link;
			spl0();
			goto psig;
		}
		spl0();
		swtch();
		if (rp->p_sig && issig())
			goto psig;
	} else {
		spl0();
		swtch();
	}
	splx(s);
	return(0);

	/*
	 * If priority was low (>PZERO) and there has been a signal,
	 * if PCATCH is set, return 1, else
	 * execute non-local goto to the qsave location.
	 */
psig:
	splx(s);
	if (disp&PCATCH)
		return(1);
	resume(u.u_procp->p_addr->pg_pfnum, u.u_qsave);
	/* NOTREACHED */
}

/*
 * Wake up all processes sleeping on chan.
 */
wakeup(chan)
	register caddr_t chan;
{
	register struct proc *p;
	register struct proc **q;
	register int s;
	char wakeupSched;

	s = spl7();
	wakeupSched = 0;
	for (q = sqhash(chan); p = *q; ) {
		if (p->p_wchan==chan && p->p_stat==SSLEEP) {
			p->p_stat = SRUN;
			p->p_wchan = 0;
			p->p_slptime = 0;
			/* take off sleep queue, put on run queue */
			*q = p->p_link;
			SETRQ(p);
			if (!(p->p_flag&SLOAD)) {
				p->p_time = 0;
				wakeupSched = 1;
			} else
			if (p->p_pri < curpri)
				runrun++;
		} else
			q = &p->p_link;
	}
	if (wakeupSched && sched_sleeping) {
		sched_sleeping = 0;
		wakeup(&sched_sleeping);
	}
	splx(s);
}

/*
 * Put proc p on priority adjusted queue
 */
setrq(p)
	struct proc *p;
{
	register int s;

	s = spl7();
	SETRQ(p);
	splx(s);
}

/*
 * Set the process running;
 * arrange for it to be swapped in if necessary.
 */
setrun(p)
	register struct proc *p;
{
	register struct proc **q;
	register int s;

	s = spl7();
	if (p->p_stat == SSLEEP) {
		/* take off sleep queue */
		for (q = sqhash(p->p_wchan); *q != p; q = &(*q)->p_link) ;
		*q = p->p_link;
		p->p_wchan = 0;
	} else
	if (p->p_stat == SRUN) {
		/* already on run queue - just return */
		/* XXX what happens here */
		splx(s);
		return;
	}
	/* put on run queue */
	p->p_stat = SRUN;
	SETRQ(p);
	if (!(p->p_flag&SLOAD)) {
		p->p_time = 0;
		if (sched_sleeping) {
			sched_sleeping = 0;
			wakeup(&sched_sleeping);
		}
	} else
	if (p->p_pri < curpri)
		runrun++;
	splx(s);
}

/*
 * This routine is called to reschedule the CPU.
 * if the calling process is not in RUN state,
 * arrangements for it to restart must have
 * been made elsewhere, usually by calling via sleep.
 * There is a race here. A process may become
 * ready after it has been examined.
 * In this case, idle() will be called and
 * will return in at most 1HZ time.
 * i.e. its not worth putting an spl() in.
 */

/* XXX these should be somewhere else */
char	noproc;
#ifdef	PROF
idle()
{
	IDLE();
}
#endif

#ifndef	GL2
#define	gr_repaint()
#define	gr_ok(x)	1
#endif

swtch()
{
	register short bestpri, n;
	register struct proc *p, *q, *pp, *pq;
	extern short roundticks, slice_size;

	noproc = 1;			/* nobody here but us kernels */
	sysinfo.pswitch++;
	(void) spl0();
	gr_repaint();

loop:
	/* search for highest-priority runnable process */
	(void) spl6();
	if (p = runq) {
		q = NULL;
		pp = NULL;
		bestpri = n = 128;
		do {
			if (p->p_flag&SLOAD) {
				if (p->p_pri < bestpri)
					bestpri = p->p_pri;
				if ((p->p_pri <= n) &&
				    (!(p->p_flag&SGR) ||
				      gr_ok(p->p_grhandle))) {
					n = p->p_pri;
					pp = p;
					pq = q;
				}
			}
			q = p;
		} while (p = p->p_link);
	} else
		goto cont;

	/* if no process is runnable, idle */
	if (pp == NULL) {
cont:
		curpri = PIDLE;
#ifdef	PROF
		idle();
#else
		IDLE();
#endif
		goto loop;
	}

	/* remove p from run queue */
	p = pp;
	q = pq;
	if (q == NULL)
		runq = p->p_link;
	else
		q->p_link = p->p_link;
	(void) spl0();
	cnt.v_swtch++;
	noproc = 0;
#ifdef	GL2
	/*
	 * If the process we chose to run doesn't have the best priority
	 * then we must have picked the process which had the pipe.  Set
	 * the pipe interrupt to force it to give up the pipe.  We will
	 * be called once this happens, and then choose the correct process
	 * to run.
	 */
	if (p->p_flag & SGR) {
		if (bestpri != n)
			fbc_setpipeint();		/* force interrupt */
		gr_switchstate(p->p_grhandle);
	}
#endif

	/* initialize time slice */
	roundticks = slice_size;
	runrun = 0;

	/* we have a process to run; if its us, just return */
	if (p == u.u_procp) {
		ASSERT(p->p_stat != SZOMB);
		if (p->p_flag & SSWAP)
			goto resume_it;
		curpri = p->p_pri;
		return;
	}

	if (u.u_procp->p_stat == SZOMB) {
		/*
		 * The process we are switching **from** is a dead process,
		 * that just got here from exit() (sys1.c).  We can now
		 * safely free its page table and udot.  Free the page
		 * table first, since p_addr points into it.  Also make
		 * sure we run at high priority until the resume completes.
		 */
		(void) spl6();
		vrelu(u.u_procp, 0);
		vrelpt(u.u_procp);
	} else {
		/*
		 * We are going to resume some other process.  Save this
		 * processes state, for later resuming.
		 */
		if (save(u.u_rsave))			/* for resume... */
			return;

		/* save floating point state */
		if (u.u_pcb.pcb_fpinuse && u.u_pcb.pcb_fpsaved==0) {
			fpsave();
			u.u_pcb.pcb_fpsaved = 1;
		}
	}

	/* if SSWAP is set, resume to u_ssave, otherwise resume to u_rsave */
resume_it:
	n = p->p_flag&SSWAP;
	p->p_flag &= ~SSWAP;
	curpri = p->p_pri;
	resume(p->p_addr->pg_pfnum, n ? u.u_ssave : u.u_rsave);
}
