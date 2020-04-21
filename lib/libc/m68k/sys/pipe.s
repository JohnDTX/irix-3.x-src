| C library -- pipe

|	pipe(f)
|	int f[2];

include(../DEFS.m4)

ENTRY(pipe)
	moveq	#42,d0
	trap	#0
	jcs	1$
 	movl	sp@(4),a0	| a0 = &fildes[1]
	movl	d0,a0@+		| fildes[0] = d0
	movl	d1,a0@		| fildes[1] = d1
	moveq	#0,d0
	rts

1$:	jmp	cerror
