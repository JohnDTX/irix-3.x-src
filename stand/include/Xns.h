/*
* $Source: /d2/3.7/src/stand/include/RCS/Xns.h,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:13:33 $
*/

/*
 * Network address and header definitions
 * for Xerox Network Systems protocols.	
 */
#ifndef XNSDEF
#define XNSDEF


/*
 * network address
 */
typedef struct {
    unsigned short	high;
    unsigned short	low;
} Xnet;	




/*
 * host address
 */
typedef struct {
    unsigned short	high;
    unsigned short 	mid;
    unsigned short	low;
} Xhost;




/*
 * socket address
 */
typedef unsigned short Xsocket;



/*
 * internet address
 */
typedef struct {
    Xnet 	net;
    Xhost    	host;
    Xsocket	socket;
} Xaddr;



/*
 * packet exchange id
 */
typedef struct {
    unsigned short	high;
    unsigned short	low;
} Xpexid;





/*
 * ethernet header
 */
typedef struct {
    Xhost		dst;
    Xhost		src;
    unsigned short	etype;
} etheader;


/*
 * internet datagram header 
 */
typedef struct {
    unsigned short	checksum;
    unsigned short	length;
    unsigned char	control;
    unsigned char	idtype;
    Xaddr		dst;
    Xaddr		src;
} Xidheader;


/*
 * echo header
 */
typedef struct {
    unsigned short operation;	
} Xechoheader;



/*
 * error header
 */
typedef struct {
    unsigned short 	errnum;
    unsigned short 	errparam;
} Xerrheader;
	


/*
 * sequenced packet protocol header
 */
typedef struct {
    unsigned char	control;
    unsigned char 	dtype;
    unsigned short	srcid;
    unsigned short	dstid;
    unsigned short	seqno;
    unsigned short	ackno;
    unsigned short	allocno;
} Xseqheader;


/*
 * ethernet + internet + sequenced packet header
 */
typedef struct {
	Xhost		edst;
	Xhost		esrc;
	unsigned short	etype;
	unsigned short	checksum;
	unsigned short	length;
	unsigned char	control;
	unsigned char	idtype;
	Xaddr		idst;
	Xaddr		isrc;
	unsigned char	tcontrol;
	unsigned char	dtype;
	unsigned short	srcid;
	unsigned short	dstid;
	unsigned short	seqno;
	unsigned short	ackno;
	unsigned short	allocno;
} Xseq;
typedef Xseq * XSEQ;


typedef struct {
	Xhost		edst;
	Xhost		esrc;
	unsigned short	etype;
	unsigned short	checksum;
	unsigned short	length;
	unsigned char	control;
	unsigned char	idtype;
	Xaddr		idst;
	Xaddr		isrc;
	unsigned short	operation;
} Xecho;
typedef Xecho * XECHO;




/*
 * packet exchange header
 */
typedef struct {
    Xpexid		id;
    unsigned short	client;
} Xpexheader;	 


#define INFOSTRLEN 100

typedef struct 
{
    etheader 	etherheader;
    short	exchid;
    char 	infostr[INFOSTRLEN];
} Xserverreq;

#define	NSIZE	64
struct bouncemsg {
	etheader	et;
	short		id;
	char		cmd;
	char		colon;
	char		infostr[NSIZE];
};
typedef struct bouncemsg * BOUNCE;

#define SERV_BOOTIRIS	'A'
#define SERV_BOOTUNIX	'B'
#define SERV_SENDFILE	'C'
#define SERV_LOGIN	'D'
#define SERV_REPLY	'E'
#define SERV_NOFILE	'F'
#define	SERV_IDENT	'G'
#define	SERV_HOSTNAME	'H'
#define	SERV_READFILE	'I'
#define	SERV_WRITEFILE	'J'
#define	SERV_OPENFILE	'K'
#define	SERV_DATE	'L'
#define	SERV_IDENT_TCP	'M'
#define	SERV_RSYSTEM	'N'

/*
 *	defines specific to XNS	
 */
#define IDETHERTYPE	0x0600		/* Xerox NS Internet ethertype */
#define	SG_DIAG		0x8013		/* SGI diagnostic type */
#define	SG_NETGAMES	0x8014		/* SGI network games */
#define	SG_BOUNCE	0x8016		/* SGI reserved type */
#define NOCHECKSUM 	0xffff		/* No check sum calculated */



/*
 * well-known sockets
 */
#define ANYSOCKET	0	/* 	xns well known sockets 		*/
#define ROUTEINFOSOCKET	1
#define ECHOSOCKET	2
#define ROUTEERRSOCKET	3

#define SERIALSOCKET  	041     /* 	iris terminal sockets	*/
#define LOADSOCKET  	042
#define STATUSSOCKET  	043
#define	LOGINSOCKET	044
#define GRAPHICSSOCKET  045	

#define NAMESOCKET	050	/*	host service sockets	*/
#define BOOTSOCKET	051
#define	FILESOCKET	052
#define	VRINGSOCKET	053
#define	KEYSOCKET	054
#define	EXECSOCKET	055
#define	RSYSTEMSOCKET	056
#define	XSHSOCKET	057

#define IDSTARTSOCKET 	3001



/*
 * IDP types
 */
#define ROUTETYPE	1
#define ECHOTYPE	2
#define ERRORTYPE	3
#define PEXTYPE		4
#define SEQTYPE		5



/*
 * echo protocol
 */
#define ECHOREQUEST 	1
#define ECHOREPLY 	2



/*
 * for packet exchange and sequenced packet protocols
 */
#define REQUESTOR 	1
#define LISTENER 	2

/*
 *	sequenced packet data stream types
 */
#define DST_SYSTEM	200
#define DST_END		254
#define DST_ENDREPLY	255
#define	DST_ENQ		(DST_SYSTEM|0100000)
#define	DST_OLDDATA	100
#define DST_OEND	101
#define DST_OENDREPLY	102
#define	DST_BLOCK	103
#define	DST_FERROR	104
#define	DST_CMD		105
#define	DST_EOF		106
#define	DST_STATUS	107
#define DST_DATA	108
#define	DST_GETFERROR	109
#define	DST_RPC		110
#define	DST_RPCDATA	111


/*
 * sequenced packet protocol
 */
#define SYSTEMPACKET 	0x80
#define SENDACK 	0x40
#define ATTENTION 	0x20
#define ENDOFMESSAGE 	0x10
#define CONNCONTROLBITS (SYSTEMPACKET | SENDACK | ATTENTION | ENDOFMESSAGE)

#define	UNKNOWN_CONN_ID	0
#define	UNKNOWN		0

#define	READTIMEOUT	(-2)



#define	EQHOST(a,b)	(a.host.high==b.host.high && \
			a.host.mid==b.host.mid &&  \
			a.host.low==b.host.low)

#define	EQNET(a,b)	(a.net.high==b.net.high && \
			a.net.low==b.net.low)

#define	EQADDR(a,b)	(a.socket==b.socket && EQNET(a,b) && EQHOST(a,b))




/*
 * Structure expected by connection setup routine.
 */
struct xns_setup {
	Xhost physaddr;
	Xaddr internet;
	char name[NSIZE];
};
typedef struct xns_setup * SETUP;


/*
 * Shape of route table entry.
 */
struct xns_route {
	struct xns_route *next;
	struct xns_route *active;
	Xhost physaddr;
	Xaddr internet;
	long age;
	char lock;
	char type;
	char hit;
	char busy;
	char name[NSIZE];
};
typedef struct xns_route * ROUTE;


#define NDEV	32			/* number of network special files */

#endif
