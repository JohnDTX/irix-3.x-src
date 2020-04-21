| C library -- ptrace

| result = ptrace(req, pid, addr, data);

include(../DEFS.m4)

ENTRY(ptrace)
	moveq	#26,d0
	movl	d2,save		| save d2 register
	clrl	_errno		| clear errno
	movl	sp@(4),a0	| fetch argument
	movl	sp@(8),d1	| fetch argument
	movl	sp@(12),a1	| fetch argument
	movl	sp@(16),d2	| fetch argument
	trap	#0
	jcs	1$
	movl	save,d2		| restore d2 register
	rts

1$:
	movl	save,d2		| restore d2 register
	jmp	cerror

	.bss
save:	.space	4
