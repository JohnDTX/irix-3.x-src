| C library -- acct

| error = acct(string);

include(../DEFS.m4)

ENTRY(acct)
	moveq	#51,d0
	movl	sp@(4),a0	| fetch argument
	trap	#0
	jcs	1$
	rts

1$:	jmp	cerror
