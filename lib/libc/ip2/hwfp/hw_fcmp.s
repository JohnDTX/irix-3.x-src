|	GB - mc68020 FPA 5/19/85
|
#ifndef FPA_DEFS
#define FPA_DEFS
#include "float.h"
include(../DEFS.m4)
#endif

/*
|
|	fcmp.s 	- H/W floating point compare routines for the Juniper
|		  FPA.  The following entries are in this module:
|
|		_f_cmp	-  compare two floats passed on the stack.
|
|	    register routines:
|		_fr_cmp -  compare two floats passed in d0/d1.
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
/*
|
|
|	OPs follow:
|
*/
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
ASENTRY(_f_cmp)
|
|	sp@(4)	- first operand	(a of a COMP b)
|	sp@(8)	- second operand
|
|	uses f1 
|
	fwritefpa(sp@(4),1)	| move second operand to f1
	fcmpx1(1,sp@(8))	| compare to first operand 
	tstb	FPA_CCR
	rts
|
|
RASENTRY(_fr_cmp)
|
|	d0	- first operand
|	d1	- second operand
|	
	fwritefpa(d0,1)		| move first operand to f1
	fcmpx1(1,d1)		| compare to second operand 
	tstb	FPA_CCR
	rts
