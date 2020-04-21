/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)socketvar.h	7.1 (Berkeley) 6/4/86
 */

#ifdef KERNEL
# ifdef SVR3
#  include "sys/fs/com_inode.h"
# endif
# ifdef SVR0
#  include "../com/com_inode.h"
# endif
#else
# ifdef SVR3
#  include <sys/fs/com_inode.h>
# endif
# ifdef SVR0
#  include <sys/com_inode.h>
# endif
#endif

/*
 * Kernel structure per socket.
 * Contains send and receive buffer queues,
 * handle on protocol and pointer to protocol
 * private data and error information.
 */
struct socket {
#ifdef sgi
	struct com_inode so_com;	/* grot for the socket 'filesystem' */
#endif
	short	so_type;		/* generic type, see socket.h */
	short	so_options;		/* from socket call, see socket.h */
	short	so_linger;		/* time to linger while closing */
	short	so_state;		/* internal state flags SS_*, below */
	caddr_t	so_pcb;			/* protocol control block */
	struct	protosw *so_proto;	/* protocol handle */
/*
 * Variables for connection queueing.
 * Socket where accepts occur is so_head in all subsidiary sockets.
 * If so_head is 0, socket is not related to an accept.
 * For head socket so_q0 queues partially completed connections,
 * while so_q is a queue of connections ready to be accepted.
 * If a connection is aborted and it has so_head set, then
 * it has to be pulled out of either so_q0 or so_q.
 * We allow connections to queue up based on current queue lengths
 * and limit on number of queued connections for this socket.
 */
	struct	socket *so_head;	/* back pointer to accept socket */
	struct	socket *so_q0;		/* queue of partial connections */
	short	so_q0len;		/* partials on so_q0 */
	struct	socket *so_q;		/* queue of incoming connections */
	short	so_qlen;		/* number of connections on so_q */
	short	so_qlimit;		/* max number queued connections */
/*
 * Variables for socket buffering.
 */
	struct	sockbuf {
		u_short	sb_cc;		/* actual chars in buffer */
		u_short	sb_hiwat;	/* max actual char count */
#ifndef	sgi
		u_short	sb_mbcnt;	/* chars of mbufs used */
		u_short	sb_mbmax;	/* max chars of mbufs to use */
#endif
		u_short	sb_lowat;	/* low water mark (not used yet) */
		short	sb_timeo;	/* timeout (not used yet) */
		struct	mbuf *sb_mb;	/* the mbuf chain */
		struct	proc *sb_sel;	/* process selecting read/write */
		short	sb_flags;	/* flags, see below */
	} so_rcv, so_snd;
#define	SB_MAX		65535		/* max chars in sockbuf */
#define	SB_LOCK		0x01		/* lock on data queue (so_rcv only) */
#define	SB_WANT		0x02		/* someone is waiting to lock */
#define	SB_WAIT		0x04		/* someone is waiting for data/space */
#define	SB_SEL		0x08		/* buffer is selected */
#define	SB_COLL		0x10		/* collision selecting */
	short	so_timeo;		/* connection timeout */
	u_short	so_error;		/* error affecting connection */
	u_short	so_oobmark;		/* chars to oob mark */
	short	so_pgrp;		/* pgrp for signals */
};

/*
 * Socket state bits.
 */
#define	SS_NOFDREF		0x001	/* no file table ref any more */
#define	SS_ISCONNECTED		0x002	/* socket connected to a peer */
#define	SS_ISCONNECTING		0x004	/* in process of connecting to peer */
#define	SS_ISDISCONNECTING	0x008	/* in process of disconnecting */
#define	SS_CANTSENDMORE		0x010	/* can't send more data to peer */
#define	SS_CANTRCVMORE		0x020	/* can't receive more data from peer */
#define	SS_RCVATMARK		0x040	/* at mark on input */

#define	SS_PRIV			0x080	/* privileged for broadcast, raw... */
#define	SS_NBIO			0x100	/* non-blocking ops */
#define	SS_ASYNC		0x200	/* async i/o notify */
#ifdef	sgi
#define	SS_MAPPEDMBUF		0x400	/* mapped mbuf in use */
#define	SS_MBUFWAIT		0x800	/* waiting for mapped mbuf to vanish*/


/* since sockets live in a pseudo-filesystem, this macro is used to get the
 * 'filesystem-dependent' inode from the independent inode.
 */
#define soc_fsptr(ip) ((struct socket *) (ip)->i_fsptr)
#endif


/*
 * Macros for sockets and socket buffering.
 */

/* how much space is there in a socket buffer (so->so_snd or so->so_rcv) */
#ifdef	sgi
#define	sbspace(sb)	((int)((sb)->sb_hiwat - (sb)->sb_cc))
#else
#define	sbspace(sb) \
    (MIN((int)((sb)->sb_hiwat - (sb)->sb_cc),\
	 (int)((sb)->sb_mbmax - (sb)->sb_mbcnt)))
#endif

/* do we have to send all at once on a socket? */
#define	sosendallatonce(so) \
    ((so)->so_proto->pr_flags & PR_ATOMIC)

/* can we read something from so? */
#define	soreadable(so) \
    ((so)->so_rcv.sb_cc || ((so)->so_state & SS_CANTRCVMORE) || \
	(so)->so_qlen || (so)->so_error)

/* can we write something to so? */
#define	sowriteable(so) \
    (sbspace(&(so)->so_snd) > 0 && \
	(((so)->so_state&SS_ISCONNECTED) || \
	  ((so)->so_proto->pr_flags&PR_CONNREQUIRED)==0) || \
     ((so)->so_state & SS_CANTSENDMORE) || \
     (so)->so_error)

/* adjust counters in sb reflecting allocation of m */
#ifdef	sgi
#define	sballoc(sb, m)	(sb)->sb_cc += (m)->m_len

/* adjust counters in sb reflecting freeing of m */
#define	sbfree(sb, m)	(sb)->sb_cc -= (m)->m_len
#else
#define	sballoc(sb, m) { \
	(sb)->sb_cc += (m)->m_len; \
	(sb)->sb_mbcnt += MSIZE; \
	if ((m)->m_off > MMAXOFF) \
		(sb)->sb_mbcnt += CLBYTES; \
}

/* adjust counters in sb reflecting freeing of m */
#define	sbfree(sb, m) { \
	(sb)->sb_cc -= (m)->m_len; \
	(sb)->sb_mbcnt -= MSIZE; \
	if ((m)->m_off > MMAXOFF) \
		(sb)->sb_mbcnt -= CLBYTES; \
}
#endif

/* set lock on sockbuf sb */
#define sblock(sb) { \
	while ((sb)->sb_flags & SB_LOCK) { \
		(sb)->sb_flags |= SB_WANT; \
		sleep((caddr_t)&(sb)->sb_flags, PZERO+1); \
	} \
	(sb)->sb_flags |= SB_LOCK; \
}

/* release lock on sockbuf sb */
#define	sbunlock(sb) { \
	(sb)->sb_flags &= ~SB_LOCK; \
	if ((sb)->sb_flags & SB_WANT) { \
		(sb)->sb_flags &= ~SB_WANT; \
		wakeup((caddr_t)&(sb)->sb_flags); \
	} \
}

#define	sorwakeup(so)	sowakeup((so), &(so)->so_rcv)
#define	sowwakeup(so)	sowakeup((so), &(so)->so_snd)

#ifdef KERNEL
struct	socket *sonewconn();
#ifdef sgi		/* hack to make S5 copyin/out look like bsd */
#define copyin(from,to,len) bsd_copyin((from),(to),(len))
#define copyout(from,to,len) bsd_copyout((from),(to),(len))
#endif
#endif
