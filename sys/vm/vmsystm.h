/*	vmsystm.h	6.1	83/07/29	*/

/*
 * Miscellaneous virtual memory subsystem variables and structures.
 */

#ifdef KERNEL
int	freemem;		/* remaining blocks of free memory */
int	avefree;		/* moving average of remaining free blocks */
int	avefree30;		/* 30 sec (avefree is 5 sec) moving average */
int	deficit;		/* estimate of needs of new swapped in procs */
int	nscan;			/* number of scans in last second */
int	desscan;		/* desired pages scanned per second */
int	lotsfree;		/* max free before clock freezes */
int	minfree;		/* minimum free pages before swapping begins */
int	desfree;		/* no of pages to try to keep free via daemon */
char	pager_sleeping;		/* non-zero if pager is sleeping */
int	vm_maxpushes;		/* maximum # of pushes per pageout() */
int	vm_savedpages;		/* total # of XSAVE'd pages in xfree() */
#endif

/*
 * Fork/vfork accounting.
 */
struct	forkstat
{
	int	cntfork;
	int	cntvfork;
	int	sizfork;
	int	sizvfork;
};
#ifdef KERNEL
struct	forkstat forkstat;
#endif

/*
 * Swap kind accounting.
 */
struct	swptstat
{
	int	pteasy;		/* easy pt swaps */
	int	ptexpand;	/* pt expansion swaps */
	int	ptshrink;	/* pt shrinking swaps */
	int	ptpack;		/* pt swaps involving spte copying */
};
#ifdef KERNEL
struct	swptstat swptstat;
#endif
