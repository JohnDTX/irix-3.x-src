/*	@(#)sinh.c	1.4	*/
/*LINTLIBRARY*/
/*
 *	sinh(arg) returns the hyperbolic sine of its floating-
 *	point argument.
 *
 *	The exponential function is called for arguments
 *	greater in magnitude than 0.5.
 *
 *	A series is used for arguments smaller in magnitude than 0.5.
 *	The coefficients are #2029 from Hart & Cheney. (20.36D)
 *
 *	cosh(arg) is computed from the exponential function for
 *	all arguments.
 */

#include <math.h>
#include <errno.h>
#include <fperr.h>
#include <sys/signal.h>
#include <nan.h>
#include <fpregs.h>
#include <fpopcodes.h>
#define XMIN	(1.0/(1L << (DSIGNIF/2)))
#define LOGHUGE	((DMAXEXP + 1) / LOG2E)
#define	LOG2E	1.4426950408889634074

long float
_lsinh(x)
register long float x;
{
	register long float y; 

	if (ISMaxExp(x)) {
		_mathfunc_id = SINH;
		_fperror.val.dval = x;
		return(_lraise_fperror(MATH,
			MANT(x)?INVALID_OP_A:INVALID_OP_F2));
	}

	y = _ABS(x);
	if (y < 0.5) {
		static long float p[] = {
			-0.2630563213397497062819489e+2,
			-0.2894211355989563807284660366e+4,
			-0.8991272022039509355398013511e+5,
			-0.6307673640497716991184787251e+6,
		}, q[] = {
			 1.0,
			-0.173678953558233699533450911e+3,
			 0.1521517378790019070696485176e+5,
			-0.6307673640497716991212077277e+6,
		};

		if (y < XMIN) /* for efficiency and to prevent underflow */
			return (x);
		y = x * x;
		return (x * _POLY3(y, p)/_POLY3(y, q));
	}
	/*
	 * A special test should be added here for those cases where exp(|x|)
	 * overflows while exp(|x|)/2 is still representable.
	 */
	if (y > LOGHUGE){
		_mathfunc_id = SINH;
		_fperror.val.dval = y;
		return(_lraise_fperror(MATH,OVERFL));
	}
	x = _lexp(x);
	return (0.5 * (x - 1.0/x));
}

long float
_lcosh(x)
register long float x;
{
	static errtype;
	if (ISMaxExp(x)) 
		errtype = MANT(x)?INVALID_OP_A:INVALID_OP_F2;
	else if (x > LOGHUGE)
		errtype = OVERFL;
	else {
		x = _lexp(x);
		return (0.5 * (x + 1.0/x));
	}
	_mathfunc_id = COSH;
    _fperror.val.dval = x;
	return(_lraise_fperror(MATH,errtype));
}

float
sinh(x)
register float x;
{
	return(_lsinh((long float)x));
}

float
cosh(x)
float x;
{
	static errtype;
	if (ISFMaxExp(x)) 
		errtype = FMANT(x)?INVALID_OP_A:INVALID_OP_F2;
	else if (x > LOGHUGE)
		errtype = OVERFL;
	else {
		/* x = exp(x) */
		*SKYCOMREG = HW_EXP;
		*SKYFLREG = x;
		while (*SKYSTATREG > 0) ;
		x = *SKYFLREG;
		return (0.5 * (x + 1.0/x));
	}
	_mathfunc_id = COSH;
    _fperror.val.fval = x;
	return(_raise_fperror(MATH,errtype));
}
