/*	@(#)hypot.c	1.6	*/
/*LINTLIBRARY*/

/*
 *	sqrt(a^2 + b^2)
 *	(but carefully)
 */

#include <errno.h>
#include <math.h>
#include <fperr.h>
#include <sys/signal.h>
#include <nan.h>
#define XMIN	(1.0/(1L << (DSIGNIF/2)))
#define SQ_TWO	1.41421356237309504880

long float
hypot(a, b)
register long float a, b;
{
	int exception=0;
	if (ISMaxExp(a)) {
			exception++;
			_fperror.val.dval = a;
	}
	if (ISMaxExp(b)) {
			exception++;
			_fperror.val.dval = b;
	}
	if (exception) {
		_mathfunc_id = HYPOT;
		return(_lraise_fperror(MATH,
			MANT(_fperror.val.dval)?INVALID_OP_A:INVALID_OP_F2));
	}
	if (a < 0)
		a = -a;
	if (b < 0)
		b = -b;
	if (a > b) {				/* make sure a <= b */
		long float t = a;
		a = b;
		b = t;
	}
	if ((a /= b) < XMIN)			/* a <= 1 */
		return (b);			/* a << 1 */
	a = a * a;
	/* use first term of Taylor series for very small angles */
	a = (a < XMIN) ? 1.0 + 0.5 * a : _lsqrt(1.0 + a); /* a <= sqrt(2) */
	if (b < MAXDOUBLE/SQ_TWO)		/* result can't overflow */
		return (a * b);
	if ((a *= 0.5 * b) < MAXDOUBLE/2)	/* result won't overflow */
		return (a + a);
	_fperror.val.dval = (fabs(a)>fabs(b))?a:b;
	_mathfunc_id=  HYPOT;
	return(_lraise_fperror(MATH,OVERFL));
}
