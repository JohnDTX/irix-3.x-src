|J. Test	1/81
|addressed unsigned long multiply: *a = *a * b

include(../DEFS.m4)

ASENTRY(aulmul)
	moveml	#0x3000,sp@-	|save d2,d3
	movl	sp@(16),a0	|a0 = pointer to a
	movl	sp@(20),d3	|d3 = b
	jra	goaulmul

RASENTRY(raulmul)
	moveml	#0x3000,sp@-	|save d2,d3
	movl	d0,d3

goaulmul:
	movl	a0@,d2		|d2 = a

	moveq	#0,d0
	movw	d2,d0		|d0 = alo, unsigned
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

	movl	d0,a0@		|store result via pointer
	moveml	sp@+,#0xC	|restore d2,d3
	rts
