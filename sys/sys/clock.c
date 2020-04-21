/*
 * $Source: /d2/3.7/src/sys/sys/RCS/clock.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:06 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/callout.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/text.h"
#include "../h/dk.h"
#include "../bsd/time.h"	/* for adjtime */
#include "machine/psr.h"
#include "machine/reg.h"
#include "machine/cpureg.h"
#include "machine/frame.h"

time_t	time, lbolt;
char	wantsoftclock;
char	noproc;
short	roundticks;

long time_usec = 0;		/* microseconds since the previous tick */
int time_ticks = 0;		/* ticks since the previous second */
int timedelta = 0;		/* number of times clock should be slewed */
static long tickdelta = 0;	/* slew until the clock has been adjusted */
static char adjtime_used = 0;	/* =1 when adjtime(2) used */

/*
 * hardclock:
 *	- called directly from the clock interrupt
 */
hardclock(sr, pc)
	long sr, pc;
{
	register struct callout *p1;
	register struct proc *p;
	register struct user *up;
	register short i, needsoftclock;
	register short cpstate;
	extern short panicing;

	if (panicing)
		return;

	up = &u;
	p = up->u_procp;

/* Adjust our clock.  This mechanism adds or subtracts a small fudge factor
 *	to the normal HZ clock to gain or loose time.  This is used by the
 *	time daemon to adjust the current time smoothly, keeping time 
 *	monotonically increasing.
 */
	if (0 != timedelta) {			/* advance microsec. clock */
		--timedelta;
		time_usec += tickdelta;
	}
	if (time_usec < 0) {			/* skip ticks to slow down */
		time_usec += 1000000/HZ;
	} else {
		if (time_usec >= 1000000/HZ) {
			time_usec -= 1000000/HZ;
			++lbolt;		/* be willing to speed up */
			++time_ticks;
		}
		++lbolt;
		if (++time_ticks >= HZ) {
			time_ticks -= HZ;
			++time;			/* advance second counter */
		}
	}


	/* update real-time timeout queue */
	p1 = calltodo.c_next;
	needsoftclock = (p1 && --p1->c_time <= 0);

	/*
	 * Charge cpu time to system/current process depending on what mode
	 * we are in.  Assume whatever is running now has been running for
	 * the entire tick (GROSS assumption).
	 */
	if (USERMODE(sr)) {
		up->u_utime++;
		if (up->u_prof.pr_scale)
			addupc((unsigned)pc, &up->u_prof, 1);
		if (p->p_nice > NZERO)
			cpstate = CP_NICE;
		else
			cpstate = CP_USER;
	} else {
		cpstate = CP_SYS;
		if (!noproc)
			up->u_stime++;
		if (noproc && BASEPRI(sr))
			cpstate = CP_IDLE;
	}

	/*
	 * Adjust current processes priority by giving it another clock
	 * tick's worth of execution time (whether it used it or not!)
	 */
	if (!noproc) {
		if (p->p_cpu < 80)
			p->p_cpu++;
		p->p_cpticks++;
	}

	/* kick off scheduler */
	if (--roundticks <= 0)
		runrun++;

	/*
	 * We maintain statistics shown by user-level statistics
	 * programs:  the amount of time in each cpu state, and
	 * the amount of time each of DK_NDRIVE ``drives'' is busy.
	 */
	cp_time[cpstate]++;
	for (i = 0, cpstate = 1; i < DK_NDRIVE; i++, cpstate <<= 1)
		if (dk_busy & cpstate)
			dk_time[i]++;

	/* kick off softclock, if not running already */
	if (needsoftclock && (wantsoftclock == 0))
		wantsoftclock = 1;
}

/*
 * softclock:
 *	- call timeout routines, at low priority
 */
softclock()
{
	register struct callout *p1;
	register caddr_t arg;
	register int (*func)();
	register int s;
	register struct callout *p2;

	for (;;) {
		s = spl7();
		if (((p1 = calltodo.c_next) == NULL) || p1->c_time > 0) {
			splx(s);
			break;
		}
		arg = p1->c_arg;
		func = p1->c_func;
		p2 = p1->c_next;	/* advance to next item */
		calltodo.c_next = p2;
		if (p2)			/* & carry forward overflow */
			p2->c_time += p1->c_time;
		p1->c_next = callfree;	/* link into freelist */
		callfree = p1;
		splx(s);
		(*func)(arg);
	}
}

/*
 * We wish to avoid doing floating point math here.  So, we approximate
 * the ccpu constant below with an integer fraction: 2360/2481 is accurate
 * to six digits (0.951229)
 *
 *	ccpu = 0.95122942450071400909 --> exp(-1/20)
 *
 * This is the original formula, using floating point math:
 *	p->p_pctcpu = ccpu * p->p_pctcpu +
 *	    	(1.0 - ccpu) * (p->p_cpticks/(float)HZ);
 *
 * We maintain p_pctcpu multiplied by 2^14 (16384) so as to preserve 4
 * digits of fixed point precision during the calculations.  The monitoring
 * tools know this, and divide by 2^14 before displaying their data.
 */
#define	MULT_LOG2	14
#define	MULT		(1<<MULT_LOG2)

/*
 * schedcpu:
 *	- called once a second to recompute process priorities
 *	- does NOT force rescheduling of processor, rather it just
 *	  computes values which will be used by swtch()
 */
schedcpu()
{
	register struct proc *p, *endproc;
	register unsigned long ul;
	extern char sched_sleeping, sched_bdsync;

	endproc = procNPROC;
	for (p = &proc[0]; p < endproc; p++) {
		if (p->p_stat && (p->p_stat != SZOMB)) {
			if (p->p_time != 127)
				p->p_time++;
			if (p->p_stat==SSLEEP || p->p_stat==SSTOP)
				if (p->p_slptime != 127)
					p->p_slptime++;
			/*
			 * Compute percentage of cpu used by the process
			 */
			if (p->p_flag & SLOAD) {
				/*
				 * First half of old formula:
				 *	ccpu * p_pctcpu
				 */
				ul = (2360 * (u_long)p->p_pctcpu) / 2481;
				/*
				 * Second half of old formula:
				 *	(1 - ccpu) * p_cpticks / HZ
				 */
				ul += (((p->p_cpticks << MULT_LOG2) -
					(((2360 * MULT) * p->p_cpticks)
					 / 2481))
				       / HZ);
				p->p_pctcpu = ul;
			}
			p->p_cpticks = 0;
			/*
			 * Figure out the next average resident set size.  If
			 * the process is growing in resident set size, then
			 * increase its average rapidly.  If it is shrinking
			 * in size (likeley that is being paged down in size)
			 * then slowing decrease the average.
			 */
			if (p->p_avgrss > p->p_rssize)
			    p->p_avgrss = (3 * p->p_avgrss + p->p_rssize) >> 2;
			else
			    p->p_avgrss = (p->p_avgrss + 3 * p->p_rssize) >> 2;

			if (p->p_clktim > 0)
				if (--p->p_clktim == 0)
					psignal(p, SIGALRM);
			p->p_cpu >>= 1;
			if (p->p_pri >= (PUSER - NZERO)) {
				p->p_pri = (p->p_cpu>>1) + p->p_nice +
					(PUSER - NZERO);
			}
		}
	}

#ifdef	HAVERTC
	{
		/* the initial value allows a pause long enough
		** so main() get the initial value for time from
		** the superblock, since this routine is started from 
		** main() before time is initialized.
		 *
		 * Every so often we update unix's time from the hardware
		 * real time clock.  If unix drifts by at most 1 minute/day,
		 * (it is actually about .1 sec/day), we need check every
		 * 2880 seconds, or 40 minutes.
		 */
#define RTC_CHECK (60*10)			/* 10 minutes in seconds*/
		static short rtcticks = RTC_CHECK;

		if (--rtcticks <= 0) {
			rtcticks = RTC_CHECK;
			/* if adjtime(2) has not been used lately, ... */
			if (!adjtime_used) {
				todclkset();
			} else if (timedelta == 0 && time_ticks == 0) {
				/* reset chip after delta is finished */
				todsettim(time);
				adjtime_used = 0;
			} else {
				rtcticks = 0;
			}
		}
	}
#endif	HAVERTC
	vmmeter();

	if (sched_sleeping) {
		sched_sleeping = 0;
		sched_bdsync = 1;
		wakeup(&sched_sleeping);
	}
	runrun++;

	/* work at the start of the second, for the sake of the tod chip */
	timeout(schedcpu, (caddr_t)0, HZ-time_ticks);
}

/*
 * timeout is called to arrange that fun(arg) is called in tim/HZ seconds.
 * An entry is sorted into the callout structure.
 * The time in each structure entry is the number of HZ's more
 * than the previous entry. In this way, decrementing the
 * first entry has the effect of updating all entries.
 *
 * The panic is there because there is nothing
 * intelligent to be done if an entry won't fit.
 *
 * This now returns an ID as the AT&T 5.3 function does.
 */
int
timeout(fun, arg, t)
	int (*fun)();
	caddr_t arg;
	register int t;
{
	register struct callout *p1, *p2, *pnew;
	register int s;
	static int id = 0;
	register int id_copy;

	s = spl7();
	if (t == 0)
		t = 1;
	if ((pnew = callfree) == NULL)
		panic("timeout table overflow");
	callfree = pnew->c_next;
	pnew->c_arg = arg;
	pnew->c_func = fun;

    /* insert pnew into correct position in callout list */
	for (p1 = &calltodo; (p2 = p1->c_next) && (p2->c_time < t); p1 = p2)
		if (p2->c_time > 0)
			t -= p2->c_time;
	p1->c_next = pnew;
	pnew->c_next = p2;
	pnew->c_time = t;
	pnew->c_id = id_copy = ++id;
	if (p2)
		p2->c_time -= t;
	splx(s);

	return id_copy;
}

/* remove a function timeout call from the callout structure.
 *	This form is used by the old (3.4/2.4 & before) version of tcp.
 */
untimeout(fun, arg)
	register int (*fun)();
	register caddr_t arg;
{
	register struct callout *p1, *p2, *p3;
	register int s;

	s = spl7();
	for (p1 = &calltodo; (p2 = p1->c_next) != 0; p1 = p2) {
		if (p2->c_func == fun && p2->c_arg == arg) {
			if (p3 = p2->c_next)	/* carry overflow or delta */
				p3->c_time += p2->c_time;
			p1->c_next = p3;
			p2->c_next = callfree;
			callfree = p2;
			break;
		}
	}
	splx(s);
}

/* remove a timeout call from the callout structure
 *	This form is similar to AT&T 5.3
 */
untimeout_id(id)
register int id;
{
	register struct callout *p1, *p2, *p3;
	register int s;

	s = spl7();
	for (p1 = &calltodo; (p2 = p1->c_next) != 0; p1 = p2) {
		if (p2->c_id == id) {
			if (p3 = p2->c_next)	/* carry overflow or delta */
				p3->c_time += p2->c_time;
			p1->c_next = p3;
			p2->c_next = callfree;
			callfree = p2;
			break;
		}
	}
	splx(s);
}

#define	PDELAY	(PZERO-1)
delay(ticks)
{
	extern wakeup();
	register int s;

	if (ticks<=0)
		return;
	s = spl7();
	timeout(wakeup, (caddr_t)u.u_procp+1, ticks);
	(void) sleep((caddr_t)u.u_procp+1, PDELAY);
	splx(s);
}


/* BSD adjtime system call 
 */
#define TICKADJ (240000/(60*HZ))	/* nominal skew of 240ms/minute */
#define BIGADJ 1000000			/* use 10x skew above 1 second */

adjtime()
{
	register struct a {
		struct timeval *delta;
		struct timeval *olddelta;
	} *uap = (struct a *)u.u_ap;
	struct timeval atv, oatv;
	register long odelta;
	long adjtime_sub();

	if (!suser()) 
		return;
	if (copyin((caddr_t)uap->delta, (caddr_t)&atv, sizeof(atv))) {
		u.u_error = EFAULT;
		return;
	}

	adjtime_used = 1;		/* remember to fix tod chip */

	odelta = adjtime_sub(atv.tv_sec*1000000 + atv.tv_usec);
	if (uap->olddelta) {
		oatv.tv_sec = odelta / 1000000;
		oatv.tv_usec = odelta % 1000000;
		if (copyout((caddr_t)&oatv, (caddr_t)uap->olddelta,
			    sizeof (struct timeval))) {
			u.u_error = EFAULT;
			return;
		}
	}
}

/* start slewing the clock by some microseconds
 *	this should also be used by the TOD chip.
 */
long
adjtime_sub(ntimedelta)
register long ntimedelta;
{
	register long ntickdelta, odelta;
	register int s;

	if (ntimedelta < 0) {
		ntimedelta = -ntimedelta;
		ntickdelta = -TICKADJ;
	} else {
		ntickdelta = TICKADJ;
	}
	ntimedelta /= TICKADJ;
	if (ntimedelta > BIGADJ/TICKADJ) {
		ntimedelta /= 10;
		ntickdelta *= 10;
	}

	s = spl7();
	odelta = timedelta * tickdelta;
	tickdelta = ntickdelta;
	timedelta = ntimedelta;
	splx(s);

	return odelta;
}

/*
 * System call handler to get current time in the BSD style.
 */
bsdgettime()
{
	register struct a {
		struct timeval *time;
	} *uap = (struct a *)u.u_ap;
	struct timeval tv;

	microtime(&tv);
	if (copyout((caddr_t)&tv, (caddr_t)uap->time,
		    sizeof (struct timeval))) {
		u.u_error = EFAULT;
	}
}

/*
 * Kernel utility function to get current time in the BSD style.
 */
microtime(tv)
	register struct timeval *tv;
{
	register int s = spl7();

	tv->tv_sec = time;
	tv->tv_usec = time_ticks * (1000000/HZ) + time_usec;
	splx(s);
}
