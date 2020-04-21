| C library -- sync()

include(../DEFS.m4)

ENTRY(sync)
	moveq	#36,d0
	trap	#0
	rts
