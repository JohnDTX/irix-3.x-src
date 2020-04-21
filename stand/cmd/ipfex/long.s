|*
|* $Source: /d2/3.7/src/stand/cmd/ipfex/RCS/long.s,v $
|* $Date: 89/03/27 17:12:27 $
|* $Revision: 1.1 $
|*

| cmpl(wp, pat, nl)
| filll(wp, pat, nl)

	.globl	_cmpl, _filll
	.globl	_clea, _cleis

_cmpl:	link	a6,#-20			| Keep a6 linkage
	moveml	#0x43c, a6@(-20)		| Save a2, d5-d2
	movl	a6@(8), a2		| address into a2
	movl	a6@(12), d2		| pattern into d2
	movl	a6@(16), d4		| number into d4
	movl	d4, d5			| copy count
	moveq	#15, d0			| get count of 2^15
	asrl	d0, d5
	andl	#0x7FFF, d4
	tstl	d5			| is there a high count?
	beq	.w5
	orw	#0x8000, d4		| set low count
	subql	#1, d5			| fix high count
.w5:	clrl	d3			| fix d3 & clr Z bit
	bras	.w1			| branch into loop
.w0:	cmpl	a2@+, d2		| compare memory word
.w1:	dbne	d4, .w0			| check compare and loop count
	cmpw	#-1, d4			| check loop count again
	beq	.w2			| yes, exhausted
	tstl	a2@-
	movl	a2@, _cleis		| Save bad word
	movl	a2, _clea		| and bad address
| Following 3 lines for continue on error
|	tstl	a2@+			| restore a2 pointer
|	clrl	d3			| clear Z bit
|	bras	.w1			| and continue
	moveq	#1,d0			| Bad return
	bras	.w3			| Quit on error case
.w2:	movw	#0x8000, d4		| reload lower counter
	clrl	d3			| clear Z again
	dbra	d5, .w1			| loop on higher counter
	clrl	d0			| Good return
.w3:	moveml	a6@(-20),#0x43c		| restore stacked regs
	unlk	a6			| and a6 linkage
	rts				| and return

_filll:	movl	d2, sp@-
	movl	sp@(16), d0		| count
	movl	d0, d2
	moveq	#15, d1
	asrl	d1, d2
	andl	#0x7FFF, d0
	tstl	d2			| is there a high count?
	beq	.f0
	orw	#0x8000, d0
	subqw	#1, d2
.f0:	movl	sp@(8), a0		| address
	movl	sp@(12), d1		| pattern
	bras	.f2
.f1:	movl	d1, a0@+		| move pattern
.f2:	dbra	d0, .f1			| loop
	movl	#0x8000, d0
	dbra	d2, .f1
	movl	sp@+, d2
	rts
