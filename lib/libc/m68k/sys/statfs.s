| C library -- statfs

| statfs(name, stbuf, buflen, fstyp);
|	char *name;
|	int buflen, fstyp;
|	struct statfs *stbuf;

include(../DEFS.m4)

ENTRY(statfs)
	moveq	#90,d0
	movl	d2,save		| save d2 register
	movl	sp@(4),a0	| fetch argument
	movl	sp@(8),d1	| fetch argument
	movl	sp@(12),a1	| fetch argument
	movl	sp@(16),d2	| fetch argument
	trap	#0
	jcs	1$
	movl	save,d2		| restore d2 register
	rts

1$:
	movl	save,d2		| restore d2 register
	jmp	cerror

	.data
save:	.long	0
