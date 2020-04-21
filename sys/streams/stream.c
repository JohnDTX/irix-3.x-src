/* Copyright 1986, Silicon Graphics Inc., Mountain View, CA. */
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * $Source: /d2/3.7/src/sys/streams/RCS/stream.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:34:51 $
 */

#ifdef mips
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/conf.h"
#include "sys/debug.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/strstat.h"
#include "sys/var.h"

#define PRINTF errprintf

#else
#include "../h/types.h"
#include "../h/param.h"
#include "../h/sysmacros.h"
#include "../h/conf.h"
#include "../streams/stream.h"
#include "../streams/stropts.h"
#include "../streams/strstat.h"

#include "../streams/strcomp.h"

#define PRINTF iprintf
#endif


extern struct stdata streams[];		/* table of streams */
extern queue_t	queue[];		/* table of queues */


/*
 * queue scheduling control variables
 */
char qrunflag;		/* set iff queues are enabled */
char queueflag;		/* set iff inside queuerun() */



/*
 * arrays to map block classes to real size and weighted size,
 * the latter being used for flow control calculations.
 */
ushort rbsize[] = {4, 16, 64, 128, 256, 512, 1024, 2048, 4096 };
/* #define TS(sz) ((sz) - (((*sz)/1024)*1024)/2) */
#define TS(sz) (sz)
static ushort bsize[]  = {
	TS(4), TS(16), TS(64), TS(128), TS(256),
	TS(512), TS(1024), TS(2048), TS(4096) };

struct queue *qhead = 0;		/* head of queues to run */
struct queue **qtailp = &qhead;		/* pointer to tail */

struct	strstat strst;			/* Streams statistics structure */
mblk_t	*mbfreelist;			/* message block freelist */
dblk_t	*dbfreelist[NCLASS];		/* data block freelists */
struct	dbalcst dballoc[NCLASS];	/* data block allocation parameters */
char	strbcflag;			/* bufcall functions ready to go */
struct strevent *sefreelist;		/* stream event cell freelist */

/*
 * Allocate a message and data block.
 * Tries to get a block large enough to hold 'size' bytes.
 * A message block and data block are allocated together, initialized,
 * and a pointer to the message block returned.  Data blocks are
 * always allocated in association with a message block, but there
 * may be several message blocks per data block (see dupb).
 * If no message blocks or data blocks (of the required size)
 * are available, NULL is returned.
 *
 * NOTE: Some code in streamio.c relies on knowledge of the underlying
 * buffer pool structure.
 */

mblk_t *
allocb(size, pri)
register size;
uint pri;
{
	register dblk_t *databp;
	register mblk_t *bp;
	register s;
	register class;

	if ((class = getclass(size)) >= NCLASS) return(NULL);

	s = splstr();

	/*
	 * get buffer - if can't in class then try one class higher
	 */
	if ((dballoc[class].dba_cnt < bcmax(class,pri)) &&
	    (databp = dbfreelist[class]))  goto gotdp;

	strst.dblk[class].fail++;
	strst.dblock.fail++;
	if (class++ == NCLASS-1) {
		splx(s);
		return(NULL);
	}
	if ((dballoc[class].dba_cnt < bcmax(class,pri) ) &&
	    (databp = dbfreelist[class])) goto gotdp;

	splx(s);
	strst.dblk[class].fail++;
	strst.dblock.fail++;
	return(NULL);

gotdp:
	/*
	 * The data buffer is in hand, try for a message block.
	 */
	if (!(bp = mbfreelist)) {
		strst.mblock.fail++;
		splx(s);
		return(NULL);
	}

	dbfreelist[class] = databp->db_freep;
	dballoc[class].dba_cnt++;
	databp->db_freep = NULL;
	mbfreelist = bp->b_next;
	BUMPUP(strst.mblock);
	BUMPUP(strst.dblock);
	BUMPUP(strst.dblk[class]);
	splx(s);

	/*
	 * initialize message block and
	 * data block descriptors
	 */
	bp->b_next = NULL;
	bp->b_prev = NULL;
	bp->b_cont = NULL;
	bp->b_datap = databp;
	bp->b_rptr = databp->db_base;
	bp->b_wptr = databp->db_base;
	databp->db_type = M_DATA;
	/*
	 * set reference count to 1 (first use)
	 */
	ASSERT(databp->db_ref == 0);
	databp->db_ref = 1;
	return(bp);
}


/*
 * test if block of given size can be allocated with a request of
 * the given priority.
 */
int
testb(size, pri)
register size;
uint pri;
{
	register class;

	if ((class = getclass(size)) >= NCLASS) return(0);
	if ((dballoc[class].dba_cnt < bcmax(class,pri)) && dbfreelist[class])
		return(1);

	if (class++ == NCLASS-1) return(0);
	if ((dballoc[class].dba_cnt < bcmax(class,pri) ) && dbfreelist[class])
		return(1);
	return(0);
}


/* Get class of buffer.  Returns NCLASS if size is greater than the
 *	the largest block class.
 * This was far slower & bigger (in generated code) as AT&T wrote it.
 */
int
getclass(size)
register size;
{
	register ushort sz = size;
	register int class;
	register ushort *cp;

	for (class = 0, cp = &rbsize[0];
	     (class < NCLASS) && (sz > *cp);
	     class++, cp++);

	return(class);
}



/*
 * Call function 'func' with argument 'arg' when there is a reasonably
 * good chance that a block of size 'size' can be allocated with
 * priority 'pri'.
 */

bufcall(size, pri, func, arg)
uint size;
uint pri;
int (*func)();
long arg;
{
	register s;
	register class;
	struct strevent *sep, **tmp;
	struct dbalcst *dbp;

	ASSERT( (size >= 0) && (size <= rbsize[NCLASS-1]) );
	class = getclass(size);

	/*
	 * fail bufcall if not enough configured buffers for given
	 * priority allocation to ever succeed.
	 */
	if (bcmax(class, pri) == 0) return(0);	/* if not BPRI_HI */
	if ((pri == BPRI_HI) && (*(&v.v_nblk4096 + (NCLASS - 1 - class)) == 0))
		return(0);

	if (!(sep = sealloc(SE_NOSLP))) {
		PRINTF("bufcall: could not allocate stream event\n");
		return(0);
	}
	dbp = &dballoc[class];
	s = splstr();
	tmp = ( (pri == BPRI_HI) ? &dbp->dba_hip :
		((pri == BPRI_LO) ? &dbp->dba_lop : &dbp->dba_medp) );
	while (*tmp) tmp = &(*tmp)->se_next;
	*tmp = sep;
	sep->se_next = NULL;
	sep->se_func = func;
	sep->se_arg = arg;
	splx(s);
	return(1);
}



/*
 * Free a message block and decrement the reference count on its
 * data block. If reference count == 0 also return the data block.
 */

freeb(bp)
register mblk_t *bp;
{
	register s;
	register class;
	register struct dbalcst *dbp;

	ASSERT(bp);

	class = bp->b_datap->db_class;
	dbp = &dballoc[class];
	s = splstr();
	ASSERT(bp->b_datap->db_ref != 0);
	if (--bp->b_datap->db_ref == 0) {
		bp->b_datap->db_freep = dbfreelist[class];
		dbfreelist[class] = bp->b_datap;
		dbp->dba_cnt--;
		if (!strbcflag && ( dbp->dba_hip ||
		    (dbp->dba_medp && (dbp->dba_cnt < dbp->dba_med)) ||
		    (dbp->dba_lop && (dbp->dba_cnt < dbp->dba_lo)))) {
			setqsched();
			strbcflag = 1;
		}
		BUMPDOWN(strst.dblock);
		BUMPDOWN(strst.dblk[class]);
	}
	bp->b_datap = NULL;
	bp->b_next = mbfreelist;
	mbfreelist = bp;
	BUMPDOWN(strst.mblock);
	splx(s);
	return;
}



/*
 * Free all message blocks in a message using freeb().
 * The message may be NULL.
 */

freemsg(bp)
register mblk_t *bp;
{
	register mblk_t *tp;

	while (bp) {
		tp = bp->b_cont;
		freeb(bp);
		bp = tp;
	}
}


/*
 * Duplicate a message block
 *
 * Allocate a message block and assign proper
 * values to it (read and write pointers)
 * and link it to existing data block.
 * Increment reference count of data block.
 */

mblk_t *
dupb(bp)
register mblk_t *bp;
{
	register s;
	register mblk_t *nbp;

	ASSERT(bp);

	s = splstr();
	if (!(nbp = mbfreelist)) {
		splx(s);
		strst.mblock.fail++;
		return(NULL);
	}
	mbfreelist = nbp->b_next;
	splx(s);
	BUMPUP(strst.mblock);

	nbp->b_next = NULL;
	nbp->b_prev = NULL;
	nbp->b_cont = NULL;
	nbp->b_rptr = bp->b_rptr;
	nbp->b_wptr = bp->b_wptr;
	(nbp->b_datap = bp->b_datap)->db_ref++;
	ASSERT(bp->b_datap->db_ref != 0);
	return(nbp);
}


/*
 * Duplicate a message block by block (uses dupb), returning
 * a pointer to the duplicate message.
 * Returns a non-NULL value only if the entire message
 * was dup'd.
 */

mblk_t *
dupmsg(bp)
register mblk_t *bp;
{
	register mblk_t *head, *nbp;

	if (!bp || !(nbp = head = dupb(bp))) return(NULL);

	while (bp->b_cont) {
		if (!(nbp->b_cont = dupb(bp->b_cont))) {
			freemsg(head);
			return(NULL);
		}
		nbp = nbp->b_cont;
		bp = bp->b_cont;
	}
	return(head);
}



/*
 * copies data from message block to newly allocated message block and
 * data block.  The copy is rounded out to full word boundaries so that
 * the (usually) more efficient word copy can be done.
 * Returns new message block pointer, or  NULL if error.
 */

mblk_t *
copyb(bp)
register mblk_t *bp;
{
	register mblk_t *nbp;
	register dblk_t *dp, *ndp;
	caddr_t base;

	ASSERT(bp);
	ASSERT(bp->b_wptr >= bp->b_rptr);

	dp = bp->b_datap;
	if (!(nbp = allocb(dp->db_lim - dp->db_base, BPRI_MED)))
		return(NULL);
	ndp = nbp->b_datap;
	ndp->db_type = dp->db_type;
	nbp->b_rptr = ndp->db_base + (bp->b_rptr - dp->db_base);
	nbp->b_wptr = ndp->db_base + (bp->b_wptr - dp->db_base);
	base = straln(nbp->b_rptr);
	strbcpy(straln(bp->b_rptr), base, straln(nbp->b_wptr + (sizeof(int)-1)) - base);
	return(nbp);
}


/*
 * copies data from message to newly allocated message using new
 * data blocks.
 * returns pointer to new message.
 * NULL if error.
 */

mblk_t *
copymsg(bp)
register mblk_t *bp;
{
	register mblk_t *head, *nbp;

	if (!bp || !(nbp = head = copyb(bp))) return(NULL);

	while (bp->b_cont) {
		if (!(nbp->b_cont = copyb(bp->b_cont))) {
			freemsg(head);
			return(NULL);
		}
		nbp = nbp->b_cont;
		bp = bp->b_cont;
	}
	return(head);
}




/*
 * link a message block to tail of message
 */

linkb(mp, bp)
register mblk_t *mp;
register mblk_t *bp;
{
	ASSERT(mp && bp);

	for (; mp->b_cont; mp = mp->b_cont);
	mp->b_cont = bp;
}


/*
 * unlink a message block from head of message
 * return pointer to new message.
 * NULL if message becomes empty.
 */

mblk_t *
unlinkb(bp)
register mblk_t *bp;
{
	register mblk_t *bp1;

	ASSERT(bp);

	bp1 = bp->b_cont;
	bp->b_cont = NULL;
	return(bp1);
}


/*
 * remove a message block "bp" from message "mp"
 *
 * Return pointer to new message or NULL if no message remains.
 * Return -1 if bp is not found in message.
 */

mblk_t *
rmvb(mp,bp)
register mblk_t *mp;
register mblk_t *bp;
{
	register mblk_t *tmp;
	register mblk_t *lastp = NULL;


	for (tmp = mp; tmp; tmp = tmp->b_cont) {
		if (tmp == bp) {
			if (lastp) lastp->b_cont = tmp->b_cont;
			else mp = tmp->b_cont;
			tmp->b_cont = NULL;
			return(mp);
		}
		lastp = tmp;
	}
	return((mblk_t *)-1);
}



/*
 * macro to check pointer alignment
 * (true if alignment is sufficient for worst case)
 */

#ifdef u3b2
#define str_aligned(X)	(((uint)(X) & 03) == 0)
#else
#define str_aligned(X)	(((uint)(X) & (sizeof(int) - 1)) == 0)
#endif

/*
 * Concatenate and align first len bytes of common
 * message type.  Len == -1, means concat everything.
 * Returns 1 on success, 0 on failure
 * After the pullup, mp points to the pulled up data.
 * This is convenient but messy to implement.
 */
int
pullupmsg(mp, len)
mblk_t *mp;
register len;
{
	register mblk_t *bp;
	register mblk_t *new_bp;
	register n;
	mblk_t *tmp;
	int s;

	ASSERT(mp != NULL);

	/*
	 * Quick checks for success or failure:
	 */
	if (len == -1) {
		if (mp->b_cont == NULL && str_aligned(mp->b_rptr))
			return(1);
		len = xmsgsize(mp);
	} else {
		ASSERT(mp->b_wptr >= mp->b_rptr);
		if (mp->b_wptr - mp->b_rptr >= len && str_aligned(mp->b_rptr))
			return(1);
		if (xmsgsize(mp) < len)
			return(0);
	}

	/*
	 * Allocate a new mblk header.  It is used later to interchange
	 * mp and new_bp.
	 */
	s = splstr();
	if ((tmp = mbfreelist) == NULL) {
		splx(s);
		return(0);
	}
	mbfreelist = tmp->b_next;
	splx(s);

	/*
	 * Allocate the new mblk.  We might be able to use the existing
	 * mblk, but we don't want to modify it in case its shared.
	 * The new dblk takes on the type of the old dblk
	 */
	if ((new_bp = allocb(len, BPRI_MED)) == NULL) {
		s = splstr();
		tmp->b_next = mbfreelist;
		mbfreelist = tmp;
		splx(s);
		return(0);
	}
	new_bp->b_datap->db_type = mp->b_datap->db_type;

	/*
	 * Scan mblks and copy over data into the new mblk.
	 * Two ways to fall out: exact count match: while (len)
	 * Bp points to the next mblk containing data or is null.
	 * Inexact match: if (bp->b_rptr != ...)  In this case,
	 * bp points to an mblk that still has data in it.
	 */
	bp = mp;
	while (len) {
		mblk_t *b_cont;

		ASSERT(bp->b_wptr >= bp->b_rptr);
		n = min(bp->b_wptr - bp->b_rptr, len);
		bcopy(bp->b_rptr, new_bp->b_wptr, n);
		new_bp->b_wptr += n;
		bp->b_rptr += n;
		len -= n;
		if (bp->b_rptr != bp->b_wptr)
			break;
		b_cont = bp->b_cont;
		if (bp != mp)	/* don't free the head mblk */
			freeb(bp);
		bp = b_cont;
	}

	/*
	 * At this point:  new_bp points to a dblk that
	 * contains the pulled up data.  The head mblk, mp, is
	 * preserved and may or may not have data in it.  The
	 * intermediate mblks are freed, and bp points to the
	 * last mblk that was pulled-up or is null.
	 *
	 * Now the tricky bit.  After this, mp points to the new dblk
	 * and tmp points to the old dblk.  New_bp points nowhere
	 */
	*tmp = *mp;
	*mp = *new_bp;
	new_bp->b_datap = NULL;

	/*
	 * If the head mblk (now tmp) still has data in it, link it to mp
	 * otherwise link the remaining mblks to mp and free the
	 * old head mblk.
	 */
	if (tmp->b_rptr != tmp->b_wptr)
		mp->b_cont = tmp;
	else {
		mp->b_cont = bp;
		freeb(tmp);
	}

	/*
	 * Free new_bp
	 */
	s = splstr();
	new_bp->b_next = mbfreelist;
	mbfreelist = new_bp;
	splx(s);

	return(1);
}

/*
 * Trim bytes from message
 *  len > 0, trim from head
 *  len < 0, trim from tail
 * Returns 1 on success, 0 on failure
 */
int
adjmsg(mp, len)
mblk_t *mp;
register int len;
{
	register mblk_t *bp;
	register n;
	int fromhead;

	ASSERT(mp != NULL);

	fromhead = 1;
	if (len < 0) {
		fromhead = 0;
		len = -len;
	}
	if (xmsgsize(mp) < len)
		return(0);

	if (fromhead) {
		bp = mp;
		while (len) {
			ASSERT(bp->b_wptr >= bp->b_rptr);
			n = min(bp->b_wptr - bp->b_rptr, len);
			bp->b_rptr += n;
			len -= n;
			bp = bp->b_cont;
		}
	} else {
		register mblk_t *save_bp;
		register unsigned char type;

		type = mp->b_datap->db_type;
		while (len) {
			bp = mp;
			while (bp && bp->b_datap->db_type == type) {
				ASSERT(bp->b_wptr >= bp->b_rptr);
				if (bp->b_wptr - bp->b_rptr > 0)
					save_bp = bp;
				bp = bp->b_cont;
			}
			n = min(save_bp->b_wptr - save_bp->b_rptr, len);
			save_bp->b_wptr -= n;
			len -= n;
		}
	}
	return(1);
}

/*
 * Return size of message of block type (bp->b_datap->db_type)
 */
int
xmsgsize(bp)
register mblk_t *bp;
{
	register unsigned char type;
	register count = 0;

	type = bp->b_datap->db_type;

	for (; bp; bp = bp->b_cont) {
		if (type != bp->b_datap->db_type)
			break;
		ASSERT(bp->b_wptr >= bp->b_rptr);
		count += bp->b_wptr - bp->b_rptr;
	}
	return(count);
}


/*
 * get number of data bytes in message
 */
int
msgdsize(bp)
register mblk_t *bp;
{
	register int count = 0;

	for (; bp; bp = bp->b_cont)
		if (bp->b_datap->db_type == M_DATA) {
			ASSERT(bp->b_wptr >= bp->b_rptr);
			count += bp->b_wptr - bp->b_rptr;
		}
	return(count);
}


/*
 * Get a message off head of queue
 *
 * If queue has no buffers then mark queue
 * with QWANTR. (queue wants to be read by
 * someone when data becomes available)
 *
 * If there is something to take off then do so.
 * If queue falls below hi water mark turn off QFULL
 * flag.  Decrement weighted count of queue.
 * Also turn off QWANTR because queue is being read.
 *
 * If queue count is below the lo water mark and QWANTW
 * is set, enable the closest backq which has a service
 * procedure and turn off the QWANTW flag.
 */

mblk_t *
getq(q)
register queue_t *q;
{
	register mblk_t *bp;
	register mblk_t *tmp;
	register s;

	ASSERT(q);

	s = splstr();
	if (!(bp = q->q_first)) q->q_flag |= QWANTR;
	else {
		if (!(q->q_first = bp->b_next))	q->q_last = NULL;
		else q->q_first->b_prev = NULL;
		for (tmp = bp; tmp; tmp = tmp->b_cont)
			q->q_count -= bsize[tmp->b_datap->db_class];
		if (q->q_count < q->q_hiwat)
			q->q_flag &= ~QFULL;
		q->q_flag &= ~QWANTR;
		bp->b_next = bp->b_prev = NULL;
	}

	str_backen(q);
	splx(s);
	return(bp);
}


/*
 * Remove a message from a queue.  The queue count and other
 * flow control parameters are adjusted and the back queue
 * enabled if necessary.
 */

rmvq(q, mp)
register queue_t *q;
register mblk_t *mp;
{
	register s;
	register mblk_t *tmp;

	ASSERT(q);
	ASSERT(mp);

	s = splstr();

	if (mp->b_prev) mp->b_prev->b_next = mp->b_next;
	else q->q_first = mp->b_next;

	if (mp->b_next) mp->b_next->b_prev = mp->b_prev;
	else q->q_last = mp->b_prev;

	mp->b_next = mp->b_prev = NULL;

	for (tmp = mp; tmp; tmp = tmp->b_cont)
		q->q_count -= bsize[tmp->b_datap->db_class];

	if (q->q_count < q->q_hiwat) q->q_flag &= ~QFULL;

	str_backen(q);
	splx(s);
}

/*
 * Empty a queue.
 * If flag is set, remove all messages.  Otherwise, remove
 * only non-control messages.  If queue falls below its low
 * water mark, and QWANTW was set before the flush, enable
 * the nearest upstream service procedure.
 */

flushq(q, flag)
register queue_t *q;
{
	register mblk_t *bp, *nbp;
	register int s;

	ASSERT(q);

	s = splstr();

	bp = q->q_first;
	q->q_first = NULL;
	q->q_last = NULL;
	q->q_count = 0;
	q->q_flag &= ~QFULL;
	splx(s);
	while (bp) {
		nbp = bp->b_next;
		if (!flag && !datamsg(bp->b_datap->db_type))
			putq(q, bp);
		else
			freemsg(bp);
		bp = nbp;
	}

	str_backen(q);
}


/*
 * Return 1 if the queue is not full.  If the queue is full, return
 * 0 (may not put message) and set QWANTW flag (caller wants to write
 * to the queue).
 */
int
canput(q)
register queue_t *q;
{
	if (!q) return(0);
	while (q->q_next && !q->q_qinfo->qi_srvp) q = q->q_next;
	if (q->q_flag & QFULL) {
		register int s;
		s = splstr();		/* can be called from anywhere */
		q->q_flag |= QWANTW;
		splx(s);
		return(0);
	}
	return(1);
}


/* The ATT code made these macros.
 *	That is a disaster if you have interrupt functions that need to use
 *	them, or if your interrupt functions need to do put's on the queues
 *	of modules, and your modules to do all run at splstr()
 *
 *	Perhaps they should be macros, but they'd be a mess, because they
 *	would need a temporary for the spl().
 */
noenable(q)
register queue_t *q;
{
	register int s;

	s = splstr();
	q->q_flag |= QNOENB;
	splx(s);
}

enableok(q)
register queue_t *q;
{
	register int s;

	s = splstr();
	q->q_flag &= ~QNOENB; 
	splx(s);
}



/*
 * Put a message on a queue.
 *
 * Messages are enqueued on a priority basis.  The priority classes
 * are PRIORITY (type >= QPCTL) and NORMAL (type < QPCTL).
 *
 * Add appropriate weighted data block sizes to queue count.
 * If queue hits high water mark then set QFULL flag.
 *
 * If QNOENAB is not set (putq is allowed to enable the queue),
 * enable the queue only if the message is PRIORITY,
 * or the QWANTR flag is set (indicating that the service procedure
 * is ready to read the queue.  This implies that a service
 * procedure must NEVER put a priority message back on its own
 * queue, as this would result in an infinite loop (!).
 */

putq(q, bp)
register queue_t *q;
register mblk_t *bp;
{
	register s;
	register mblk_t *tmp;
	register mcls = queclass(bp);

	ASSERT(q && bp);

	s = splstr();

	/*
	 * If queue is empty or queue class of message is less than
	 * that of the last one on the queue, tack on to the end.
	 */
	if ( !q->q_first || (mcls <= queclass(q->q_last)) ){
		if (q->q_first) {
			q->q_last->b_next = bp;
			bp->b_prev = q->q_last;
		} else {
			q->q_first = bp;
			bp->b_prev = NULL;
		}
		bp->b_next = NULL;
		q->q_last = bp;

	} else {
		register mblk_t *nbp = q->q_first;

		while (queclass(nbp) >= mcls) nbp = nbp->b_next;
		bp->b_next = nbp;
		bp->b_prev = nbp->b_prev;
		if (nbp->b_prev) nbp->b_prev->b_next = bp;
		else q->q_first = bp;
		nbp->b_prev = bp;
	}

	for (tmp = bp; tmp; tmp = tmp->b_cont)
		q->q_count += bsize[tmp->b_datap->db_class];
	if (q->q_count >= q->q_hiwat) q->q_flag |= QFULL;

	if ( (mcls > QNORM) ||
	     (canenable(q) && (q->q_flag & QWANTR)) )
		qenable(q);

	splx(s);
}


/*
 * Put stuff back at beginning of Q according to priority order.
 * See comment on putq above for details.
 */

putbq(q, bp)
register queue_t *q;
register mblk_t *bp;
{
	register s;
	register mblk_t *tmp;
	register mcls = queclass(bp);

	ASSERT(q && bp);
	ASSERT(bp->b_next == NULL);

	s = splstr();

	/*
	 * If queue is empty of queue class of message >= that of the
	 * first message, place on the front of the queue.
	 */
	if ( !q->q_first || (mcls >= queclass(q->q_first))) {
		bp->b_next = q->q_first;
		bp->b_prev = NULL;
		if (q->q_first) q->q_first->b_prev = bp;
		else q->q_last = bp;
		q->q_first = bp;
	}
	else {
		register mblk_t *nbp = q->q_first;

		while ((nbp->b_next) && (queclass(nbp->b_next) > mcls))
				nbp = nbp->b_next;

		if (bp->b_next = nbp->b_next)
			nbp->b_next->b_prev = bp;
		else
			q->q_last = bp;
		nbp->b_next = bp;
		bp->b_prev = nbp;
	}

	for (tmp = bp; tmp; tmp = tmp->b_cont)
		q->q_count += bsize[tmp->b_datap->db_class];
	if (q->q_count >= q->q_hiwat) q->q_flag |= QFULL;

	if ( (mcls > QNORM) ||
	     (canenable(q) && q->q_flag & QWANTR) )
		qenable(q);

	splx(s);
}


/*
 * Insert a message before an existing message on the queue.  If the
 * existing message is NULL, the new messages is placed on the end of
 * the queue.  The queue class of the new message is ignored.
 * All flow control parameters are updated.
 */

insq(q, emp, mp)
register queue_t *q;
register mblk_t *emp, *mp;
{
	register mblk_t *tmp;

	if (mp->b_next = emp) {
		if (mp->b_prev = emp->b_prev)
			emp->b_prev->b_next = mp;
		else
			q->q_first = mp;
		emp->b_prev = mp;
	} else {
		if (mp->b_prev = q->q_last)
			q->q_last->b_next = mp;
		else
			q->q_first = mp;
		q->q_last = mp;
	}

	for (tmp = mp; tmp; tmp = tmp->b_cont)
		q->q_count += bsize[tmp->b_datap->db_class];

	if (q->q_count >= q->q_hiwat) q->q_flag |= QFULL;

	if (canenable(q) && (q->q_flag & QWANTR)) qenable(q);
}


/*
 * Create and put a control message on queue.
 */
int
putctl(q, type)
queue_t *q;
{
	register mblk_t *bp;

	if (datamsg(type) || !(bp = allocb(0, BPRI_HI)))
		return(0);
	bp->b_datap->db_type = type;
	(*q->q_qinfo->qi_putp)(q, bp);
	return(1);
}



/*
 * Control message with a single-byte parameter
 */
int
putctl1(q, type, param)
queue_t *q;
{
	register mblk_t *bp;

	if (datamsg(type) ||!(bp = allocb(1, BPRI_HI)))
		return(0);
	bp->b_datap->db_type = type;
	*bp->b_wptr++ = param;
	(*q->q_qinfo->qi_putp)(q, bp);
	return(1);
}



/*
 * Init routine run from main at boot time.  This contains some
 * machine dependent code.
 */

strinit()
{
	register dblk_t *databp;
	register mblk_t *msgbp;
	register i;
	register size;
	register unsigned char *base;
	int class, *vnp;
#ifdef mips
#define STRLOFRAC strlofrac
	extern char strlofrac;
#define STRMEDFRAC strmedfrac
	extern char strmedfrac;
#endif

	/*
	 * Set up initial stream event cell free list.  sealloc()
	 * may allocate new cells for the free list if the initial list is
	 * exhausted.
	 */
	sefreelist = NULL;
	for (i=0; i < nstrevent; i++) {
		strevent[i].se_next = sefreelist;
		sefreelist = &strevent[i];
	}


	/*
	 * Allocate space for streams buffers.
	 */

	size = (v.v_nblk4096*4096 + v.v_nblk2048*2048 +
		v.v_nblk1024*1024 + v.v_nblk512*512 + v.v_nblk256*256 +
		v.v_nblk128*128 + v.v_nblk64*64 + v.v_nblk16*16 +
		v.v_nblk4*4);

	/*
	 * XXX There should be a much fancier allocation scheme.
	 */
	if (size == 0
	    || !(base = (unsigned char *)malloc(size))) {
		PRINTF("strinit: cannot allocate stream data blocks\n");
		return;
        }

	/*
	 * Initialize buffers space and set up datablock freelists
	 */
	databp = &dblock[0];
	for (class = NCLASS-1, vnp = &v.v_nblk4096;
	     class >=0; class--, vnp++) {
		dbfreelist[class] = NULL;
		dballoc[class].dba_lo = ((*vnp)*STRLOFRAC+99)/100;
		dballoc[class].dba_med = ((*vnp)*STRMEDFRAC+99)/100;
		for (i=0; i<*vnp; i++, databp++) {
			databp->db_class = class;
			databp->db_base = base;
			databp->db_lim = base + rbsize[class];
			base += rbsize[class];
			databp->db_freep = dbfreelist[class];
			dbfreelist[class] = databp;
		}
	}

	/*
	 * set up of message block freelist.
	 */
	mbfreelist = NULL;
	for (i=0; i<nmblock; i++) {
		msgbp = &mblock[i];
		msgbp->b_next = mbfreelist;
		mbfreelist = msgbp;
	}
}



/*
 * allocate a pair of queues
 */
queue_t *
allocq()
{
	register queue_t *qp;

	qp = (queue_t*)malloc(2*sizeof(queue_t));
	if (!qp) {
		PRINTF("allocq: out of queues\n");
		strst.queue.fail++;
	} else {
		bzero((caddr_t)qp, 2*sizeof(*qp));
		qp->q_flag = QREADR;
		BUMPUP(strst.queue);
	}

	return qp;
}


/*
 * free a queue pair
 */
freeq(q)
queue_t *q;
{
	ASSERT(q->q_flag & QREADR);

	free((char*)q);
	BUMPDOWN(strst.queue);
}



/*
 * return the queue upstream from this one
 */

queue_t *
backq(q)
register queue_t *q;
{
	ASSERT(q);

	q = OTHERQ(q);
	if (q->q_next) {
		q = q->q_next;
		return(OTHERQ(q));
	}
	return(NULL);
}



/*
 * Send a block back up the queue in reverse from this
 * one (e.g. to respond to ioctls)
 */

qreply(q, bp)
register queue_t *q;
mblk_t *bp;
{
	ASSERT(q && bp);

	q = OTHERQ(q);
	ASSERT(q->q_next);
	(*q->q_next->q_qinfo->qi_putp)(q->q_next, bp);
}



/*
 * Streams Queue Scheduling
 *
 * Queues are enabled through qenable() when they have messages to
 * process.  They are serviced by queuerun(), which runs each enabled
 * queue's service procedure.  The call to queuerun() is processor
 * dependent - the general principle is that it be run whenever a queue
 * is enabled but before returning to user level.  For system calls,
 * the function runqueues() is called if their action causes a queue
 * to be enabled.  For device interrupts, queuerun() should be
 * called before returning from the last level of interrupt.  Beyond
 * this, no timing assumptions should be made about queue scheduling.
 */


/*
 * Enable a queue: put it on list of those whose service procedures are
 * ready to run and set up the scheduling mechanism.
 */

qenable(q)
register queue_t *q;
{
	register s;

	ASSERT(q);

	if (!q->q_qinfo->qi_srvp) return;

	s = splstr();
	ASSERT(0 != q->q_ptr);		/* it better be a real queue */
	/*
	 * Do not place on run queue if already enabled.
	 */
	if (q->q_flag & QENAB) {
		splx(s);
		return;
	}

	/*
	 * mark queue enabled and place on run list
	 */
	q->q_flag |= QENAB;

	*qtailp = q;
	q->q_link = NULL;
	qtailp = &q->q_link;

	/*
	 * set up scheduling mechanism
	 */
	setqsched();
	splx(s);
}



/* Run the service procedures of each enabled queue
 *
 * Called by service mechanism (processor dependent) if there
 * are queues to run.  The mechanism is reset.
 */

queuerun()
{
	register queue_t *q;
	register s;
	register i;
	register int *vnp;
	struct strevent *sep;
	int count, total;
	struct dbalcst *dbp;

	s = splstr();
	do {
		if (strbcflag) {
			strbcflag = 0;
			for (i=NCLASS-1, vnp = &v.v_nblk4096;
			     i>=0; i--, vnp++){
				dbp = &dballoc[i];
				count = *vnp - dbp->dba_cnt;
				total = 0;
				while ( mbfreelist &&
					(count > 0) && (sep = dbp->dba_hip)) {
					count--;
					total++;
					dbp->dba_hip = sep->se_next;
					(*sep->se_func)(sep->se_arg);
					sefree(sep);
				}
				count = (dbp->dba_med - dbp->dba_cnt) - total;
				while ( mbfreelist
					&& (count > 0)
					&& (sep = dbp->dba_medp)) {
					count--;
					total++;
					dbp->dba_medp = sep->se_next;
					(*sep->se_func)(sep->se_arg);
					sefree(sep);
				}
				count = (dbp->dba_lo - dbp->dba_cnt) - total;
				while ( mbfreelist &&
					(count > 0) && (sep = dbp->dba_lop)) {
					count--;
					dbp->dba_lop = sep->se_next;
					(*sep->se_func)(sep->se_arg);
					sefree(sep);
				}
			}
		}

		while (q = qhead) {
			ASSERT(q->q_flag & QENAB);
			ASSERT(0 != q->q_ptr);
			if (!(qhead = q->q_link))
				qtailp = &qhead;
			q->q_flag &= ~QENAB;
			if (q->q_qinfo->qi_srvp) {
				splx(s);
				(*q->q_qinfo->qi_srvp)(q);
				(void)splstr();
			}
		}
	} while (strbcflag);

	qrunflag = 0;
	splx(s);
}

#ifndef mips
/*
 * Function to kick off queue scheduling for those system calls
 * that cause queues to be enabled (read, recv, write, send, ioctl).
 */

runqueues()
{
	register s;

	s = splhi();
	if (qrunflag && !queueflag) {
		queueflag = 1;
		splx(s);
		queuerun();
		s = splhi();
		queueflag = 0;
	}
	splx(s);
}
#endif


/*
 * find module
 *
 * return index into fmodsw
 * or -1 if not found
 */
int
findmod(name)
register char *name;
{
	register int i, j;

	for (i = 0; i < fmodcnt; i++)
		for (j = 0; j < FMNAMESZ + 1; j++) {
			if (fmodsw[i].f_name[j] != name[j])
				break;
			if (name[j] == '\0')
				return(i);
		}
	return(-1);
}



/*
 * return number of messages on queue
 */
int
qsize(qp)
register queue_t *qp;
{
	register count = 0;
	register mblk_t *mp;

	ASSERT(qp);

	for (mp = qp->q_first; mp; mp = mp->b_next)
		count++;

	return(count);
}


/*
 * allocate a stream event cell
 */

struct strevent *
sealloc(slpflag)
int slpflag;
{
	register s;
	register struct strevent *sep;
	register int i;
	static sepgcnt = 0;
#define NEW_SE 20

retry:
	s = splstr();
	if (sefreelist) {
		sep = sefreelist;
		sefreelist = sep->se_next;
		splx(s);
		sep->se_procp = NULL;
		sep->se_events = 0;
		sep->se_next = NULL;
		return(sep);
	}

	if (slpflag == SE_NOSLP		/* make some more if we may */
	    || sepgcnt >= maxsepgcnt
	    || !(sep = (struct strevent*)malloc(sizeof(*sep) * NEW_SE))) {
		splx(s);
		return(NULL);
	}

	sepgcnt++;
	for (i = 0; i < NEW_SE; i++, sep++) {
		sep->se_next = sefreelist;
		sefreelist = sep;
	}

	splx(s);
	goto retry;
}


/*
 * free a stream event cell
 */

sefree(sep)
register struct strevent *sep;
{
	register s;

	s = splstr();
	sep->se_next = sefreelist;
	sefreelist = sep;
	splx(s);
}


/*
 * 'back-enable' when things have drained
 */
str_backen(q)
register queue_t *q;
{
	register int s = splstr();

	if (q->q_count <= q->q_lowat && (q->q_flag&QWANTW)) {
		q->q_flag &= ~QWANTW;

		/* find nearest back queue with service proc */
		q = backq(q);
		while (q) {
			if (q->q_qinfo->qi_srvp) {
				qenable(q);
				break;
			}
			q = backq(q);
		}
	}
	splx(s);
}



/* concatenate two messages
 */
str_conmsg(mpp,mep,new)
register mblk_t **mpp;			/* append to this (maybe 0) */
register mblk_t **mep;			/* which ends here */
register mblk_t *new;			/* this message */
{
	register mblk_t *bp;
	register int s = splstr();

	if (!*mpp)
		*mpp = new;
	else
		(*mep)->b_cont = new;

	do {				/* find end of combination */
		bp = new;
		new = new->b_cont;
	} while (NULL != new);

	*mep = bp;
	splx(s);
}
