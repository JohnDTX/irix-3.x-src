| C library -- ftruncate

| error = ftruncate(fd,length)

include(../DEFS.m4)

ENTRY(ftruncate)
	moveq	#32,d0
	movl	sp@(4),a0	| fd
	movl	sp@(8),d1	| length
	trap	#0
	jcs	1$
	moveq	#0,d0
	rts

1$:	jmp	cerror
