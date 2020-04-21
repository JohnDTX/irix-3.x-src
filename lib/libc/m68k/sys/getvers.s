| C library -- open

| success = getversion(type, buf)
| success == -1 means error

include(../DEFS.m4)

ENTRY(getversion)
	moveq	#66,d0
	movl	sp@(4),a0	| string
	movl	sp@(8),d1	| rw_mode
	trap	#0
	jcs	1$
	moveq	#0, d0
	rts

1$:	jmp	cerror	
