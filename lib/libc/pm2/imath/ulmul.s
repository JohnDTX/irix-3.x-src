|J. Test	1/81
|unsigned long multiply: c = a * b

include(../DEFS.m4)

ASENTRY(ulmul)
	moveml	#0x3000,sp@-	|save d2,d3
	movl	sp@(12),d2	|d2 = a
	movl	sp@(16),d3	|d3 = b
	jra	goulmul

RASENTRY(rulmul)
	moveml	#0x3000,sp@-	|save d2,d3
	movl	d0,d2		|d2 = a
	movl	d1,d3		|d3 = b

goulmul:
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

3$:	moveml	sp@+,#0xC	|restore d2,d3
	rts
