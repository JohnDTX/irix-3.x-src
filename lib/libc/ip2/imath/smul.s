| multiply: long = smul(long, short);
|
| GB - SGI. mc68020 version  5/19/85

include(../DEFS.m4)

ASENTRY(smul)
|
|	sp@(4)	- first operand
|	sp@(8)	- second operand
|
	movl	sp@(4),d0
	mulsl	sp@(8),d0
	rts
