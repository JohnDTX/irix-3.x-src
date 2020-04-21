| C library -- rmdir

| rmdir(dirname);
|	char *dirname;

include(../DEFS.m4)

ENTRY(rmdir)
	moveq	#87,d0
	movl	sp@(4),a0	| fetch argument
	trap	#0
	jcs	1$
	rts

1$:	jmp	cerror	
