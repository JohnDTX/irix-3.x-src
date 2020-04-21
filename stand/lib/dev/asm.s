|	asm.s	1.1	01/08/86

|
| Return the 1's complement checksum of the word aligned buffer
| at s, for n bytes.
|
| in_cksum(s,n) 
| u_short *s; int n;
	
	.globl	_in_cksum
_in_cksum:
	movl	sp@(4),a0
	movl	sp@(8),d1
	asrl	#1,d1
	subql	#1,d1
	clrl	d0
1$:
	addw	a0@+,d0
	bccs	2$
	addqw	#1,d0
2$:
	dbra	d1,1$
	notw	d0
	rts
