/*	@(#)jn.c	1.7	*/
/*LINTLIBRARY*/

/*
 *	floating point Bessel's function of
 *	the first and second kinds and of
 *	integer order.
 *
 *	int n;
 *	double x;
 *	jn(n,x);
 *
 *	returns the value of Jn(x) for all
 *	integer values of n and all real values
 *	of x.
 *
 *	There are no error returns.
 *	Calls j0, j1.
 *
 *	For n=0, j0(x) is called,
 *	for n=1, j1(x) is called,
 *	for n<x, forward recursion is used starting
 *	from values of j0(x) and j1(x).
 *	for n>x, a continued fraction approximation to
 *	j(n,x)/j(n-1,x) is evaluated and then backward
 *	recursion is used starting from a supposed value
 *	for j(n,x). The resulting value of j(0,x) is
 *	compared with the actual value to correct the
 *	supposed value of j(n,x).
 *
 *	yn(n,x) is similar in all respects, except
 *	that forward recursion is used for all
 *	values of n>1.
 */

#include <math.h>
#include <errno.h>
#include <fperr.h>
#include <sys/signal.h>
#include <nan.h>


long float
jn(n,x)
int n;
long float x;
{
	int i;
	long float a, b, temp;
	long float xsq, t;
	long float j0(), j1();

	if (ISMaxExp(x)) {
		_mathfunc_id = JN;
		_fperror.val.dval = x;
		return(_lraise_fperror(MATH,
			MANT(x)?INVALID_OP_A:INVALID_OP_F2));
	}
	if(n<0){
		n = -n;
		x = -x;
	}
	if(n==0) return(j0(x));
	if(n==1) return(j1(x));
	if(x == 0.) return(0.);
	if(n>x) goto recurs;

	a = j0(x);
	b = j1(x);
	for(i=1;i<n;i++){
		temp = b;
		b = (2.*i/x)*b - a;
		a = temp;
	}
	return(b);

recurs:
	xsq = x*x;
	for(t=0,i=n+16;i>n;i--){
		t = xsq/(2.*i - t);
	}
	t = x/(2.*n-t);

	a = t;
	b = 1;
	for(i=n-1;i>0;i--){
		temp = b;
		b = (2.*i/x)*b - a;
		a = temp;
	}
	return(t*j0(x)/b);
}

long float
yn(n,x)
int n;
long float x;
{
	int i;
	int sign;
	long float a, b, temp;
	long float y0(), y1();

	if (ISMaxExp(x)) {
		_mathfunc_id = YN;
		_fperror.val.dval = x;
		return(_lraise_fperror(MATH,
			MANT(x)?INVALID_OP_A:INVALID_OP_F2));
	}

	if (x <= 0) {
		_fperror.val.dval = x;
		_mathfunc_id = YN;
		return(_lraise_fperror(MATH,DOMAIN_ERROR));
	}
	sign = 1;
	if(n<0){
		n = -n;
		if(n%2 == 1) sign = -1;
	}
	if(n==0) return(y0(x));
	if(n==1) return(sign*y1(x));

	a = y0(x);
	b = y1(x);
	for(i=1;i<n;i++){
		temp = b;
		b = (2.*i/x)*b - a;
		a = temp;
	}
	return(sign*b);
}
