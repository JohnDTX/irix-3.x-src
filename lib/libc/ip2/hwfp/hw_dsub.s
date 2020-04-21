|	GB - mc68020 FPA 5/19/85
|
#ifndef FPA_DEFS
#define FPA_DEFS
#include "float.h"
include(../DEFS.m4)
#endif

/*
|
|	dsub.s 	- H/W floating point subtract routines for the Juniper
|		  FPA.  The following entries are in this module:
|
|		_d_sub	-  subtract two doubles passed on the stack.
|
|	    indirect routines:
|		_d_isub -  subtract the double on the stack (sp@(8)) from 
|			   that whose address is on the stack at sp@(4).
|		_dr_isub - subtract the double in d0/d1 from that whose
|			   address is in a0.
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
ASENTRY(_d_sub)
|
|	sp@(4)	- first subtrahend doublelong (a of a-b)
|	sp@(12)	- second subtrahend doublelong
|
|	uses f1,f0 
|
	dwritefpahi(sp@(4),1)	| move first subtrahend to f1 - msl
	dwritefpalo(sp@(8),1)	| move first subtrahend to f1 - lsl
	dwritefpalo(sp@(16),0)	| move second subtrahend to f0 - lsl
	dsubx1upd(1,sp@(12))	| subtract first subtrahend and 
				|	update f1 with result
	dreadfpahi(1,d0)
	dreadfpalo(1,d1)
	rts
|
|
RASENTRY(_dr_isub)
|
|	d0/d1	- second subtrahend	(b of a-b)
|	a0	- address of first subtrahend, and address of destination
|
	dwritefpahi(a0@,1)	| move first subtrahend to f1 - msl
	dwritefpalo(a0@(4),1)	| move first subtrahend to f1 - lsl
	dwritefpalo(d1,0)	| movl lsl of second subtrahend to f0 low
	dsubx1upd(1,d0)		| store result indirect
	dreadfpalo(1,a0@(4))	| get lsl of result
	dreadfpahi(1,a0@)	| get msl of result
	movl	a0@(4),d1
	movl	a0@,d0
	rts
|
|
ASENTRY(_d_isub)
|
|	sp@(8)	- second subtrahend double
|	sp@(4)	- address of first subtrahend, and address of destination
|
	movl	sp@(4),a0	| get indirect address
	dwritefpahi(a0@,1)	| get msl of first subtrahend
	dwritefpalo(a0@(4),1) 	| get lsl of first subtrahend
	dwritefpalo(sp@(12),0) 	| get lsl of second subtrahend
	dsubx1upd(1,sp@(8))	| move in msl of second and do the subtraction
				|    (f1 - sp@(8)) into f1.
	dreadfpalo(1,a0@(4)) 	| get lsl of result
	dreadfpahi(1,a0@)	| get msl of result
	movl	a0@(4),d1
	movl	a0@,d0
	rts
