| C library -- async_daemon

| (void) async_daemon();	/* never returns */

include(../DEFS.m4)

ENTRY(async_daemon)
	moveq	#97,d0
	trap	#0
	rts
