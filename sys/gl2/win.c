/*
 * Window streams pseudo driver for connecting the keyboard to the current
 * active window.
 *
 * $Source: /d2/3.7/src/sys/gl2/RCS/win.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:28:50 $
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/errno.h"
#include "../h/termio.h"
#include "../h/file.h"
#include "../h/conf.h"
#include "../h/signal.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/printf.h"
#include "machine/cpureg.h"

#undef	imin
#undef	imax

#include <gl2/gl.h>
#include <gl2/gltypes.h>
#include <gl2/device.h>
#include <gl2/window.h>

#undef QFULL
#include "../streams/stream.h"
#include "../streams/stropts.h"
#include "../streams/strids.h"
#include "../streams/stty_ld.h"
#include "../streams/strcomp.h"

extern struct stty_ld def_stty_ld;

/* stream stuff */

static struct module_info winm_info = {
	STRID_WIN,			/* module ID */
	"WIN",				/* module name */
	0,				/* minimum packet size */
	INFPSZ,				/* infinite maximum packet size */
	256,				/* hi-water mark */
	0,				/* lo-water mark */
};



static int wn_open();
static wn_rsrv(), wn_close();
static struct qinit wn_rinit = {
	NULL, wn_rsrv, wn_open, wn_close, NULL, &winm_info, NULL
};

static wn_wput(), wn_wsrv();
static struct qinit wn_winit = {
	wn_wput, wn_wsrv, NULL, NULL, NULL, &winm_info, NULL
};

struct streamtab wninfo = {&wn_rinit, &wn_winit, NULL, NULL};



/* a data structure to fake a device out of a window
 */
struct win {
	u_char	wn_state;		/* current state		*/

	u_char	wn_litc;		/* escape next char--e.g. XOFF	*/
	u_char	wn_stopc;		/* output XOFF character	*/
	u_char	wn_startc;		/* output XON character		*/

	struct termio wn_termio;
#define wn_iflag wn_termio.c_iflag	/* use some of the bits		*/
#define wn_line wn_termio.c_line	/* 'line discipline'		*/

	u_char	wn_index;		/* window number		*/

	queue_t	*wn_rq, *wn_wq;		/* our queues			*/
};
static struct win *win[NTXPORTS];


/* state for window display timer */
static enum {WNT_IDLE, WNT_RUNNING} wntimer_state = WNT_IDLE;
#define WNT_FREQ 20			/* do it this often per second */


/* use only these bits in our iflag */
#define IFLAG_MASK (IGNBRK|BRKINT|IGNPAR|IXON|IXANY|IXOFF)
/* force cflag to this */
#define WN_CFLAG (CS8|CREAD|CLOCAL|B19200)

/* bits in wn_state
 *	wn_state may be changed by interrupts!
 */
#define WN_TXSTOP	0x01		/* output stopped by received XOFF */
#define WN_LIT		0x02		/* have seen literal character	*/
#define WN_BLOCK	0x04		/* XOFF sent because input full */
#define WN_STALLED	0x08		/* stalled by graphics		*/



/* open a window, initialize the graphics if needed
 */
/* ARGSUSED */
static int
wn_open(rq, dev, flag, sflag)
register queue_t *rq;			/* our new read queue */
dev_t dev;
int flag;
int sflag;
{
	register short d;
	register struct win *wp;
	register queue_t *wq = WR(rq);

	if (sflag) {			/* do 'clone' open */
		for (d = 1;		/* (do not allocate console) */
		     d < NTXPORTS && NULL != win[d];
		     d++) continue;
	} else {
		d = minor(dev);
	}

	if (d >= NTXPORTS) {
		u.u_error = ENXIO;
		return OPENFAIL;
	}

	wp = win[d];
	if (!wp) {
		win[d] = (struct win*)1;
		wp = (struct win*)calloc(1, sizeof(struct win));
		if (!wp) {
			win[d] = 0;
			u.u_error = ENOMEM;
			return OPENFAIL;
		}

		wp->wn_litc = CLNEXT;
		wp->wn_stopc = CSTOP;
		wp->wn_startc = CSTART;
		wp->wn_termio = def_stty_ld.st_termio;
		wp->wn_termio.c_cflag = WN_CFLAG;	
		wp->wn_index = d;

		rq->q_ptr = (caddr_t)wp;	/* connect device to stream */
		wq->q_ptr = (caddr_t)wp;
		wp->wn_rq = rq;
		wp->wn_wq = wq;

		if (!strdrv_push(rq,"stty_ld",dev)) {
			win[d] = 0;
			free((char*)wp);
			return OPENFAIL;
		}

		win[d] = wp;

		if (d)
			tx_open(d);

	} else if ((struct win*)rq->q_ptr != wp) {
		u.u_error = ENOSR;	/* fail if already open */
		return OPENFAIL;	/* (eg. as the console) */

	} else {
		ASSERT(rq == wp->wn_rq);
		ASSERT((struct win*)wp->wn_rq->q_ptr == wp);
		ASSERT((struct win*)wp->wn_wq->q_ptr == wp);
	}

	return d;			/* return successfully */
}



/* close the window, cleaning up the state of things
 */
static
wn_close(rq)
queue_t *rq;
{
	register struct win *wp = (struct win*)rq->q_ptr;
	register u_char tn = wp->wn_index;
	register int s;

	ASSERT(tn < NTXPORTS && win[tn] == wp);

	s = spltty();
	win[tn] = NULL;
	splx(s);
	free((char*)wp);

	if (tn)				/* never close the console window */
		tx_close(tn);
}



/* repaint the screen
 *	This timer is used to batch screen updates.  That dramatically
 *	improves the output rate.
 */
static
wnredisplay()
{
	register struct win **wpp;
	register int s;
	int refire;

	if (!consoleOnPTY)
		tx_repaint(1);

	/* try to unjam any stalled windows */
	refire = 0;
	s = spltty();
	for (wpp = &win[0]; wpp < &win[NTXPORTS]; wpp++) {
		register struct win *wp = *wpp;

		if (wp && (wp->wn_state & WN_STALLED)) {
			/*
			 * If window is stalled, and it is no longer locked,
			 * enable output again and allow new painting.  If
			 * the textport is still locked, don't clear the
			 * stalled state, and refire the timer again so that
			 * we will try again shortly.
			 */
			if (tx_lock(wp->wn_index)) {
				tx_unlock(wp->wn_index);
				wp->wn_state &= ~WN_STALLED;
				qenable(wp->wn_wq);
			} else
				refire = 1;
		}
	}
	if (refire) {
		(void) timeout(wnredisplay, (caddr_t)0, HZ/WNT_FREQ);
	} else {
		wntimer_state = WNT_IDLE;
	}
	splx(s);
}



/* start output timer,
 *	if not already running
 */
static
wn_time()
{
	register int s;

	s = spltty();
	if (wntimer_state != WNT_RUNNING) {
		wntimer_state = WNT_RUNNING;
		(void)timeout(wnredisplay, (caddr_t)0, HZ/WNT_FREQ);
	}
	splx(s);
}



/* set parameters
 */
static
wn_tcset(wp,bp)
register struct win *wp;
register mblk_t *bp;
{
	register struct iocblk *iocp;
	register struct termio *tp;
	register int s;

	iocp = (struct iocblk*)bp->b_rptr;
	tp = STERMIO(bp);

	tp->c_cflag = WN_CFLAG;		/* we are always fast */
	s = spltty();
	wp->wn_termio = *tp;
	splx(s);

	iocp->ioc_count = 0;
	bp->b_datap->db_type = M_IOCACK;
}



/* send a bunch of 1 or more characters up the stream
 */
static
wn_rsrv(rq)
register queue_t *rq;			/* our read queue */
{
	register mblk_t *bp;
	register struct win *wp = (struct win*)rq->q_ptr;

	ASSERT(wp->wn_index < NTXPORTS && win[wp->wn_index] == wp);

	for (;;) {
		if (!canput(rq->q_next))	/* quit if upstream choked */
			break;

		if (!(bp = getq(rq)))	/* or we have caught up */
			break;

		putnext(rq, bp);
	}
}



/* output 'put' function
 */
static
wn_wput(wq, bp)
queue_t *wq;				/* out write queue */
register mblk_t *bp;
{
	register struct win *wp = (struct win*)wq->q_ptr;
	register struct iocblk *iocp;
	register int s;

	ASSERT(wp->wn_index < NTXPORTS && win[wp->wn_index] == wp);

	switch (bp->b_datap->db_type) {

	case M_FLUSH:
		if (*bp->b_rptr & FLUSHW) {
			s = spltty();
			wp->wn_state &= ~WN_TXSTOP;
			splx(s);
			qenable(wq);
		}
		sdrv_flush(wq,bp);
		break;

	case M_DATA:
		putq(wq, bp);
		break;

	case M_DELAY:			/* ignore timing requests */
		freemsg(bp);
		break;

	case M_IOCTL:
		iocp = (struct iocblk*)bp->b_rptr;
		switch (iocp->ioc_cmd) {
		case TCXONC:
			ASSERT(iocp->ioc_count == sizeof(int));
			switch (*(int*)(bp->b_cont->b_rptr)) {
			case 0:		/* stop output */
				s = spltty();
				wp->wn_state |= WN_TXSTOP;
				splx(s);
				break;
			case 1:		/* resume output */
				s = spltty();
				wp->wn_state &= ~WN_TXSTOP;
				splx(s);
				qenable(wp->wn_wq);
				break;
			default:
				iocp->ioc_error = EINVAL;
				break;
			}
			bp->b_datap->db_type = M_IOCACK;
			iocp->ioc_count = 0;
			qreply(wq, bp);
			break;

		case TCSBRK:
			putq(wq, bp);
			break;

		case TCSETA:
			ASSERT(iocp->ioc_count == sizeof(struct termio));
			wn_tcset(wp,bp);
			qreply(wq,bp);
			break;

		case TCSETAW:
		case TCSETAF:
			ASSERT(iocp->ioc_count == sizeof(struct termio));
			putq(wq, bp);
			break;

		case TCGETA:
			tcgeta(wq,bp, &wp->wn_termio);
			break;

		default:
			bp->b_datap->db_type = M_IOCNAK;
			qreply(wq,bp);
			break;
		}
		break;


	default:
		sdrv_error(wq,bp);
	}
}



/* send characters to the window
 */
static
wn_wsrv(wq)
register queue_t *wq;
{
	register struct win *wp = (struct win*)wq->q_ptr;
	register mblk_t *bp, *nbp;
	register int total;
	int s;

	ASSERT(wp->wn_index < NTXPORTS && win[wp->wn_index] == wp);

	if (!tx_lock(wp->wn_index)) {	/* textport in use right now */
		s = spltty();		/* so try again later */
		wp->wn_state |= WN_STALLED;
		splx(s);
		noenable(wq);
		wn_time();
		return;
	}

	s = spltty();
	wp->wn_state &= ~WN_STALLED;
	splx(s);
	enableok(wq);
	total = 0;
	for(;;) {
		if (wp->wn_state & WN_TXSTOP)	/* cannot output if stopped */
			break;

		if (!(bp = getq(wq)))	/* quit after last message */
			break;

		if (M_IOCTL == bp->b_datap->db_type) {
			register struct iocblk *iocp;

			iocp = (struct iocblk*)bp->b_rptr;

			switch (iocp->ioc_cmd) {
			case TCSETAW:
				wn_tcset(wp,bp);
				break;
			case TCSETAF:
				(void)putctl1(wp->wn_rq->q_next,
					      M_FLUSH, FLUSHR);
				wn_tcset(wp,bp);
				break;
			case TCSBRK:	/* ignore break requests */
				bp->b_datap->db_type = M_IOCACK;
				iocp->ioc_count = 0;
				break;
			default:
				panic("???");
			}
			qreply(wq,bp);
			continue;
		}


		do {			/* process an entire message */
			register u_char *cp = bp->b_rptr;
			register int cnt = (bp->b_wptr - cp);

			ASSERT(M_DATA == bp->b_datap->db_type);

			if (cnt > 0) {
				total += cnt;
				tx_addchars(wp->wn_index, cp, cnt);
			}

			nbp = rmvb(bp,bp);
			freeb(bp);
		} while (bp = nbp);
	}

	tx_unlock(wp->wn_index);
	if (total)			/* remember to fix the screen */
		wn_time();
}



/* take ASCII data from kgl and stuff it into the windows input queue
 *	- this is effectively the window interrupt routine which is called
 *	  by the duart interrupt routine...
 */
int	wn_softintr();
int	(*kbd_intr)() = wn_softintr;

win_softintr(tn, c, isbreak)
short tn, isbreak;
char c;
{
	/* Yes, this stuff seems odd to me.  However, it is what version 3.4
	 * did.
	 */
	(*kbd_intr)(tn, c, isbreak);
}

wn_softintr(tn, c, isbreak)
short int tn;
register unsigned char c;
short int isbreak;
{
	register struct win *wp = win[tn];
	register mblk_t *bp;

	if (!wp)			/* quit if not open */
		return;

	/* translate key stroke into something meaningful */
	if (isbreak) {
		if (wp->wn_iflag & IGNBRK)
			return;
		if (wp->wn_iflag & BRKINT) {
			flushq(wp->wn_rq, FLUSHDATA);
			(void)putctl1(wp->wn_rq->q_next, M_FLUSH, FLUSHRW);
			(void)putctl1(wp->wn_rq->q_next, M_PCSIG, SIGINT);
			return;
		}
		c = '\0';
	}

	/* start or stop output (if permitted) when we get XOFF or XON
	 */
	if (wp->wn_iflag & IXON) {
		register u_char cs = c & 0x7f;

		if ((WN_TXSTOP & wp->wn_state)
		    && (cs == wp->wn_startc
			|| ((IXANY & wp->wn_iflag)
			    && (cs != wp->wn_stopc
				|| wp->wn_line == LDISC0)))) {
			wp->wn_state &= ~WN_TXSTOP;
			qenable(wp->wn_wq);
			if (cs == wp->wn_startc)
				return;
		} else if (WN_LIT & wp->wn_state) {
			wp->wn_state &= ~WN_LIT;
		} else if (cs == wp->wn_stopc) {
			wp->wn_state |= WN_TXSTOP;
			return;
		} else if (cs == wp->wn_startc) {
			return;		/* ignore extra control-Qs */
		} else if (cs == wp->wn_litc	/* just note escape */
			   && LDISC0 != wp->wn_line) {
			wp->wn_state |= WN_LIT;
		}
	}


	if (wp->wn_rq->q_flag & QFULL)
		return;			/* quit on buffer overflow */
	bp = allocb(2, BPRI_HI);
	if (!bp)			/* cry if we do not get a buffer */
		return;

	*bp->b_wptr++ = c;

	putq(wp->wn_rq, bp);		/* let service function send it up */
}
