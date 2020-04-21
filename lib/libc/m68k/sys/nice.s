| C library-- nice

| error = nice(hownice)

include(../DEFS.m4)

ENTRY(nice)
	moveq	#34,d0
	movl	sp@(4),a0	| fetch argument
	trap	#0
	jcs	1$
	rts

1$:	jmp	cerror
