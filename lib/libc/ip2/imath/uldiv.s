|unsigned long division: quotient = dividend / divisor
|
| GB - SGI. mc68020 version  5/19/85

include(../DEFS.m4)

ASENTRY(uldiv)
|
|	sp@(4) - dividend
|	sp@(8) - divisor
|
	movl	sp@(4),d0	|dividend
	movl	sp@(8),d1	|divisor
	bra	gouldiv

RASENTRY(ruldiv)
|
|	d0	- dividend
|	d1	- divisor
|
gouldiv:
	divul	d1,d0		|result in d0, where it belongs.
	rts	
