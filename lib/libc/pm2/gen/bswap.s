|
| bswap(source, destination, count):
|	block transfer subroutine; assumes even addresses and count
|

include(../DEFS.m4)

ENTRY(bswap)
	movl	sp@(4),a1	| source
	movl	sp@(8),a0	| destination
	movl	sp@(12),d1	| count
1$:	cmpl	#8,d1
	jlt	2$
	movepl	a1@(ZERO:w),d0	| get odds
	movepl	d0,a0@(1)	| place even
	movepl	a1@(1),d0	| get evens
	movepl	d0,a0@(ZERO:w)	| place odd
	addql	#8,a0
	addql	#8,a1
	subql	#8,d1
	jra	1$
2$:	tstl	d1		| um um I love leftovers
	jeq	4$
3$:	movw	a1@+,d0
	rolw	#8,d0
	movw	d0,a0@+
	subql	#2,d1
	jgt	3$
4$:	rts
ZERO = 0
