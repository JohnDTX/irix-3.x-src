|	GB - mc68020 FPA 5/19/85
|
#ifndef FPA_DEFS
#define FPA_DEFS
#include "float.h"
include(../DEFS.m4)
#endif

/*
|
|	ddiv.s 	- H/W floating point divide routines for the Juniper
|		  FPA.  The following entries are in this module:
|
|		_d_div	-  divide two doubles passed on the stack.
|
|	    indirect routines:
|		_d_idiv -  divide the double whose address is on the 
|			   stack at sp@(4) by that on the stack (sp@(8)).
|		_dr_idiv - divide the double whose address is in a0
|			   from the double in d0/d1		
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
ASENTRY(_d_div)
|
|	sp@(4)	- dividend doublelong (a of a/b)
|	sp@(12)	- divisor doublelong
|
|	uses f1,f0 
|
	dwritefpahi(sp@(4),1)	| move dividend to f1 - msl
	dwritefpalo(sp@(8),1)	| move dividend to f1 - lsl
	dwritefpalo(sp@(16),0)	| move divisor to f0 - lsl
	ddivx1upd(1,sp@(12))	| divide dividend and 
				|	update f1 with result
	dreadfpahi(1,d0)
	dreadfpalo(1,d1)
	rts
|
|
RASENTRY(_dr_idiv)
|
|	d0/d1	- divisor	(b of a/b)
|	a0	- address of dividend, and address of destination
|
	dwritefpahi(a0@,1)	| move dividend to f1 - msl
	dwritefpalo(a0@(4),1)	| move dividend to f1 - lsl
	dwritefpalo(d1,0)	| movl lsl of divisor to f0 low
	ddivx1upd(1,d0)		| divide f1/d[01] and update f1 with result 
	dreadfpalo(1,a0@(4))	| get lsl of result
	movl	a0@(4),d1
	dreadfpahi(1,a0@)	| get msl of result
	movl	a0@,d0
	rts
|
|
ASENTRY(_d_idiv)
|
|	sp@(8)	- divisor double
|	sp@(4)	- address of dividend, and address of destination
|
	movl	sp@(4),a0	| get indirect address
	dwritefpahi(a0@,1)	| get msl of dividend
	dwritefpalo(a0@(4),1) 	| get lsl of dividend
	dwritefpalo(sp@(12),0) 	| get lsl of divisor
	ddivx1upd(1,sp@(8))	| divide f1/sp@(8) and update f1
	dreadfpalo(1,a0@(4)) 	| get lsl of result
	movl	a0@(4),d1
	dreadfpahi(1,a0@)	| get msl of result
	movl	a0@,d0
	rts
