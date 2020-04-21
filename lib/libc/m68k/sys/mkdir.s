| C library -- mkdir

| mkdir(dirname, mode);
|	char *dirname;
|	int mode;

include(../DEFS.m4)

ENTRY(mkdir)
	moveq	#88,d0
	movl	sp@(4),a0	| fetch argument
	movl	sp@(8),d1	| fetch argument
	trap	#0
	jcs	1$
	rts

1$:	jmp	cerror	
