/*
 * The callout structure is for a routine arranging
 * to be called by the clock interrupt
 * (clock.c) with a specified argument,
 * in a specified amount of time.
 * Used, for example, to time tab delays on typewriters.
 *
 * $Source: /d2/3.7/src/sys/h/RCS/callout.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:29:12 $
 */

struct	callout {
	int	c_time;			/* incremental time */
	caddr_t	c_arg;			/* argument to routine */
	int	(*c_func)();		/* routine */
	struct	callout *c_next;	/* link to next entry */
	int	c_id;			/* ID for its removal */
};

#ifdef	KERNEL
struct	callout *callout;		/* callout table */
struct	callout *callfree;		/* free list of callout entries */
struct	callout calltodo;		/* next item to do */
short	ncallout;			/* # of the beasties */
#endif
