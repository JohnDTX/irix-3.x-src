/*
 * Kernel debugging support
 *
 * $Source: /d2/3.7/src/sys/debug/RCS/flags.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:27:46 $
 */

#ifdef OS_DEBUG
/*
 * Debugging flags.  Using a procedure call, dbgtest, the kernel can test
 * if a debugging flag is enabled or disabled.  Using dbgon, the kernel
 * can enable a debugging flag.  Using dbgoff, the kernel can disable
 * a debugging flag.
 */

/*
 * Maximum number of settable flags.
 */
#define	MAXFLAGS	256

/*
 * flag numbers
 */
/* system calls/traps */
#define	DBG_SYSCALL		1
#define	DBG_TRAP		2
#define	DBG_PAGEFAULT		3

/* filesystem */
#define	DBG_NAMEI		10

/* i/o sub-system */
#define	DBG_WRITELOCK		50
#define	DBG_SII_WRITELOCK	51
#define	DBG_DSD_WRITELOCK	52
#define	DBG_PHYSIO		53

/* kmem allocaiton */
#define	DBG_KMEM_MEM		100
#define	DBG_KMEM_MAP		101
#define	DBG_KMEM_DMA		102

#ifdef	DBG_NAMES
/*
 * Symbolic versions of flag names
 */
struct dbg_name {
	int	flag;
	char	*symbol;
};

struct	dbg_name dbg_names[] = {
	{ DBG_SYSCALL,		"syscall" },
	{ DBG_TRAP,		"trap" },
	{ DBG_PAGEFAULT,	"page fault" },
	{ DBG_NAMEI,		"namei" },
	{ DBG_WRITELOCK,	"write lock" },
	{ DBG_SII_WRITELOCK,	"storager write lock" },
	{ DBG_DSD_WRITELOCK,	"dsd write lock" },
	{ DBG_PHYSIO,		"physical i/o" },
	{ DBG_KMEM_MEM,		"kernel memory allocation" },
	{ DBG_KMEM_MAP,		"kernel memory map" },
	{ DBG_KMEM_DMA,		"kernel memory dma map" },
	0
};
#endif	/* DEBUG_NAMES */

extern	int	dbgtest();
extern	void	dbgon(), dbgoff();

#else	/* OS_DEBUG */

/* define away procedures */
#define	dbgtest(x)	(0)
#define	dbgon(x)
#define	dbgoff(x)

#endif	/* OS_DEBUG */
