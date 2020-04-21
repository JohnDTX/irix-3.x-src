/*
 * Lightweight process data structures.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/h/RCS/lw.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:46:59 $
 */
#ifndef	___LW__
#define	___LW__

#include "sys/types.h"
#ifndef	_JBLEN
#ifdef mips
#include "setjmp.h"
#else
#include "sys/setjmp.h"
#endif
#endif

/*
 * Queue linkage for circular process queues
 */
typedef	struct	lwqueue {
	struct	lwqueue	*pred;		/* pointer to predecessor */
	struct	lwqueue	*succ;		/* pointer to successor */
} lwqueue;

/*
 * Layout of a message
 */
typedef	struct	lwmsg {
	long	m_type;			/* type of message */
	long	m_value;		/* value of message */
	struct	lwmsg *m_next;		/* next message in msg queue */
	char	*m_data;		/* other data than value */
} lwmsg;

/*
 * Registration block for handling message targeting.
 */
typedef	struct	lwregblk {
	lwqueue	qlink;			/* linkage for hash lists */
	long	r_type;			/* type of entry */
	long	r_value;		/* value */
	int	r_checkvalue;		/* non-zero if value is checked */
	struct	lwproc *r_proc;		/* process registered */
} lwregblk;

/*
 * Lightweight process definition.
 */
typedef	struct	lwproc {
	lwqueue	qlink;			/* queue linkage */
	char	*lw_name;		/* name of the process */
	short	lw_flags;		/* random state flags */
	char	*lw_event;		/* event proc is waiting for */
	char	*lw_stack;		/* pointer to stack memory */
	lwmsg	*lw_queue;		/* head of message queue */
	jmp_buf	lw_pcb;			/* used for context switching */
} lwproc;

/* lw_flags */
#define	LW_QWAIT	0x0001		/* waiting for message */

#if defined(IP2) || defined(PM2)
/*
 * Set the pc in the pcb
 */
#define	SETPC(pcb, pc)	(pcb)[0] = ((int) (pc))

/*
 * Set the sp in the pcb
 */
#define	SETSP(pcb, sp)	(pcb)[12] = ((int) (sp))
#endif

/*
 * Append the given process to the end of the given circular queue.
 */
#define	LWQUEUE(q, p) \
{ \
	ASSERT((q)->pred); \
	ASSERT((q)->succ); \
	(p)->qlink.succ = (q); \
	(p)->qlink.pred = (q)->pred; \
	(p)->qlink.pred->succ = &(p)->qlink; \
	(q)->pred = &(p)->qlink; \
}

/*
 * Remove the head item off the given queue.  If the queue is empty,
 * return the queue pointer.
 */
#define	LWDEQUEUE(q, p) \
{ \
	(p) = (lwproc *) ((q)->succ); \
	LWREMOVE(p); \
}

/*
 * Remove the given thing off the queue its on.
 */
#define	LWREMOVE(p) \
{ \
	ASSERT((p)->qlink.pred); \
	ASSERT((p)->qlink.succ); \
	ASSERT((p)->qlink.pred != &(p)->qlink); \
	ASSERT((p)->qlink.succ != &(p)->qlink); \
	(p)->qlink.succ->pred = (p)->qlink.pred; \
	(p)->qlink.pred->succ = (p)->qlink.succ; \
	(p)->qlink.succ = 0; \
	(p)->qlink.pred = 0; \
}

/*
 * Initialize a circular queue
 */
#define	LWQINIT(q) \
{ \
	(q)->pred = (q); \
	(q)->succ = (q); \
}

/* default stack size */
#define	LWSTACKSIZE	2048

/* exported procedures */
extern	lwproc	*lwstart();
extern	int	lwexit();
extern	lwmsg	*lwnewmsg();
extern	lwmsg	*lwgetmsg();
extern	void	lwputmsg();
extern	void	lwsendmsg();

#endif	/* ___LW__ */
