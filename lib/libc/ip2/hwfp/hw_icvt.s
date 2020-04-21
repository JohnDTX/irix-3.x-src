|
|	GB - mc68020 FPA 5/19/85
|
#ifndef FPA_DEFS
#define FPA_DEFS
#include "float.h"
include(../DEFS.m4)
#endif

/*
|
|	icvt.s 	- H/W floating point convert routines for the Juniper
|		  FPA.  The following entries are in this module:
|
|		_i_2_d	-  convert the integer on the stack to a 
|			   long float.
|		_i_2_f  -  convert the integer on the stack to an
|			   float.
|		_ir_2_d -  convert the integer in d0 to a long float.
|		_ir_2_f -  convert the integer in d0 to an float.
|		float   -  same as _i_2_f
|
|
|	Floating point exception handling - 
|
|	    When an error is detected in an operation in this module, 
|	a call to the appropriate floating point error handler is made
|	with arguments to indicate the error condition.  This consists
|	of a call to __lraise_fperror or __raise_fperror with the arguments
|	op and type.
|
*/
	.globl	__raise_fperror
	.globl	__lraise_fperror
|
|
|	OPs follow:
|
ADD 	=	1
SUB	=	2
MUL	=	3
DIV	=	4
FIX   	=	5	| precision to integer 
PRECISION =	6	| precision change to given precision 
MOD	=	7
CMP	=	8

|
|	TYPEs
|
INVALID_OP_A	=0x110
INVALID_OP_B2	=0x122
INVALID_OP_C	=0x130
INVALID_OP_D1	=0x141
INVALID_OP_D2	=0x142
INVALID_OP_E1	=0x151
INVALID_OP_E2	=0x152
INVALID_OP_G	=0x170
INVALID_OP_H	=0x180
|
DIVZERO		=0x200
OVERFLOW	=0x300
|
|
ASENTRY(__lfloat)
ASENTRY(_i_2_d)
|
|	sp@(4)	- input integer
|
|	uses f0 
|
	idcvtx0(sp@(4))		| convert, leaving in result reg
	dreadfpalo(f,d1)
	dreadfpahi(f,d0)
	rts
|
|
RASENTRY(_ir_2_d)
|
|	d0	- input integer
|
	idcvtx0(d0)		| convert, leaving in result reg
	dreadfpalo(f,d1)
	dreadfpahi(f,d0)
	rts
|
|
ASENTRY(_float)
ASENTRY(_i_2_f)
|
|	sp@(4)	- input integer
|
|	uses f0 
|
	ifcvtx0(sp@(4))		| convert, leaving in result reg
	freadfpa(f,d0)
	rts
|
|
RASENTRY(_ir_2_f)
|
|	d0	- input integer
|
	ifcvtx0(d0)		| convert, leaving in result reg
	freadfpa(f,d0)
	rts
