|J. Test	1/81
|addressed signed long division: *dividend = *dividend/divisor
|
| GB - SGI. mc68020 version  5/19/85

include(../DEFS.m4)

ASENTRY(aldiv)
|
|	addressed long division.  
|	
|	sp@(4) - address of dividend
|	sp@(8) - divisor
|
	movl	sp@(8),d0
	movl	sp@(4),a0
	bra		.goraldiv

RASENTRY(raldiv)
.goraldiv:
|
|	a0 - address of dividend
|	d0 - divisor
|
	movl	a0@,d1
	divsl	d0,d1
	movl	d1,a0@
	movl	d1,d0
	rts
