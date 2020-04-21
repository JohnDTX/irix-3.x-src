/* Copyright 1986, Silicon Graphics Inc., Mountain View, CA. */
/* Pseudo-teletype Driver
 *	(Actually two drivers, requiring two entries in 'cdevsw')
 *
 * $Source: /d2/3.7/src/sys/streams/RCS/spty.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:34:46 $
 */

#ifdef SVR3
#include "sys/sbd.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/types.h"
#include "sys/conf.h"
#include "sys/systm.h"
#include "sys/termio.h"
#include "sys/errno.h"
#include "sys/ioctl.h"
#include "sys/signal.h"
#include "sys/pcb.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/fs/s5dir.h"
#include "sys/user.h"
#include "sys/file.h"
#include "sys/proc.h"
#include "sys/buf.h"
#include "sys/debug.h"
#include "sys/pty_ioctl.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/strids.h"
#include "sys/stty_ld.h"

#else
#include "pty.h"
#include "../h/param.h"
#include "../h/config.h"
#include "../h/types.h"
#include "../h/systm.h"
#include "../h/termio.h"
#include "../h/errno.h"
#include "../h/ioctl.h"
#include "../h/signal.h"
#include "../h/user.h"
#include "../h/conf.h"
#include "../h/file.h"
#include "../h/proc.h"
#include "../h/buf.h"
#include "../h/pty_ioctl.h"
#include "../streams/stream.h"
#include "../streams/stropts.h"
#include "../streams/strids.h"
#include "../streams/stty_ld.h"
#include "../streams/strcomp.h"
#endif

extern mblk_t *str_allocb();

extern struct stty_ld def_stty_ld;


/* controller stream stuff
 */
static struct module_info ptcm_info = {
	STRID_PTC,			/* module ID */
	"PTC",				/* module name */
	0,				/* minimum packet size */
	INFPSZ,				/* infinite maximum packet size */
	256,				/* hi-water mark */
	0,				/* lo-water mark */
};

static int ptc_open();
static ptc_rsrv(), ptc_close();
static struct qinit ptc_rinit = {
	NULL, ptc_rsrv, ptc_open, ptc_close, NULL, &ptcm_info, NULL
};

static ptc_wput(), ptc_wsrv();
static struct qinit ptc_winit = {
	ptc_wput, ptc_wsrv, NULL, NULL, NULL, &ptcm_info, NULL
};

struct streamtab ptcinfo = {&ptc_rinit, &ptc_winit, NULL, NULL};


/* slave stream stuff
 */
static struct module_info ptsm_info = {
	STRID_PTS,			/* module ID */
	"PTS",				/* module name */
	0,				/* minimum packet size */
	INFPSZ,				/* infinite maximum packet size */
	256,				/* hi-water mark */
	0,				/* lo-water mark */
};

static int pts_open();
static pts_rsrv(), pts_close();
static struct qinit pts_rinit = {
	NULL, pts_rsrv, pts_open, pts_close, NULL, &ptsm_info, NULL
};

static pts_wput(), pts_wsrv();
static struct qinit pts_winit = {
	pts_wput, pts_wsrv, NULL, NULL, NULL, &ptsm_info, NULL
};

struct streamtab ptsinfo = {&pts_rinit, &pts_winit, NULL, NULL};


/*
 * We allocate one of these structures for every open ptc/tty pair
 */
struct pty {
	queue_t	*pts_rq, *pts_wq;	/* slave queues			*/
	queue_t	*ptc_rq, *ptc_wq;	/* controller queues		*/

	long	pt_ic;			/* handle for graphics queue	*/

	struct termio pt_termio;
#define pt_line pt_termio.c_line
#define pt_iflag pt_termio.c_iflag
#define pt_cflag pt_termio.c_cflag

	u_char	pt_litc;		/* escape next char--e.g. XOFF	*/
	u_char	pt_stopc;		/* output XOFF character	*/
	u_char	pt_startc;		/* output XON character		*/

	u_char	pt_index;		/* port number			*/

	u_char	pt_pkt;			/* pending 'packet' bits	*/

	u_char	pts_state, ptc_state;	/* current state		*/
	struct	winsize	pt_winsize;	/* size of window		*/
};

/* 'invalid' value for pt_ic */
#ifdef SVR3
#define NULLICNO (-1)
#else
#define NULLICNO 0
#endif

#define MAX_MAXPTY 100			/* cannot configure more ptys */
					/* ps does not like ttyq100 */
					/* cannot have minor dev #s >127 */
#ifdef SVR3
#define MIN_CLONE 1			/* start cloning here */
					/* reserve devices below this */
#define	MAXPTY	maxpty
extern int maxpty;			/* lboot parameter in SVR3 */
#else
#define MIN_CLONE 0
#define	MAXPTY	100			/* const in SVR0 (if it were there) */
#if MAXPTY > MAX_MAXPTY
? ? ? too many ptys
#endif
#endif
static struct pty *pty[MAX_MAXPTY];

/* force cflag to this */
#define PT_CFLAG (CS8|CREAD|CLOCAL)

/* slave state bits */
#define PT_UNBORN	0x01		/* never opened */
#define	PT_DEAD		0x02		/* have lived and died already	*/
#define PTS_TXSTOP	0x04		/* output stopped by received XOFF */
#define PTS_LIT		0x08		/* have seen literal character	*/

/* controller state bits */
#define PTC_QUEUED	0x04		/* 'queued' device		*/
#define PTC_RD_ARMED	0x08		/* 'read-ok' armed		*/
#define PTC_PKT		0x10		/* 'packet mode' on		*/
#define PTC_PKT_SET	0x20		/* 'packet mode' set in head	*/

#define MAX_PKT_DATA	256		/* maximum data in 'packet mode' */



#ifdef SVR3
/* initialize this module
 */
ptcinit()
{
	if (MAXPTY > MAX_MAXPTY) {
		dri_printf("ptc: too many pty\'s: %d > %d\n",
			   MAXPTY, MAX_MAXPTY);
		MAXPTY = MAX_MAXPTY;
	}
}
#endif


/* allocate pty structure for the given dev, if its not already allocated
 */
static struct pty *			/* 0=failed */
pt_alloc(d)
register short d;
{
	register struct pty *pt;

	for (;;) {
		pt = pty[d];
		if (!pt) {
			pty[d] = (struct pty *)1;
			pt = (struct pty *)calloc(1, sizeof(struct pty));
			if (!pt) {
				pty[d] = 0;
				wakeup((caddr_t)&pty[d]);
				return 0;
			}
			pt->pt_ic = NULLICNO;
			pt->pt_litc = CLNEXT;
			pt->pt_stopc = CSTOP;
			pt->pt_startc = CSTART;
			pt->pt_termio = def_stty_ld.st_termio;
			pt->pt_cflag |= PT_CFLAG|B19200;
			pt->pt_index = d;
			pt->ptc_state = PT_UNBORN;
			pt->pts_state = PT_UNBORN;
			pty[d] = pt;

		} else if (pt == (struct pty *)1) {
			(void)sleep((caddr_t)&pty[d], PCATCH|STIPRI);
			continue;
		}

		return pt;
	}
}



/* do 'XON' */
static
pt_resume(pt)
register struct pty *pt;
{
	pt->pts_state &= ~PTS_TXSTOP;
	ptc_queue(pt, QPTY_START);
	qenable(pt->pts_wq);
}


/* set 'tty' parameters
 */
static
pt_tcset(pt,bp)
register struct pty *pt;
register mblk_t *bp;
{
	register struct iocblk *iocp;
	register struct termio *tp;

	iocp = (struct iocblk*)bp->b_rptr;
	tp = STERMIO(bp);

	if (pt->ptc_state & PTC_PKT) {	/* do packet stuff */
		register ushort iflag = (tp->c_iflag & IXON);
		if ((iflag ^ pt->pt_iflag) & IXON) {
			register int s = splstr();
			pt->pt_pkt &= ~(TIOCPKT_DOSTOP|TIOCPKT_NOSTOP);
			if (iflag == IXON) {
				pt->pt_pkt |= TIOCPKT_DOSTOP;
			} else {
				pt->pt_pkt |= TIOCPKT_NOSTOP;
			}
			splx(s);
		}
	}

	pt->pt_termio = *tp;

	if (PTS_TXSTOP & pt->pts_state
	    && !(pt->pt_iflag & IXON))
		pt_resume(pt);

	iocp->ioc_count = 0;
	bp->b_datap->db_type = M_IOCACK;
}



#define SWINSIZE(bp) ((struct winsize *)(bp)->b_cont->b_rptr)
#define WIN_IOCP(bp) ((struct iocblk*)bp->b_rptr)

/* get current window size
 */
static
pt_gwinsz(wq,bp,pt)
register queue_t *wq;
register mblk_t *bp;
register struct pty *pt;
{
	ASSERT(WIN_IOCP(bp)->ioc_count == sizeof(struct winsize));

	*SWINSIZE(bp) = pt->pt_winsize;
	bp->b_datap->db_type = M_IOCACK;
	qreply(wq,bp);
}


/* set window size
 */
static
pt_swinsz(wq,bp,pt)
register queue_t *wq;			/* out write queue */
register mblk_t *bp;
register struct pty *pt;
{
	register u_char df;
	ASSERT(WIN_IOCP(bp)->ioc_count == sizeof(struct winsize));

	df = bcmp((char*)&pt->pt_winsize, (char*)SWINSIZE(bp),
		  sizeof(struct winsize));
	if (0 != df)
		pt->pt_winsize = *SWINSIZE(bp);
	bp->b_datap->db_type = M_IOCACK;
	WIN_IOCP(bp)->ioc_count = 0;
	qreply(wq,bp);

	if (0 != df && pt->pts_rq)	/* signal size change to all slaves */
		(void)putctl1(pt->pts_rq->q_next, M_PCSIG, SIGWINCH);
}


/* slave side open
 */
static int
pts_open(rq, dev, flag, sflag)
register queue_t *rq;			/* our new read queue */
dev_t dev;
int flag;
int sflag;
{
	register struct pty *pt;
	register queue_t *wq = WR(rq);
	register ushort d;

	if (sflag) {			/* do not do slave 'clone' open */
		u.u_error = ENODEV;
		return OPENFAIL;
	} else {
		d = minor(dev);
	}
	if (d >= MAXPTY) {
		u.u_error = ENODEV;
		return OPENFAIL;
	}
	if (!(pt = pt_alloc(d))) {
		u.u_error = ENOMEM;
		return OPENFAIL;
	}

	if (!pt->pts_rq) {		/* connect new device to stream */
		if (
#if MIN_CLONE > 0
		    d >= MIN_CLONE &&
#endif
		    ((pt->pts_state & PT_DEAD)
		     || (pt->ptc_state & PT_DEAD))) {
			u.u_error = EIO;
			return OPENFAIL;	/* ignore zombies */
		}

		if (!strdrv_push(rq,"stty_ld",dev))
			return OPENFAIL;

		rq->q_ptr = (caddr_t)pt;
		wq->q_ptr = (caddr_t)pt;
		pt->pts_rq = rq;
		pt->pts_wq = wq;
		pt->pts_state &= ~(PT_UNBORN|PT_DEAD);


		while (!pt->ptc_rq	/* wait for controller if asked */
		       && !(flag & FNDELAY)) {
			if (sleep((caddr_t)pt, STIPRI|PCATCH)) {
				u.u_error = EINTR;
				pt->pts_rq = 0;
				pt->pts_wq = 0;
				if (pt->ptc_state & (PT_UNBORN|PT_DEAD)) {
					free((char*)pt);
					pty[d] = 0;
				} else {	/* send HUP to controller */
					pt->pts_state |= PT_DEAD;
#if MIN_CLONE > 0
					if (d >= MIN_CLONE)
#endif
					    (void)putctl(pt->ptc_rq->q_next,
							     M_HANGUP);
				}
				return OPENFAIL;
			}
		}

	} else {
		ASSERT((struct pty*)pt->pts_rq->q_ptr == pt);
		ASSERT((struct pty*)pt->pts_wq->q_ptr == pt);
	}

	return d;
}



/* close down a pty, for the slave
 *	- close this end down and adjust state so that controller
 *	  will find out on next operation
 */
static
pts_close(rq)
queue_t *rq;
{
	register struct pty *pt = (struct pty*)rq->q_ptr;
	register ushort d = pt->pt_index;

	ASSERT(d < MAXPTY && pty[d] == pt);

	str_unbcall(rq);		/* stop waiting for buffers */

	if (pt->ptc_state & (PT_UNBORN|PT_DEAD)) {
		free((char*)pt);
		pty[d] = 0;

	} else {			/* let controlling tty know */
		pt->pts_state |= PT_DEAD;
		pt->pts_rq = 0;
		pt->pts_wq = 0;
#if MIN_CLONE > 0
		if (d >= MIN_CLONE)
#endif
			(void)putctl(pt->ptc_rq->q_next, M_HANGUP);
		flushq(pt->ptc_wq, FLUSHALL);
	}
}



/* slave output 'put' function
 */
static
pts_wput(wq, bp)
register queue_t *wq;
register mblk_t *bp;
{
	register struct pty *pt = (struct pty*)wq->q_ptr;
	register struct iocblk *iocp;

	ASSERT(pt->pt_index < MAXPTY && pty[pt->pt_index] == pt);

	switch (bp->b_datap->db_type) {

	case M_FLUSH:
		if (pt->ptc_state & PTC_PKT) {	/* handle packet mode */
			if (*bp->b_rptr & FLUSHW)
				pt->pt_pkt |= TIOCPKT_FLUSHWRITE;
			if (*bp->b_rptr & FLUSHR)
				pt->pt_pkt |= TIOCPKT_FLUSHREAD;
		}
		if (*bp->b_rptr & FLUSHW)
			pt_resume(pt);
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
				pt->pts_state |= PTS_TXSTOP;
				ptc_queue(pt, QPTY_STOP);
				break;
			case 1:		/* resume output */
				pt_resume(pt);
				break;
			default:
				iocp->ioc_error = EINVAL;
				break;
			}
			bp->b_datap->db_type = M_IOCACK;
			iocp->ioc_count = 0;
			qreply(wq, bp);
			break;

		case TCSETA:
			ASSERT(iocp->ioc_count == sizeof(struct termio));
			pt_tcset(pt,bp);
			qreply(wq,bp);
			break;

		case TCSETAW:
		case TCSETAF:
			ASSERT(iocp->ioc_count == sizeof(struct termio));
			putq(wq, bp);
			break;

		case TCGETA:
			tcgeta(wq,bp, &pt->pt_termio);
			break;

		case TCSBRK:
			putq(wq, bp);
			break;

		case TIOCGWINSZ:	/* get window size */
			pt_gwinsz(wq,bp,pt);
			break;

		case TIOCSWINSZ:		/* set window size */
			pt_swinsz(wq,bp,pt);
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



/* send characters to the controller, from the slave
 *	We come here only if the controller got behind, or if we encountered
 *	an IOCTL that needed to be done in sequence.
 */
static
pts_wsrv(wq)
register queue_t *wq;
{
	register struct pty *pt = (struct pty*)wq->q_ptr;
	register mblk_t *bp;
	register queue_t *crq = pt->ptc_rq;
	u_char did = 0;

	ASSERT(pt->pt_index < MAXPTY && pty[pt->pt_index] == pt);

	if (!crq)			/* forget it if no controller */
		return;

	if (pt->ptc_state & PTC_PKT_SET) {
		register struct stroptions *sop;
		bp = str_allocb(sizeof(struct stroptions), wq, BPRI_LO);
		if (!bp)
			return;
		bp->b_datap->db_type = M_SETOPTS;
		sop = (struct stroptions*)bp->b_rptr;
		bp->b_wptr += sizeof(struct stroptions);
		sop->so_flags = SO_READOPT;
		sop->so_readopt = ((pt->ptc_state & PTC_PKT) ? RMSGD : RNORM);
		putnext(crq, bp);
		pt->ptc_state &= ~PTC_PKT_SET;
	}

	for (;;) {
		if (TIOCPKT_DATA != pt->pt_pkt) {
			bp = str_allocb(1,wq,BPRI_LO);
			if (!bp)
				break;
			*bp->b_wptr++ = pt->pt_pkt;
			putnext(crq, bp);
			pt->pt_pkt = TIOCPKT_DATA;
		}

		if (!(bp = getq(wq)))	/* quit after last message */
			break;

		switch (bp->b_datap->db_type) {
		case M_IOCTL:
			{
				register struct iocblk *iocp;
				iocp = (struct iocblk*)bp->b_rptr;

				switch (iocp->ioc_cmd) {
				case TCSETAW:
					pt_tcset(pt,bp);
					break;
				case TCSETAF:
					(void)putctl1(RD(wq)->q_next,
						      M_FLUSH,FLUSHR);
					if (pt->ptc_state & PTC_PKT)
						pt->pt_pkt|=TIOCPKT_FLUSHREAD;
					pt_tcset(pt,bp);
					break;
				case TCSBRK:	/* let output empty */
					bp->b_datap->db_type = M_IOCACK;
					iocp->ioc_count = 0;
					break;
				default:
					panic("???");
				}
				qreply(wq,bp);
			}
			break;


		case M_DATA:
			if ((pt->pts_state & PTS_TXSTOP)
			    || !canput(crq->q_next)) {
				putbq(wq, bp);
				goto for_exit;
			}

			if (pt->ptc_state & PTC_PKT) {	/* need header in */
				register mblk_t *hbp;	/* packet mode */
				hbp = str_allocb(1,wq,BPRI_LO);
				if (!hbp) {
					putbq(wq, bp);
					goto for_exit;
				}
				*hbp->b_wptr++ = TIOCPKT_DATA;
				hbp->b_cont = bp;
				if (bp->b_cont	/* limit msg size */
				    && msgdsize(hbp) > MAX_PKT_DATA) {
					putbq(wq, bp->b_cont);
					bp->b_cont = 0;
				}
				bp = hbp;
			}
			putnext(crq, bp);	/* send data up */
			did = 1;
			break;

		default:		/* the put fnc checked all msgs */
			panic("?");
		}
	}
for_exit:;

	if (did
	    && (pt->ptc_state & PTC_RD_ARMED)) {
		ptc_queue(pt, QPTY_CANREAD);
		pt->ptc_state &= ~PTC_RD_ARMED;
	}
}



/* slave read service function
 *	Take data from the controller, and send it up to the slave.
 *	We have to scan the data, looking for XON and XOFF.  Worse, we must
 *	discard XON and XOFF when we find them.
 */
static
pts_rsrv(rq)
queue_t *rq;
{
	struct pty *pt = (struct pty*)rq->q_ptr;
	register u_char *cp, *lim;
	register mblk_t *obp;
	register mblk_t *ibp;

	ASSERT(pt->pt_index < MAXPTY && pty[pt->pt_index] == pt);

	ibp = NULL;
	for (;;) {
		if (!ibp) {
			if (!canput(rq->q_next))
				break;	/* quit if slave constipated */
			if (!(ibp = getq(rq))) {	/* quit if no more, */
				register queue_t *cwq;	/* after awakening */
				cwq = pt->ptc_wq;	/* controller */
				if (0 != cwq
				    && 0 != cwq->q_first)
					qenable(cwq);
				break;
			}
		}

		ASSERT(M_DATA == ibp->b_datap->db_type);

		if (!(obp = dupb(ibp))) {	/* quit if no msg blocks */
			putbq(rq, ibp);
			(void)bufcall(0, BPRI_LO, qenable, (long)rq);
			break;
		}

		cp = obp->b_rptr;
		lim = obp->b_wptr;
		while (cp < lim) {
			/* start or stop output (if permitted) when we get
			 * XOFF or XON */
			if (pt->pt_iflag & IXON) {
				register u_char cs = *cp & 0x7f;

				if ((PTS_TXSTOP & pt->pts_state)
				    && (cs == pt->pt_startc
					|| ((IXANY & pt->pt_iflag)
					    && (cs != pt->pt_stopc
						|| pt->pt_line == LDISC0)))) {
					pt_resume(pt);
					if (cs == pt->pt_startc) {
						ibp->b_rptr++;
						break;
					}
				} else if (PTS_LIT & pt->pts_state) {
					pt->pts_state &= ~PTS_LIT;
				} else if (cs == pt->pt_stopc) {
					pt->pts_state |= PTS_TXSTOP;
					ptc_queue(pt, QPTY_STOP);
					ibp->b_rptr++;
					break;
				} else if (cs == pt->pt_startc) {
					ibp->b_rptr++;
					break;	/* ignore extra control-Qs */
				} else if (cs == pt->pt_litc
					   && LDISC0 != pt->pt_line) {
					pt->pts_state |= PTS_LIT;
				}
			}

			cp++;
		}

		ibp->b_rptr += (cp - obp->b_rptr);
		if (ibp->b_rptr >= ibp->b_wptr) {
			register mblk_t *nbp;
			nbp = rmvb(ibp,ibp);
			freemsg(ibp);
			ibp = nbp;
		}

		obp->b_wptr = cp;
		if (cp > obp->b_rptr) {	/* send the data up stream */
			putnext(rq, obp);
		} else {
			freemsg(obp);
		}
	}
}



/* put something in the controllers graphics queue
 */
ptc_queue(pt, t)
register struct pty *pt;
int t;
{
#if !defined(KOPT_NOGL) && !defined(GL1)
	if (pt->pt_ic != NULLICNO
	    && (pt->ptc_state & PTC_QUEUED)) {
		/*
		 * Put something in the queue.  Give the process
		 * the pty index so that if its managing more
		 * than one pty, it can figure out which one
		 * the event is for.
		 */
#ifdef SVR3
		gl_anyqenter(pt->pt_ic, t, pt->pt_index);
#else
		gr_qenter(pt->pt_ic, t, pt->pt_index);
#endif
	}
#endif
}



/* controller open
 */
/*ARGSUSED*/
static int
ptc_open(rq, dev, flag, sflag)
register queue_t *rq;
dev_t dev;
int flag;
int sflag;
{
	register struct pty *pt;
	register queue_t *wq = WR(rq);
	register ushort d;

	if (sflag) {			/* do 'clone' open */
		for (d = MIN_CLONE; d < MAXPTY && NULL != pty[d]; d++)
			continue;
	} else {
		d = minor(dev);
	}
	if (d >= MAXPTY) {
		u.u_error = ENODEV;
		return OPENFAIL;
	}
	if (!(pt = pt_alloc(d))) {
		u.u_error = ENOMEM;
		return OPENFAIL;
	}

	if (
#if MIN_CLONE > 0
	    d >= MIN_CLONE &&
#endif
	    (!(pt->ptc_state & PT_UNBORN)	/* fail if already open */
	     || (pt->pts_state & PT_DEAD))) {	/* ignore zombies */
		u.u_error = EIO;
		wakeup((caddr_t)pt);	/* awaken slave */
		return OPENFAIL;
	}

	rq->q_ptr = (caddr_t)pt;	/* connect new device to stream */
	wq->q_ptr = (caddr_t)pt;
	pt->ptc_rq = rq;
	pt->ptc_wq = wq;
	pt->ptc_state &= ~(PT_UNBORN|PT_DEAD);

	wakeup((caddr_t)pt);		/* awaken slave */

	return d;
}



/* close down a pty, for the controller
 *	- close this end down and adjust state so that slave
 *	  will find out on next operation
 */
static
ptc_close(rq)
queue_t *rq;
{
	register struct pty *pt = (struct pty*)rq->q_ptr;
	register ushort d = pt->pt_index;

	ASSERT(d < MAXPTY && pty[d] == pt);

	if (pt->pts_state & (PT_UNBORN|PT_DEAD)) {
		free((char*)pt);
		pty[d] = 0;

	} else {			/* let slave know */
		pt->ptc_state |= PT_DEAD;
		pt->ptc_state &= ~PTC_QUEUED;
		pt->pt_ic = NULLICNO;
		pt->ptc_rq = 0;
		pt->ptc_wq = 0;
#if MIN_CLONE > 0
		if (d >= MIN_CLONE)
#endif
			(void)putctl(pt->pts_rq->q_next, M_HANGUP);
	}
}



/* controller output 'put' function
 *	queue data for the slave to 'input'
 */
static
ptc_wput(wq, bp)
register queue_t *wq;			/* out write queue */
register mblk_t *bp;
{
	register struct pty *pt;
	register struct iocblk *iocp;
	register struct proc *procp;
	register queue_t *srq;		/* slave read queue */

	pt = (struct pty*)wq->q_ptr;
	srq = pt->pts_rq;

	ASSERT(pt->pt_index < MAXPTY && pty[pt->pt_index] == pt);

	switch (bp->b_datap->db_type) {

	case M_FLUSH:
		sdrv_flush(wq,bp);
		if (srq)
			qenable(srq);
		break;

	case M_DATA:			/* send data to slave */
		if (!srq)
			freemsg(bp);
		else {
			if (!wq->q_first && canput(srq))
				putq(srq, bp);
			else {		/* if slave is constipated, */
				noenable(wq);	/* queue it here to */
				putq(wq, bp);	/* throttle controller head */
			}
		}
		break;

	case M_DELAY:			/* ignore timing requests */
		freemsg(bp);
		break;

	case M_IOCTL:
		iocp = (struct iocblk*)bp->b_rptr;
		switch (iocp->ioc_cmd) {
#ifndef	KOPT_NOGL
		case PTIOC_QUEUE:	/* tag device as queued */
			ASSERT(iocp->ioc_count == sizeof(procp));

			procp = *(struct proc**)(bp->b_cont->b_rptr);
#ifdef SVR3
			pt->pt_ic = gl_procptoicno(procp);
			if (pt->pt_ic != NULLICNO)
				pt->ptc_state |= (PTC_QUEUED|PTC_RD_ARMED);
			else
				iocp->ioc_error = EINVAL;
#else
			if ((procp->p_flag & SGR) && procp->p_grhandle) {
				pt->ptc_state |= (PTC_QUEUED|PTC_RD_ARMED);
				pt->pt_ic = procp->p_grhandle;
			} else {
				iocp->ioc_error = EINVAL;
			}
#endif
			bp->b_datap->db_type = M_IOCACK;
			iocp->ioc_count = 0;
			qreply(wq,bp);
			break;
#endif

		case TIOCGWINSZ:		/* get window size */
			pt_gwinsz(wq,bp,pt);
			break;

		case TIOCSWINSZ:		/* set window size */
			pt_swinsz(wq,bp,pt);
			break;

		case TCSETAF:
			flushq(wq,FLUSHDATA);	/* flush and fall thru */
		case TCSETA:
		case TCSETAW:
			ASSERT(iocp->ioc_count == sizeof(struct termio));
			pt_tcset(pt,bp);
			/* This is a bit of a kludge.  To tell the line
			 * discipline code of the new mode, we send an ACK up
			 * the other stream. */
			if (NULL != srq) {
				register mblk_t *obp = dupmsg(bp);
				if (NULL != obp)
					putnext(srq,obp);
			}
			qreply(wq,bp);
			break;

		case TCGETA:
			tcgeta(wq,bp, &pt->pt_termio);
			break;

		case TCSBRK:		/* similute break to slave */
			if (0 == *(int*)bp->b_cont->b_rptr
			    && BRKINT == (pt->pt_iflag & (IGNBRK|BRKINT))
			    && srq != NULL) {
				flushq(srq, FLUSHDATA);
				(void)putctl1(srq->q_next, M_FLUSH, FLUSHRW);
				(void)putctl1(srq->q_next, M_PCSIG, SIGINT);
			}
			bp->b_datap->db_type = M_IOCACK;
			iocp->ioc_count = 0;
			qreply(wq,bp);
			break;

		case TIOCPKT:
			if (0 != *(int*)bp->b_cont->b_rptr) {
				pt->ptc_state |= PTC_PKT;
				pt->pt_pkt = TIOCPKT_DATA;
			} else {
				pt->ptc_state &= ~PTC_PKT;
			}
			pt->ptc_state |= PTC_PKT_SET;
			if (srq)
				qenable(srq);
			bp->b_datap->db_type = M_IOCACK;
			iocp->ioc_count = 0;
			qreply(wq,bp);
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



/* service controller output queue
 *	The controller enqueues its output directly up toward the slave stream
 *	head.  Therefore, this function is awakened only when the controller
 *	used canput() and was turned down, and things have now drained.
 */
static
ptc_wsrv(rq)
register queue_t *rq;			/* our read queue */
{
	register mblk_t *bp;
	register struct pty *pt = (struct pty*)rq->q_ptr;
	register queue_t *wq, *srq;
	register int s;

	ASSERT(pt->pt_index < MAXPTY && pty[pt->pt_index] == pt);

	wq = pt->ptc_wq;
	srq = pt->pts_rq;

	if (!srq)
		flushq(wq, FLUSHALL);

	s = spltty();
	while (canput(srq)
	       && 0 != (bp = getq(wq)))
		putq(srq, bp);
	splx(s);
}



/* service controller input queue
 *	The slave enqueues its output directly on the controller stream head.
 *	Therefore, this function is awakened only when the slave used canput()
 *	on the contoller head and was turned down.  The controller stream
 *	has now drained and has 'back-enabled' the controller queue, awakening
 *	this function.
 */
static
ptc_rsrv(rq)
register queue_t *rq;			/* controller read queue */
{
	register struct pty *pt = (struct pty*)rq->q_ptr;
	register queue_t *swq;

	ASSERT(pt->pt_index < MAXPTY && pty[pt->pt_index] == pt);

	swq = pt->pts_wq;
	if (swq)			/* just activate slave output */
		qenable(swq);
}


#ifdef SVR3
/*
 * This function is special for kernel printfs to come out when the graphics
 * console is enabled. It is called only by prom_putc() in the sprom driver
 * when its time to flush a kernel printf out to the graphics port.
 * The ptc rq is given to this routine and must be turned into the pts wq.
 */
cons_pts_wput(cwq, bp)
register queue_t *cwq;
register mblk_t *bp;
{
	register struct pty *pt = (struct pty*)cwq->q_ptr;
	register queue_t *swq;
	ASSERT(pt->ptc_wq == cwq);

	swq = pt->pts_wq;
	/*
	 * If the slave write q pntr is null then no one has the slave open.
	 * Return -1 in this case.
	 * This should be a temporary transient condition which would occur
	 * in between the time wsh goes away and grcond has time to respond
	 * to it.
	 */
	if (swq == NULL)
		return(-1);

	pts_wput(swq, bp);
	return(0);
}
#endif SVR3
