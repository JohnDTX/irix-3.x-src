/*
 * Excelan Multibus Controller Header
 *
 */



/*
 * ethernet physical address
 */
struct physnet {
	char addr[6];
};

#define 	MAXPACKET	1520	/* largest legal ethernet packet */

/*
 * PORTB bits (for read) (bits 7-4 and bit 2 are undefined)
 */
#define	NX_READY_	0x8	/* '0' means NX is ready */
#define NX_INT		0x2	/* '1' means NX is interrupting */
#define NX_ERROR_	0x1	/* '0' means NX has error condition */

/*
 * choices of controller mode - as issued by the mode command
 */
#define DISCONNECT_FROM_NET	0
#define CONNECT_PERFECT		1
#define CONNECT_TO_NET		2
#define CONNECT_PROMISCUOUS	3

/*
 * host-controller message buffer status definitions
 */
#define NX_BUSY		3
#define HOST_BUSY	2
#define NX_IDLE		1
#define HOST_IDLE	0

#define NX_OWNER	1	/* mask to determine ctrlr owns msgbuf */



/*
 * exos init message
 * conforms to the manual (no byte swabbing).
 */
struct init {
	short	res0;		/* reserved */
	char	vers[4];	/* exos version */
	char	code;		/* completion code */
	char	mode;		/* operating mode */
	char	format[2];	/* data format option */
	char	res1[3];	/* reserved */
	char	amode;		/* address mode */
	char	res2;		/* reserved */
	char	mapsize;	/* map size */
	char	mmap[32];	/* memory map */
	long	mvblk;		/* movable block addresses */
	char	nproc;		/* number of processes */
	char	nmbox;		/* number of mailboxes */
	char	naddr;		/* number of address slots */
	char	nhost;		/* number of hosts */
	long	hxseg;		/* host to exos segment address */
	short	hxhead;		/* host to exos q header */
	char	hxitype;	/* host to exos interrupt type */
	char	hxival;		/* host to exos interrupt value */
	long	hxiaddr;	/* host to exos interrupt address */
	long	xhseg;		/* exos to host segment address */
	short	xhhead;		/* exos to host q header */
	char	xhitype;	/* host to exos interrupt type */
	char	xhival;		/* host to exos interrupt value */
	long	xhiaddr;	/* host to exos interrupt address */
};


/*
 * Link Level Controller Commands and Messages
 */
#define	NET_MODE	0x8
#define	NET_ADDRS	0x9
#define	NET_RECV	0xa
#define	NET_STATS	0xb
#define	NET_XMIT	0xc
#define	NET_READ	0xd
#define	NET_XMIT_LOOP	0xe		/* receive our own packets */



/*
 * Request Mask Bits  (our copy of the manual is wrong, this is correct).
 */
#define	READREQ		2
#define	WRITEREQ	1


/*
 * exos message declaratons start here.
 */
struct mode_msg {
	unsigned short link;		/* controller's link to next buf */
	char res;
	char status;
	unsigned short length;		/* length of data portion */
	short	pad;
	long	id;
	char	request;
	char	reply;
	char	reqmask;
	char	errmask;
	char	mode;
	char	nop;
};
typedef struct mode_msg *MODE;

struct addrs_msg {
	unsigned short link;		/* controller's link to next buf */
	char res;
	char status;
	unsigned short length;		/* length of data portion */
	short	pad;
	long	id;
	char	request;
	char	reply;
	char	reqmask;
	char	slot;
	struct physnet	addr;
};
typedef struct addrs_msg *ADDRS;

struct recv_msg {
	unsigned short link;		/* controller's link to next buf */
	char res;
	char status;
	unsigned short length;		/* length of data portion */
	short	pad;
	long	id;
	char	request;
	char	reply;
	char	reqmask;
	char	slot;
};
typedef struct recv_msg *RECV;

struct stats_msg {
	unsigned short link;		/* controller's link to next buf */
	char res;
	char status;
	unsigned short length;		/* length of data portion */
	short	pad;
	long	id;
	char	request;
	char	reply;
	char	reqmask;
	char	res1;
	short	nobj;
	short	index;
	long	baddr;
};
typedef struct stats_msg *STATS;
struct stats_reply
{
	long	SentErrorFree;
	long	AbortedExcessCollisions;
	long	AbortedLateCollision;
	long	TdrDelay;
	long	RecvErrorFree;
	long	RecvAlignError;
	long	RecvCrcError;
	long	RecvLost;
};


struct	block {
	short	len;
	long	addr;
};

struct xmit_msg {
	unsigned short link;		/* controller's link to next buf */
	char res;
	char status;
	unsigned short length;		/* length of data portion */
	short	pad;
	long	id;
	char	request;
	char	reply;
	char	slot;
	char	nblocks;
	struct	block block[8];
};
typedef struct xmit_msg *XMIT;


typedef struct xmit_msg NxMsg;
typedef struct xmit_msg NxReply;

struct read_msg {
	unsigned short link;		/* controller's link to next buf */
	char res;
	char status;
	unsigned short length;		/* length of data portion */
	short	pad;
	long	id;
	char	request;
	char	reply;
	char	slot;
	char	nblocks;
	struct	block block[8];
};
typedef struct read_msg *READ;





/*
 * host/controller message buffer.
 */
#define	MDATA	32		/* size of data portion of message */
#define	NRBUFS	16		/* number of receive message buffers */
#define	NWBUFS	16		/* number of send message buffers */

struct msgbuf {
	NxMsg msg;		/* largest message */
	struct msgbuf *next;		/* host's link to next buf */
	char raddr;
};
typedef struct msgbuf * MP;

/*
 * Part of a msgbuf common to all messages.
 * (declared for convenience of MSGLEN)
 */
struct common {
	short link;
	char status;
	char res;
	short length;
};
#define	MSGLEN	(sizeof (struct xmit_msg) - sizeof (struct common))






/*
 * DMA area, contains msg queues for excelan interface, plus
 * data buffers and protocol headers.  Everything is grouped here
 * for convenience of the memory allocator.
 * Output/Send messages denote host to controller activity.
 * Input/Read/Receive messages denote controller to host activity.
 *
 * djb - took out netbufs and connection arrays (Xns stuff)
 * replaced with pktbuf (data buffers for standalone read/write)
 */
struct mqueues {
	unsigned short roffset;		/* controller's read queue header */
	unsigned short woffset;		/* output queue header */
	struct msgbuf *rhead;		/* host queue header */
	struct msgbuf *whead;
	struct msgbuf rbufs[NRBUFS];	/* receive messages */
	struct msgbuf wbufs[NWBUFS];	/* send messages */
};


/*
 * Device addresses (physical Multibus i/o addrs) for Port A and Port B
 */

#define nx_ioaddr	0x7FFC
#define nx_OLDioaddr	0x0010
#define PORTA_OFF	busfix(0)
#define PORTB_OFF	busfix(1)


/*
 * Construct 8088 segment address from a 68000 address.
 */
#define	SEG(a)	((a& ~0xf)<<12) | (a&0xf)
#define	SEGH(a)	(a>>4)
#define	SEGL(a)	(a&0xf)


/*
 * macro's to convert between excelan header offsets (X)
 * and host addresses (H).
 */
#define	XTOH(a)		((unsigned int)a + (unsigned int)nxvseg)
#define	HTOX(a)		((unsigned int)a - (unsigned int)nxvseg)


/*
 * test pattern
 */
struct testp {
	char b0, b1, b2, b3;
	short w0, w1;
	long l0;
};

/*
 * Common section for standalone excelan driver.
 *  modified from nxcommon.c (unix nx, xns driver)
 */






/*
 * a 'request' structure is used to keep track of outstanding messages that
 * have been sent to the controller.  The int svc routine indicates completed
 * requests by setting the complete field to TRUE.  The controller's reply msg
 * is copied to *reply by the int svc routine if reply is non-NULL.
 */
struct request {
    NxMsg		*msg;		/* pointer to msg buffer */
    NxReply		*replymsg;	/* pointer to reply buffer (or 0) */
    char		complete;	/* request complete flag	*/
};

#define	NXNORM		0
#define	NXFAST		1
#define	NXRAW		2

extern MP igetmsgbuf();
extern MP getmsgbuf();


#define	MAX(a,b)	(((a)>(b))?a:b)

#define MIN(a,b)	(((a)<(b))?a:b)
