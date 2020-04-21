/*
 * $Source: /d2/3.7/src/include/RCS/math.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:11:37 $
 */

#include <sgimath.h>
extern int errno, signgam;

extern long float erf(), erfc();
/* 
			NOTE!!! matherr() defunct.


   matherr() is not used on IRIS 2000 or 3000 series machines.
   A custom IEEE-based floating-point exception system is used
   instead.  Please refer to "IRIS Floating Point" in the UNIX
   Programmers Manual Vol 2B.

   definitions for matherr() and its structures are left 
   for compatibility only.
*/

extern int matherr();
#include <values.h>
#define HUGE	MAXFLOAT


#define _ABS(x)	((x) < 0 ? -(x) : (x))
#define _REDUCE(TYPE, X, XN, C1, C2)	{ \
	long float x1 = (long float)(TYPE)X, x2 = X - x1; \
	X = x1 - (XN) * (C1); X += x2; X -= (XN) * (C2); }
#define _POLY1(x, c)	((c)[0] * (x) + (c)[1])
#define _POLY2(x, c)	(_POLY1((x), (c)) * (x) + (c)[2])
#define _POLY3(x, c)	(_POLY2((x), (c)) * (x) + (c)[3])
#define _POLY4(x, c)	(_POLY3((x), (c)) * (x) + (c)[4])
#define _POLY5(x, c)	(_POLY4((x), (c)) * (x) + (c)[5])
#define _POLY6(x, c)	(_POLY5((x), (c)) * (x) + (c)[6])
#define _POLY7(x, c)	(_POLY6((x), (c)) * (x) + (c)[7])
#define _POLY8(x, c)	(_POLY7((x), (c)) * (x) + (c)[8])
#define _POLY9(x, c)	(_POLY8((x), (c)) * (x) + (c)[9])

struct exception {
	int type;
	char *name;
	long float arg1;
	long float arg2;
	long float retval;
	};

#define 	DOMAIN		01
#define		SING		02
#define		OVERFLOW	03
#define		UNDERFLOW	04
#define		TLOSS		05
#define		PLOSS		06


/*
some AT&T additions
*/
/* some useful constants */
#define M_E	2.7182818284590452354
#define M_LOG2E	1.4426950408889634074
#define M_LOG10E	0.43429448190325182765
#define M_LN2	0.69314718055994530942
#define M_LN10	2.30258509299404568402
#define M_PI	3.14159265358979323846
#define M_PI_2	1.57079632679489661923
#define M_PI_4	0.78539816339744830962
#define M_1_PI	0.31830988618379067154
#define M_2_PI	0.63661977236758134308
#define M_2_SQRTPI	1.12837916709551257390
#define M_SQRT2	1.41421356237309504880
#define M_SQRT1_2	0.70710678118654752440
