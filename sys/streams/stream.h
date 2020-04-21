/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

/*
 * $Header: /d2/3.7/src/sys/streams/RCS/stream.h,v 1.1 89/03/27 17:34:53 root Exp $
 */

#ifdef SVR3				/* Clover has everything */
#define NUSTRM 1			/* but BSD configuration facilities */

#ifdef KERNEL
#define free kern_free			/* malloc() is different in S5R3 */
#define malloc kern_malloc
#define calloc kern_calloc
#endif KERNEL
#endif SVR3


/*
 * data queue
 */
struct	queue {
	struct	qinit	*q_qinfo;	/* procs and limits for queue */
	struct	msgb	*q_first;	/* first data block */
	struct	msgb	*q_last;	/* last data block */
	struct	queue	*q_next;	/* Q of next stream */
	struct	queue	*q_link;	/* to next Q for scheduling */
	caddr_t	q_ptr;			/* to private data structure */
	ushort	q_count;		/* number of blocks on Q */
	ushort	q_flag;			/* queue state */
	short q_minpsz;			/* min packet size accepted by this module */
	short q_maxpsz;			/* max packet size accepted by this module */
	ushort q_hiwat;			/* queue high water mark */
	ushort q_lowat;			/* queue low water mark */
};

typedef struct queue queue_t;

/*
 * Queue flags
 */
#define	QENAB	01			/* Queue is already enabled to run */
#define	QWANTR	02			/* Someone wants to read Q */
#define	QWANTW	04			/* Someone wants to write Q */
#define	QFULL	010			/* Q is considered full */
#define	QREADR	020			/* This is the reader (first) Q */
#define	QNOENB	0100			/* Don't enable Q via putq */



/*
 * module information structure
 */
struct module_info {
	ushort	mi_idnum;		/* module id number */
	char	*mi_idname;		/* module name */
	short   mi_minpsz;		/* min packet size accepted */
	short   mi_maxpsz;		/* max packet size accepted */
	ushort	mi_hiwat;		/* hi-water mark */
	ushort	mi_lowat;		/* lo-water mark */
};


/*
 * queue information structure
 */
struct	qinit {
	int	(*qi_putp)();		/* put procedure */
	int	(*qi_srvp)();		/* service procedure */
	int	(*qi_qopen)();		/* called on startup */
	int	(*qi_qclose)();		/* called on finish */
	int	(*qi_qadmin)();		/* for 3bnet only */
	struct module_info *qi_minfo;	/* module information structure */
	struct module_stat *qi_mstat;	/* module statistics structure */
};




/*
 * Streamtab (used in cdevsw and fmodsw to point to module or driver)
 */

struct streamtab {
	struct qinit *st_rdinit;
	struct qinit *st_wrinit;
	struct qinit *st_muxrinit;
	struct qinit *st_muxwinit;
};



/*
 * Header for a stream: interface to rest of system
 */

struct stdata {
	struct	queue *sd_wrq;		/* write queue */
	struct	msgb *sd_iocblk;	/* return block for ioctl */
	struct	inode *sd_inode;	/* backptr, for hangups */
	struct	streamtab *sd_strtab;	/* pointer to streamtab for stream */
	long	sd_flag;		/* state/flags */
	long	sd_iocid;		/* ioctl id */
	ushort	sd_iocwait;		/* procs waiting to do ioctl */
	short	sd_pgrp;		/* process group, for signals */
	ushort	sd_wroff;		/* write offset */
	unsigned char sd_error;		/* hangup or error to set u.u_error */

	int	sd_pushcnt;		/* number of pushes done on stream */
	struct  strevent *sd_siglist;	/* pid linked list to rcv SIGPOLL */
	struct	strevent *sd_pollist;	/* pid linked list to wakeup poll() */
	int	sd_sigflags;		/* logical OR of all siglist events */
	int	sd_pollflags;		/* logical OR of all sellist events */

	short	sd_vtime;		/* polling read parameter */
};



/*
 * stdata flag field defines
 */
#define	IOCWAIT		01		/* Someone wants to do ioctl */
#define RSLEEP		02		/* Someone wants to read/recv msg */
#define	WSLEEP		04		/* Someone wants to write */
#define STRPRI	       010		/* An M_PCPROTO is at stream head */
#define	STRHUP	       020		/* Device has vanished */
#define	STWOPEN	       040		/* waiting for 1st open */
#define STPLEX	      0100		/* stream is being multiplexed */
/*		      0200			*/
#define RMSGDIS	      0400		/* read msg discard */
#define RMSGNODIS    01000		/* read msg no discard */
#define STRERR	     02000		/* fatal error from M_ERROR */
#define STRTIME      04000		/* used with timeout strtime */
#define STR2TIME    010000		/* used with timeout str2time */
#define STR3TIME    020000		/* used with timeout str3time */
#define STFIONBIO   040000		/* do BSD-style non-blocking errors */
#define STRISTTY   0100000		/* is a System V style tty */


/*
 * structure for storing triples of mux'ed streams
 */
struct linkblk {
	queue_t *l_qtop;	/* lowest level write queue of upper stream */
	queue_t *l_qbot;	/* highest level write queue of lower stream */
	int      l_index;	/* file[] index for lower stream. */
};


/*
 * Structure of list of processes to be sent SIGSEL signal
 * on request, or for processes sleeping on select().  The valid
 * SIGSEL events are defined in stropts.h, and the valid select()
 * events are defined in select.h.
 */
struct strevent {
	union {
		struct {
			struct proc	*procp;
			long		events;
			} e;
		struct {
			int (*func)();
			long arg;
			} b;
	} x;
	struct strevent *se_next;
};

#define se_procp x.e.procp
#define se_events x.e.events
#define se_func x.b.func
#define se_arg x.b.arg

#define SE_SLEEP	0	/* ok to sleep in allocation */
#define SE_NOSLP	1	/* don't sleep in allocation */


/*
 *  Data block descriptor
 */
struct datab {
	struct datab	*db_freep;
	unsigned char	*db_base;
	unsigned char	*db_lim;
	unsigned short	db_ref;
	unsigned char	db_type;
	unsigned char	db_class;
};


/*
 * Message block descriptor
 */
struct	msgb {
	struct	msgb	*b_next;
	struct  msgb	*b_prev;
	struct	msgb	*b_cont;
	unsigned char	*b_rptr;
	unsigned char	*b_wptr;
	struct datab	*b_datap;
};

typedef struct msgb mblk_t;
typedef struct datab dblk_t;




/*
 * Data block allocation information.  Defines cutoffs for allocation
 * priorities; bufcall lists.
 */

struct dbalcst {
	int dba_cnt;
	int dba_lo;
	int dba_med;
	struct strevent *dba_lop;
	struct strevent *dba_medp;
	struct strevent *dba_hip;
};



/********************************************************************************/
/*			Streams message types					*/
/********************************************************************************/


/*
 * Data and protocol messages (regular priority)
 */
#define	M_DATA		00		/* regular data */
#define M_PROTO		01		/* protocol control */

/*
 * Control messages (regular priority)
 */
#define	M_BREAK		010		/* line break */
#define M_PASSFP	011		/* pass file pointer */
#define	M_SIG		013		/* generate process signal */
#define	M_DELAY		014		/* real-time xmit delay (1 param) */
#define M_CTL		015		/* device-specific control message */
#define	M_IOCTL		016		/* ioctl; set/get params */
#define M_SETOPTS	020		/* set various stream head options */


/*
 * Control messages (high priority; go to head of queue)
 */
#define	M_IOCACK	0201		/* acknowledge ioctl */
#define	M_IOCNAK	0202		/* negative ioctl acknowledge */
#define M_PCPROTO	0203		/* priority proto message */
#define	M_PCSIG		0204		/* generate process signal */
#define	M_FLUSH		0206		/* flush your queues */
#define	M_STOP		0207		/* stop transmission immediately */
#define	M_START		0210		/* restart transmission after stop */
#define	M_HANGUP	0211		/* line disconnect */
#define M_ERROR		0212		/* fatal error used to set u.u_error */


/*
 * Queue message class definitions.
 */
#define QNORM    0			/* normal messages */
#define QPCTL 0200			/* priority cntrl messages */



/*
 *  IOCTL structure - this structure is the format of the M_IOCTL message type.
 */
struct iocblk {
	int	ioc_cmd;		/* ioctl command type */
	ushort	ioc_uid;		/* effective uid of user */
	ushort	ioc_gid;		/* effective gid of user */
	uint	ioc_id;			/* ioctl id */
	uint	ioc_count;		/* count of bytes in data field */
	int	ioc_error;		/* error code */
	int	ioc_rval;		/* return value  */
};


/*
 * Options structure for M_SETOPTS message.  This is sent upstream
 * by driver to set stream head options.
 */
struct stroptions {
	short so_flags;			/* options to set */
	short so_readopt;		/* read option */
	ushort so_wroff;		/* write offset */
	short so_minpsz;		/* minimum read packet size */
	short so_maxpsz;		/* maximum read packet size */
	ushort so_hiwat;		/* read queue high water mark */
	ushort so_lowat;		/* read queue low water mark */

	short	so_vtime;		/* polling read parameter */
};

/* flags for stream options-set message
 */

#define SO_ALL		077		/* set all standard 5.3 options */
#define SO_READOPT	 01		/* set read option */
#define SO_WROFF	 02		/* set write offset */
#define SO_MINPSZ	 04		/* set min packet size */
#define SO_MAXPSZ	010		/* set max packet size */
#define SO_HIWAT	020		/* set high water mark */
#define SO_LOWAT	040		/* set low water mark */

#define SO_ISTTY       0100		/* mark this as a 'tty' */
#define SO_VTIME       0200		/* set vtime */




/********************************************************************************/
/*		Miscellaneous parameters and flags				*/
/********************************************************************************/

/*
 * Default timeout in seconds for ioctls and close
 */
#define STRTIMOUT 15

/*
 * Stream head default high/low water marks
 */
#define STRHIGH 512
#define STRLOW	128

/*
 * flag values for stream io waiting procedure (strwaitq)
 */
#define WRITEWAIT	0x1	/* waiting for write event */
#define READWAIT	0x2	/* waiting for read event */
#define NOINTR		0x4	/* error is not to be set for signal */

/*
 * sleep priorities for stream io
 */
#define	STIPRI	PZERO+3
#define	STOPRI	PZERO+3

/*
 * Block allocation parameters
 */
#define NCLASS 9			/* number of block classes */
#define QBSIZE		65		/* min size for block allocation retries */
#define MAXBSIZE	4096		/* max block size */
#define MAXIOCBSZ	1024		/* max ioctl data block size */

/*
 * Copy modes for tty and I_STR ioctls
 */
#define	U_TO_K	01			/* User to Kernel */
#define	K_TO_K  02			/* Kernel to Kernel */

/*
 * Values for stream flag in open to indicate module open, clone open;
 * return value for failure.
 */
#define MODOPEN		0x1		/* open as a module */
#define CLONEOPEN	0x2		/* open for clone, pick own minor device */
#define OPENFAIL	-1		/* returned for open failure */

/*
 * Priority definitions for block allocation.
 */
#define BPRI_LO		1
#define BPRI_MED	2
#define BPRI_HI		3

/*
 * Value for packet size that denotes infinity
 */
#define INFPSZ		-1

/*
 * Flags for flushq()
 */
#define FLUSHALL	1	/* flush all messages */
#define FLUSHDATA	0	/* don't flush control messages */


/************************************************************************/
/*	Defintions of Streams macros and function interfaces.		*/
/************************************************************************/

/*
 * determine block allocation cutoff for given class and priority.
 */
#define bcmax(class, pri) ( (pri) == BPRI_LO ? dballoc[class].dba_lo : \
			   ((pri) == BPRI_HI ? nmblock : dballoc[class].dba_med))

/*
 *  Queue scheduling macros
 */
#ifdef SVR3
#define setqsched()     {qrunflag = 1; timepoke();}
#else SVR3
#define setqsched()     (qrunflag = 1)	/* set up queue scheduler */
#endif SVR3
#define qready()	qrunflag	/* test if queues are ready to run */

/*
 * Definition of spl function needed to provide critical region protection
 * for streams drivers and modules.
 */
#define splstr() spltty()
#ifndef SVR3
#define splhi() splmax()
#endif SVR3


/*
 * noenable - set queue so that putq() will not enable it.
 * enableok - set queue so that putq() can enable it.
 * canenable - check if queue can be enabled by putq().
 *
 *	These must protect themselves against interrupts because they are
 *	messing with flags that are also messed with in interrupt handlers.
 *	Therefore, we have made them functions.
 */
extern noenable(), enableok();
#define canenable(q)	(!((q)->q_flag & QNOENB))

/*
 * Finding related queues
 */
#define	OTHERQ(q)	((q)->q_flag&QREADR? (q)+1: (q)-1)
#define	WR(q)		((q)+1)
#define	RD(q)		((q)-1)

/*
 * put a message of the next queue of the given queue
 */
#define putnext(q, mp)	((*(q)->q_next->q_qinfo->qi_putp)((q)->q_next, mp))

/*
 * Test if data block type is one of the data messages (i.e. not a control
 * message).
 */
#define datamsg(type) (type == M_DATA || type == M_PROTO || type == M_PCPROTO)

/*
 * extract queue class of message block
 */
#define queclass(bp) (bp->b_datap->db_type & QPCTL)

/*
 * Align address on next lower word boundary
 */
#define straln(a)	(caddr_t)((long)(a) & ~(sizeof(int)-1))

/*
 * Copy data from one data buffer to another.
 * The addresses must be word aligned - if not, use bcopy!
 */
#ifdef u3b2
/*
 * Use the MOVBLW instruction on the 3b2.
 */
asm	void
strbcpy(s, d, c)
{
%mem	s,d,c;

	MOVW	s,%r0
	MOVW	d,%r1
	MOVW	c,%r2
	ADDW2	&3,%r2
	LRSW3	&2,%r2,%r2
	MOVBLW
}

#else
#define	strbcpy(s, d, c)	bcopy(s, d, c)
#endif

/*
 * declarations of common routines
 */
extern mblk_t *rmvb();
extern mblk_t *dupmsg();
extern mblk_t *copymsg();
extern mblk_t *allocb();
extern mblk_t *unlinkb();
extern mblk_t *dupb();
extern mblk_t *copyb();
extern mblk_t *getq();
extern int    putq();
extern queue_t *backq();
extern queue_t *allocq();
extern int    qenable();
extern mblk_t *unlinkb();
extern mblk_t *unlinkmsg();
extern int    pullupmsg();
extern int    adjmsg();
extern struct queue *getendq();
extern struct linkblk *findlinks();
extern struct file *getf();
extern struct strevent *sealloc();
extern int   sefree();

/*
 * shared or externally configured data structures
 */
#ifndef STREAM_DEF			/* do not define them twice */
extern mblk_t	mblock[];		/* table of msg blk desc */
extern dblk_t	dblock[];		/* table of data blk desc */
#endif
extern ushort	rbsize[];		/* map of class to real block size */
extern struct linkblk linkblk[];	/* multiplexor link table */
extern struct strevent strevent[];	/* table of stream event cells */
extern struct strstat strst;		/* Streams statistics structure */
extern queue_t *qhead, **qtailp;	/* runnable queue list */
extern int strmsgsz;			/* maximum stream message size */
extern int strctlsz;			/* max size of ctl part of message */
extern int nmblock;			/* number of msg blk desc */
extern int nmuxlink;			/* number of multiplexor links */
extern int nstrpush;			/* maxmimum # of pushes allowed */
extern int nstrevent;			/* initial # of stream event cells */
extern int maxsepgcnt;			/* page limit for event cell alloc */
extern int pollwait;			/* poll sleeps on &pollwait */
extern char qrunflag;			/* set if there are queues to run */
