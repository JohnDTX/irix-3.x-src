|
|	GB 4/17/86.  This routine is necessary because the %%$#! asm68k
|	is too stupid to know that symbols can be > 8 characters.
|
|	%_FTRUNC is called from %_TRUNC in uio.asm
|
	.globl %_FTRUNC,_ftruncate
	.text
%_FTRUNC:
	jmp _ftruncate
