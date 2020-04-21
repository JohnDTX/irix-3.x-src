/*
 * Data structures for kernel profiling
 *
 * $Source: /d2/3.7/src/sys/h/RCS/kprof.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:29:40 $
 */

#ifdef	PROF

#define	NPROCEDURES	1000

/* allowable range of pc's */
#define	PCRANGE		0x80000

/* number of bytes in histogram buffer (well known in locore.s) */
#define	PROFSIZE	(128*1024)

/* data type of histogram info */
#define	HISTTYPE	long

/* # of slots in prof buf */
#define	PROFSLOTS	(PROFSIZE/sizeof(HISTTYPE))

/*
 * PROFSHIFT is the amount to shift a kernel pc by, so as to scale the
 * 256Kb worth of possible kernel pc's into the PROFSIZE (bytes) buffer.
 * Since each entry in the buffer is a long, we have to shift 2 more places
 * to get the actual slot #. So:
 *	PCRANGE / PROFSLOTS is the amount to scale the pc by
 *	PROFSHIFT should be that value, log2
 *
 * In essence, the assembly code does this:
 *	slotnumber = pc >> PROFSHIFT
 *	profbuf[slotnumber]++;
 */
#define	PROFSHIFT	4		/* log2(PCRANGE / PROFSLOTS) */

#define	TICKRATE	2000			/* hz, that is */

#ifndef	LOCORE
/* whether or not profiling is enabled */
extern	char	profiling;

/* each procedure call generates an mcount data structure */
struct	mcount {
	int	(*mc_func)();
	long	mc_count;
} mcbuf[NPROCEDURES];

/* pointer to buffer for holding mcount information */
struct	mcount *mcountbuf;

/* buffer containing long counters for timing ticks (valloc'd) */
long	*profbuf;
#endif
#endif
