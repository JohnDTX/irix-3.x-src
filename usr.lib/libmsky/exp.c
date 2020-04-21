/*	@(#)exp.c	1.6	*/
/*LINTLIBRARY*/
/*
 *	exp returns the exponential function of its double-precision argument.
 *	Returns ERANGE error and value 0 if argument too small,
 *	   value HUGE if argument too large.
 *	Algorithm and coefficients from Cody and Waite (1980).
 *	Calls ldexp.
 *
 *	This version uses hardware floating point for single precision.
 *  (GB -SGI).
 */

#include <math.h>
#include <errno.h>
#include <fperr.h>
#include <sys/signal.h>
#include <nan.h>
#include <fpregs.h>
#include <fpopcodes.h>
#define LOGHUGE	((DMAXEXP + 1) / LOG2E)
#define LOGTINY	((DMINEXP - 1) / LOG2E)
#define XMIN	(1.0/(1L << (DSIGNIF/2)))
#define LOG2E	1.4426950408889634074
#define HALF	q[3] /* 0.5 */

long float
_lexp(arg)
register long float arg;
{
	static long float p[] = {
		0.31555192765684646356e-4,
	        0.75753180159422776666e-2,
	        0.25000000000000000000e0,
	}, q[] = {
		0.75104028399870046114e-6,
	        0.63121894374398503557e-3,
	        0.56817302698551221787e-1,
	        0.50000000000000000000e0,
	};
	register long float x, y;
	register int n;

	if (ISMaxExp(arg)) {
		_fperror.val.dval = arg;
		_mathfunc_id = EXP;
		return(_lraise_fperror(MATH,
			MANT(arg)?INVALID_OP_A:INVALID_OP_F2));
	}
	if (arg < LOGTINY) {
		_fperror.val.dval = arg;
		_mathfunc_id= EXP;
		return(_lraise_fperror(MATH,UNDERFL));
	}
	if (arg > LOGHUGE) {
		_fperror.val.dval = arg;
		_mathfunc_id= EXP;
		return(_lraise_fperror(MATH,OVERFL));
	}
	if ((x = _ABS(arg)) < XMIN)
		return (1);
	n = (int)(x * LOG2E + HALF);
	y = (long float)n;
	_REDUCE(int, x, y, 0.693359375, -2.1219444005469058277e-4);
	if (arg < 0) {
		x = -x;
		n = -n;
	}
	y = x * x;
	x *= _POLY2(y, p);
	return (_lldexp(HALF + x/(_POLY3(y, q) - x), n + 1));
}

float exp(x)
float x;
{
	static errtype;
	*SKYCOMREG = HW_EXP;
	*SKYFLREG = x;
	/* now do the test for max exp while waiting */
	_fperror.val.fval = x;
	_mathfunc_id = EXP;
	if (ISFMaxExp(x)) 
		errtype = FMANT(x)?INVALID_OP_A:INVALID_OP_F2;
	else errtype = 0;
	while (*SKYSTATREG > 0) ;
	x = *SKYFLREG;
	if (!errtype) {
		if (x==0.0) errtype = UNDERFL;
		else if (ISFMaxExp(x)) errtype = OVERFL;
		else return(x);
	}
	return(_raise_fperror(MATH,errtype));
}

