| C library -- chroot

| error = chroot(string);

include(../DEFS.m4)

ENTRY(chroot)
	moveq	#61,d0
	movl	sp@(4),a0	| fetch argument
	trap	#0
	jcs	1$
	rts

1$:	jmp	cerror
