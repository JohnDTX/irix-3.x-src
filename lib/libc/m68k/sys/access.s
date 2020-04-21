| C library -- access

|  return = access(file, request)
|  test ability to access file in all indicated ways
|  1 - read
|  2 - write
|  4 - execute

include(../DEFS.m4)

ENTRY(access)
	moveq	#33,d0
	movl	sp@(4),a0	| fetch argument
	movl	sp@(8),d1	| fetch argument
	trap	#0
	jcs	1$
	rts

1$:	jmp	cerror
