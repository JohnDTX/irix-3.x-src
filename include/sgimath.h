/*
 * $Source: /d2/3.7/src/include/RCS/sgimath.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:11:53 $
 */

/*	This file contains definitions for the float versions
	of those math routines which are available from the IEEE
	syntactics package.
*/

/* These single precision routines are all from the Syntactics package */

extern float fabs();	
extern float modf();
extern float asin();
extern float acos();
extern float atan2();
extern float sinh();
extern float cosh();
extern float tanh();
extern float pow();
extern float up_i();		/* float version of pow where b is integer*/
extern float sin();
extern float cos();
extern float atan();
extern float exp();
extern float log();
extern float sqrt();
extern float tan();
extern float log10();
extern float floor();
extern float ceil();
extern float atof();
extern float fmod();


/* The renamed MIT routines follow */

extern long float _lasin();
extern long float _lacos();
extern long float _latan2();
extern long float _lsinh();
extern long float _lcosh();
extern long float _ltanh();
extern long float _lpow();
extern long float _lsin();
extern long float _lcos();
extern long float _latan();
extern long float _lexp();
extern long float _llog();
extern long float _lsqrt();
extern long float _ltan();
extern long float _llog10();
extern long float _lfloor(),_lceil();

extern long float _lldexp(), _lfrexp();
extern long float hypot(), _latof();
extern long float gamma();
extern long float j0(), j1(), jn(), y0(), y1(), yn();

/* the following two routines were added 7/16/86 by GB in response
   to scr1021
*/
extern long float drand48(),erand48();

/* and these long float routines are also syntactics */
extern long float _lfabs();	
extern long float _lmodf();
extern long float _lfmod();
