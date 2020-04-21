|
| bcmp(b1, b2, bytecount):
|	block compare subroutine; returns 0 if identical, non-zero otherwise
|

include(../DEFS.m4)

ENTRY(bcmp)
	movl	sp@(4),a0	| b1
	movl	sp@(8),a1	| b2

| Compare bytes, one byte at a time
	movl	sp@(12),d0	| bytecount
	jeq	99$		| nothing to do
1$:	cmpmb	a0@+,a1@+	| compare a long
	jne	99$		| mismatch
	subql	#1, d0
	jne	1$		| Loop until done
99$:	rts			| Return the 0 d0 (we matched)
