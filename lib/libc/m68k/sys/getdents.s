| C library -- getdents

include(../DEFS.m4)

ENTRY(getdents)
	moveq	#89,d0
	movl	sp@(4),a0	| fd
	movl	sp@(8),d1	| buf
	movl	sp@(12),a1	| sizeof(buf)
	trap	#0
	jcs	1$
	rts

1$:	jmp	cerror	
