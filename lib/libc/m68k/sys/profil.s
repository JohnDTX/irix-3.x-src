| C library --  profil

include(../DEFS.m4)

ENTRY(profil)
	moveq	#44,d0
	movl	sp@(4),a0	| fetch argument
	movl	sp@(8),d1	| fetch argument
	movl	sp@(12),a1	| fetch argument
	movl	d2,save		| save d2 register
	movl	sp@(16),d2	| fetch argument
	trap	#0
	movl	save,d2		| restore d2 register
	rts

	.bss
save:	.space	4
