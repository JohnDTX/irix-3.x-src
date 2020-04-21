| C library -- getgid

| gid = getgid();

include(../DEFS.m4)

ENTRY(getgid)
	moveq	#47,d0
	trap	#0
	rts
