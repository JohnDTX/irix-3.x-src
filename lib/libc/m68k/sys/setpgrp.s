| C library -- setpgrp

include(../DEFS.m4)

ENTRY(setpgrp)
	moveq	#39,d0
	movw	#1,a0		| 1 argument
	trap	#0
	jcs	1$
	rts

1$:	jmp	cerror
