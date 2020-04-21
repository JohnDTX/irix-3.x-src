/*
 * Silicon Graphics graphics system call support.
 * This is implemented as a driver, to provide a select facility.
 *
 * $Source: /d2/3.7/src/sys/gl2/RCS/sgigsc.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:28:47 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/buf.h"
#include "../h/file.h"
#include "../h/inode.h"
#include "../streams/stream.h"
#include "../h/conf.h"
#include "../h/printf.h"
#include "../h/sgigsc.h"
#include "../vm/vm.h"
#include "machine/pte.h"

char	sg_lock;		/* graphics lock variable for sleeping */
char	sg_mousetied;		/* flag indicating mouse/cursor binding */
char	consoleOnPTY;		/* non-zero if console using pty */
extern	queue_t *consoleWQ;	/* console stream write queue */

/* forward references */
void	sgunqmem(), sgqmem();

extern	struct pte *iolock();

void
sgigsc()
{
	register struct a {
		int	cmd;
		int	argp;
	} *uap = (struct a *) u.u_ap;
	struct sgigsc_qmem qm;

	/*
	 * Handle commands that can be done by any process.
	 */
	switch (uap->cmd) {
	  case SGWM_WMRUNNING:
		u.u_rval1 = (sgstate.flags & WMRUNNING) != 0;
		return;
	  case SGWM_MEWM:
		if (sgstate.flags & WMRUNNING)
			u.u_error = EBUSY;
		else {
			sgstate.flags |= WMRUNNING;
			sgstate.wman = u.u_procp;
		}
		return;
	  case SGWM_LOSTGR:
		if (!(u.u_procp->p_flag & SGR))
			u.u_rval1 = 1;
		return;
	  default:
		break;
	}

	/*
	 * Rest of operations can only be done by the window manager.
	 * If this process is not the window manager, punt right away.
	 */
	if (sgstate.wman != u.u_procp) {
		u.u_error = EPERM;
		return;
	}

	switch (uap->cmd) {
	  case SGWM_LOCK:
		sgstate.flags |= LOCKED;
		break;
	  case SGWM_UNLOCK:
		sgstate.flags &= ~LOCKED;
		wakeup(&sg_lock);
		break;
	  case SGWM_QMEM:
		if (copyin(uap->argp, (caddr_t) &qm, sizeof(qm)))
			u.u_error = EFAULT;
		else {
			sgunqmem();
			sgqmem(&qm);
		}
		break;
	  case SGWM_UNQMEM:
		sgunqmem();
		break;
	  case SGWM_TIEMOUSE:
		sg_mousetied = 1;
		break;
	  case SGWM_UNTIEMOUSE:
		sg_mousetied = 0;
		break;
	  default:
		u.u_error = ENXIO;
		break;
	}
}

/*
 * Cleanup gsc state after window manager exits.
 */
void
sgCleanup()
{
	if (sgstate.wman == u.u_procp) {
		register struct proc *p;

		/*
		 * Kill all active graphics processes.  Issue a wakeup on the
		 * graphics lock, in case we were locked.  This will enable
		 * locked out graphics processes to get the signal.
		 */
		for (p = proc; p < procNPROC; p++) {
			if (p->p_flag & SGR)
				psignal(p, SIGKILL);
		}
		wakeup(&sg_lock);
		sgunqmem();
		sgstate.flags = 0;
		sgstate.wman = NULL;

		/*
		 * Reset console back to where it use to be.
		 * Other half of this (user side) will happen when
		 * the console stream is shut down.  See hack.c
		 */
		setConsole(CONSOLE_NOT_ON_PTY);
		resetConsole();
	}
}

/*
 * Return non-zero if the processes graphics queue is not empty.
 * If this process is the window manager, return non-zero if there
 * is something in the gsc q.  Otherwise, return non-zero if
 * there is something in the gl q.
 */
int
gr_queuenotempty(p)
	struct proc *p;
{
	if (p->p_flag & SGR) {
		if (sgstate.wman == p) {
			if ((sgstate.flags & QMEM) &&
			    (sgstate.q->qin != sgstate.q->qout))
				return 1;
		} else {
			if (!gr_isqempty(p->p_grhandle))
				return 1;
		}
	}
	return 0;
}

/*
 * Map in the users q memory into the kernel virtual memory, locking down
 * the users pages.  Check that the user is referring to previously
 * allocated memory.  Return non-zero on error.
 */
void
sgqmem(qm)
	struct sgigsc_qmem *qm;
{
	register struct pte *kernpte, *userpte;
	register int i;
	long kmx;

	/*
	 * Record the q sizing parameters.
	 */
	sgstate.entries = qm->entries;
	sgstate.uvaddr = qm->base;
	sgstate.qlen = sizeof(struct sgigsc_q) - sizeof(struct sgigsc_qentry) +
			qm->entries * sizeof(struct sgigsc_qentry);

	/*
	 * Now allocate kernel virtual memory space to hold a duplicate
	 * mapping of the users memory.  Insure that all of the users
	 * pages are paged in and locked down.
	 */
	/* impose a limit on the amount of memory to tie down */
	if (sgstate.qlen > 32768) {
		u.u_error = E2BIG;
		return;
	}

	/* lock down (and page in) the pages */
	ASSERT(sgstate.pte == NULL);
	if ((userpte = iolock(sgstate.uvaddr, sgstate.qlen)) == NULL) {
		u.u_error = EINVAL;
		return;
	}

	/* allocate kernel map space to map the pages in */
	sgstate.pages = btoc(((long)sgstate.uvaddr & PGOFSET) + sgstate.qlen);
	kmx = kmap_alloc(sgstate.pages, 1);
	sgstate.q = (struct sgigsc_q *)
		((long)kmxtob(kmx) + ((long)sgstate.uvaddr & PGOFSET));

	/* copy users pte's to Usrptmap */
	sgstate.pte = kernpte = &Usrptmap[kmx];
	for (i = sgstate.pages; --i >= 0; )
		*kernpte++ = *userpte++;

	/* copy sw page table info to hw page table info */
	ptaccess(sgstate.pte, (struct pte *) sgstate.q, sgstate.pages);

	sgstate.flags |= QMEM;
}

/*
 * Release q memory.
 */
void
sgunqmem()
{
	if (!(sgstate.flags & QMEM))
		return;

	/* do this first to disable sgqenter from any more queueing */
	sgstate.flags &= ~QMEM;

	ASSERT(sgstate.pte);
	iounlock(sgstate.pte, sgstate.uvaddr, sgstate.qlen, B_READ);
	sgstate.pte = NULL;
}

/*
 * Attempt to enter something in the q.
 */
void
sgqenter(qe)
	register struct sgigsc_qentry *qe;
{
	register struct sgigsc_q *q;
	register struct sgigsc_qentry *e;
	register int next;
	int s;
	extern int time_ticks;
	extern long time_usec;

	if (!(sgstate.flags & QMEM))		/* if no q */
		return;
	q = sgstate.q;

	/* next is the location where the next qenter will go */
	s = splmax();
	next = q->qin + 1;
	if (next >= sgstate.entries)
		next = 0;
	if (next == q->qout) {			/* if q is empty */
		splx(s);
		return;
	}

	/* enter into q */
	e = &q->q[q->qin];
	e->timeStamp.seconds = time;
	e->timeStamp.microseconds = time_ticks*(1000000/HZ) + time_usec;
	e->event = qe->event;
	e->ev = qe->ev;

	if (q->qin == q->qout)			/* if q was empty */
		selwakeup(sgstate.wman, 0);
	q->qin = next;				/* advance to next qin */
	splx(s);
}

/*
 * Output a character to the console's pseudo-tty.  Also output it to
 * the kernel window device so that if we switch back to the kernel
 * window device, earlier printf's will appear.
 */
ptyPutChar(c)
	int c;
{
	if (consoleOnPTY && consoleWQ) {
		register mblk_t *mp;

		mp = allocb(1, BPRI_HI);
		if (mp) {
			*mp->b_wptr++ = c;
			putnext(consoleWQ, mp);
			if (qready())
				runqueues();
		}
		grputchar(c);
	} else {
		setConsole(CONSOLE_ON_WIN);
		(*con_putchar)(c);
	}
}
