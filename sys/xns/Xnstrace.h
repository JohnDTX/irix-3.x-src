#ifdef	XTRACE

#ifdef	MSGTRACE
#define NXTRACE(a)
#else
#define NXTRACE(a)		nxtrace a
#endif

#define XXTRACE(a)		nxtrace a

/*
 * An array (NXCELLS deep) is kept of these data structures keeping a log
 * of the last NXCELLS trace messages
 */

#define NXCELLS	600

struct	xtrace {
	char	*code;			/* pointer to message */
	long	p0, p1, p2, p3, p4;	/* some values */
	time_t	time;			/* time stamp */
} xtbuf[NXCELLS];

short	xtp;				/* current index into xtbuf[] */

#else
#define	NXTRACE(a)
#define	XXTRACE(a)
#endif
