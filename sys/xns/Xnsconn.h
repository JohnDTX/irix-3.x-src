/*
 * Netbuf struct and netbuf queues.
 */

# ifdef IFXNS
/*
 * A netbuf may Either contain data directly via nbp->info, or
 * indirectly via an mbuf if nbp->m is non-0.  In either case,
 * area, while mtod(nbp) points to the current point within
 * the base area.
 */
# include "../h/mbuf.h"
# define MBUFIZE(p, b)	((p)->m = (b) , (p)->data = MBUFDATA(b))
# define UNMBUFIZE(p)	(((p)->m != 0 ? m_freem((p)->m) : 0) \
				, (p)->m = 0 , (p)->data = (p)->info)
# define MBUFDATA(b)	((b)->m_next != 0 ? mtod((b)->m_next, char *) \
				: mtod(b, char *) + sizeof (char *))
# endif IFXNS

#define	MAXPACKET	1520
#define	MINPACKET	64
struct netbuf {
	char	*data;			/* points to info field */
	u_short	len;			/* size of data segment */
	u_short	seq;			/* protocol sequence number */
	struct	netbuf *next;		/* queue link */
	struct	netbuf *perm;		/* permanent link */
	long	nxpaddr;		/* physical addr (for dma) */
	char	btype;			/* netbuf type */
	char	dtype;			/* data type */
	char	quack;			/* queue up ack */
	char	control;
# ifdef IFXNS
	struct	mbuf *m;		/* ptr to mbuf if any */
# endif IFXNS
	char	info[MAXPACKET];	/* data goes in here */
};
typedef struct netbuf * NBP;
#define	NETBUFS	30

/*
 * Mbuf simulation is done with these macros and definitions
 */
#ifdef	notdef
/* arguments to getnbuf() and getmsgbuf() */
#define	M_DONTWAIT	1
#define	M_WAIT		0
#endif

# define nxmtod(p, t)		((t)(p)->data)

struct lnetbuf {
	char *data;			/* points to info field */
	u_short len;			/* size of data segment */
	u_short seq;			/* protocol sequence number */
	struct netbuf *next;		/* queue link */
	struct netbuf *perm;		/* permanent link */
	long nxpaddr;			/* physical addr (for dma) */
	char btype;			/* netbuf type */
	char dtype;			/* netbuf type */
	char quack;			/* queue up ack */
	char control;
};

/*
 * netbuf types
 */
#define	N_BUF	1			/* local buf */
#define	N_USER	2			/* user space area */
#define	N_LOCAL	3			/* dedicated local area */

# define B_NFREE	000		/* buf is free */
# define B_XMIT		010		/* buf being xmitted, on out q */
# define B_RAWXMIT	020		/* buf being xmitted, not on out q */
# define B_RECV		030		/* buf used for recv */
# define B_INPQ		040		/* buf on an in q */
# define B_USAGE	070		/* mask */

# define N_DONE		0100



/*
 * header for a queue of netbufs
 */
struct netqueue {
	struct netbuf *head;		/* == front == beginning */
	struct netbuf *tail;		/* == back == last buf */
	short nbufs;
};
typedef struct netqueue * NBQ;

/*
 * A connection is represented by a protocol header
 * plus some unix and dma glue.  When we want to support
 * another protocol, the Xseq part would become a union.
 */
struct conn {
	Xseq	header;			/* XNS header */
	struct	netqueue inpq;		/* input buffer queue */
	struct	netqueue outq;		/* retransmission queue */
	struct	netqueue pending;	/* place to hold output buf */
	NBP	trans;			/* current trans buffer */
	NBP	release;		/* buffers to release */
	ROUTE	route;			/* points to a route entry */
	caddr_t	paddr;			/* dma address of header */
	struct	conn *next;		/* list of conn structs */
	struct	tty *utp;		/* unix minor device */
	u_short	nextseq;		/* next output sequence */
	u_short	lastseq;		/* previous output sequence */
	u_short	rackno;			/* received ackno */
	u_short	ralloc;			/* received allocation */
	u_short	allocno;		/* allocation (to remote) */
	u_short	rseq;			/* last received sequence */
	u_short	nextrseq;		/* next expected rseq */
	u_short	rtime;			/* time of last received packet */
	u_short	ttime;			/* time of last timeout */
	u_short sockin;			/* incoming socket number */
	u_char	state;			/* unix state */
	u_char	ntries;			/* attempt counter */
	u_char	mode;			/* connection mode */
	u_char	dir;			/* direction (callout==1) */
	char	hbusy;			/* header is busy */
	char	asend;			/* send alloc */
	char	blocked;		/* waiting for allocation */
	char	hwaiting;		/* process sleeping on the header */
};
typedef struct conn * CONN;


/*
 * connection.state
 */
#define	S_FREE		000
#define	S_CLOSING	001
#define	S_FAILCALL	002
#define	S_FLUSH		004
#define	S_CLEANUP	010
#define	S_ACTIVE	020
#define	S_CALLOUT	040


#define	SOCKBASE	3001
#define	ALLSOCK		(SOCKBASE-1)

u_short the_time;

/*
 * Turn this bit on in a dtype to tell xns_xack to request the remote
 * host to send us an ack.
 */
#define	XACK_SENDACK	0x100
