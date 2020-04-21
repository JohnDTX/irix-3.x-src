| C library -- exect

| exect(file, argv, env);

| where argv is a vector argv[0] ... argv[x], 0
| last vector element must be 0

| The same as execve except that it sets the TBIT causing
| a trace trap on the first instruction of the executed process,
| to give a chance to set breakpoints.

include(../DEFS.m4)

ENTRY(exect)
	moveq	#59,d0		| basic exec call
	movl	sp@(4),a0	| fetch argument
	movl	sp@(8),d1	| fetch argument
	movl	sp@(12),a1	| fetch argument
	orw	#0x8000,sr	| set trace bit
	trap	#0
	jmp	cerror
