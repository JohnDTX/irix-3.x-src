| phys(physn, laddr, size, paddr);

include(../DEFS.m4)

ENTRY(phys)
	moveq	#55,d0
	movl	sp@(4),a0	| fetch argument
	movl	sp@(8),d1	| fetch argument
	movl	sp@(12),a1	| fetch argument
	movl	d2,save
	movl	sp@(16),d2	| fetch argumemt
	trap	#0
	jcs	1$
	moveq	#0,d0
	movl	save,d2
	rts

1$:	movl	save,d2
	jmp	cerror

	.bss
save:	.space	4
