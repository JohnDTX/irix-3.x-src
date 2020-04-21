|addressed unsigned long multiply: *a = *a * b
|
| GB - SGI. mc68020 version  5/19/85

include(../DEFS.m4)

ASENTRY(aulmul)
|
|	addressed long multiplication.  
|	
|	sp@(4) - address of first operand
|	sp@(8) - second operand
|
	movl	sp@(4),a0
	movl	sp@(8),d0
	bra		goaulmul

RASENTRY(raulmul)
|
|	a0 - address of first operand
|	d0 - second operand
|
goaulmul:
	mulul	a0@,d0
	movl	d0,a0@
	rts
