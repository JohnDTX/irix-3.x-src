|J. Test	1/81
|addressed unsigned long remainder: *dividend = *dividend % divisor

include(../DEFS.m4)

ASENTRY(aulrem)
	moveml	#0x3000,sp@-	|need d2,d3 registers
	movl	sp@(12),a0	|a0 = dividend pointer
	movl	sp@(16),d1	|divisor
	jra	goaulrem

RASENTRY(raulrem)
	moveml	#0x3000,sp@-	|need d2,d3 registers
	movl	d0,d1
goaulrem:
	movl	a0@,d0		|d0 = dividend
	movl	d0,d2		|save dividend

	cmpl	#0x10000,d1	|divisor < 2 ** 16?
	jge	1$		|no, divisor must be < 2 ** 16
	clrw	d0		|d0 =
	swap	d0		|   dividend high
	divu	d1,d0		|yes, divide
	movw	d2,d0		|d0 = remainder high + quotient low
	divu	d1,d0		|divide
	clrw	d0		|d0 = 
	swap	d0		|   remainder
	jra	4$		|return

1$:	movl	d1,d3		|save divisor
2$:	lsrl	#0x1,d0		|shift dividend
	lsrl	#0x1,d1		|shift divisor
	cmpl	#0x10000,d1	|divisor < 2 ** 16?
	jge	2$		|no, continue shift
	divu	d1,d0		|yes, divide
	andl	#0xFFFF,d0	|erase remainder
	movl	d3,d1		|  and saved divisor on stack
	jbsr	rulmul		|  as arguments
	cmpl	d0,d2		|original dividend >= lmul result?
	jge	3$		|yes, quotient should be correct
	subl	d3,d0		|no, fixup 
3$:	subl	d2,d0		|calculate
	negl	d0		|  remainder

4$:	movl	d0,a0@		|store result through pointer
	moveml	sp@+,#0xC	|restore registers
	rts
