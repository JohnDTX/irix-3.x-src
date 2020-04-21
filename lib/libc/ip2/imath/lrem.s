|signed long remainder: a = a % b
|
| GB - SGI. mc68020 version  5/19/85

include(../DEFS.m4)

ASENTRY(lrem)
|
|	signed long remainder.  
|	
|	sp@(4) - dividend
|	sp@(8) - divisor
|
	movl	sp@(4),d0
	movl	sp@(8),d1
	bra	golrem

RASENTRY(rlrem)
|
|	d0 - dividend
|	d1 - divisor
|
golrem:
	divsll	d1,d1:d0	|remainder in d1
	movl	d1,d0
	rts
