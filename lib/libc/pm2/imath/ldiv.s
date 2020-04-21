|J. Test	1/81
|signed long division: quotient = dividend / divisor

include(../DEFS.m4)

ASENTRY(ldiv)
	moveml	#0x3C00,sp@-	|need d2,d3,d4,d5 registers
	moveq	#1,d5		|sign of result
	movl	sp@(20),d0	|dividend
	jge	1$
	negl	d0
	negb	d5
1$:	movl	d0,d3		|save positive dividend
	movl	sp@(24),d1	|divisor
	jge	goldiv
	negl	d1
	negb	d5
	jra	goldiv

RASENTRY(rldiv)
	moveml	#0x3C00,sp@-	|need d2,d3,d4,d5 registers
	moveq	#1,d5		|sign of result
	tstl	d0		|dividend
	jge	8$
	negl	d0
	negb	d5
8$:	movl	d0,d3		|save positive dividend
	tstl	d1		|divisor
	jge	goldiv
	negl	d1
	negb	d5

goldiv:
	movl	d1,d4		|save positive divisor

	cmpl	#0x10000,d1	|divisor >= 2 ** 16?
	jge	3$		|yes, divisor must be < 2 ** 16
	clrw	d0		|divide dividend
	swap	d0		|  by 2 ** 16
	divu	d1,d0		|get the high order bits of quotient
	movw	d0,d2		|save quotient high
	movw	d3,d0		|dividend low + remainder * (2**16)
	divu	d1,d0		|get quotient low
	swap	d0		|temporarily save quotient low in high
	movw	d2,d0		|restore quotient high to low part of register
	swap	d0		|put things right
	jra	5$		|return

3$:	lsrl	#1,d0		|shift dividend
	lsrl	#1,d1		|shift divisor
	cmpl	#0x10000,d1	|divisor < 2 ** 16?
	jge	3$		|no, continue shift
	divu	d1,d0		|yes, divide, remainder is garbage
	andl	#0xFFFF,d0	|get rid of remainder
	movl	d0,d2		|save quotient
	movl	d4,d1		|  and saved divisor on stack
	jbsr	rulmul		|  as arguments
	cmpl	d0,d3		|original dividend >= lmul result?
	jge	4$		|yes, quotient should be correct
	subql	#1,d2		|no, fix up quotient

4$:	movl	d2,d0		|move quotient to d0
5$:	tstb	d5		|sign of result
	jge	6$
	negl	d0
6$:	moveml	sp@+,#0x3C	|restore registers
	rts
