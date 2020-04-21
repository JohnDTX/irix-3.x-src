| C library -- execve

| execve(file, argv, env);

| where argv is a vector argv[0] ... argv[x], 0
| last vector element must be 0

include(../DEFS.m4)

ENTRY(execve)
	moveq	#59,d0		| basic exec call
	movl	sp@(4),a0	| fetch argument
	movl	sp@(8),d1	| fetch argument
	movl	sp@(12),a1	| fetch argument
	trap	#0
	jmp	cerror
