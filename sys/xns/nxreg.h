/*
 * Excelan Multibus Controller Header
 */


/*
 * ethernet physical address
 */
struct physnet {
	char addr[6];
};

/*
 * exos init message:
 *	- conforms to the manual (no byte swabbing)
 *	- 80 bytes
 */
struct init {
	short	res0;		/* reserved */
	u_char	vers[4];	/* exos version */
	u_char	code;		/* completion code */
	u_char	mode;		/* operating mode */
	char	format[2];	/* data format option */
	char	res1[3];	/* reserved */
	char	amode;		/* address mode */
	char	res2;		/* reserved */
	char	imapsize;	/* map size */
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
# define NET_MODE	0x8
# define NET_ADDRS	0x9
# define NET_RECV	0xA
# define NET_STATS	0xB
# define NET_XMIT	0xE
# define NET_READ	0xD

/*
 * Request Mask Bits  (our copy of the manual is wrong, this is correct).
 */
#define	READREQ		2
#define	WRITEREQ	1

/*
 * Error Mask Bits
 */
#define ALIGNERR	0x10
#define CRCERR		0x20


/*
 * exos message declaratons start here.
 */
struct mode_msg {
	ushort link;		/* controller's link to next buf */
	char res;
	char status;
	ushort length;		/* length of data portion */
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
	ushort link;		/* controller's link to next buf */
	char res;
	char status;
	ushort length;		/* length of data portion */
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
	ushort link;		/* controller's link to next buf */
	char res;
	char status;
	ushort length;		/* length of data portion */
	short	pad;
	long	id;
	char	request;
	char	reply;
	char	reqmask;
	char	slot;
};
typedef struct recv_msg *RECV;

struct stats_msg {
	ushort link;		/* controller's link to next buf */
	char res;
	char status;
	ushort length;		/* length of data portion */
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

struct	block {
	short	len;
	long	addr;
};

struct xmit_msg {
	ushort link;		/* controller's link to next buf */
	char res;
	char status;
	ushort length;		/* length of data portion */
	short	pad;
	long	id;
	char	request;
	char	reply;
	char	slot;
	char	nblocks;
	struct	block block[8];
};
typedef struct xmit_msg *XMIT;

struct read_msg {
	ushort link;		/* controller's link to next buf */
	char res;
	char status;
	ushort length;		/* length of data portion */
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
	struct xmit_msg msg;		/* largest message */
	struct msgbuf *next;		/* host's link to next buf */
};
typedef struct msgbuf *MP;

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
#define	EXOSMAX	(sizeof (struct xmit_msg))


/*
 * DMA area, contains msg queues for excelan interface, plus
 * data buffers and protocol headers.  Everything is grouped here
 * for convenience of the memory allocator.
 * Output/Send messages denote host to controller activity.
 * Input/Read/Receive messages denote controller to host activity.
 */
struct mqueues {
# ifndef IFXNS
	short roffset;			/* controller's read queue header */
	short woffset;			/* output queue header */
	struct msgbuf *rhead;		/* host queue header */
	struct msgbuf *whead;
	struct msgbuf rbufs[NRBUFS];	/* receive messages */
	struct msgbuf wbufs[NWBUFS];	/* send messages */
	struct init imsg;
# endif  IFXNS
	struct netbuf netbufs[NETBUFS];	/* dma buffers */
	struct conn connection[NDEV];	/* XNS headers */
};

/*
 * Device addresses for Port A and Port B
 */
u_char	*PORTA;
u_char	*PORTB;

/* PORTB bits */
# define NX_READY_	0x08		/* not ready */
# define NX_INT		0x02		/* interrupting */
# define NX_ERROR_	0x01		/* not error */

/*
 * macro defs for raising and lowering interrupt priority.
 * Used to block timeout's (xns_timer) and interface.
 */
#define USEPRI	register int s;
#define	LOCK	(s=spl2())
#define	UNLOCK	splx(s)

/*
 * macro's to convert between excelan header offsets (X)
 * and host addresses (H).
 */
#define	XTOH(a)		((int)a + (int)nxva)
#define	HTOX(a)		((int)a - (int)nxva)

/*
 * test pattern
 */
struct	testp {
	char b0, b1, b2, b3;
	short w0, w1;
	long l0;
};


/*----- stuff added for auto-reset feature*/
struct stats_reply {
	long	nxs_okxmit;	/* frames sent with no errors */
	long	nxs_aborts;	/* frames aborted because of collisions */
	long	nxs_reserved;
	long	nxs_tdr;	/* time domain reflectometer */
	long	nxs_okrcv;	/* frames recieved with no errors */
	long	nxs_misaligned;	/* frames received with alignment error */
	long	nxs_crc;	/* frames received with crc error */
	long	nxs_lost;	/* frames lost because of no buffers */
};

/* message status bits */
# define MX_HOST	00
# define MX_EXOS	01
# define MX_BUSY	02
# define MX_OVERFLOW	04

typedef struct xmit_msg NxMsg;

/* size of exos memory segment */
# define NXSEGSIZE	512
# define NXSEGMASK	(NXSEGSIZE-1)

/* net connection modes */
# define NET_DISCONNECT		0
# define NET_CONNECT		1
# define NET_PROMISCUOUS	3

# define NX_TRAILER_SIZE	4	/* sizeof CRC trailer */
# define MYADDRSLOT		253	/* slot of own enet address */

/* macros for ethernet addresses */
# define EQPHYSNET(a, b)	EQ3((short *)(a), (short *)(b))
# define EQ3(a, b)		(0[a]==0[b]&&1[a]==1[b]&&2[a]==2[b])
