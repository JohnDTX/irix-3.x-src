| C library -- times

include(../DEFS.m4)

ENTRY(times)
	moveq	#43,d0
	movl	sp@(4),a0	| fetch argument
	trap	#0
	jcs	1$
	rts

1$:	jmp	cerror
