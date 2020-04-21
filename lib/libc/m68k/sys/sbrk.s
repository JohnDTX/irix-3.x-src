| C library -- sbrk

| old = sbrk(increment);
| sbrk gets increment more core, and returns a pointer
|	to the beginning of the new core area

include(../DEFS.m4)

ENTRY(sbrk)
	movl	nd,d0
	addl	d0,sp@(4)	| incr + end = new low memory location
	moveq	#17,d0
	movl	sp@(4),a0	| fetch argument
	trap	#0
	jcs	lcerror
	movl	nd,d0		| old nd location becomes value
	movl	sp@(4),nd	| set new end
	rts

lcerror:jmp	cerror

ENTRY(brk)
	moveq	#17,d0
	movl	sp@(4),a0	| fetch argument
	trap	#0
	jcs	lcerror	
	movl	sp@(4),nd	| end = lowest memory location not used
	moveq	#0,d0
	rts

	.data
	.globl	_end
nd:	.long	_end
