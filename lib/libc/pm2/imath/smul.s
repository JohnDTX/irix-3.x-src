| multiply: long = smul(long, short);

include(../DEFS.m4)

ASENTRY(smul)
	moveml	#0x3000,sp@-	|save d2,d3
	movl	sp@(12),d2	| d2 = a
	movl	d2,d1		|sign of result
	jge	1$
	negl	d2
1$:	movl	sp@(16),d3	| d3 = a
	jge	2$
	negl	d3
	negl	d1

2$:	movw	d2,d0		|d0 = alo, unsigned
	mulu	d3,d0		|d0 = b*alo, unsigned
	swap	d2		|swap alo-ahi
	mulu	d3,d2		|d2 = b*ahi, unsigned
	swap	d2		|d2 =
	clrw	d2		|   (ahi*blo + alo*bhi)*(2**16)
	addl	d2,d0		|d0 = alo*blo + (ahi*blo + alo*bhi)*(2**16)
	tstl	d1		|sign of result
	jge	3$
	negl	d0

3$:	moveml	sp@+,#0xC	|restore d2,d3
	rts
