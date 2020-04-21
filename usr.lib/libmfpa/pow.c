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
#define FLOGHUGE	((FMAXEXP + 1) / LOG2E)
#define FLOGTINY	((FMINEXP - 1) / LOG2E)
#define LOG2E	1.4426950408889634074

float
pow(x, y)
float x, y;
{
	register long ly;
	register float z;

	_mathfunc_id = POW;
	if (ISFMaxExp(x)) {
		_fperror.val.fval = x;
		return(_raise_fperror(MATH,
			FMANT(x)?INVALID_OP_A:INVALID_OP_F2));
	}
	if (ISFMaxExp(y)) {
		_fperror.val.fval = y;
		return(_raise_fperror(MATH,
			FMANT(y)?INVALID_OP_A:INVALID_OP_F2));
	}

	if (x > 0) {
		if ((z = y * log(x)) > FLOGHUGE) {
			_fperror.val.fval = y;
			return(_raise_fperror(MATH,OVERFL));
			}
		if (z < FLOGTINY) {
			return(_raise_fperror(MATH,UNDERFL));
			}
		return (exp(z));
	}
	if (x == 0) {
		if (y <= 0)
			goto domain;
		return (0);
	}
	ly = (long)y;
	if ((float)ly != y) {
domain:
		_fperror.val.fval = y;
		return(_raise_fperror(MATH,DOMAIN_ERROR));
	}
	x = exp(y * log(-x));
	return (ly % 2 ? -x : x);
}
