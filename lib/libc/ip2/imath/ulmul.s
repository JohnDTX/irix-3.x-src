|unsigned long multiply: c = a * b
|
| GB - SGI. mc68020 version  5/19/85

include(../DEFS.m4)

ASENTRY(ulmul)
	movl	sp@(4),d0
	movl	sp@(8),d1
	bra	goulmul

RASENTRY(rulmul)
goulmul:
	mulul	d1,d0		|result in d0 where it belongs
	rts
