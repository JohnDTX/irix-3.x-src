	.globl	_cmpw, _perr, _fillw

_cmpw:	link	a6,#-20			| Keep a6 linkage
	moveml	#/43c, a6@(-20)		| Save a2, d5-d2
	movl	a6@(8), a2		| address into a2
	movl	a6@(12), d2		| pattern into d2
	movl	a6@(16), d4		| number into d4
	movl	d4, d5			| copy count
	movl	#15, d0			| get count of 2^15
	asrl	d0, d5
	andl	#/7FFF, d4
	orw	#/8000, d4		| set low count
	subql	#1, d5			| fix high count
	clrl	d3			| fix d3 & clr Z bit
	bras	.w1			| branch into loop
.w0:	movw	a2@+, d3		| fetch memory word
	cmpw	d3, d2			| compare it
.w1:	dbne	d4, .w0			| check compare and loop count
	cmpw	#-1, d4			| check loop count again
	beq	.w2			| yes, exhausted
	movl	d3, sp@-		| no, comparison error, call
	movl	d2, sp@-		|   perr(a, h, pat, is)
	movl	a6@(20), sp@-
	tstw	a2@-
	movl	a2, sp@-
	jbsr	_perr
	addl	#16, sp			| pop stack
	tstw	a2@+			| restore a2 pointer
	clrl	d3			| clear Z bit
	bras	.w1			| and continue
.w2:	movw	#/8000, d4		| reload lower counter
	clrl	d3			| clear Z again
	dbra	d5, .w1			| loop on higher counter
	moveml	a6@(-20),#0x43c		| restore stacked regs
	unlk	a6			| and a6 linkage
	rts				| and return

_fillw:	movl	d2, sp@-
	movl	sp@(16), d0		| count
	movl	d0, d2
	movl	#15, d1
	asrl	d1, d2
	andl	#/7FFF, d0
	orw	#/8000, d0
	subqw	#1, d2
	movl	sp@(8), a0		| address
	movl	sp@(12), d1		| pattern
	bras	.f1
.f0:	movw	d1, a0@+		| move pattern
.f1:	dbra	d0, .f0			| loop
	movl	#/8000, d0
	dbra	d2, .f1
	movl	sp@+, d2
	rts
