/*	@(#)tanh.c	1.3	*/
/*LINTLIBRARY*/

/*
 *	_ltanh(arg) computes the hyperbolic tangent of its floating
 *	point argument.
 *
 *	_lsinh and _lcosh are called except for large arguments, which
 *	would cause overflow improperly.
 */

#include <fpregs.h>
#include <fpopcodes.h>
long float _lsinh(), _lcosh();
float sinh(),cosh();

long float
_ltanh(arg)
long float arg;
{
	long float sign;

	sign = 1.;
	if(arg < 0.) {
		arg = -arg;
		sign = -1.;
	}

	if(arg > 21.)
		return(sign);

	return(sign*_lsinh(arg)/_lcosh(arg));
}


float
tanh(arg)
float arg;
{
	float sign;

	sign = 1.;
	if(arg < 0.) {
		arg = -arg;
		sign = -1.;
	}

	if(arg > 21.)
		return(sign);

	return(sign*sinh(arg)/cosh(arg));
}
