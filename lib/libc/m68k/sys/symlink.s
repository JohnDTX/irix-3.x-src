| C library -- symlink

| status = symlink(text,tgtname)

include(../DEFS.m4)

ENTRY(symlink)
	moveq	#80,d0
	movl	sp@(4),a0
	movl	sp@(8),d1
	trap	#0
	jcs	1$
	moveq	#0, d0
	rts

1$:	jmp	cerror	
