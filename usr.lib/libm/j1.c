/*	@(#)j1.c	1.7	*/
/*LINTLIBRARY*/

/*
 *	floating point Bessel's function
 *	of the first and second kinds
 *	of order one
 *
 *	j1(x) returns the value of J1(x)
 *	for all real values of x.
 *
 *	There are no error returns.
 *	Calls sin, cos, sqrt.
 *
 *	There is a niggling bug in J1 which
 *	causes errors up to 2e-16 for x in the
 *	interval [-8,8].
 *	The bug is caused by an inappropriate order
 *	of summation of the series.  rhm will fix it
 *	someday.
 *
 *	Coefficients are from Hart & Cheney.
 *	#6050 (20.98D)
 *	#6750 (19.19D)
 *	#7150 (19.35D)
 *
 *	y1(x) returns the value of Y1(x)
 *	for positive real values of x.
 *	For x<=0, error number EDOM is set and a
 *	large negative value is returned.
 *
 *	Calls sin, cos, sqrt, log, j1.
 *
 *	The values of Y1 have not been checked
 *	to more than ten places.
 *
 *	Coefficients are from Hart & Cheney.
 *	#6447 (22.18D)
 *	#6750 (19.19D)
 *	#7150 (19.35D)
 */

#include <math.h>
#include <errno.h>
#include <fperr.h>
#include <sys/signal.h>
#include <nan.h>
#define XMAX (3.0 * (1L << (DSIGNIF/2)))

static long float pzero, qzero;
static long float tpi	= .6366197723675813430755350535e0;
static long float pio4	= .7853981633974483096156608458e0;
static long float p1[] = {
	0.581199354001606143928050809e21,
	-.6672106568924916298020941484e20,
	0.2316433580634002297931815435e19,
	-.3588817569910106050743641413e17,
	0.2908795263834775409737601689e15,
	-.1322983480332126453125473247e13,
	0.3413234182301700539091292655e10,
	-.4695753530642995859767162166e7,
	0.2701122710892323414856790990e4,
};
static long float q1[] = {
	0.1162398708003212287858529400e22,
	0.1185770712190320999837113348e20,
	0.6092061398917521746105196863e17,
	0.2081661221307607351240184229e15,
	0.5243710262167649715406728642e12,
	0.1013863514358673989967045588e10,
	0.1501793594998585505921097578e7,
	0.1606931573481487801970916749e4,
	1.0,
};
static long float p2[] = {
	-.4435757816794127857114720794e7,
	-.9942246505077641195658377899e7,
	-.6603373248364939109255245434e7,
	-.1523529351181137383255105722e7,
	-.1098240554345934672737413139e6,
	-.1611616644324610116477412898e4,
	0.0,
};
static long float q2[] = {
	-.4435757816794127856828016962e7,
	-.9934124389934585658967556309e7,
	-.6585339479723087072826915069e7,
	-.1511809506634160881644546358e7,
	-.1072638599110382011903063867e6,
	-.1455009440190496182453565068e4,
	1.0,
};
static long float p3[] = {
	0.3322091340985722351859704442e5,
	0.8514516067533570196555001171e5,
	0.6617883658127083517939992166e5,
	0.1849426287322386679652009819e5,
	0.1706375429020768002061283546e4,
	0.3526513384663603218592175580e2,
	0.0,
};
static long float q3[] = {
	0.7087128194102874357377502472e6,
	0.1819458042243997298924553839e7,
	0.1419460669603720892855755253e7,
	0.4002944358226697511708610813e6,
	0.3789022974577220264142952256e5,
	0.8638367769604990967475517183e3,
	1.0,
};
static long float p4[] = {
	-.9963753424306922225996744354e23,
	0.2655473831434854326894248968e23,
	-.1212297555414509577913561535e22,
	0.2193107339917797592111427556e20,
	-.1965887462722140658820322248e18,
	0.9569930239921683481121552788e15,
	-.2580681702194450950541426399e13,
	0.3639488548124002058278999428e10,
	-.2108847540133123652824139923e7,
	0.0,
};
static long float q4[] = {
	0.5082067366941243245314424152e24,
	0.5435310377188854170800653097e22,
	0.2954987935897148674290758119e20,
	0.1082258259408819552553850180e18,
	0.2976632125647276729292742282e15,
	0.6465340881265275571961681500e12,
	0.1128686837169442121732366891e10,
	0.1563282754899580604737366452e7,
	0.1612361029677000859332072312e4,
	1.0,
};

long float
j1(arg)
long float arg;
{
	long float xsq, n, d, x;
	int i;

	_mathfunc_id = J1;
	_fperror.val.dval = arg;
	if (ISMaxExp(arg)) 
		return(_lraise_fperror(MATH,
			MANT(arg)?INVALID_OP_A:INVALID_OP_F2));

	x = arg;
	if(x < 0.) x = -x;
	if(x > CUBRTHUGE){
		return(_lraise_fperror(MATH,UNDERFL));
		}
	if(x > XMAX){
		_fperror.val.dval = _lsqrt(tpi/x)*pzero;
		return(_lraise_fperror(MATH,PARTIAL_SLOSS));
		}	
	if(x > 8.){
		asympt(x);
		n = x - 3.*pio4;
		n = _lsqrt(tpi/x)*(pzero*_lcos(n) - qzero*_lsin(n));
		if(arg <0.) n = -n;
		return(n);
	}
	xsq = x*x;
	for(n=0,d=0,i=8;i>=0;i--){
		n = n*xsq + p1[i];
		d = d*xsq + q1[i];
	}
	return(arg*n/d);
}

long float
y1(arg)
long float arg;
{
	long float xsq, n, d, x;
	int i;

	_fperror.val.dval = arg;
	_mathfunc_id = Y1;
	if (ISMaxExp(arg)) 
		return(_lraise_fperror(MATH,
			MANT(arg)?INVALID_OP_A:INVALID_OP_F2));

	x = arg;
	if(x <= 0.){
		return(_lraise_fperror(MATH,DOMAIN));
	}
	if(x > CUBRTHUGE){
		return(_lraise_fperror(MATH,UNDERFL));
		}
	if(x > XMAX){
		_fperror.val.dval = _lsqrt(tpi/x)*qzero;
		return(_lraise_fperror(MATH,PARTIAL_SLOSS));
		}
	if(x > 8.){
		asympt(x);
		n = x - 3*pio4;
		return(_lsqrt(tpi/x)*(pzero*_lsin(n) + qzero*_lcos(n)));
	}
	xsq = x*x;
	for(n=0,d=0,i=9;i>=0;i--){
		n = n*xsq + p4[i];
		d = d*xsq + q4[i];
	}
	return(x*n/d + tpi*(j1(x)*_llog(x)-1./x));
}

static
asympt(arg)
long float arg;
{
	long float zsq, n, d;
	int i;
	zsq = 64./(arg*arg);
	for(n=0,d=0,i=6;i>=0;i--){
		n = n*zsq + p2[i];
		d = d*zsq + q2[i];
	}
	pzero = n/d;
	for(n=0,d=0,i=6;i>=0;i--){
		n = n*zsq + p3[i];
		d = d*zsq + q3[i];
	}
	qzero = (8./arg)*(n/d);
}
