/*	@(#)tan.c	1.7	*/
/*LINTLIBRARY*/

/*
 *	floating point tangent
 *
 *	A series is used after range reduction.
 *	Coefficients are #4285 from Hart & Cheney. (19.74D)
 */

#include <errno.h>
#include <math.h>
#include <fperr.h>
#include <sys/signal.h>
#include <nan.h>
#define	XMAX	(3.0 * (1L << (DSIGNIF/2)))

extern int errno;
static long float invpi	  = 1.27323954473516268;
static long float p0	 = -0.1306820264754825668269611177e+5;
static long float p1	  = 0.1055970901714953193602353981e+4;
static long float p2	 = -0.1550685653483266376941705728e+2;
static long float p3	  = 0.3422554387241003435328470489e-1;
static long float p4	  = 0.3386638642677172096076369e-4;
static long float q0	 = -0.1663895238947119001851464661e+5;
static long float q1	  = 0.4765751362916483698926655581e+4;
static long float q2	 = -0.1555033164031709966900124574e+3;

long float
_ltan(arg)
long float arg;
{
	int ploss_pending=0;
	long float sign, temp, e, x, xsq;
	int flag, i;

	if (ISMaxExp(arg)) {
		_mathfunc_id = TAN;
		_fperror.val.dval = arg;
		return(_lraise_fperror(MATH,
			MANT(arg)?INVALID_OP_A:INVALID_OP_F2));
	}
	if(arg >= CUBRTHUGE){
		_mathfunc_id = TAN;
		_fperror.val.dval = arg;
		return(_lraise_fperror(MATH,CANT_REDUCE_RANGE));
		}
	if(arg >= XMAX){
		_mathfunc_id = TAN;
		ploss_pending++;
	}
	flag = 0;
	sign = 1.;
	if(arg < 0.){
		arg = -arg;
		sign = -1.;
	}
	arg = arg*invpi;   /*overflow?*/
	x = _lmodf(arg,&e);
	i = e;
	switch(i%4) {
	case 1:
		x = 1. - x;
		flag = 1;
		break;

	case 2:
		sign = - sign;
		flag = 1;
		break;

	case 3:
		x = 1. - x;
		sign = - sign;
		break;

	case 0:
		break;
	}

	xsq = x*x;
	temp = ((((p4*xsq+p3)*xsq+p2)*xsq+p1)*xsq+p0)*x;
	temp = temp/(((1.0*xsq+q2)*xsq+q1)*xsq+q0);

	if(flag == 1) {
		if(temp == 0.) {
			_mathfunc_id = TAN;
			_fperror.val.dval = arg;
			return(_lraise_fperror(MATH,OVERFL));
		}
		temp = 1./temp;
	}
	if (ploss_pending) {
		_fperror.val.dval = sign*temp;
		return(_lraise_fperror(MATH,PARTIAL_SLOSS));
	}
	return(sign*temp);
}
