| C library -- read

| nread = read(file, buffer, count);
| nread ==0 means eof; nread == -1 means error

include(../DEFS.m4)

ENTRY(read)
	moveq	#3,d0
	movl	sp@(4),a0	| fetch argument
	movl	sp@(8),d1	| fetch argument
	movl	sp@(12),a1	| fetch argument
	trap	#0
	jcs	1$
	rts

1$:	jmp	cerror	
