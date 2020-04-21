/*
 * $Source: /d2/3.7/src/sys/ipII/RCS/vmparam.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:31:07 $
 */
/*
 * Machine dependent constants for IP2
 */

/*
 * USRTEXT is the start of the user text/data space, while USRSTACK
 * is the top (end) of the user stack.  LOWPAGES and HIGHPAGES are
 * the number of pages from the beginning of the P0 region to the
 * beginning of the text and from the beginning of the P1 region to the
 * beginning of the stack respectively.
 */

#define	P1PAGES		0x800		/* number of pages in P1 region */

/* pages above P1 region */
#define	GRPAGES		2		/* pages above P1 for graphics */
#define	HIGHPAGES	(GRPAGES)

#define	USRTEXT		0x1000		/* lowest possible addr of P0 region */
#define	USRSTACK	0x1fffe000	/* top of user stack + 1 */
#define	USRSTACK_SHMEM	0x1fffe000	/* base of user gl shared memory */

/*
 * Virtual memory related constants
 */
#define	MAXTDSIZ	(0x3400)	/* max text+data+stack size (clicks) */
#define	MAXSSIZ		P1PAGES		/* max stack size (clicks) */

/*
 * USRPTSIZE is the size of the kernel virtual region bounded by
 * USRPT_VBASE and USRPT_VLIMIT.
 */
/* XXX there is a copy of this constant defined in machine/param.h */
#ifndef	USRPTSIZE
/* was 224 */
#define	USRPTSIZE	/*btoc(USRPT_VLIMIT-USRPT_VBASE) == */ 480
#endif

/*
 * The size of the clock loop.
 */
#define	LOOPPAGES	(maxfree - firstfree)

/*
 * The time for a process to be blocked before being very swappable.
 * This is a number of seconds which the system takes as being a non-trivial
 * amount of real time.  You probably shouldn't change this;
 * it is used in subtle ways (fractions and multiples of it are, that is, like
 * half of a ``long time'', almost a long time, etc.)
 * It is related to human patience and other factors which don't really
 * change over time.
 */
#define	MAXSLP 		20

/*
 * A swapped in process is given a small amount of core without being bothered
 * by the page replacement algorithm.  Basically this says that if you are
 * swapped in you deserve some resources.  We protect the last SAFERSS
 * pages against paging and will just swap you out rather than paging you.
 * Note that each process has at least UPAGES+1 pages which are not
 * paged anyways so this
 * number just means a swapped in process is given around 25k bytes.
 * Just for fun: current memory prices are 4600$ a megabyte on VAX (4/22/81),
 * so we loan each swapped in process memory worth 100$, or just admit
 * that we don't consider it worthwhile and swap it out to disk which costs
 * $30/mb or about $0.75.
 */
#define	SAFERSS		(16384 / NBPG)	/* nominal ``small'' resident set size
					   protected against replacement */

/*
 * DISKRPM is used to estimate the number of paging i/o operations
 * which one can expect from a single disk controller.
 */
/* XXX this is junk; make it system + drive dependent */
#define	DISKRPM		60

/*
 * Klustering constants.  Klustering is the gathering
 * of pages together for pagein/pageout, while clustering
 * is the treatment of hardware page size as though it were
 * larger than it really is.
 *
 * KLMAX gives maximum cluster size in page
 * units.  Note that KLMAX must be <= DMMIN in dmap.h.
 */

#define	KLMAX	(4)
#define	KLSEQL	(2)		/* in klust if vadvise(VA_SEQL) */
#define	KLIN	(2)		/* default data/stack in klust */
#define	KLTXT	(2)		/* default text in klust */
#define	KLOUT	(4)

/*
 * KLSDIST is the advance or retard of the fifo reclaim for sequential
 * processes data space.
 */
#define	KLSDIST	2		/* klusters advance/retard for seq. fifo */

/*
 * The units here are blocks, defined by the ctod/dtoc macros.  We insure
 * that DMMIN is appropriate by making it the number of sectors in a
 * maximum kluster.  If these constants are changed, insure that NDMAP and
 * NXDAD in "machine/param.h" are updated.
 */
#define	DMMIN	ctod(KLMAX)
#define	DMMAX	1024
#define	DMTEXT	1024

/*
 * Paging thresholds (see vm_sched.c).
 * Strategy of 2/1/86:
 *	lotsfree is 512k bytes, but at most 1/4 of memory
 *	desfree is 200k bytes, but at most 1/8 of memory
 *	minfree is 64k bytes, but at most 1/2 of desfree
 */
#define	LOTSFREE	(512 * 1024)
#define	LOTSFREEFRACT	4
#define	DESFREE		(128 * 1024)
#define	DESFREEFRACT	8
#define	MINFREE		(64 * 1024)
#define	MINFREEFRACT	2

/*
 * Believed threshold (in megabytes) for which interleaved
 * swapping area is desirable.
 */
#define	LOTSOFMEM	2

/* XXX */
/* Abstract machine dependent operations */
#define	pcbb(p)		((p)->p_addr[0].pg_pfnum)
#define	setp0br(x)	u.u_pcb.pcb_p0br = (x)
#define	setp0lr(x)	u.u_pcb.pcb_p0lr = (x)
#define	setp1br(x)	u.u_pcb.pcb_p1br = (x)
#define	setp1lr(x)	u.u_pcb.pcb_p1lr = (x)
#define	initp1br(x)	((x) - P1PAGES)			/* XXX */
#define	tlbzap(p)
