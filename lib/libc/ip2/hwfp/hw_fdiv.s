|	GB - mc68020 FPA 5/19/85
|
#ifndef FPA_DEFS
#define FPA_DEFS
#include "float.h"
include(../DEFS.m4)
#endif

/*
|
|	fdiv.s 	- H/W floating point divide routines for the Juniper
|		  FPA.  The following entries are in this module:
|
|		_f_div	-  divide two floats passed on the stack.
|
|	    register routines:
|		_fr_div -  divide two floats passed in d0/d1.
|
|	    indirect routines:
|		_fr_idiv - divide the float in d0 to that whose address
|			   is in a0.
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
ASENTRY(_f_div)
|
|	sp@(4)	- dividend
|	sp@(8)	- divisor
|
|	uses f1 
|
	fwritefpa(sp@(4),1)	| move dividend to f1
	fdivx1upd(1,sp@(8))	| divide dividend by sp@(8)
	freadfpa(1,d0)
	rts
|
|
RASENTRY(_fr_div)
|
|	d0	- dividend
|	d1	- divisor
|	
	fwritefpa(d0,1)		| move dividend to f1
	fdivx1upd(1,d1)		| divide by f1
	freadfpa(1,d0)		| get result in d0
	rts
|
|
ASENTRY(_f_idiv)
|
|	sp@(8)	- divisor
|	sp@(4)	- address of dividend, and address of destination
|
	movl	sp@(4),a0	| get address for indirect
	fwritefpa(a0@,1)	| move dividend to f1
	fdivx1upd(1,sp@(8))	| divide by divisor and update f1
	freadfpa(1,a0@)		| store result indirect
	movl	a0@,d0
	rts
|
|
RASENTRY(_fr_idiv)
|
|	d0	- divisor (b of a/b)
|	a0	- address of dividend, and address of destination
|
	fwritefpa(a0@,1)	| move dividend to f1
	fdivx1upd(1,d0)		| divide by divisor 
				|	and update f1
	freadfpa(1,a0@)		| store result indirect
	movl	a0@,d0
	rts
