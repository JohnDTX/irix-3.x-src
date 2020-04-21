| C library -- getpgrp

include(../DEFS.m4)

ENTRY(getpgrp)
	moveq	#39,d0
	subl	a0,a0		| 0 argument
	trap	#0
	rts
