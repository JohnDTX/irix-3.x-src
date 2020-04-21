| C library -- syscall

|  Interpret a given system call

include(../DEFS.m4)

ENTRY(syscall)
	movl	sp@(4),d0	| system call number
	movl	sp@(8),a0	| fetch argument 1
	movl	sp@(12),d1	| fetch argument 2
	movl	sp@(16),a1	| fetch argument 3
	moveml	#0x040C,save	| save registers
	movl	sp@(20),d2	| fetch argument 4
	movl	sp@(24),a2	| fetch argument 5
	movl	sp@(28),d3	| fetch argument 6
	trap	#0
	moveml	save,#0x040C	| restore registers
	jcs	1$
	rts

1$:	jmp	cerror	

	.bss
save:	.space	12
