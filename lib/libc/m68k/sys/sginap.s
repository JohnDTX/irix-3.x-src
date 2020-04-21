| C library -- sginap

| error =  sginap(ticks);
| int error;
| long ticks;

include(../DEFS.m4)

ENTRY(sginap)
	moveq	#70,d0
	movl	sp@(4),a0	| fetch argument
	trap	#0
	jcs	1$
	rts

1$:	jmp	cerror
