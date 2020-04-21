| C library -- geteuid

| uid = geteuid();
| returns effective uid

include(../DEFS.m4)

ENTRY(geteuid)
	moveq	#24,d0
	trap	#0
	movl	d1,d0
	rts
