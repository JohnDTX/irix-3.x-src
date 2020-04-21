#include "fpa.h"
	.globl	dbg
dbg:
	linkw	a6,#-64
	moveml	#0x3f00,a7@-
	movl	fpaop(DOUBLEHI,0,0),d4
	movl	fpaop(DOUBLELO,0,0),d5
	movl	fpaop(DOUBLEHI,1,0),d6
	movl	fpaop(DOUBLELO,1,0),d7
	movb	fpaop(OR,0,0),d2
	movb	fpaop(CR,0,0),d3
	moveml	a7@+,#0x00fc
	unlk	a6
	rtd	#0
