| C library --  getpid 

| get process ID

include(../DEFS.m4)

ENTRY(getpid)
	moveq	#20,d0
	trap	#0
	rts
