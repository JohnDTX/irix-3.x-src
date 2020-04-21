| C library -- close

| error =  close(file);

include(../DEFS.m4)

ENTRY(close)
	moveq	#6,d0
	movl	sp@(4),a0	| fetch argument
	trap	#0
	jcs	1$
	moveq	#0,d0
	rts

1$:	jmp	cerror
