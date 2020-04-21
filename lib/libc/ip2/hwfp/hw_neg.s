|
|	neg.s - negate routines.  Simply change the most significant
|		bit.  
|
#ifndef FPA_DEFS
#define FPA_DEFS
#include "float.h"
include(../DEFS.m4)
#endif

ASENTRY(_d_neg)
ASENTRY(__lfneg)
		movl	sp@(8),d1	|move least signif word to d1  
ASENTRY(_f_neg)
ASENTRY(_fneg)
		movl	sp@(4),d0	|move msl to d0
RASENTRY(_dr_neg)
RASENTRY(_fr_neg)
		bchg	#31,d0		|and invert the bit.
                rts

