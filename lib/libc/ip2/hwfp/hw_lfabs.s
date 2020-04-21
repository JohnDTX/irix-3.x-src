|
| fabs.s - floating point absolute value functions.
|	   these are separate since they are also
|  	   used for h/w floating point.
|

#ifndef FPA_DEFS
#define FPA_DEFS
#include "float.h"
include(../DEFS.m4)
#endif

ENTRY(_lfabs)
	movl	a7@(8),d1	|move high order word to d1  (GB)
	movl	a7@(4),d0
	bclr	#31,d0
	rts
