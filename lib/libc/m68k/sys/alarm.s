| C library - alarm

include(../DEFS.m4)

ENTRY(alarm)
	moveq	#27,d0
	movl	sp@(4),a0	| fetch argument
	trap	#0
	rts
