/*	@(#)ldexp.c	2.4	*/
/*LINTLIBRARY*/
/*
 * ldexp(value, exp) returns the quantity value * 2 ** exp.
 * In the event of overflow, on UNIX a floating-point exception is signalled.
 * In the event of underflow, zero is returned.
 *
 * This is the version for long floats.
 */

#include <sys/signal.h>
#include <fperr.h>
#include <nan.h>
#include <values.h>
/* Largest long int power of 2 */
#define MAXSHIFT	(BITSPERBYTE * sizeof(long) - 2)
#if gcos
#undef KILLFPE()
#define KILLFPE()	/* let it abort via hardware trap */
#endif

long float
_lldexp(value, exp)
register long float value;
register int exp;
{
	extern long float _lfrexp();
	int old_exp;

	if (exp == 0 || value == 0.0) /* nothing to do for zero */
		return (value);
#ifndef	pdp11
#ifndef	u3b
	(void)	/* Conditional only for 4.2 expediency!  Fix for 5.0! */
#endif
#endif
	_lfrexp(value, &old_exp);
	if (exp > 0) {
		if (exp + old_exp > MAXBEXP) /* overflow */ {
			return(_lraise_fperror(CONVERT,OVERFL));
		}
		for ( ; exp > MAXSHIFT; exp -= MAXSHIFT)
			value *= (1L << MAXSHIFT);
		return (value * (1L << exp));
	}
	if (exp + old_exp < MINBEXP) /* underflow */
		return(_lraise_fperror(CONVERT,UNDERFLOW_A));
	for ( ; exp < -MAXSHIFT; exp += MAXSHIFT)
		value *= 1.0/(1L << MAXSHIFT); /* mult faster than div */
	return (value / (1L << -exp));
}
