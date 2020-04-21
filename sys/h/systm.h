/* @(#)systm.h	1.3 */
/*
 * RANDOM set of variables used by more than one routine.
 */
extern	time_t lbolt;		/* time in HZ since last boot */
extern	time_t time;		/* time in sec from 1970 */

extern	char runin;		/* scheduling flag */
extern	char runout;		/* scheduling flag */
extern	char runrun;		/* scheduling flag */
extern	char curpri;		/* current priority */
extern	int hz;			/* clock frequency */

/* additions due to paging subsystem */
extern	short maxmem;		/* actual max memory per process */
extern	daddr_t swplo;		/* swap area base address */
extern	daddr_t nswap;		/* swap area length */

extern	dev_t rootdev;		/* device of the root */
extern	dev_t swapdev;		/* swapping device */
extern	short rootfs;		/* root fs to use when forced */
extern	int *nofault;		/* when nonzero, buserr cause longjmp */
extern	int sspeed;		/* default console speed */
extern	char regloc[];		/* locs of saved user registers (trap.c) */
extern	short havefpa;		/* system has a floating point accelerator */
extern	short kdebug;		/* non-zero if kernel debugging is on */

extern	char hostname[];	/* this hosts name */
extern	short hostnamelen;	/* length of above */
extern	char domainname[];	/* this host's domain name */
extern	short domainnamelen;	/* length of above */

int	minphys();
int	memall();
int	vmemall();

/* kernel memory allocation routines */
#ifdef HEAP_DUMP
char	*_malloc();
char	*_calloc();
#else
char	*malloc();
char	*calloc();
#endif
char	*realloc();
void	free();

/* resource map routines */
long	rmalloc();
