| C library -- wait

| pid = wait(0);
|   or,
| pid = wait(&status);

| pid == -1 if error
| status indicates fate of process, if given

include(../DEFS.m4)

ENTRY(wait)
	moveq	#7,d0
	trap	#0
	jcs	2$
	tstl	sp@(4)		| wait(0)?
	beq	1$		| yes, return
	movl	sp@(4),a0	| a0 = &status
	movl	d1,a0@		| status = d1
1$:	rts

2$:	jmp	cerror
