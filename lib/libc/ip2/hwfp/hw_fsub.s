|	GB - mc68020 FPA 5/19/85
|
#ifndef FPA_DEFS
#define FPA_DEFS
#include "float.h"
include(../DEFS.m4)
#endif

/*
|
|	fsub.s 	- H/W floating point subtract routines for the Juniper
|		  FPA.  The following entries are in this module:
|
|		_f_sub	-  subtract two floats passed on the stack.
|		_d_sub	-  subtract two doubles passed on the stack.
|
|	    register routines:
|		_fr_sub -  subtract two floats passed in d0/d1.
|
|	    indirect routines:
|		_fr_isub - subtract the float in d0 to that whose address
|			   is in a0.
|		_dr_isub - subtract the double in d0/d1 to that whose
|			   address is in a0.
|
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
ASENTRY(_f_sub)
|
|	sp@(4)	- first subtrahend (a of a-b)
|	sp@(8)	- second subtrahend
|
|	uses f1 
|
	fwritefpa(sp@(4),1)	| move first subtrahend to f1
	fsubx1upd(1,sp@(8))	| subtract second subtrahend and 
				|	update f1 with result
	freadfpa(1,d0)
	rts
|
|
RASENTRY(_fr_sub)
|
|	d0	- first subtrahend (a of a-b)
|	d1	- second subtrahend
|	
	fwritefpa(d0,1)		| move first subtrahend to f1
	fsubx1upd(1,d1)		| subtract second subtrahend and 
				|	update f1 with result
	freadfpa(1,d0)		| get result in d0
	rts
|
|
ASENTRY(_f_isub)
|
|	sp@(8)	- second subtrahend (b of a-b)
|	sp@(4)	- address of first subtrahend, and address of destination
|
	movl	sp@(4),a0	| get address for indirect
	fwritefpa(a0@,1)	| move first subtrahend to f1
	fsubx1upd(1,sp@(8))	| subtract second and update f1
	freadfpa(1,a0@)		| store result indirect
	movl	a0@,d0
	rts
|
|
RASENTRY(_fr_isub)
|
|	d0	- second subtrahend (b of a-b)
|	a0	- address of first subtrahend, and address of destination
|
	fwritefpa(a0@,1)	| move first subtrahend to f1
	fsubx1upd(1,d0)		| subtract in indirect second and update f1
	freadfpa(1,a0@)		| store result indirect
	movl	a0@,d0
	rts
