| C library -- getuid

| uid = getuid();

include(../DEFS.m4)

ENTRY(getuid)
	moveq	#24,d0
	trap	#0
	rts
