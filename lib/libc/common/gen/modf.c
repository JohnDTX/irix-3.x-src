/*	@(#)modf.c	1.5	*/
/*LINTLIBRARY*/
/*
 * modf(value, iptr) returns the signed fractional part of value
 * and stores the integer part indirectly through iptr.
 *
 * There are two versions of this routine.  This is the single
 * precision version.  The double precision version is named _lmodf.
 */

#include <sys/signal.h>
#include <fperr.h>
#include <nan.h>
#include <values.h>

float
modf(value, iptr)
float value; /* don't declare register, because of KILLNaN! */
register float *iptr;
{
	/*register*/ float absvalue,temp;

	/* raise exception on Not-a-Number or infinity (3b or SGI only) */
	KILLFNaN(value); 
	absvalue = ((value >= 0.0) ? value : -value) ;
	temp = FMAXPOWTWO;
	if ( absvalue >= temp)
		*iptr = value; /* it must be an integer */
	else {
		*iptr = absvalue + temp; /* shift fraction off right */
		*iptr -= temp; /* shift back without fraction */
		while (*iptr > absvalue) /* above arithmetic might round */
			*iptr -= 1.0; /* test again just to be sure */
		if (value < 0.0)
			*iptr = -*iptr;
	}
	return (value - *iptr); /* signed fractional part */
}
