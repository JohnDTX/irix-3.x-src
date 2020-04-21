| unsigned multiply: long = usmul(long, short);
|
| GB - SGI. mc68020 version  5/19/85

include(../DEFS.m4)

ASENTRY(usmul)
|
|	sp@(4)	- first operand
|	sp@(8)	- second (short) operand
|
	movl	sp@(8),d0
	mulul	sp@(4),d0
	rts
