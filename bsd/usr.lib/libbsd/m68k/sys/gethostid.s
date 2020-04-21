| C library -- gethostid

include(../DEFS.m4)

ENTRY(gethostid)
	moveq	#104,d0
	trap	#0
	rts
