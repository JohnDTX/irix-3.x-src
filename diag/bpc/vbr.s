	.text
	.globl	getvbr
getvbr:
	.word 0x4E7A
	.word 0x0801
	rts

	.text
	.globl	echo
echo:
	movl sp@(4),d0
	rts

	.text
	.globl	setvbr
setvbr:
	movl sp@(4),d0
	.word 0x4E7B
	.word 0x0801
	rts
