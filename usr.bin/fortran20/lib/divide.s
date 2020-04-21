
#include "fpa.h"
.LL0:
	.data
	.text
	.globl	_divide
_divide:
	link	a6,#-.F1
	moveml	#0x3fff,a6@(-.S2:w)
	movl	fpaop(DOUBLEHI,0,0),d4
	movl	fpaop(DOUBLELO,0,0),d5
	movl	fpaop(DOUBLEHI,1,0),d6
	movl	fpaop(DOUBLELO,1,0),d7
| A1 = 24
	movl	a6@(0x14),a6@(-4)
	movl	a6@(0x10),a6@(-8)
	lea	a6@(-8),a0
	movl	a0,a6@(-28)
	movl	a6@(-28),a0
	movl	a0@,d0
	andl	#0xfff00000,d0
	movl	a6@(-28),a0
	movl	d0,a0@
	movl	a6@(0x14),0x8410:w
	movl	a6@(0x10),0x8310:w
	movl	a6@(-4),0x8400:w
	movl	a6@(-8),0x9110:w
	movl	0x83f0:w,d1
	movl	0x84f0:w,d0

	movl	d1,fpaop(DOUBLEHI,1,0)
	movl	d0,fpaop(DOUBLELO,1,0)
	movl	a6@(12),fpaop(DOUBLELO,0,0)
	movl	a6@(8),fpaop(DIVDBLUPD,1,0)
	movl	fpaop(DOUBLEHI,1,0), a6@(-16)
	movl	fpaop(DOUBLELO,1,0), a6@(-12)

	movl	a6@(-8),fpaop(DOUBLEHI,1,0)
	movl	a6@(-4),fpaop(DOUBLELO,1,0)
	movl	a6@(12),fpaop(DOUBLELO,0,0)
	movl	a6@(8),fpaop(DIVDBLUPD,1,0)
	movl	fpaop(DOUBLEHI,1,0), a6@(-8)
	movl	fpaop(DOUBLELO,1,0), a6@(-4)


	movl	a6@(-12),0x8410:w
	movl	a6@(-16),0x8310:w
	movl	a6@(-4),0x8400:w
	movl	a6@(-8),0x8d10:w
	movl	0x83f0:w,a7@(0x34)
	movl	0x84f0:w,a7@(0x38)
.L12:
	movl	d4,fpaop(DOUBLEHI,0,0)
	movl	d5,fpaop(DOUBLELO,0,0)
	movl	d6,fpaop(DOUBLEHI,1,0)
	movl	d7,fpaop(DOUBLELO,1,0)
	moveml	a6@(-.S2),#0x3fff
	unlk	a6
	rtd	#8
.F1 = 36
.S1 = 0x0
.S2 = 0x800
.M1 = 132		| 0 + 132
| end
	.globl	fltused
	.data
