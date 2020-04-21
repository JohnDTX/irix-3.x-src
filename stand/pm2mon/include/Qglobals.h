/*
 *	Quirk Global Variables Structure and Kernel-wide macros
 *		    Paul Haeberli and David J. Brown
 *			    June 20, 1983
 */

#ifndef QUIRKGLOBALS
#define QUIRKGLOBALS		/* Quirk Globals are now IN */

#include "ringbufs.h"
#include "Qtypes.h"

/* 
 * SCHED may change later to use software scheduled interrupts
 * it' s just procedure invocation for now
 */
#define	SCHED(routine)		(routine());
#define	MAKEREADY(devBits)	(Qreadyvector |= devBits)
#define	MAKENOTREADY(devBits)	(Qreadyvector &= ~devBits)

#ifndef TRUE
#define 	TRUE 		1
#endif

#ifndef FALSE
#define 	FALSE 		0
#endif

extern
ulong   Qreadyvector;  /* the Q ready vector */
extern
long 	Qupcount;      /* this is incremented while we are waiting */ 
extern
long	Qtime;	       /* for higher-level timers */

extern
char	do_debug;

#endif QUIRKGLOBALS
