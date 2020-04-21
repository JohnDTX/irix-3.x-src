/*
 * pagemap stuff
 *
 * $Source: /d2/3.7/src/sys/ipII/RCS/cx.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:30:41 $
 */

/*
 * The pagemap is shared among many process's by a base/limit register.
 * The base/limit registers must be loaded for a user process when that
 * user process begins execution.
 *
 * The choice of which areas of the pagemap to use are based entirely on
 * size.
 *
 * Currently the IP2 utilizes the page map in the following manner:
 *
 *    32mb    16mb  4mb     4mb       8mb
 *     user processes      kernel  user stack
 *
 * Since each process contains two memory regions (p0 == data, p1 == stack)
 * the context allocation code has to determine which pieces of the map
 * are used by both regions, and allocate accordingly.
 */

/*
 * the number of contexts is computed in the following manner:
 *
 * NCX = number of ptes in the pagemap / number of ptes per 1 context
 *
 *  So for the IP2: NCX = 16384/4
 */
#define	NCXLOG2		12		/* LOG2( NCX )			*/
#define	NCX		(1<<NCXLOG2)	/* number of contexts		*/
#define	PTEPCX		4		/* number of ptes per context	*/
#define	PPCXLOG2	2		/* LOG2( PTEPCX )		*/

#define	CXBUSY		8192		/* context is in use */
#define	NIL		8193		/* end-of-chain value */

/* kernels context */
#define	KCX		0xd00		/* kernels context # */

#ifndef	LOCORE
/*
 * context structure, holds all information for allocating/deallocating
 * contexts, also holds the head of the active procs currently using contexts
 */
struct	cx {
	short	cx_tduser;		/* users text/data context */
	short	cx_suser;		/* users stack context */
	short	cx_tdsize;
	short	cx_ssize;
	char	cx_zeroinuse;		/* set if large context is in use */
	u_short	cx_map[NCX+1];		/* allocation map */
	u_short	cx_free[NCXLOG2+1];	/* headers to free lists */
	struct	proc *cx_head;		/* lru list */
} cx;
#endif	LOCORE
