/*	@(#)ldexp.c	2.4	*/
/*LINTLIBRARY*/
/*
 * ldexp(value, exp) returns the quantity value * 2 ** exp.
 * In the event of overflow, on UNIX a floating-point exception is signalled.
 * In the event of underflow, zero is returned.
 *
 * Two versions of this routine exist.  This version is for 32-bit floats.
 * The long float routine is under the name _lldexp.c
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

float
ldexp(value, exp)
register float value;
register int exp;
{
	extern float frexp();
	int old_exp;

	if (exp == 0 || value == 0.0) /* nothing to do for zero */
		return (value);
#ifndef	pdp11
#ifndef	u3b
	(void)	/* Conditional only for 4.2 expediency!  Fix for 5.0! */
#endif
#endif
	frexp(value, &old_exp);
	if (exp > 0) {
		if (exp + old_exp > FMAXEXP) /* overflow */ {
			return(_raise_fperror(CONVERT,OVERFL));
		}
		for ( ; exp > MAXSHIFT; exp -= MAXSHIFT)
			value *= (1L << MAXSHIFT);
		return (value * (1L << exp));
	}
	if (exp + old_exp < FMINEXP) /* underflow */
		return(_raise_fperror(CONVERT,UNDERFLOW_A));
	for ( ; exp < -MAXSHIFT; exp += MAXSHIFT)
		value *= 1.0/(1L << MAXSHIFT); /* mult faster than div */
	return (value / (1L << -exp));
}
