/*
* $Source: /d2/3.7/src/stand/include/RCS/nx.h,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:13:48 $
*/

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

/*                 start of code added by djb 
 *
 *    packet buffers for standalone read/write
 * pktbufs incorporate a buffer big enough for a maximum-size enet
 * packet (MAXPACKET) and a 'ready' field which is used to flag
 * that the buffer is full/empty.
 *
 * NPKTBUFS - number of packet buf structs, presently 2 each for
 * reading and writing
 *
 * replymsgvectors are used to wait for reply messages from the 
 * controller.  The low 24 bits may be a pointer to a buffer where the
 * contents of the reply message is to be copied.  This is the case if 
 * the vector has it's COPYREPLY bit set.  If the value of the vector
 * is NOCOPY, the message contents are not copied anywhere.
 */


#define 	MAXPACKET	1520	/* largest legal ethernet packet */

struct pktbuf {
    short	ready;			/* this buffer is available if set */
    char	readReply[64];		/* size of biggest reply msg	 */
    char	data[MAXPACKET];
};

#define NPKTBUFS	4

/*
 * PORTB bits (for read) (bits 7-4 and bit 2 are undefined)
 */
#define	NX_NOT_READY	0x8	/* '0' means NX is ready */
#define NX_INT		0x2	/* '1' means NX is interrupting */
#define NX_ERROR	0x1	/* '0' means NX has error condition */

/*
 * choices of controller mode - as issued by the mode command
 */
#define	CONNECT_TO_NET	2

/*
 * host-controller message buffer status definitions
 */
#define NX_BUSY		3
#define HOST_BUSY	2
#define NX_IDLE		1
#define HOST_IDLE	0

#define NX_OWNER	1	/* mask to determine ctrlr owns msgbuf */


/*                  end of code added by djb  
*/

/*
 * exos init message
 * conforms to the manual (no byte swabbing).
 */
struct init {
	short	res0;		/* reserved */
	char	vers[4];	/* exos version */
	unsigned char	code;		/* completion code */
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
	struct xmit_msg msg;		/* largest message */
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

#define	devaddr	0x7ffc
#define	PORTA	(devaddr + 1)
#define	PORTB	(devaddr + 0)


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
#define	XTOH(a)		((unsigned int)a + (unsigned int)nxva)
#define	HTOX(a)		((unsigned int)a - (unsigned int)nxva)


/*
 * test pattern
 */
struct testp {
	char b0, b1, b2, b3;
	short w0, w1;
	long l0;
};

/*
 * a 'request' structure is used to keep track of outstanding messages that
 * have been sent to the controller.  The int svc routine indicates completed
 * requests by setting the complete field to TRUE.  The controller's reply msg
 * is copied to *reply by the int svc routine if reply is non-NULL.
 */
struct request {
    XMIT		msg;		/* pointer to a msg buffer	*/
    char		*reply;		/* pointer to a reply buffer	*/
    char		complete;	/* request complete flag	*/
};

char	*PortA,			/* Virtual Address of Controller A port */
	*PortB;			/* Virtual Address of Controller B port */

char		*_readdatabuf;	/* nxread's data buffer for reading	*/
char		*readreply;	/* nxread's reply message for reading	*/
struct request	*readrequest;	/* nxread's vector to check completion	*/

char		*writedatabuf;	/* nxwrite's data buffer		*/
struct request	*writerequest;	/* nxwrite's vector to check completion	*/

char	*mbmalloc();		/* mbmalloc returns pointer to space	*/

long nxva;			/* virtual address of multibus area */
long nxpa;			/* physical address of multibus area */

struct mqueues *nxmp;		/* points to message queues */
struct msgbuf *lastrmp;		/* oldest read queue pointer */
struct physnet myaddr;		/* my physical network addr */


char nxreads;			/* number of read buffers set up */
#define	NREADS	1		/* max buffers tied up waiting for input */

struct init *nxim;

char _nxpresent;			/* zero if interface not answering */


#define	NXNORM		0
#define	NXFAST		1
#define	NXRAW		2

MP _igetmsgbuf();
MP _getmsgbuf();

