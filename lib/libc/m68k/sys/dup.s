| C library -- dup

|	f = dup(of [ ,nf])
|	f == -1 for error

include(../DEFS.m4)

ENTRY(dup)
	moveq	#41,d0
	movl	sp@(4),a0	| fetch argument
	movl	sp@(8),d1	| fetch argument
	trap	#0
	jcs	1$
	rts

1$:	jmp	cerror
