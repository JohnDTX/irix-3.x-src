/*	@(#)frexp.c	1.5	*/
/*LINTLIBRARY*/
/*
 * frexp(value, eptr)
 * returns a double x such that x = 0 or 0.5 <= |x| < 1.0
 * and stores an integer n such that value = x * 2 ** n
 * indirectly through eptr.
 *
 * there are two versions of this at SGI.  This version returns a float
 * and receives a float as the value arg.  The double precision version
 * (_lfrexp) uses long floats. It is in _lfrexp.c. (GB) 9/19/83.
 */
#include <sys/signal.h>
#include <fperr.h>
#include <nan.h>

float
frexp(value, eptr)
float value; /* don't declare register, because of KILLNan! */
register int *eptr;
{
	register float absvalue;

	/* raise exception on Not-a-Number or infinity (3b or SGI only) */
	KILLFNaN(value); 
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
