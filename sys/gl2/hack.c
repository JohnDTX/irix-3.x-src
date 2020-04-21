#include "../h/param.h"
#include "../h/printf.h"
#include "../h/sgigsc.h"
#include "../streams/stream.h"
#include "../streams/stropts.h"
#include "../streams/strids.h"
#include "../streams/strcomp.h"

static struct module_info hack_module_info = {
	STRID_HACK,			/* module ID */
	"hack",				/* module name */
	0,				/* minimum packet size */
	INFPSZ,				/* infinite maximum packet size	*/
	1,				/* hi-water mark */
	0,				/* lo-water mark */
};

static int hack_open();
static hack_put(), hack_close();
static struct qinit hack_rinit = {
	hack_put, NULL, hack_open, hack_close, NULL, &hack_module_info, NULL
};
static struct qinit hack_winit = {
	hack_put, NULL, NULL, NULL, NULL, &hack_module_info, NULL
};

struct streamtab hack_info = {&hack_rinit, &hack_winit, NULL, NULL};

queue_t	*consoleWQ;
static char switchedQueues;
static queue_t *firstWQ, *secondWQ;

/*
 * Make the streams underneath "q1" point to "q2"'s streams.  Make
 * the streams underneath "q2" point to "q1"'s streams.  That is,
 * exchange their pointers.  We KNOW that both "q1" and "q2" are the
 * write side of the stream.
 */
static void
hack_switch(q1, q2)
register queue_t *q1, *q2;
{
	register queue_t *q1_ld, *q2_ld;
	register queue_t *t;
	int s;

	s = splstr();
	q1_ld = RD(q1->q_next);
	q2_ld = RD(q2->q_next);

	/* exchange read pointers */
	t = q1_ld->q_next;
	q1_ld->q_next = q2_ld->q_next;
	q2_ld->q_next = t;

	/* exchange write pointers */
	t = q1->q_next;
	q1->q_next = q2->q_next;
	q2->q_next = t;
	splx(s);
}

/* open a new stream
 */
/* ARGSUSED */
static int
hack_open(rq, dev, flag, sflag)
register queue_t *rq;			/* our read queue */
dev_t dev;
int flag;
int sflag;
{
	register queue_t *wq;

	if (sflag != MODOPEN)
		return OPENFAIL;

	wq = WR(rq);
	if (!firstWQ) {
		firstWQ = wq;
	} else {
		if (!secondWQ && (wq != firstWQ)) {
			secondWQ = wq;
		}
	}
	return 0;
}

/* close a stream
 *	This is called when the stream is being dismantled or when this
 *	module is being popped.
 */
static
hack_close(rq)
register queue_t *rq;
{
	queue_t *wq;

	wq = WR(rq);
	if (switchedQueues) {
		hack_switch(firstWQ, secondWQ);
		switchedQueues = 0;
		firstWQ = 0;
		secondWQ = 0;
	}

	if (consoleOnPTY) {
		/* switch kernel printfs back */
		consoleWQ = 0;
		setConsole(CONSOLE_NOT_ON_PTY);
	}
}

/* accept a new output message
 */
static
hack_put(q, bp)
register queue_t *q;
register mblk_t *bp;
{
	switch (bp->b_datap->db_type) {
	case M_HANGUP:
		/* gobble these */
		freemsg(bp);
		break;
	case M_IOCTL:
		if (q->q_flag & QREADR) {
			putnext(q, bp);
			break;
		}
		switch (((struct iocblk*)bp->b_rptr)->ioc_cmd) {
		case SG3K_CONSOLE:
			if (switchedQueues || !firstWQ || !secondWQ) {
				/* oops */
				bp->b_datap->db_type = M_IOCNAK;
				qreply(q, bp);
			} else {
				consoleWQ = q->q_next;
				hack_switch(firstWQ, secondWQ);
				switchedQueues = 1;
				setConsole(CONSOLE_ON_PTY);
				bp->b_datap->db_type = M_IOCACK;
				qreply(q, bp);
			}
			break;

		default:		/* send other IOCTLs on */
			putnext(q, bp);
			break;
		}
		break;

	default:
		putnext(q, bp);
		break;
	}
}
