/*	@(#)log.c	1.7	*/
/*LINTLIBRARY*/
/*
 *	log returns the natural logarithm of its double-precision argument.
 *	Returns EDOM error and value -HUGE if argument <= 0.
 *	Algorithm and coefficients from Cody and Waite (1980).
 *	Calls frexp.
 */

#include <errno.h>
#include <math.h>
#include <fperr.h>
#include <sys/signal.h>
#include <nan.h>
#define SQRT2_OVER_2	0.70710678118654752440
#define C1	0.693359375
#define C2	-2.121944400546905827679e-4
#define LOG10E	0.43429448190325182765

long float
_llog(x)
register long float x;
{
	static long float p[] = {
		-0.78956112887491257267e0,
		 0.16383943563021534222e2,
		-0.64124943423745581147e2,
	}, q[] = {
		 1.0,
		-0.35667977739034646171e2,
		 0.31203222091924532844e3,
		-0.76949932108494879777e3,
	};
	register long float y;
	int n;

	if (ISMaxExp(x)) {
		_mathfunc_id = LOG;
		_fperror.val.dval = x;
		return(_lraise_fperror(MATH,
			MANT(x)?INVALID_OP_A:INVALID_OP_F2));
	}

	if (x <= 0) {
		_mathfunc_id = LOG;
		_fperror.val.dval = x;
		return (_lraise_fperror(MATH,DOMAIN_ERROR));
	}
	y = 1.0;
	x = _lfrexp(x, &n);
	if (x < SQRT2_OVER_2) {
		n--;
		y = 0.5;
	}
	x = (x - y)/(x + y);
	x += x;
	y = x * x;
	x += x * y * _POLY2(y, p)/_POLY3(y, q);
	y = (long float)n;
	x += y * C2;
	return (x + y * C1);
}

long float
_llog10(arg)
long float arg;
{
	return (_llog(arg) * LOG10E);
}
