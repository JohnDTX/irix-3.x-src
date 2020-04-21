|signed long multiply: c = a * b
|
| GB - SGI. mc68020 version  5/19/85

include(../DEFS.m4)

ASENTRY(lmul)
	movl	sp@(4),d0
	movl	sp@(8),d1
	bra	golmul

RASENTRY(rlmul)
golmul:
	mulsl	d1,d0		|result in d0 where it belongs
	rts
