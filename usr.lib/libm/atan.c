/*	@(#)atan.c	1.5	*/
/*LINTLIBRARY*/
/*
 *	atan returns the value of the arctangent of its
 *	argument in the range [-pi/2, pi/2].
 *
 *	atan2 returns the arctangent of x/y
 *	in the range [-pi, pi].
 *
 *	There are no error returns.
 *
 *	Coefficients are #5077 from Hart & Cheney (19.56D).
 *
 *	This version is for the Sky library. The single precision routines 
 *	use the Sky floating point processor.
 */

#include <math.h>
#include <fperr.h>
#include <sys/signal.h>
#include <nan.h>
#define SQ_TWO	1.41421356237309504880
#define PI	3.14159265358979323846

/*
 *	atan makes its argument positive and
 *	calls the inner routine s_atan.
 */
 
long float
_latan(x)
long float x;
{
	extern long float s_atan();

	if (ISMaxExp(x)) {
		_mathfunc_id = ATAN;
		_fperror.val.dval = x;
		return(_lraise_fperror(MATH,
			MANT(x)?INVALID_OP_A:INVALID_OP_F2));
	}
	return (x < 0 ? -s_atan(-x) : s_atan(x));
}

/*
 *	x_atan evaluates a series valid in the
 *	range [-0.414..., +0.414...].
 */

static long float
x_atan(x)
long float x;
{
	static long float p[] = {
		0.161536412982230228262e2,
		0.26842548195503973794141e3,
		0.11530293515404850115428136e4,
		0.178040631643319697105464587e4,
		0.89678597403663861959987488e3,
	}, q[] = {
		1.0,
		0.5895697050844462222791e2,
		0.536265374031215315104235e3,
		0.16667838148816337184521798e4,
		0.207933497444540981287275926e4,
		0.89678597403663861962481162e3,
	};
	long float xsq = x * x;

	return (x * _POLY4(xsq, p)/_POLY5(xsq, q));
}

/*
 *	s_atan reduces its argument (known to be positive)
 *	to the range [0, 0.414...] and calls x_atan.
 */

static long float
s_atan(x)
long float x;
{
	return (x < SQ_TWO - 1 ? x_atan(x) :
	    x > SQ_TWO + 1 ? PI/2 - x_atan(1/x) :
	    PI/4 + x_atan((x - 1)/(x + 1)));
}

/*
 *	atan2 discovers what quadrant the angle
 *	is in and calls s_atan.
 */

long float
_latan2(x, y)
long float x, y;
{
	static errtype;
	if ((!ISMaxExp(x)) && (!ISMaxExp(y))) {
		/* GB - atan2 is defined even if y == 0
		if (y != 0.0) {
		*/
			if (x + y == x)
				return (x < 0 ? -PI/2 : PI/2);
			if (y < 0)
				return (x < 0 ? -PI + s_atan(x/y) : PI - s_atan(-x/y));
			return (x < 0 ? -s_atan(-x/y) : s_atan(x/y));
		/*
		}
		errtype = DOMAIN_ERROR;
		_fperror.val.dval = 0.0;
		*/
	} else {
		if (ISMaxExp(x)) 
			_fperror.val.dval = x;
		else
			_fperror.val.dval = y;
		if (MANT(_fperror.val.dval))
			errtype = INVALID_OP_A;
		else errtype = INVALID_OP_F2;
	}
	_mathfunc_id = ATAN;
	return (_lraise_fperror(MATH,errtype));
}
