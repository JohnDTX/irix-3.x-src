|
| fabs.s - floating point absolute value functions.
|	   these are separate since they are also
|  	   used for h/w floating point.
|

include(../DEFS.m4)

ENTRY(fabs)
	movl	a7@(4),d0
	bclr	#31,d0
	rts
