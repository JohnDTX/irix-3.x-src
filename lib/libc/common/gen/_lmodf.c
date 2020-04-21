/*	@(#)modf.c	1.5	*/
/*LINTLIBRARY*/
/*
 * _lmodf(value, iptr) returns the signed fractional part of value
 * and stores the integer part indirectly through iptr.
 *
 * This is the long float version.
 *
 */

#include <sys/signal.h>
#include <fperr.h>
#include <nan.h>
#include <values.h>

long float
_lmodf(value, iptr)
long float value; /* don't declare register, because of KILLNaN! */
register long float *iptr;
{
	register long float absvalue;

	/* raise exception on Not-a-Number or infinity (3b or SGI only) */
	KILLNaN(value); 
	if ((absvalue = (value >= 0.0) ? value : -value) >= MAXPOWTWO)
		*iptr = value; /* it must be an integer */
	else {
		*iptr = absvalue + MAXPOWTWO; /* shift fraction off right */
		*iptr -= MAXPOWTWO; /* shift back without fraction */
		while (*iptr > absvalue) /* above arithmetic might round */
			*iptr -= 1.0; /* test again just to be sure */
		if (value < 0.0)
			*iptr = -*iptr;
	}
	return (value - *iptr); /* signed fractional part */
}
