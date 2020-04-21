| getppid -- get parent process ID

include(../DEFS.m4)

ENTRY(getppid)
	moveq	#20,d0
	trap	#0
	movl	d1,d0
	rts
