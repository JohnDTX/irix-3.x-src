| C library -- stime

| error = stime(&long)

include(../DEFS.m4)

ENTRY(stime)
	movl	sp@(4),a0	| C version contains ptr to time
	movl	a0@,a0		| fetch argument
	moveq	#25,d0
	trap	#0
	jcs	1$		|if error
	rts

1$:	jmp	cerror
