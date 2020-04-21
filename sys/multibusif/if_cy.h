/* $Header: /d2/3.7/src/sys/multibusif/RCS/if_cy.h,v 1.1 89/03/27 17:32:01 root Exp $ */

/* 
 * if_cy.h - parameters for Cypress network code
 * 
 * Author:	Thomas Narten
 * 		Dept. of Computer Sciences
 * 		Purdue University
 * Date:	Thu Oct 24 1985
 * Copyright (c) 1985 Thomas Narten
 *
 * NOTE: To include this file, the following files must also be included.
 *       #include <sys/types.h>
 *	 #include <sys/socket.h>
 *	 #include <sys/time.h>
 *	 #include <netinet/in.h>
 *	 #include <net/if.h>
 * There are other files that may need to be included, but are not required
 * for all applications.
 */

#define	STARTUP	"Cypress network interface version 2.1"
#define TRUE	    -1
#define FALSE	    0
#define DLE	0203		/* used to escape reserved octets appearing */
				/* in the data portion of the packet */
#define STX	0202		/* packet separator. never appears in data */
#define DLE_STX 0205		/* escaped STX */
#define DLE_DLE 0204		/* escaped escape */

#define	CYMTU	    578		/* pick size that is small, but */
				/* unlikely to get fragmented */
#define CLINEMAX    6		/* maximum lines we support on any Implet*/
#ifndef sgi
#define CCYBUFMAX   (4*CLINEMAX)	/* need 2 buffers per line */
#endif sgi
#define CHOSTSMAX   256			/* maximum number of cypress hosts */
#define CCHTTYBUF   2048	/* size of tty buffer */
				/* needs to be large for xboxes with */
				/* NN handling, as they don't fragment */
				/* packets */
#ifdef sgi
struct cy_dmabuf {
    unsigned cyd_cch;		/* number of chars in packet (buffer) */
    char cyd_state;		/* state of BUFFER */
#define CYDFREE	0
#define CYDMAINPROGRESS 1
#define CYDALLOCATED	2
#define CYDMAREADY	3
    char cyd_rgch[CYMTU*2];	/* need 2*MTU for character stuffing */
};
#endif sgi

#define CY_QDRAIN(ifq)          ((ifq)->ifq_len >= cy_cqlowat)

#ifndef sgi
#define CYINPUT(ch, tp) {\
    *tp->cy_cp++ = ch; \
    if ((ch&0xff) == STX  || ++tp->cy_inbuf == CCHTTYBUF) \
	cy_tpktint(tp); \
    }

struct cy_dmabuf *cy_getdmabuf();
#endif sgi


struct cyln {			/* cypress line control block */
    int cyl_cpsNN;		/* count of nearest neighbor packets sent */
    int cyl_cpsdirect;		/* count of direct packets sent */
    int cyl_cpsflood;		/* count of flood packets sent */
    int cyl_cpsrpf;		/* count of RPF packets sent */
    int cyl_cprNN;		/* count of nearest neighbor packets sent */
    int cyl_cprdirect;		/* count of direct packets recieved */
    int cyl_cprflood;		/* count of flood packets recieved */
    int cyl_cprrpf;		/* count of RPF packets recieved */
    int cyl_cchr;		/* number of characters recieved */
    int cyl_cchs;		/* number of characters sent */
    int cyl_cpsip;		/* number of packets sent up to IP */
    int cyl_cprip;		/* number of packets recieved from IP */
    int cyl_csilo;		/* number of silo overflows on this line */

    int cy_flags;		/* status of the line */
    int	cy_cesc;    		/* # of characters escaped in packets */
    int cy_copdrop;		/* # of dropped forwarded packets */
    int cy_ctpbusy;		/* number of times tty was active during  */
    int cy_ctpidle;		/*   "    "    "    "   "  idle      " */
#ifdef sgi
    struct st_c {
	queue_t *cy_rq, *cy_wq;		/* stream queues		 */
	int	cy_unit;		/* line number of cypress device */
	struct mbuf *cy_wmbuf;		/* mbuf chain currently being sent */
	u_char *cy_wptr;
	u_short cy_wlim;
	struct mbuf *cy_rmbase;		/* growing input mbuf chain	 */
	struct mbuf *cy_rmbtail;
	u_short cy_rlen;
	u_char cy_rdle;			/* waiting for byte after DEL	 */
    } *cy_tp;
#else sgi
    struct cy_dmabuf *cyl_pdmabuf1,	/* 2 buffers. The idea is while  */
		      *cyl_pdmabuf2;	/* dma is happening with one,  */
					/* we should be filling the other */
    union {
	struct tty *cyl_utp;	/* tty control block */
	struct ifnet *cyl_uif;	/* network interface */
    } cy_udev;
#define cy_tp cy_udev.cyl_utp
#define cyl_if cy_udev.cyl_uif
#endif sgi
    struct in_addr cyl_dest;	/* host at other end of line */
    struct ifqueue cy_send;	/* queue for (locally generated) packets */
#ifndef sgi
    int    (*cyl_output)();	/* routine to call to perform output if not a tty*/
#endif sgi
};

struct cy_hdr {			/* header for cypress packet */
#ifdef vax
    u_char
	cyh_hopcnt:4,		/* hopcount, decrement at each hop */
	cyh_handling:2,		/* how do we handle (route) the packet? */
	cyh_type:2;		/* What kind (protocol) of packet is this */
#endif vax
/* 
 * of course Sun has a different idea on what the correct order is...
 */
#ifdef NFS30
    u_char
	cyh_type:2,		/* What kind (protocol) of packet is this */
	cyh_handling:2,		/* how do we handle (route) the packet? */
	cyh_hopcnt:4;		/* hopcount, decrement at each hop */
#endif
#ifdef sgi		/* sgi is like Sun */
    u_char
	cyh_type:2,		/* What kind (protocol) of packet is this */
	cyh_handling:2,		/* how do we handle (route) the packet? */
	cyh_hopcnt:4;		/* hopcount, decrement at each hop */
#endif sgi
    char cyh_dest;		/* how we are supposed to handle it */
};
/* 
 * Types of packets that we understand
 */
#define CYT_IP  03		/* is an IP packet */
#define CYT_CP  02		/* Control Cypress packet */
#define CYT_XX  01		/* reserved for future use */
#define CYT_EXTENDED 00		/* extended type, look at third byte of packet */

/* 
 * The types of handling (routing) that we know about
 */
#define CYH_FLOOD   00		/* send packet out on all lines (except src)*/
#define CYH_RPF	    01		/* reverse path forwarding */
#define CYH_NN	    02		/* nearest neighbor routing */
#define CYH_DIRECT  03		/* route packet using routing tables */



#define CYF_LINEUP  0x1
#define CYF_ISTTY   0x2
#define CYF_MONITORON 0x4
#define CYF_CARRIER 0x8		/* currently have carrier on line */

#ifndef sgi
/*
 * device state changes
 */
#define CYLS_LOSTCARRIER  1	/* lost carrier on line */
#define CYLS_REGAINCARRIER 2	/* regained carrier */
#define CYLS_INVALIDDMVADDR 3		/* bad dmv address ?? */
#endif sgi

#define mptpcyln(tp)	(&rgcyln[tp->cy_unit])
#ifndef sgi
/* 
 * definitions used in implementing the interface between the tty
 * drivers and the Cypress routing code.  Characters are dumped into a
 * fixed length buffer until a packet delimiter is encountered.  At
 * this time, the buffer is queued and a new one is allocated by the
 * tty driver.  The original buffer is then processed via a software
 * interrupt, and returned to the pull of free buffers.
 */

struct cy_buf {
    struct cy_buf *cyb_pbuf;	/* used in queueing buffers */
    struct cyln *cyb_pcyln;	/* which tty the packet came from */
    int cyb_cch;		/* number of characters in buffer */
    char cyb_state;		/* allocated or free */
    char cyb_rgch[CCHTTYBUF];	/* space for characters */
};

#define CYBS_FREE   0
#define CYBS_ALLOCATED    1	/* currently allocated to a tty driver */
#define CYBS_QUEUED 2		/* has packet in it that needs to be prcessed */
#endif sgi


/* 
 * Definitions used in performance monitoring.
 */
#define CCHLDATA 26		/* how much of bad packet we want to save */
				/* must be smaller than MLEN (/sys/h/mbuf.h) */
				/* for things to work properly */
struct cy_logrec {
#ifdef sgi
    time_t cyl_time;		/* time the log was made */
#else sgi
    struct timeval cyl_time;	/* time the log was made */
#endif sgi
    u_char cyl_type;		/* what this record is a log of */
    u_char cyl_ln;		/* line number */
    short  cyl_len;		/* how many bytes are in the data portion? */
    char   cyl_data[CCHLDATA];	/* first bytes of packet in question */
} ;

#define CYPRIO	(PZERO+1)
#define CYL_CLOGRECMAX	60
#define CYL_QOVERFLOW	1	/*   queue overflowed since last "read" */
#define CYL_SILOOVERFLOW 2	/* number of silo overflows */
#define CYL_NOBUFFERAVAIL 3	/* no buffer avail at interrupt time */
				/* log the line number and size of */
				/* packet that generated interrupt */
#define CYL_BADROUTE	4	/* direct packet was routed out over the*/
				/* same line that it came in on */
#define CYL_BADPKTTYPE	5	/* packet arrived that we don't know */
				/* how to handle */
#define CYL_BADPKTLEN	6	/* packet whose length was too long */
				/* was input */
#define CYL_NOMBUFS	7	/* no mbufs available */
#define CYL_PKTTOOBIG	8	/* in preparing a packet to be dumped to */
				/* a line, the buffer size exceeded. */
				/* This shouldn't happen, but might (??) */
				/* if we get garbled input that causes */
				/* a packet delimeter to get lost */
#define CYL_HOPCNTZERO  9	/* looping packet? its hpcnt went to 0 */
#define CYL_MTUEXCEEDED 10	/* packet larger than MTU handed to interface */
#define CYL_LOSTCARRIER 11	/* line lost carrier */
#define CYL_REGAINCARRIER 12	/* carrier detected again */
#define CYL_BADSTATECHANGE 13	/* unknown state change from one of the lines */
#define CYL_INVALIDDMVADDR 14	/* ??? dmv not talking to other side */
#define CYLOGNEXT(i)	(((i)+1) == CYL_CLOGRECMAX ? 0 : ((i)+1))
#ifdef sgi
#define CYL_MONITORTIMEOUT (HZ/20)
#define cyTurnOnMonitor()    if ((cy_dev.cyd_flags&CYF_MONITORON) == 0) {\
		    cy_dev.cyd_flags |= CYF_MONITORON;	\
		    timeout(cyMonitor, (caddr_t)0, CYL_MONITORTIMEOUT); \
		}
#else
#define CYL_MONITORTIMEOUT (hz/20)
#define cyTurnOnMonitor()    if ((cy_dev.cyd_flags&CYF_MONITORON) == 0) {\
			    cy_dev.cyd_flags |= CYF_MONITORON;	\
			    timeout(cyMonitor, NULL, CYL_MONITORTIMEOUT); \
				}
#endif
/* 
 * start of definitions used ioctl calls.
 */
struct cy_ioarg1 {
    int   Cyi_ln;			/* line number */
    struct sockaddr_in Cyi_sin;	/* host at other end of line */
    char Cyi_name[IFNAMSIZ];	/* interface to configure as a line */
};
struct cy_ioctlarg {
	char  cyi_name[IFNAMSIZ];	/* if name, e.g. "cy0" */
	union {
	    struct cy_ioarg1 cyu_arg1;
	    struct cy_logrec Cyi_logrec;
	} cyi_u;
#define cyi_ln cyi_u.cyu_arg1.Cyi_ln
#define cyi_sin cyi_u.cyu_arg1.Cyi_sin
#define cyi_logrec cyi_u.Cyi_logrec
#define cyi_saddr cyi_u.cyu_arg1.Cyi_sin.sin_addr.s_addr
#define cyi_ifname cyi_u.cyu_arg1.Cyi_name
} ;

#define CYIOC_ADDLINE	_IOW(c, 1, struct cy_ioctlarg)
#define CYIOC_SDESTADDR _IOW(c, 2, struct cy_ioctlarg)
#define CYIOC_SROUTE	_IOW(i, 3, struct cy_ioctlarg)
#define CYIOC_GETLOGREC _IOWR(i, 4, struct cy_ioctlarg)
#define CYIOC_FLUSHROUTES _IOW(i, 5, struct cy_ioctlarg)
#define CYIOC_SROUTEIP	_IOW(i, 6, struct cy_ioctlarg)
#define CYIOC_GNETNUM	_IOW(i, 7, struct cy_ioctlarg)
#ifndef sgi
#define CYIOC_ATTACHIF _IOW(i, 8, struct cy_ioctlarg)
#endif

#if sgi
#ifdef SVR3
#define ECY_BASE LASTERRNO
#else sgi
#define ECY_BASE 100
#endif sgi
#define ECYBADADDR	ECY_BASE
#define ELINEBUSY	(ECY_BASE+1)
#define ETTYBUSY	(ECY_BASE+2)
#define EBADLINE	(ECY_BASE+3)
#define ECYBADNET	(ECY_BASE+4)
#else sgi
#define ECYBADADDR	100
#define ELINEBUSY	101
#define ETTYBUSY	102
#define EBADLINE	103
#define ECYBADNET	104
#endif sgi


struct cy_dev {
    int cyd_cipup;		/* # input packets recieved from above(TCP)*/
    int cyd_cipln;		/* # input packets recieved from lines */
    int cyd_copln;		/* # output packets sent out on lines */
    int cyd_copup;		/* # output packets sent upward */
    int cyd_crint;		/* number of times dmf interrupt routine called */
    int cyd_logrechd;		/* head of queue. tl+1 == hd ==> full queue */
    int cyd_logrectl;		/* tail of queue. hd == tl ==> empty queue */
    int cyd_flags;		/* global flags */
    struct in_addr  cyd_haddr;	/* IP host address of this interface */
    struct in_addr  cyd_baddr;	/* IP broadcast address of this interface */
    struct in_addr  cyd_naddr;	/* network portion of our address */
    int	cyd_hid;		/* IP hostnumber of interface (Implet ID) */
#ifndef sgi
    struct cy_buf   *cyd_pbufqhd;		/* head of queue */
    struct cy_buf cyd_buf[CCYBUFMAX];
#endif sgi
    struct cy_logrec cyd_rglogrec[CYL_CLOGRECMAX];	/* queue of log records */
    short MPhln[CHOSTSMAX];	/* mapping of host number to line number */
    struct ifnet cyd_if;	/* interface for cypress device */
#ifndef sgi
    struct cy_dmabuf cyd_rgdmabuf[2*CLINEMAX];
				/* 2 buffers per line so that we can */
				/* fill one buffer while dma is going on */
#endif sgi
};
#define CYR_NOROUTE	-1	/* value in routing table for non-existant */
				/* route */
#define CYR_THISHOST	-2	/* send the packet to IP. This is a hack */
				/* that will allow us to do a funny */
				/* kind of subnetting, that is have and */
				/* IP route to a specific Cypress host  */
				/* that is not connected to other Cypress */
				/* machines via 9600 b lines. */

#define CYPROTO_CY 1		/* Cypress packets */
#define CYPROTO_RAW 0		/* For ioctls to protocol */
extern struct cy_dev cy_dev;
/* 
 * list of the variable name abreviations used in cypress code.
 * 
 * ln - line number
 * lr - log record
 * h  - host (as in a cypress host Number)
 */

#define AF_CYPRESS 17
