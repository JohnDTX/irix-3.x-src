# undef  DEBUG
/* # define QDEBUG		/* buffer debugging */
# define BDEBUG		/* additional buffer debugging */

#include "ib.h"

#include "../h/param.h"
#include "../h/buf.h"

#include "../h/ib_ioctl.h"
#include "../gpib/ib_ieee.h"
#include "../gpib/ib_reg.h"
#include "../gpib/ib.defs"

# ifndef QDEBUG
# undef  DEBUG
# endif  QDEBUG

# ifdef DEBUG
# undef DEBUG
# define DEBUG ib_q_debug
# endif DEBUG
#include "../gpib/ib_dbg.h"


extern struct ibconn *ibconnp[];
# ifdef BDEBUG
int ib_nbufs = 0;
int ib_maxnbufs = 0;
# endif BDEBUG



/*
 * Qinit() --
 * initialise bufq structure.
 */
Qinit(qp)
    register struct bufq *qp;
{
    qp->bq_flags = 0;
    qp->bq_head = qp->bq_tail = 0;
}

/*
 * Qpush() --
 * put a buffer on the head of the queue.
 * wakeup any waiting for a queued ibbuf.
 * always called at hi pri (already).
 */
Qpush(qp,bp)
    register struct bufq *qp;
    register struct buf *bp;
{
    dprintf((" Qpush $%x[$%x]",bp,KVADDR(bp)));
    dassert(BFLAGS(bp)&BQ_MINE);
    dassert(!(BFLAGS(bp)&BQ_ONQ));
    dassert(bp->b_dev==NODEV&&bp->b_flags&B_INVAL);
    if( qp->bq_head == 0 )
	qp->bq_tail = bp;
    else
	qp->bq_head ->b_head = bp;
    bp->b_head = 0;
    bp->b_tail = qp->bq_head;
    qp->bq_head = bp;
    if( qp->bq_flags&BQ_WANTED )
    {
	qp->bq_flags &= ~BQ_WANTED;
	WAKEUP(qp);
    }
    BFLAGS(bp) |= BQ_ONQ;
}

/*
 * Qque() --
 * put a buf on the tail of the queue.
 * wakeup any waiting for a queued ibbuf.
 */
Qque(qp,bp)
    register struct bufq *qp;
    register struct buf *bp;
{
USEPRI;

    dprintf((" Qque $%x[$%x]",bp,KVADDR(bp)));
    dassert(BFLAGS(bp)&BQ_MINE);
    dassert(!(BFLAGS(bp)&BQ_ONQ));
    dassert(bp->b_dev==NODEV&&bp->b_flags&B_INVAL);
RAISE;
    if( qp->bq_head == 0 )
	qp->bq_head = bp;
    else
	qp->bq_tail ->b_tail = bp;
    bp->b_tail = 0;
    bp->b_head = qp->bq_tail;
    qp->bq_tail = bp;
    if( qp->bq_flags & BQ_WANTED )
    {
	qp->bq_flags &= ~BQ_WANTED;
	WAKEUP(qp);
    }
    BFLAGS(bp) |= BQ_ONQ;
LOWER;
}

/*
 * Qpop() --
 * take a buf from the head of the queue.
 * wakeup any waiting for a full queue to drain.
 */
struct buf *
Qpop(qp)
    register struct bufq *qp;
{
    register struct buf *bp;
USEPRI;

    dprintf((" Qpop"));
RAISE;
    while( (bp = qp->bq_head) == 0 )
    {
	qp->bq_flags |= BQ_WANTED;
	SLEEP(qp,IBPRI);
    }
    if( (qp->bq_head = bp->b_tail) == 0 )
	qp->bq_tail = 0;
    else
	qp->bq_head ->b_head = 0;
    bp->b_head = bp->b_tail = 0;
    dassert(BFLAGS(bp)&BQ_MINE);
    dassert(BFLAGS(bp)&BQ_ONQ);
    dassert(bp->b_dev==NODEV&&bp->b_flags&B_INVAL);
    BFLAGS(bp) &= ~BQ_ONQ;
LOWER;
    return bp;
}

/*
 * Qdel() --
 * remove a buf from the queue.
 * always called at hi pri (already).
 */
Qdel(qp,bp)
    register struct bufq *qp;
    register struct buf *bp;
{
    dprintf((" Qdel $%x[$%x]",bp,KVADDR(bp)));
    dassert(BFLAGS(bp)&BQ_MINE);
    dassert(BFLAGS(bp)&BQ_ONQ);
    dassert(bp->b_dev==NODEV&&bp->b_flags&B_INVAL);
    if( bp->b_head == 0 )
    {
	dassert(bp==qp->bq_head);
	qp->bq_head = bp->b_tail;
    }
    else
    {
	bp->b_head ->b_tail = bp->b_tail;
    }
    if( bp->b_tail == 0 )
    {
	dassert(bp==qp->bq_tail);
	qp->bq_tail = bp->b_head;
    }
    else
    {
	bp->b_tail ->b_head = bp->b_head;
    }
    bp->b_head = bp->b_tail = 0;

    BFLAGS(bp) &= ~BQ_ONQ;
}

/*
 * Qalloc() --
 * get a buf from the head of the freeq.
 * may return 0, but never hangs.
 * always called at hi pri (already).
 */
struct buf *
Qalloc(dev)
    char dev;
{
    register struct buf *bp;
    register struct ibvars *vp;

    dprintf((" Qalloc"));
    vp = ibconnp[dev]->c_if;
    if( (bp = vp->freeq.bq_head) != 0 )
    {
	/*bp = Qpop(dev);*/
	if( ((&vp->freeq)->bq_head = bp->b_tail) == 0 )
	    (&vp->freeq)->bq_tail = 0;
	else
	    (&vp->freeq)->bq_head ->b_head = 0;
	bp->b_head = bp->b_tail = 0;
	dassert(BFLAGS(bp)&BQ_MINE);
	dassert(BFLAGS(bp)&BQ_ONQ);
	dassert(bp->b_dev==NODEV&&bp->b_flags&B_INVAL);
	BFLAGS(bp) = BQ_MINE;
	DEV(bp) = dev;
	vp->nfreebufs--;
    }
    return bp;
}

/*
 * Qfree() --
 * stash a buf on the tail of the freeq.
 * if the freeq has excess, give it back
 * to the system.
 */
Qfree(bp)
    register struct buf *bp;
{
    register struct ibvars *vp;

    dassert(BFLAGS(bp)&BQ_MINE);
    dassert(!(BFLAGS(bp)&BQ_ONQ));
    dassert(bp->b_dev==NODEV&&bp->b_flags&B_INVAL);
    vp = ibconnp[DEV(bp)]->c_if;

    if( vp->nfreebufs >= vp->maxfreebufs )
    {
	/*Qrelse(bp);*/
	dprintf((" Qfrelse $%x[$%x]", bp, KVADDR(bp)));
# ifdef BDEBUG
	ib_nbufs--;
# endif BDEBUG
	B_RELSE(bp);
    }
    else
    {
USEPRI;
	/*Qque(&vp->freeq,bp);*/
	dprintf((" Qffree $%x[$%x]",bp,KVADDR(bp)));
RAISE;
	if( (&vp->freeq)->bq_head == 0 )
	    (&vp->freeq)->bq_head = bp;
	else
	    (&vp->freeq)->bq_tail ->b_tail = bp;
	bp->b_tail = 0;
	bp->b_head = (&vp->freeq)->bq_tail;
	(&vp->freeq)->bq_tail = bp;
	BFLAGS(bp) = BQ_MINE|BQ_ONQ;
	vp->nfreebufs++;
LOWER;
    }
}

/*
 * Qget() --
 * get a buf from the system.
 * may hang, but never returns 0.
 * (since geteblk() arbitrarily lowers
 * pri, may only be called at low pri).
 */
struct buf *
Qget(dev)
    char dev;
{
    extern struct buf *geteblk();

    register struct buf *bp;

    GET_EBLK(bp);
# ifdef BDEBUG
    ib_nbufs++;
    if( ib_nbufs > ib_maxnbufs )
	ib_maxnbufs = ib_nbufs;
# endif BDEBUG
    dprintf((" Qget $%x[$%x]", bp, KVADDR(bp)));
    DEV(bp) = dev;
    BFLAGS(bp) = BQ_MINE;
    dassert(bp->b_dev==NODEV&&bp->b_flags&B_INVAL);
    bp->b_head = bp->b_tail = 0;

    return bp;
}

/*
 * Qrelse() --
 * return a buf to the system.
 */
Qrelse(bp)
    register struct buf *bp;
{
    dprintf((" Qrelse $%x[$%x]", bp, KVADDR(bp)));
    dassert(BFLAGS(bp)&BQ_MINE);
    dassert(!(BFLAGS(bp)&BQ_ONQ));
    dassert(bp->b_dev==NODEV&&bp->b_flags&B_INVAL);
# ifdef BDEBUG
    ib_nbufs--;
# endif BDEBUG
    B_RELSE(bp);
}

/*
 * Qfreeq() --
 * free all bufs associated with the queue.
 */
Qfreeq(qp)
    register struct bufq *qp;
{
    register struct buf *bp;
USEPRI;

RAISE;
    while( (bp = qp->bq_head) != 0 )
    {
	qp->bq_head = bp->b_tail;
	BFLAGS(bp) &= ~BQ_ONQ;
	Qfree(bp);
    }
    qp->bq_tail = 0;
    qp->bq_flags = 0;
LOWER;
}

/*
 * Qrelseq() --
 * return a string of bufs to the system.
 */
Qrelseq(qp)
    register struct bufq *qp;
{
    register struct buf *bp;
USEPRI;

RAISE;
    while( (bp = qp->bq_head) != 0 )
    {
	qp->bq_head = bp->b_tail;
	BFLAGS(bp) &= ~BQ_ONQ;
	Qrelse(bp);
    }
    qp->bq_tail = 0;
    qp->bq_flags = 0;
LOWER;
}
