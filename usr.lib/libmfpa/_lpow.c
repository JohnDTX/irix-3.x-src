#include <errno.h>
#include <math.h>
#include <fperr.h>
#include <sys/signal.h>
#include <nan.h>
#define LOGHUGE	((DMAXEXP + 1) / LOG2E)
#define LOGTINY	((DMINEXP - 1) / LOG2E)
#define LOG2E	1.4426950408889634074

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
