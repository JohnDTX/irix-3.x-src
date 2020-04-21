/*	@(#)asin.c	1.7	*/
/*LINTLIBRARY*/
/*
 *	asin(arg) and acos(arg) return the arcsin, arccos,
 *	respectively of their arguments.
 *
 *	Arctan is called after appropriate range reduction.
 *	This leads to very high errors for argument values near 1.0.
 *
 *	These routines are used for the double precision versions
 * 	of the floating point routines. Their names have been changed
 *	not to protect the innocent, but to avoid conflict with the
 *	default single- precision routines. (GB) SGI 9/19/83
 */

#include <errno.h>
#include <math.h>
#define PI_OVER_2	1.57079632679489661923
#include <fperr.h>
#include <sys/signal.h>
#include <nan.h>

static errtype;

long float
_lasin(x)
register long float x;
{
	if (!ISMaxExp(x)) {
		if (x >= -1 && x <= 1)
			return (_latan2(x, _lsqrt(1.0 - x * x)));
		errtype = DOMAIN_ERROR;
	} else {
		if (MANT(x))
			errtype = INVALID_OP_A;
		else errtype = INVALID_OP_F2;
	}
	_fperror.val.dval = x;
	_mathfunc_id=ASIN;
	return(_lraise_fperror(MATH,errtype));
}

long float
_lacos(x)
register long float x;
{
	if (!ISMaxExp(x)) {
		if (x >= -1 && x <= 1)
			return (PI_OVER_2 - _latan2(x, _lsqrt(1.0 - x * x)));
		errtype = DOMAIN_ERROR;
	} else {
		if (MANT(x))
			errtype = INVALID_OP_A;
		else errtype = INVALID_OP_F2;
	}
	_fperror.val.dval = x;
	_mathfunc_id=ACOS;
	return(_lraise_fperror(MATH,errtype));
}
