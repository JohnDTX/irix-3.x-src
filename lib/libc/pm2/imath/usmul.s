| long = usmul(long, short);

include(../DEFS.m4)

ASENTRY(usmul)
	movl	d2,sp@-		|save d2
	movl	sp@(8),d2	|d2 = a
	movl	sp@(12),d1	|d1 = b

	movw	d2,d0		|d0 = alo, unsigned
	mulu	d1,d0		|d0 = blo*alo, unsigned
	swap	d2		|swap alo-ahi
	mulu	d1,d2		|d2 = blo*ahi, unsigned
	swap	d2		|d1 =
	clrw	d2		|   (ahi*blo + alo*bhi)*(2**16)
	addl	d2,d0		|d0 = alo*blo + (ahi*blo + alo*bhi)*(2**16)

3$:	movl	sp@+,d2		|restore d2
	rts
