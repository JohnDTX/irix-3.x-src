| C library - pause

include(../DEFS.m4)

ENTRY(pause)
	moveq	#29,d0
	trap	#0
	jcs	1$
	moveq	#0,d0
	rts

1$:	jmp	cerror	
