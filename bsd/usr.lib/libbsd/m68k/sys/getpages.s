| C library -- getpagesize

| error =  getpagesize();

include(../DEFS.m4)

ENTRY(getpagesize)
	moveq	#105,d0
	trap	#0
	rts
