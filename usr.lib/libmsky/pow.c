/*	@(#)pow.c	1.8	*/
/*LINTLIBRARY*/
/*
 *	computes x^y.
 *	uses _llog and _lexp
 */

#include <errno.h>
#include <math.h>
#include <fperr.h>
#include <sys/signal.h>
#include <nan.h>
#include <fpregs.h>
#include <fpopcodes.h>
#define LOGHUGE	((DMAXEXP + 1) / LOG2E)
#define LOGTINY	((DMINEXP - 1) / LOG2E)
#define LOG2E	1.4426950408889634074

static errtype;

long float
_lpow(x, y)
register long float x, y;
{
	register long ly;
	register long float z;

	_mathfunc_id = POW;
	if (ISMaxExp(x)) {
		_fperror.val.dval = x;
		return(_lraise_fperror(MATH,
			MANT(x)?INVALID_OP_A:INVALID_OP_F2));
	}
	if (ISMaxExp(y)) {
		_fperror.val.dval = y;
		return(_lraise_fperror(MATH,
			MANT(y)?INVALID_OP_A:INVALID_OP_F2));
	}

	if (x > 0) {
		if ((z = y * _llog(x)) > LOGHUGE) {
			_fperror.val.dval = y;
			return(_lraise_fperror(MATH,OVERFL));
			}
		if (z < LOGTINY) {
			return(_lraise_fperror(MATH,UNDERFL));
			}
		return (_lexp(z));
	}
	if (x == 0) {
		if (y <= 0)
			goto domain;
		return (0);
	}
	ly = (long)y;
	if ((long float)ly != y) {
domain:
		_fperror.val.dval = y;
		return(_lraise_fperror(MATH,DOMAIN_ERROR));
	}
	x = _lexp(y * _llog(-x));
	return (ly % 2 ? -x : x);
}

float pow(x,y)
float x,y;
{
	float result;
	*SKYCOMREG = HW_POW;
	*SKYFLREG = x;
	while (*SKYSTATREG > 0) ;
	*SKYFLREG = y;
	/* check the operands while waiting */
	errtype = 0;
	_mathfunc_id = POW;
	if (ISFMaxExp(x)) {
		errtype = FMANT(x)?INVALID_OP_A:INVALID_OP_F2;
		_fperror.val.fval = x;
	}
	if (ISFMaxExp(y)) {
		errtype = FMANT(y)?INVALID_OP_A:INVALID_OP_F2;
		_fperror.val.fval = y;
	}
	while (*SKYSTATREG > 0) ;
	result = *SKYFLREG;
	if (!errtype) {
		if (x == 0) {
			if (y <= 0) {
				errtype = DOMAIN_ERROR;
				_fperror.val.fval = 0;
			} else {
				return (0);
			}
		}
		else if (result == 0.0) {
			errtype = UNDERFL;
			_fperror.val.fval = result;
		}
		else if (ISFMaxExp(result)) {
			errtype = OVERFL;
			_fperror.val.fval = result;
		}
		else return(result);
	}

	return(_raise_fperror(MATH,errtype));
}

