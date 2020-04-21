/*
 * Pseudo-teletype Driver
 * (Actually two drivers, requiring two entries in 'cdevsw')
 *
 * $Source: /d2/3.7/src/sys/sys/RCS/tty_pty.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:44 $
 */

#include "pty.h"

#include "../h/param.h"
#include "../h/config.h"
#include "../h/types.h"
#include "../h/systm.h"
#include "../h/termio.h"
#include "../h/errno.h"
#include "../h/tty.h"
#include "../h/ioctl.h"
#include "../h/signal.h"
#include "../h/user.h"
#include "../h/conf.h"
#include "../h/file.h"
#include "../h/proc.h"
#include "../h/buf.h"
#include "../h/pty_ioctl.h"

#define	MAXPTY	50

/*
 * pts == /dev/tty[pP]?
 * ptc == /dev/ptc[pP]?
 */

/*
 * We allocate one of these structures for every open ptc/tty pair
 */
struct	pty {
	short	pt_state;
	long	pt_ic;
	struct	winsize pt_size;
	struct	tty pt_tty;
};
struct	pty *pty[MAXPTY];

/* slave state bits */
#define	PTS_WAITING	0x0001		/* waiting for controller */
#define	PTS_OPEN	0x0002		/* open and running */
#define	PTS_SENTHUP	0x0004		/* sent slave a hup */

/* controller state bits */
#define	PTC_OPEN	0x0100		/* open and running */
#define	PTC_OSLEEP	0x0200		/* waiting for data for ptcread */
#define	PTC_ISLEEP	0x0400		/* waiting for data for ptcwrite */
#define	PTC_QUEUE	0x0800		/* queue when ptcread will work */

/*
 * ptyalloc:
 *	- allocate pty structure for the given dev, if its not already
 *	  allocated
 *	- return 0 on failure, 1 on success
 */
ptyalloc(dev)
	dev_t dev;
{
	if (pty[dev] == NULL) {
		pty[dev] = (struct pty *)1;
		pty[dev] = (struct pty *) calloc(1, sizeof(struct pty));
		if (pty[dev] == NULL)
			return (0);
		wakeup((caddr_t)&pty[dev]);
	} else {
		while (pty[dev] == (struct pty *)1)
			(void) sleep((caddr_t)&pty[dev], TTIPRI);
	}
	return (1);
}

/*
 * ptsopen:
 *	- slave side of pty open
 *	- wait until carrier is on, which signifies that the controller
 *	  sid of the pty has been opened
 */
/*ARGSUSED*/
ptsopen(dev, flag)
	dev_t dev;
	int flag;
{
	register struct tty *tp;
	register struct pty *pt;

	dev = minor(dev);
	if (dev >= MAXPTY) {
		u.u_error = ENXIO;
		return;
	}
	if (ptyalloc(dev) == 0) {
		u.u_error = ENOMEM;
		return;
	}

	pt = pty[dev];
	tp = &(pt->pt_tty);
	if ((tp->t_state & ISOPEN) == 0)
		ttinit(tp);

	while ((pt->pt_state & PTC_OPEN) == 0) {
		tp->t_state |= WOPEN;
		ptcsleep(pt, PTS_WAITING);
	}
	pt->pt_state |= PTS_OPEN;
	(*linesw[tp->t_line].l_open)(tp);
}

/*
 * ptsclose:
 *	- close down a pty
 *	- close this end down and adjust tty state so that controller
 *	  will find out on next operation
 */
/* ARGSUSED */
ptsclose(dev, flag)
	dev_t dev;
{
	register struct tty *tp;
	register struct pty *pt;

	dev = minor(dev);
	pt = pty[dev];
	tp = &(pt->pt_tty);
	(*linesw[tp->t_line].l_close)(tp);

	/* let controlling tty know */
	pt->pt_state &= ~PTS_OPEN;
	ptcwakeup(pt, (PTC_OSLEEP | PTC_ISLEEP));

	/* if controller is already gone, free up data structure */
	if ((pt->pt_state & PTC_OPEN) == 0) {
		free(pty[dev]);
		pty[dev] = 0;
	}
}

/*
 * ptszot:
 *	- slam down dead pty connection
 */
ptszot(pt)
	register struct pty *pt;
{
	if (pt->pt_state & PTS_SENTHUP)
		psignal(u.u_procp, SIGKILL);
	else {
		pt->pt_state |= PTS_SENTHUP;
		psignal(u.u_procp, SIGHUP);
	}
}

/*
 * ptsread:
 *	- read data which was written by ptcwrite
 *	- input is canonacal-ized here
 */
ptsread(dev)
	dev_t dev;
{
	register struct tty *tp;
	register struct pty *pt;

	dev = minor(dev);
	pt = pty[dev];
	tp = &(pt->pt_tty);
	if (pt->pt_state & PTC_OPEN) {
		(*linesw[tp->t_line].l_read)(tp);
		ptcwakeup(pt, PTC_ISLEEP);
	} else 
		ptszot(pt);
}

/*
 * ptswrite:
 *	- write data for ptcread to read
 */
ptswrite(dev)
	dev_t dev;
{
	register struct tty *tp;
	register struct pty *pt;

	dev = minor(dev);
	pt = pty[dev];
	tp = &(pt->pt_tty);
	if (pt->pt_state & PTC_OPEN)
		(*linesw[tp->t_line].l_write)(tp);
	else
		ptszot(pt);
}

ptsproc(tp, cmd)
	register struct tty *tp;
	int cmd;
{
        extern int ttrstrt();

        switch(cmd) {
          case T_TIME:
                tp->t_state &= ~TIMEOUT;
                goto start;
          case T_WFLUSH:
		tp->t_tbuf.c_size -= tp->t_tbuf.c_count;
		tp->t_tbuf.c_count = 0;
		/* FALL THROUGH */
          case T_RESUME:
		ptcqueue(tp->t_index, QPTY_START);
                tp->t_state &= ~TTSTOP;
		/* FALL THROUGH */
          case T_OUTPUT:
start:
		/*
		 * See if controlling tty is ready for some data
		 */
		ptcwakeup(pty[tp->t_index], PTC_OSLEEP);
		ptcqueue(tp->t_index, QPTY_CANREAD);
                break;
          case T_SUSPEND:
		ptcqueue(tp->t_index, QPTY_STOP);
                tp->t_state |= TTSTOP;
                break;
          case T_BLOCK:
                tp->t_state &= ~TTXON;
                tp->t_state |= TBLOCK;
                break;
          case T_RFLUSH:
                if (!(tp->t_state & TBLOCK))
                        break;
		/* FALL THROUGH */
          case T_UNBLOCK:
                tp->t_state &= ~(TTXOFF|TBLOCK);
                goto start;
          case T_BREAK:
                tp->t_state |= TIMEOUT;
                timeout(ttrstrt, (caddr_t)tp, (short) hz / 4);
                break;
        }
}

/*ARGSUSED*/
ptsioctl(dev, cmd, addr, flag)
	register caddr_t addr;
	register dev_t dev;
{
	register struct pty *pt;

	dev = minor(dev);
	pt = pty[dev];
	if (cmd == PTIOC_GETWINSIZE) {
		if (copyout((caddr_t) &(pt->pt_size), addr,
			    sizeof(struct winsize)))
			u.u_error = EFAULT;
	} else
		(void) ttiocom(&(pt->pt_tty), cmd, (int)addr, flag);
}

/*ARGSUSED*/
ptcopen(dev, flag)
	dev_t dev;
	int flag;
{
	register struct tty *tp;
	register struct pty *pt;

	dev = minor(dev);
	if (dev >= MAXPTY) {
		u.u_error = ENXIO;
		return;
	}
	if (ptyalloc(dev) == 0) {
		u.u_error = ENOMEM;
		return;
	}

	pt = pty[dev];
	tp = &(pt->pt_tty);
	if (pt->pt_state & (PTC_OPEN | PTS_OPEN)) {
		u.u_error = EIO;
		return;
	}

	/* set some state of slave tty */
	tp->t_iflag = ICRNL|ISTRIP|IGNPAR;
	tp->t_oflag = OPOST|ONLCR|TAB3;
	tp->t_lflag = ISIG|ICANON;			/* no echo */
	tp->t_proc = ptsproc;
	tp->t_index = dev;
	tp->t_state |= CARR_ON;

	pt->pt_state |= PTC_OPEN;
	ptcwakeup(pt, PTS_WAITING);	/* wakeup sleepers in ptsopen() */
}

/*
 * ptcclose:
 *	- close down slave and controlling tty
 *	- mark closed by shutting down virtual carrier
 */
/* ARGSUSED */
ptcclose(dev, flag)
	dev_t dev;
	int flag;
{
	register struct tty *tp;
	register struct pty *pt;

	dev = minor(dev);
	pt = pty[dev];
	tp = &(pt->pt_tty);
	if (tp->t_state & ISOPEN)
		signal(tp->t_pgrp, SIGHUP);
	tp->t_state &= ~CARR_ON;	/* virtual carrier gone */

	/* wakeup anything else that might need waking up */
	ptcwakeup(pt, PTC_ISLEEP | PTC_OSLEEP);

	/* flush lingering data ignored by slave */
	ttyflush(tp, FREAD|FWRITE);

	/* if slave is already gone, free up data structure */
	if ((pt->pt_state & PTS_OPEN) == 0) {
		free(pt);
		pty[dev] = 0;
	} else {
		pt->pt_state &= ~(PTC_QUEUE | PTC_OPEN);
		pt->pt_ic = 0;
	}
}

/*
 * ptcread:
 *	- read data from a ptswrite
 *	- similar to a "device" output routine, except that the output
 *	  goes to the user
 *	- return whatever data is ready; wait if none is ready
 *	- wakeup ptswrite when flow is blocked
 */
ptcread(dev)
	dev_t dev;
{
	register struct tty *tp;
	register struct ccblock *tbuf;
	register int (*output)();
	register struct pty *pt;
	register int n;
	register int sentsome;

	/*
	 * Spill data out of clists into user memory
	 */
	dev = minor(dev);
	pt = pty[dev];
	tp = &(pt->pt_tty);
	tbuf = &tp->t_tbuf;
	output = linesw[(short) tp->t_line].l_output;
	sentsome = 0;
	do {
		if ((tp->t_state & (CARR_ON | ISOPEN)) == 0)
			return;
		if (tp->t_state & (TIMEOUT | TTSTOP | BUSY))
			goto wait_a_bit;
		if ((tbuf->c_ptr == 0) || (tbuf->c_count == 0)) {
			if (tbuf->c_ptr)
				tbuf->c_ptr -= tbuf->c_size - tbuf->c_count;
			if (!(CPRES & (*output)(tp))) {
				if (sentsome)
					break;
wait_a_bit:
#ifndef	KOPT_NOGL
				/*
				 * We aren't allowed to sleep when the
				 * pty is being queued
				 */
				if (pt->pt_state & PTC_QUEUE)
					return;
#endif
				ptcsleep(pt, PTC_OSLEEP);
				continue;
			}
		}
		n = tbuf->c_count;
		if (n > u.u_count)
			n = u.u_count;
		tp->t_state |= BUSY;
		iomove(tbuf->c_ptr, n, B_READ);
		if (u.u_error == 0) {
			tbuf->c_ptr += n;
			tbuf->c_count -= n;
			sentsome = 1;
		}
		tp->t_state &= ~BUSY;
	} while (!u.u_error && n);
}

/*
 * ptcwrite:
 *	- write data for a ptsread to read
 *	- simulates user input
 */
ptcwrite(dev)
	dev_t dev;
{
	register struct tty *tp;
	register struct ccblock *cbp;
	register int (*input)();
	register struct pty *pt;
	register int c;

	dev = minor(dev);
	pt = pty[dev];
	tp = &(pt->pt_tty);
	input = linesw[tp->t_line].l_input;
	while (u.u_count) {
		/*
		 * See if connection went down while we were sleeping
		 */
		if ((tp->t_state & (CARR_ON | ISOPEN)) == 0)
			return;
		/*
		 * Block input if it would overflow the raw input
		 * queue.  ptsread() knows to wake us up.
		 */
		if (tp->t_rawq.c_cc > TTXOHI) {
			ptcsleep(pt, PTC_ISLEEP);
			continue;
		}
		c = cpass();
		if (u.u_error)
			break;
		cbp = &tp->t_rbuf;
		if (cbp->c_ptr == NULL)		/* drop */
			continue;
		if (tp->t_iflag & IXON) {
			register char ctmp;

			ctmp = c & 0177;
			if (tp->t_state&TTSTOP) {
				if (ctmp == CSTART || tp->t_iflag&IXANY)
					(*tp->t_proc)(tp, T_RESUME);
			} else {
				if (ctmp == CSTOP)
					(*tp->t_proc)(tp, T_SUSPEND);
			}
			if (ctmp == CSTART || ctmp == CSTOP)
				continue;
		}
		*cbp->c_ptr++ = c;
		if (--cbp->c_count == 0) {
			cbp->c_ptr -= cbp->c_size;
			(*input)(tp);
		}
	}

	/* cleanup input left in buffer but not passed to line discipline */
	if (cbp->c_size != cbp->c_count) {
		cbp->c_ptr -= cbp->c_size - cbp->c_count;
		(*input)(tp);
	}
}

/*
 * ptcioctl:
 *	- issue control commands
 */
/* ARGSUSED */
ptcioctl(dev, cmd, addr, flag)
	dev_t dev;
	int cmd;
	caddr_t addr;
	int flag;
{
	register struct tty *tp;
	register struct pty *pt;

	dev = minor(dev);
	pt = pty[dev];
	tp = &(pt->pt_tty);
	switch (cmd) {
#ifndef	KOPT_NOGL
	  case PTIOC_QUEUE:		/* tag device as queued */
		if ((u.u_procp->p_flag & SGR) && u.u_procp->p_grhandle) {
			pt->pt_state |= PTC_QUEUE;
			pt->pt_ic = u.u_procp->p_grhandle;
		} else
			u.u_error = EINVAL;
		break;
#endif
	  case PTIOC_BREAK:		/* fake a break */
		/*
		 * If we are not ignoring breaks and we want to have
		 * breaks treated as an interrupt, then deal with the
		 * break
		 */
		if (!(tp->t_iflag & IGNBRK) && (tp->t_iflag & BRKINT)) {
			signal(tp->t_pgrp, SIGINT);
			ttyflush(tp, (FREAD|FWRITE));
		}
		break;
	  case PTIOC_SETWINSIZE:	/* set window size */	
		if (copyin(addr, &pt->pt_size, sizeof(struct winsize)))
			u.u_error = EFAULT;
		/*
		 * Tag the state bits noting that a window size change has
		 * happened.  Wakeup the slave if its sleeping, so that it
		 * can process the window size change, if it wants to.
		 */
		break;
	  default:
		u.u_error = EINVAL;
		break;
	}
}

/*
 * ptcwakeup:
 *	- wakeup tty if event is being waited for
 */
ptcwakeup(pt, event)
	register struct pty *pt;
	int event;
{
	if (pt->pt_state & event) {
		pt->pt_state &= ~event;
		wakeup((caddr_t) &(pt->pt_tty.t_state));
	}
}

/*
 * ptcsleep:
 *	- sleep on a given event
 *	- avoid deadlock by insuring that both controller and slave
 *	  don't both sleep
 */
ptcsleep(pt, event)
	register struct pty *pt;
	int event;
{
	pt->pt_state |= event;
	sleep((caddr_t) &(pt->pt_tty.t_state), TTIPRI);
}

/*
 * ptcqueue:
 *	- put something in the controllers queue
 */
ptcqueue(ptynum, t)
	int ptynum;
	int t;
{
#ifndef	KOPT_NOGL
	register struct pty *pt;

	pt = pty[ptynum];
	if (pt->pt_ic && (pt->pt_state & PTC_QUEUE)) {
		/*
		 * Put something in the queue.  Give the process
		 * the pty index so that if its managing more
		 * than one pty, it can figure out which one
		 * the event is for.
		 */
		gr_qenter(pt->pt_ic, t, pt->pt_tty.t_index);
	}
#endif
}
