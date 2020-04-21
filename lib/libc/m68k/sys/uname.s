| C library -- uname

| uname(unixname);
| unixname[0], ...unixname[7] contain the unixname

include(../DEFS.m4)

ENTRY(uname)
	moveq	#57,d0
	movl	sp@(4),a0	| fetch argument
	subl	a1,a1		| uname
	trap	#0
	jcs	1$
	moveq	#0,d0
	rts

1$:	jmp	cerror
