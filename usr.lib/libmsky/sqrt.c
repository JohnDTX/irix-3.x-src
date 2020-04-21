/*	@(#)sqrt.c	1.6	*/
/*LINTLIBRARY*/
/*
 *	sqrt returns the square root of its double-precision argument,
 *	using Newton's method.
 *	Returns EDOM error and value 0 if argument negative.
 *	Calls _lfrexp and _lldexp.
 */

#include <errno.h>
#include <math.h>
#include <fperr.h>
#include <sys/signal.h>
#include <nan.h>
#include <fpregs.h>
#include <fpopcodes.h>
#define ITERATIONS	4

long float
_lsqrt(x)
register long float x;
{
	register long float y;
	int iexp;
	register int i;
	static errtype;

	if (ISMaxExp(x)) 
		errtype = MANT(x)?INVALID_OP_A:INVALID_OP_F2;
	else if (x <= 0) {
			if (x != 0) 
				errtype = DOMAIN_ERROR;
			else return (0);
	} else {
		y = _lfrexp(x, &iexp);
		if (iexp % 2) {
			iexp--;
			y += y;
		}
		y = _lldexp(y + 1, iexp/2 - 1);
		for (i = ITERATIONS; --i >= 0; )
			y = 0.5 * (y + x / y);
		return (y);
	}
	/* exception has occurred.  type in errtype */
	_fperror.val.dval = x;
	_mathfunc_id = SQRT;
	return(_lraise_fperror(MATH,errtype));
}

float
sqrt(x)
float x;
{
	float result;
	static errtype;

	/* GB fix for scr 1455 (1/22/86). sky board returns 10-23 for sqrt(0) */
	if (x == 0) return(0);

	/* start the operation */
	*SKYCOMREG = HW_SQRT;
	*SKYFLREG = x;

	/* now test for erroneous input */
	errtype = 0;
	if (ISFMaxExp(x)) 
		errtype = FMANT(x)?INVALID_OP_A:INVALID_OP_F2;
	while (*SKYSTATREG > 0) ;
	result = *SKYFLREG;
	if (!errtype) {
		if (x < 0) 
			errtype = DOMAIN_ERROR;
		else
			return(result);
	} 

	/* exception has occurred.  type in errtype */
	_fperror.val.fval = x;
	_mathfunc_id = SQRT;
	return(_raise_fperror(MATH,errtype));
}

