| C library -- BSD_getime

| r = BSD_getime(time)
| struct timeval *time;

include(../DEFS.m4)

ENTRY(BSD_getime)
	moveq	#124,d0
	movl	sp@(4),a0	| fetch argument
	trap	#0
	jcs	1$
	moveq	#0, d0
	rts

1$:	jmp	cerror
