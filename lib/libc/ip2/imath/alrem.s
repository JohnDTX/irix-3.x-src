| 
|addressed signed long remainder: *dividend = *dividend % divisor
|
| GB - SGI. mc68020 version  5/19/85

include(../DEFS.m4)

ASENTRY(alrem)
|
|	addressed signed long remainder.  
|	
|	sp@(4) - address of dividend
|	sp@(8) - divisor
|
	movl	sp@(8),d0
	movl	sp@(4),a0
	bra	goalrem

RASENTRY(ralrem)
|
|	a0 - address of dividend
|	d0 - divisor
|
goalrem:
|	
|	d1 is volatile, so we can clobber it.
|	
	movl	a0@,d1		|get dividend
	divsll	d0,d0:d1	|do the 32/32 division 
				|and get remainder in d0
	movl	d0,a0@
	rts
