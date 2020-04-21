/*	defs.h	1.1	01/08/86	*/


/*
 * typedefs
 */
typedef	long		iaddr_t;	/* internet address type */
typedef	struct ether_addr eaddr_t;	/* ether address type */
typedef unsigned short	n_short;
typedef unsigned long	n_time;

#if !defined(lowendian)
/*
 * Macros for number representation conversion.
 */
#define	ntohl(x)	(x)
#define	ntohs(x)	(x)
#define	htonl(x)	(x)
#define	htons(x)	(x)
#endif

#include "Xns.h"
#include "dprintf.h"
#include "errno.h"
#include "types.h"
#include "bootp/inet.h"
#include "bootp/tftp.h"
#include "bootp/bootp.h"
#include "bootp/ether.h"

/*
 * Define constants
 */
#define BOOTNTRIES	4	/* Number of bcasts for BOOTP */

/*
 * Packet formats
 */
struct ipmsg {			/* IP packet */
	struct ether_header eh;
	struct ip ih;
};

struct arpmsg {			/* ARP packet */
	struct ether_header eh;
	struct ether_arp ah;
};

struct udpmsg {			/* IP/UDP portion of packet */
	struct ip ih;
	struct udphdr uh;
};

struct bootpmsg {		/* BOOTP packet */
	struct ether_header eh;
	struct ip ih;
	struct udphdr uh;
	struct bootp bh;
};

struct tftpmsg {		/* TFTP packet (no data) */
	struct ether_header eh;
	struct ip ih;
	struct udphdr uh;
	struct tftphdr th;
};

struct tftpdmsg {		/* full TFTP packet (with data) */
	struct ether_header eh;
	struct ip ih;
	struct udphdr uh;
	struct tftphdr th;
	char data[SEGSIZE];	/* maximum of 512 data bytes per packet */
};

/*
 * Note that we are now using the Berkeley version of tftp.h,
 * which defines struct tftphdr to include one data byte.
 * So we need the following define in places where we used
 * to use (sizeof (struct tftphdr)).
 */
#define _THP_	((struct tftphdr *)0)	/* shorthand */
#define TFTP_HDR_SIZE	((int)(_THP_->th_data) - (int)(_THP_))

/*
 * Global symbols
 */
extern char *_readdatabuf;	/* Global network input buffer */
int _tcperr;			/* TCP error returned by lower level rtns */
iaddr_t btmyiaddr;		/* Internet address of this host */
eaddr_t btmyeaddr;		/* Ethernet address of this host */

/* BOOTP results */

iaddr_t bootp_siaddr;		/* Internet address of BOOTP server */
iaddr_t bootp_giaddr;		/* Internet address of gateway */
char bootp_sname[64];		/* Name of BOOTP server */
char bootp_file[128];		/* Name of BOOTP file */
int bootp_xid;			/* BOOTP transaction id */

/* Mini ARP address cache and outgoing IP message queue */

iaddr_t arpiaddr;		/* Current ARP Internet address */
struct ether_addr arpeaddr;	/* Current ARP Ethernet address */
int arpwait;			/* ARP pending flag */
int arpvalid;			/* Current ARP mapping is valid */
struct ipmsg *arpqueue;		/* Outgoing msg waiting for ARP reply */
int arpqueuelen;		/* Length of pending outgoing message */

/* TFTP connection state */

int tftp_blkno;			/* Last ACKed block for current TFTP conn */
int tftp_eof;			/* Final block has been recvd from TFTP */
char *tftp_dataptr;		/* Pointer to input TFTP data */
short tftp_srvport;		/* Server UDP port number for TFTP */
short tftp_myport;		/* Client UDP port number for TFTP */
