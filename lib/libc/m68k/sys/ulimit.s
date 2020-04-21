| C library -- ulimit

| value = ulimit(request, argument);
|	long ulimit(), argument;
|	int request;

include(../DEFS.m4)

ENTRY(ulimit)
	moveq	#63,d0
	movl	sp@(4),a0	| fetch argument
	movl	sp@(8),d1	| fetch argument
	trap	#0
	jcs	1$
	rts

1$:	jmp	cerror	
