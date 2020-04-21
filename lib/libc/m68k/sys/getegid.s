| C library -- getegid

| gid = getegid();
| returns effective gid

include(../DEFS.m4)

ENTRY(getegid)
	moveq	#47,d0
	trap	#0
	movl	d1,d0
	rts
