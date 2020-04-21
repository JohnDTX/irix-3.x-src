/*
 * pagemap stuff
 *
 * $Source: /d2/3.7/src/sys/pmII/RCS/cx.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:33 $
 */

/*
 * The pagemap is shared among many process's by a virtual hash (an xor)
 * with the upper 8 address bits.  The context register contains the
 * hash info for the current process, and is loaded by the system when
 * a user process begins execution.
 *
 * The choice of which areas of the pagemap to use are based entirely on
 * size.
 *
 * Since each process contains two memory regions (p0 == data, p1 == stack)
 * the context allocation code has to determine which pieces of the map
 * are used by both regions, and allocate accordingly.  To make this choice
 * easier, we allocate only the bottom half of the context region, using
 * the upper half for "reflections" (stack regions).
 */

/*
 * Context allocation map:
 *	- two data structures are allocated in this code
 *	- the cxmap[] is used contain the buddy allocation of the actual
 *	  context numbers (0-127)
 *	- the free lists headed by cxfree[] are singly linked in the forward
 *	  direction only
 */

#define	NCXLOG2		7
#define	NCX		128
#define	CXBUSY		0xFF		/* context is in use */
#define	NIL		0xFE		/* end-of-chain value */

/* kernels context (directly related to virtual constants in cpureg.h) */
#define	KCX		0x20		/* kernels context # */

#ifndef	LOCORE
struct	cx {
	char	cx_user;		/* current running users context */
	char	cx_zeroinuse;		/* set if large context is in use */
	u_char	cx_map[NCX+1];		/* allocation map */
	u_char	cx_free[NCXLOG2+1];	/* headers to free lists */
	struct	proc *cx_head;		/* lru list */
/* XXX	struct	proc *cx_tail;		*/
} cx;
#endif	LOCORE
