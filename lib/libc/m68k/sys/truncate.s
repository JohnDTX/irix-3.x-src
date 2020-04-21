| C library -- truncate

| error = truncate(path,length)

include(../DEFS.m4)

ENTRY(truncate)
	moveq	#31,d0
	movl	sp@(4),a0	| path
	movl	sp@(8),d1	| length
	trap	#0
	jcs	1$
	moveq	#0,d0
	rts

1$:	jmp	cerror
