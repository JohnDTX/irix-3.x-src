/*	@(#)sin.c	1.6	*/
/*LINTLIBRARY*/
/*
 *	C program for double-precision sin/cos.
 *	Returns EDOM error and value 0 if argument too large.
 *	Algorithm and coefficients from Cody and Waite (1980).
 */

#include <math.h>
#include <errno.h>
#include <fperr.h>
#include <sys/signal.h>
#include <nan.h>
#include <fpregs.h>
#include <fpopcodes.h>
#define XMIN	(1.0/(1L << (DSIGNIF/2)))
#define XMAX	(3.0 * (1L << (DSIGNIF/2)))
#define ONE_OVER_PI	0.31830988618379067154
#define PI_OVER_TWO	1.57079632679489661923

static errtype;

long float
_lsin(x)
long float x;
{
	extern long float sin_cos();

	if (ISMaxExp(x)) {
		_mathfunc_id = SIN;
		_fperror.val.dval = x;
		return(_lraise_fperror(MATH,
			MANT(x)?INVALID_OP_A:INVALID_OP_F2));
	}
	return (sin_cos(x, 0));
}

static long float
sin_cos(x, cosflag)
long float x;
int cosflag;
{
	long float y;
	register int neg;
	register long n;
	
	_fperror.val.dval = x;
	if (neg = (x < 0))
		x = -x;
	y = x;
	if (cosflag) {
		_mathfunc_id = COS;
		neg = 0;
		y += PI_OVER_TWO;
		}
	else 
		_mathfunc_id = SIN;
	if (y > XMAX){
		return(_lraise_fperror(MATH,CANT_REDUCE_RANGE));
		}
	n = (long)(y * ONE_OVER_PI + 0.5);
	y = (long float)n;
	if (cosflag)
		y -= 0.5;
	_REDUCE(long, x, y, 3.1416015625, -8.908910206761537356617e-6);
	if (_ABS(x) > XMIN) {
		/* skip for efficiency and to prevent underflow */
		static long float p[] = {
			 0.27204790957888846175e-14,
			-0.76429178068910467734e-12,
			 0.16058936490371589114e-9,
			-0.25052106798274584544e-7,
			 0.27557319210152756119e-5,
			-0.19841269841201840457e-3,
			 0.83333333333331650314e-2,
			-0.16666666666666665052e0,
		};

		y = x * x;
		x += x * y * _POLY7(y, p);
	}
	return (neg ^ (int)(n % 2) ? -x : x);
}

long float
_lcos(x)
long float x;
{
	if (ISMaxExp(x)) {
		_mathfunc_id = COS;
		_fperror.val.dval = x;
		return(_lraise_fperror(MATH,
			MANT(x)?INVALID_OP_A:INVALID_OP_F2));
	}
	return (sin_cos(x, 1));
}

float sin(x)
float x;
{
	*SKYCOMREG = HW_SIN;
	*SKYFLREG = x;
	/* check the operand */
	errtype = 0;
	_mathfunc_id = SIN;
	if (ISFMaxExp(x)) {
		_fperror.val.fval = x;
		errtype = FMANT(x)?INVALID_OP_A:INVALID_OP_F2;
	}
	while (*SKYSTATREG > 0) ;
	x = *SKYFLREG;
	if (ISFMaxExp(x)) errtype = CANT_REDUCE_RANGE;
	if (!errtype) return(x);
	return(_raise_fperror(MATH,errtype));
}

float cos(x)
float x;
{
	*SKYCOMREG = HW_COS;
	*SKYFLREG = x;
	/* check the operand */
	errtype = 0;
	_mathfunc_id = COS;
	if (ISFMaxExp(x)) {
		_fperror.val.fval = x;
		errtype = FMANT(x)?INVALID_OP_A:INVALID_OP_F2;
	}
	while (*SKYSTATREG > 0) ;
	x = *SKYFLREG;
	if (ISFMaxExp(x)) errtype = CANT_REDUCE_RANGE;
	if (!errtype) return(x);
	return(_raise_fperror(MATH,errtype));
}
