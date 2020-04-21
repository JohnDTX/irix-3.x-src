/*
 * $Source: /d2/3.7/src/sys/ipII/RCS/param.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:30:55 $
 */
/*
 * Machine dependent constants for IP2 (juniper).
 * These constants are the same as for the PMII.  There are a
 * few additions located at the end.
 */
#define	FPA	1		/* for user.h */
#define	HAVERTC			/* we DO have a real time clock */
#define	HZ	64

#define	NBPG	4096		/* bytes/page */
#define	PGOFSET	(NBPG-1)	/* byte offset into page */
#define	PGSHIFT	12		/* LOG2(NBPG) */

#ifdef	KERNEL
/* maximum chunk of physical i/o */
#define	MAXPHYS		(256*1024)
#define	MAXUPRC		100	/* max # of processes per user */
#endif

#define	SSIZE	1		/* initial stack size/NBPG */
#define	SINCR	1		/* increment of stack/NBPG */
#define	UPAGES	1		/* pages of u-area */

#ifndef BITSPERBYTE
#define	BITSPERBYTE		8	/* # of bits per byte */
#endif

#ifndef	BITSPERBYTESHIFT
#define	BITSPERBYTESHIFT	3	/* log2(BITSPERBYTE) */
#endif

/*
 * Basic block size:  This models the underlying sector size of all media
 * present. (often called a ``bb'').  We ASSUME in this paremeterization
 * that a bb is smaller than a page!
 */
#define	BBSHIFT		9			/* size of bb, in bytes, log2 */
#define	BBSIZE		(1<<BBSHIFT)		/* size of bb, in bytes */
#define	BBMASK		(BBSIZE-1)
#define	BBPGSHIFT	(PGSHIFT - BBSHIFT)	/* bb's per page, log2 */
#define	BBPG		(1<<BBPGSHIFT)		/* bb's per page */

/* bytes to basic blocks and vice-versa */
#define	BTOBBT(n)	((n)>>BBSHIFT)
#define	BTOBB(n)	(((n)+BBSIZE-1)>>BBSHIFT)
#define	BBTOB(n)	((n)<<BBSHIFT)

/* bb's to pages and vice-versa */
#define	BBTOPG(n)	(((n) + BBPG - 1) >> BBPGSHIFT)
#define	PGTOBB(n)	((n) << BBPGSHIFT)

/*
 * Macros to decode processor status word.
 */
#define	BASEPRI(ps)	(((ps) & PS_IPL) == 0)	/* CPU base priority */
#define	USERMODE(ps)	(((ps) & PS_SUP) == 0)	/* User mode definition */

#define	DELAY(n)	{ register ulong N = (n); while (--N); }

/*
 * These are extra large, just to make things work easier
 */
#define	NDMAP	64		/* size of the swap area map */
#define	NXDAD	32		/* 16Mb, given DMTEXT == 1024 */

/* XXX */
#define	setredzone(a, b)
#define	suiword(a, b)	suword(a, b)
#define	fuiword(a)	fuword(a)


#define	ONEMEGPG	0x100	/* number of pages in 1mb	*/
#define	ONEMEG		(0x100<<PGSHIFT)
#define	HALFMEGPG	0x80
#define	HALFMEG		(0x80<<PGSHIFT)

#define	MAXMEMMB	32	/* maximum memory in 1 megabyte chunks	*/

/* XXX there is a copy of this constant defined in machine/vmparam.h */
#ifndef	USRPTSIZE
#define	USRPTSIZE	/*btoc(USRPT_VLIMIT-USRPT_VBASE) == */ 480
#endif

#ifdef	KERNEL
#ifdef	INET
/* for mbuf code... */
#define	NMBCLUSTERS	USRPTSIZE
#endif
#endif

/* machine dependent idle code */
#define	IDLE() \
	asm("stop #0x2000");

#define	spltty()	spl6()
#define	splmax()	spl7()

/* for backwards iris compatability.... */
#undef	IP2
#undef	ipII
#undef	IPII
#undef	juniper
#define	IP2
#define	ipII
#define	IPII
#define	juniper

#ifdef	INET
/* for 4.2 compatability... */
#define	ovbcopy(f, t, n)	bcopy(f, t, n)
#define	imin(a, b)		((a) < (b) ? (a) : (b))
#define	insque(q,p)	_insque((caddr_t)q,(caddr_t)p)
#define	remque(q)	_remque((caddr_t)q)
/* #define	queue(q,p)	_queue((caddr_t)q,(caddr_t)p) */
/* #define	dequeue(q)	_dequeue((caddr_t)q) */

/* splimp must be highest network if priority */
#define	splimp()	spl2()

/* splnet must be non-zero; it can be less than splimp() */
#define	splnet()	spl1()

#endif	/* INET */
