|J. Test	1/81
|signed long multiply: c = a * b

include(../DEFS.m4)

ASENTRY(lmul)
	moveml	#0x3800,sp@-	|save d2,d3,d4
	movl	sp@(16),d2	| d2 = a
	movl	d2,d4		|sign of result
	jge	1$
	negl	d2
1$:	movl	sp@(20),d3	| d3 = b
	jge	.L1
	jra	golmul

RASENTRY(rlmul)
	moveml	#0x3800,sp@-	|save d2,d3,d4
	movl	d0,d2		| d2 = a
	movl	d2,d4		|sign of result
	jge	8$
	negl	d2
8$:	movl	d1,d3		| d3 = b
	jge	.L1

golmul:
	negl	d3
	negl	d4

.L1:	movw	d2,d0		|d0 = alo, unsigned
	mulu	d3,d0		|d0 = blo*alo, unsigned
	movw	d2,d1		|d1 = alo
	swap	d2		|swap alo-ahi
	mulu	d3,d2		|d2 = blo*ahi, unsigned
	swap	d3		|swap blo-bhi
	mulu	d3,d1		|d1 = bhi*alo, unsigned
	addl	d2,d1		|d1 = (ahi*blo + alo*bhi)
	swap	d1		|d1 =
	clrw	d1		|   (ahi*blo + alo*bhi)*(2**16)
	addl	d1,d0		|d0 = alo*blo + (ahi*blo + alo*bhi)*(2**16)
	tstl	d4		|sign of result
	jge	3$
	negl	d0

3$:	moveml	sp@+,#0x1C	|restore d2,d3,d4
	rts
