| C library -- fork

| pid = fork();

| pid == 0 in child process; pid == -1 means error return
| in child, parents id is in par_uid if needed

include(../DEFS.m4)

ENTRY(fork)
	moveq	#2,d0
	trap	#0
	bras	1$		| child  return
	jcc	2$		| parent return
	jmp	cerror
1$:	moveq	#0,d0
2$:	rts
