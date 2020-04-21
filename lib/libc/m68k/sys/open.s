| C library -- open

| file = open(string, rw_mode, [access_mode])
| file == -1 means error

include(../DEFS.m4)

ENTRY(open)
	moveq	#5,d0
	movl	sp@(4),a0	| string
	movl	sp@(8),d1	| rw_mode
	movl	sp@(12),a1	| [access_mode]
	trap	#0
	jcs	1$
	rts

1$:	jmp	cerror	
