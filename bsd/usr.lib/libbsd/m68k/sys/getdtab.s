| C library -- getdtablesize

include(../DEFS.m4)

ENTRY(getdtablesize)
	moveq	#103,d0
	trap	#0
	rts
