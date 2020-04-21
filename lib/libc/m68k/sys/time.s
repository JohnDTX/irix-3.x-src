| C library -- time

| tvec = time(tvec);

| tvec[0], tvec[1] contain the time

include(../DEFS.m4)

ENTRY(time)
	moveq	#13,d0
	trap	#0
	tstl	sp@(4)		|time(0)?
	jeq	1$		|yes, return
	movl	sp@(4),a0	|a0 = &tloc
	movl	d0,a0@		|tloc = d0
1$:	rts
