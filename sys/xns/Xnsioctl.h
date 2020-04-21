/*
 * Xns ioctl's
 */
#define	NXIOCTYPE	('x'<<8)

#define	NXIOTEST	(('x'<<8)|1)
#define	NXIOFAST	(('x'<<8)|2)
#define	XNSINIT		(('x'<<8)|3)
#define	NXIOBOUNCE	(('x'<<8)|4)
#define	NXIORAW		(('x'<<8)|5)
#define	NXSOCKET	(('x'<<8)|6)
#define	NXPHYSADDR	(('x'<<8)|7)
#define	NXSOCKWAIT	(('x'<<8)|8)
#define	NXIOCHECK	(('x'<<8)|9)
#define	NXCONNECT	(('x'<<8)|10)
#define	NXAVAIL		(('x'<<8)|11)
#define	NXIOSLOW	(('x'<<8)|12)
#define	NXSETPGRP	(('x'<<8)|13)
#define	NXWRITE		(('x'<<8)|15)
#define	NXREAD		(('x'<<8)|16)
#define	NXTSTAMP	(('x'<<8)|17)
#define	NXBLOCKIO	(('x'<<8)|18)
#define	NXBLOCKOFF	(('x'<<8)|19)
#define	NXGETROUTE	(('x'<<8)|20)
#define	NXGETCONN	(('x'<<8)|21)
#define	NXGETSOCK	(('x'<<8)|22)
#define	NXPUTSLOT	(('x'<<8)|23)
#define	NXGETSLOT	(('x'<<8)|24)
#define	NXSETRCV	(('x'<<8)|25)
#define	NXREADS		(('x'<<8)|26)
#define NXSETSOCKWAIT   (('x'<<8)|27)   /* NXSOCKET + NXSOCKWAIT */
#define NXAVOPEN	(('x'<<8)|28)
#define NXWATCH		(('x'<<8)|29)
#define	NXBOINK		(('x'<<8)|30)
#define	NXSTATS		(('x'<<8)|31)
#define NXUPROTO	(('x'<<8)|32)

struct xnsio {
	char *addr;
	long count;
	char dtype;
	char control;
};

/*
 * Data returned from NXSTATS call
 */
struct	nxstat {
	long	nxs_okxmit;	/* frames sent with no errors */
	long	nxs_aborts;	/* frames aborted because of collisions */
	long	nxs_reserved;
	long	nxs_tdr;	/* time domain reflectometer */
	long	nxs_okrcv;	/* frames recieved with no errors */
	long	nxs_misaligned;	/* frames received with alignment error */
	long	nxs_crc;	/* frames received with crc error */
	long	nxs_lost;	/* frames lost because of no buffers */
};
