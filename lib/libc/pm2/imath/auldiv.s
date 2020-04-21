|J. Test	1/81
|addressed unsigned long division: *dividend = *dividend / divisor

include(../DEFS.m4)

ASENTRY(auldiv)
	moveml	#0x3800,sp@-	|need d2,d3,d4 registers
	movl	sp@(16),a0	|a0 = dividend pointer
	movl	a0@,d0		|d0 = dividend
	movl	d0,d3		|save dividend
	movl	sp@(20),d1	|divisor
	movl	d1,d4		|save divisor
	jra	goauldiv

RASENTRY(rauldiv)
	moveml	#0x3800,sp@-	|need d2,d3,d4 registers
	movl	d0,d1
	movl	d1,d4		|save divisor
	movl	a0@,d0		|d0 = dividend
	movl	d0,d3		|save dividend

goauldiv:
	cmpl	#0x10000,d1	|divisor >= 2 ** 16?
	jge	1$		|yes, divisor must be < 2 ** 16
	clrw	d0		|divide dividend
	swap	d0		|  by 2 ** 16
	divu	d1,d0		|get the high order bits of quotient
	movw	d0,d2		|save quotient high
	movw	d3,d0		|dividend low + remainder * (2**16)
	divu	d1,d0		|get quotient low
	swap	d0		|temporarily save quotient low in high
	movw	d2,d0		|restore quotient high to low part of register
	swap	d0		|put things right
	jra	3$		|return

1$:	lsrl	#0x1,d0		|shift dividend
	lsrl	#0x1,d1		|shift divisor
	cmpl	#0x10000,d1	|divisor < 2 ** 16?
	jge	1$		|no, continue shift
	divu	d1,d0		|yes, divide, remainder is garbage
	andl	#0xFFFF,d0	|get rid of remainder
	movl	d0,d2		|save quotient
	movl	d4,d1		|  and saved divisor on stack
	jbsr	rulmul		|  as arguments
	cmpl	d0,d3		|original dividend >= lmul result?
	jge	2$		|yes, quotient should be correct
	subql	#1,d2		|no, fix up quotient
2$:	movl	d2,d0		|move quotient to d0

3$:	movl	d0,a0@		|store result via pointer
	moveml	sp@+,#0x1C	|restore registers
	rts
