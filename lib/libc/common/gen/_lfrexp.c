/*	@(#)frexp.c	1.5	*/
/*LINTLIBRARY*/
/*
 * frexp(value, eptr)
 * returns a double x such that x = 0 or 0.5 <= |x| < 1.0
 * and stores an integer n such that value = x * 2 ** n
 * indirectly through eptr.
 *
 */
#include <sys/signal.h>
#include <fperr.h>
#include <nan.h>

long float
_lfrexp(value, eptr)
long float value; /* don't declare register, because of KILLNan! */
register int *eptr;
{
	register long float absvalue;

	/* raise exception on Not-a-Number or infinity (3b or SGI only) */
	KILLNaN(value); 
	*eptr = 0;
	if (value == 0.0) /* nothing to do for zero */
		return (value);
	absvalue = (value > 0.0) ? value : -value;
	for ( ; absvalue >= 1.0; absvalue *= 0.5)
		++*eptr;
	for ( ; absvalue < 0.5; absvalue += absvalue)
		--*eptr;
	return (value > 0.0 ? absvalue : -absvalue);
}
