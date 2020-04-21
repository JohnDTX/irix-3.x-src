/*
 * Machine dependent constants for pmII.
 *
 * $Source: /d2/3.7/src/sys/pmII/RCS/param.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:45 $
 */
#define	FP_SKY	1		/* for user.h */
#undef	HAVERTC			/* we DON'T have a real time clock */
#define	HZ	60

#define	NBPG	4096		/* bytes/page */
#define	PGOFSET	(NBPG-1)	/* byte offset into page */
#define	PGSHIFT	12		/* LOG2(NBPG) */

#ifdef	KERNEL
/* maximum chunk of physical i/o */
#define	MAXPHYS		(256*1024)

#ifdef	GL1
#define	MAXUPRC		30
#else
#define	MAXUPRC		50	/* max # of processes per user */
#endif

#endif

#define	SSIZE	1		/* initial stack size/NBPG */
#define	SINCR	1		/* increment of stack/NBPG */
#define	UPAGES	1		/* pages of u-area */

#ifndef	BITSPERBYTE
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
#define	BBTOPGT(n)	((n) >> BBPGSHIFT)
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

/* XXX there is a copy of this constant defined in machine/vmparam.h */
#ifndef	USRPTSIZE
#define	USRPTSIZE	208
#endif

#ifdef	KERNEL
#ifdef	INET
/* for mbuf code... */
#define	NMBCLUSTERS	USRPTSIZE
#endif
#endif

/* XXX */
#define	setredzone(a, b)
#define	suiword(a, b)	suword(a, b)
#define	fuiword(a)	fuword(a)

/* machine dependent idle code */
#define	IDLE() \
	asm("stop #0x2000");

#define	spltty()	spl6()
#define	splmax()	spl7()

/* for backwards compatability */
#undef	PMII
#undef	pmII
#undef	PM2
#define	PMII
#define	pmII
#define	PM2

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
