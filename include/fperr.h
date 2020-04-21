/*
 * $Source: /d2/3.7/src/include/RCS/fperr.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:11:27 $
 */

/* fperr.h - include file for floating point error handlers.

   This file has the definitions required for the floating point 
   exception structure and the type defines for the types of errors.

   When a floating point exception occurs, the type of exception (as
   enumerated here) is set in _fperror.type, and the value is zeroed.
   A kill with SIGFPE is then issued.  If the kill is returned from,
   the value in the fpexception is taken as the result of the operation.

   The operation field tells what operation was in progress when the
   exception occurred.  The precision field tells the precision of the
   result of this operation.

*/
#ifndef __FPERRH__
#define __FPERRH__
#include <fpsignal.h>

struct {
	/* the value field normally contains the value to be assigned
	   as the result at the end of the operation.  In the case of fp
	   compare, this value will replace the erroneous operand, and the
	   operation will be retried.  The value is set to zero by default.

	   The math routines may pass in the erroneous operand for printing
	   in the val field.  This is zeroed by fpsignal.  If a user takes
	   control of SIGFPE, the field should be modified so as to avoid looping.
	   If the math error was PARTIAL_SLOSS, the val field contains the 
	   calculated and possibly erroneous result */
	union {
		unsigned long uval;
		float fval;
		long float dval;
	      } val;
	/* the operation field has one of the operation codes listed below.
	   This is filled in from the first arg to _[l]raise_fperror.
	*/
	unsigned char operation,
	/* the precision field is filled in according to whether _raise_fperror
	   or _lraise_fperror was called to report the exception
	*/
		precision;
	/* the type field is the type code of the exception, enumerated below */
	unsigned short type;
	} _fperror;

/* operation codes */
#define ADD 	1
#define SUB	2
#define MUL	3
#define DIV	4
#define FIX     5	/* precision to integer */
#define PRECISION 6	/* precision change to given precision */
#define MOD	7
#define CMP	8
#define CONVERT 9	/* atof, ecvt, ldexp, etc... */
#define MATH	10	/* from the math library */
#define NOPS    MATH

/* the following definitions are for values assigned to _mathfunc_id
	when a fp exception is raised in the math library */
#define SIN	1
#define COS	2
#define TAN	3
#define LOG	4
#define EXP	5
#define SQRT	6
#define POW	7
#define ASIN	8
#define ACOS	9
#define SINH	10
#define COSH	11
#define ATAN2	12
#define ATAN	13
#define UP_I	14
#define GAMMA	15
#define HYPOT	16
#define J0	17
#define J1	18
#define Y0	19
#define Y1	20
#define YN	21
#define LOG10	22
#define TANH	23
#define JN	24
#define NMATH_ROUTINES JN

/* precisions */
#define UNKNOWN  0
#define SINGLE   1
#define DOUBLE  2

/* type codes */
/* these are the equivalent of turbo fpa DIVZERO */
#define INVALID_OP_D1	0x141		/* 0/0 */
#define INVALID_OP_D3	0x143		/* divisor not normalized */
#define INVALID_OP_E1	0x151		/* <num> MOD 0 */
#define DIVZERO		0x200

/* this is the equivalent of the turbo fpa OVERFLOW */
#define OVERFL		0x300

/* these are the equivalent of the turbo fpa UNDERFLOW */
#define UNDERFL		0x400
#define UNDERFLOW_A	0x401
#define UNDERFLOW_B	0x402

/* INVALID_OP_F3 is  the equivalent of turbo fpa DENORM */
#define INVALID_OP_F3	0x163		/* operand not normalized */

/* these are the equivalent of turbo fpa NANOP */
#define INVALID_OP	0x100		/* NaN/INF as operand */
#define INVALID_OP_A	0x110		/* NaN as operand */

/* these INVALID_OPs are the equivalent of turbo fpa ILLEGALOP */
#define INVALID_OP_B	0x120		/* both operands INF */
#define INVALID_OP_B1	0x121		/* ditto.. */
#define INVALID_OP_B2	0x122		/* ditto.. */
#define INVALID_OP_C	0x130		/* 0 * INF */
#define INVALID_OP_D2	0x142		/* INF/INF */
#define INVALID_OP_E2	0x152		/* INF MOD <num.> */
#define	INVALID_OP_F2	0x162		/* operand INF */
#define INVALID_OP_G	0x170		/* result not representable */
#define INVALID_OP_H	0x180		/* unordered comparison (one Nan) */
#define INVALID_OP_H1	0x181		/* unordered comparison (both INF) */
#define DOMAIN_ERROR	0x600
#define CANT_REDUCE_RANGE  0x700 /* number too large for valid range redn */

/* this is the equivalent of INEXACT using the turbo fpa */
#define PARTIAL_SLOSS	0x801	/* result may have lost significance */

#define ILLEGAL_OP 0x900
#define INEXACT	0xa00
#define UNKN_ERROR 0xb00

/* in this case, HUGE should be printed */
#define CONVERT_INFINITY 0x500		/* ecvt (or such) of INF */

long float _lraise_fperror();
float _raise_fperror();

/*  the math routines set this global to the
	code (above) for the name of the offending routine */
int _mathfunc_id;
int _is_software;

#endif
