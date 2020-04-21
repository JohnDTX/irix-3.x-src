|addressed unsigned long division: *dividend = *dividend / divisor
|
| GB - SGI. mc68020 version  5/19/85

include(../DEFS.m4)

ASENTRY(auldiv)
|
|	addressed long division.  
|	
|	sp@(4) - address of dividend
|	sp@(8) - divisor
|
	movl	sp@(8),d0
	movl	sp@(4),a0
	bra		gorauldiv

RASENTRY(rauldiv)
gorauldiv:
|
|	a0 - address of dividend
|	d0 - divisor
|
	movl	a0@,d1
	divul	d0,d1
	movl	d1,a0@
	movl	d1,d0
	rts
