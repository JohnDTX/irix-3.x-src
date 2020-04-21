/* XXX remove these */
#ifdef	KERNEL

#include "nex.h"
# define NEX	NNEX
# if	NEX > 0
# define IFXNS		/* code hacked for coexistence with if_ex */
# endif	NEX > 0

#include "../xns/Xns.h"
#include "../xns/Xnsconn.h"
#include "../xns/Xnstrace.h"
#include "../xns/Xnsioctl.h"
#include "../xns/nxreg.h"

#else

#include <xns/Xns.h>
#include <xns/Xnsconn.h>
#include <xns/Xnstrace.h>
#include <xns/Xnsioctl.h>
#include <xns/nxreg.h>

#endif

/* XXX interface stuff --> move to ifxns.h */
struct	physnet myaddr;			/* my physical network addr */
Xaddr	myinternet;
struct	physnet bcastaddr;
struct	bouncemsg bounceproto;
Xaddr	localnet;			/* xns local network addr */
struct	netbuf *freenetbufs;
struct	netbuf *permnetbufs;
char bwaiting;				/* set if somebody needs a net buf */

#define	RSIZE	20
struct	xns_route xns_rtab[RSIZE];

/*
 * This structure contains state needed on a per tty basis for managing
 * the user requests whether or not they have an active connection.
 */
struct	xnsmisc {
	short	x_state;		/* state of non-connection */
	short	x_socket;		/* non-connection socket number */
	CONN	x_conp;			/* if a connection this is its conp */
	NBP	x_pend;			/* pending netbuf */
	short	x_owner;		/* pid of opener */
	struct tty *x_ttyp;		/* associated tty */
	short	x_uetype;		/* etype for user proto if any */
	short	x_maxinpq;		/* limit for x_inpq */
	struct	netqueue x_inpq;	/* input queue for non-connections */
} xnsmisc[NDEV];

/* x_state's */
#define	X_FAST		0x0001		/* fast (no line disciplines) */
#define	X_RAW		0x0002		/* raw writes (ethernet) */
#define	X_BLOCK		0x0004		/* block mode read/writes */
#define	X_MULTI		0x0008		/* recieve multicast packets */
#define	X_DISCARD	0x0010		/* when using X_MULTI, discard packets
					   to ourselves */
#define	X_EOF		0x0020		/* eof has been detected */
#define	X_CONNECTED	0x0040		/* x_conp is valid */
#define	X_SETSOCKET	0x0080		/* a socket has been chosen */
#define X_UPROTO	0x0200		/* user protocol */
#define X_PGRP		0x0400		/* did SETPGRP */
#define X_EOFPEND	0x0800		/* EOF is pending */
#define X_WRERROR	0x1000		/* had a write error */

/* XXX use these someday */
#define	X_LISTEN	0x0100		/* listening for a connection */

/* XXX nx grot --> move to nxreg.h */
long nxva;			/* virtual address of multibus area */
char nxintflag;			/* non-zero if on interrupt stack */
struct mqueues *nxmp;		/* points to message queues */
struct msgbuf *lastrmp;		/* oldest read queue pointer */
struct msgbuf *curxmp;		/* current xmit msg buf */
int nxfreecount;
char mwaiting;			/* non-zero if somebody needs a msgbuf */
char nxreads;			/* number of read buffers set up */
#define	NREADS	3		/* max buffers tied up waiting for input */
char nxpresent;			/* zero if interface not answering */

struct tty nx_tty[NDEV];

#define	NXDEV(d)	(minor(d)&0x7F)

/* XXX END nx grot */

#define	NOSLEEP		1
#define	SLEEPOK		0

struct	xnsmisc *sgbouncemisc;

u_short	idnumber;

NBP	getnbuf();
MP	getmsgbuf();
ROUTE	xns_getroute();

#ifndef	MAX
#define	MAX(a,b)	((a>b)?a:b)
#endif
#ifndef	MIN
#define	MIN(a,b)	((a<b)?a:b)
#endif

#define htons(x)	(x)
#define ntohs(x)	(x)
#define	HTONS(x)
#define NTOHS(x)
#define SYSTIME		time

extern char hostname[];
extern short hostnamelen;
extern int duputchar();

short xns_ready;
struct xns_route *xns_bcast;

#define	RQUEUEMAX	4
#define	SQUEUEMAX	4
#define	RWINDOW		8
short	xns_rqueuemax;			/* max unread receive packets */
short	xns_squeuemax;			/* max allowed output queue size */
short	xns_rwindow;			/* nominal receive window */

# ifdef SYSTEMV
# define XNSTTYINDEX(tp)	((tp)->t_index)
# else  SYSTEMV
# define XNSTTYINDEX(tp)	((tp)-nx_tty)
# endif SYSTEMV
