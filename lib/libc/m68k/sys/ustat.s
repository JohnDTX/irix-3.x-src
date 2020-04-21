| C library -- ustat - get superblock info

| ustat(dev, info)
| char *info;

include(../DEFS.m4)

ENTRY(ustat)
	moveq	#57,d0
	movl	sp@(8),a0	| fetch argument
	movl	sp@(4),d1	| fetch argument
	movw	#2,a1		| ustat
	trap	#0
	jcs	1$
	moveq	#0,d0
	rts

1$:	jmp	cerror
