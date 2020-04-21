/*	@(#)stak.h	1.2	*/
/*	3.0 SID #	1.2	*/
#
/*
 *	UNIX shell
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */

/* To use stack as temporary workspace across
 * possible storage allocation (eg name lookup)
 * a) get ptr from `relstak'
 * b) can now use `pushstak'
 * c) then reset with `setstak'
 * d) `absstak' gives real address if needed
 */
#define		relstak()	(staktop-stakbot)
#define		absstak(x)	(stakbot+Rcheat(x))
#define		setstak(x)	(staktop=absstak(x))
/* #define		pushstak(c)	(*staktop++=(c)) */
#define pushstak(c) {if (staktop >= brkend) setbrk(brkincr); *staktop++ = c; }
#define		zerostak()	(*staktop=0)

/* Used to address an item left on the top of
 * the stack (very temporary)
 */
#define		curstak()	(staktop)

/* `usestak' before `pushstak' then `fixstak'
 * These routines are safe against heap
 * being allocated.
 */
#define		usestak()	{locstak();}

/* for local use only since it hands
 * out a real address for the stack top
 */
extern STKPTR		locstak();

/* Will allocate the item being used and return its
 * address (safe now).
 */
#define		fixstak()	endstak(staktop)

/* For use after `locstak' to hand back
 * new stack top and then allocate item
 */
extern STKPTR		endstak();

/* Copy a string onto the stack and
 * allocate the space.
 */
extern STKPTR		cpystak();

/* Allocate given ammount of stack space */
extern STKPTR		getstak();

/* A chain of ptrs of stack blocks that
 * have become covered by heap allocation.
 * `tdystak' will return them to the heap.
 */
extern BLKPTR		stakbsy;

/* Base of the entire stack */
extern STKPTR		stakbas;

/* Top of entire stack */
extern STKPTR		brkend;

/* Base of current item */
extern STKPTR		stakbot;

/* Top of current item */
extern STKPTR		staktop;

/* Used with tdystak */
extern STKPTR		savstak();
