|
| bzero(p, n):
|	writes n bytes of zeros, starting at p
|

include(../DEFS.m4)

ENTRY(bzero)
	movl	sp@(4),d1	|p
	movl	sp@(8),d0	|n
	jeq	7$		|nothing to do
	addl	d0,d1		|&p[n]
	movl	d1,a0		|save it
	andl	#1,d1		|word aligned?
	jeq	1$		|yes, potentially long moves
	clrb	a0@-		|clear up to word boundry
	subql	#1,d0		|one less byte to clear
	jeq	7$		|nothing left

1$:	movl	d0,d1		|copy n
	andl	#0xffffff00,d1	|m = number of 256 byte blocks left * 256
	jeq	3$		|none

	subl	d1,d0		|we will do this many bytes in next loop
	asrl	#8,d1		|number of blocks left
	moveml	#0xFF7E,sp@-	|save registers
	movl	d1,sp@-		|number of blocks goes on top of stack
	movl	#zeros,a1
	moveml	a1@,#0x7CFF	|clear out a bunch of registers
	movl	d0,a1		|and this one too

2$:	moveml	#0xFF7E,a0@-	|clear out 14 longs worth
	moveml	#0xFF7E,a0@-	|clear out 14 longs worth
	moveml	#0xFF7E,a0@-	|clear out 14 longs worth
	moveml	#0xFF7E,a0@-	|clear out 14 longs worth
	moveml	#0xFF00,a0@-	|clear out 8 longs worth, total of 256 bytes
	subql	#1,sp@		|one more block, any left?
	jgt	2$		|yes, do another pass

	movl	sp@+,d1		|just pop stack
	moveml	sp@+,#0x7EFF	|give me back the registers

3$:	movl	d0,d1		|copy n left
	andl	#0xfffffff8,d1		|this many longs left
	jeq	5$		|none
	subl	d1,d0		|do this many in next loop

4$:	clrl	a0@-		|clear a long's worth
	subql	#4,d1		|this many bytes in a long
	jgt	4$		|if there are more

5$:	tstl	d0		|anything left?
	jeq	7$		|no, just stop here

6$:	clrb	a0@-		|clear 1 byte's worth
	subql	#1,d0		|one less byte to do
	jgt	6$		|if any more

7$:	rts			|that's it

zeros:	.long	0,0,0,0,0,0,0,0,0,0,0,0,0	|13 long  of zeros
