|---------------------------------------------------------------------------
|
| SUBROUTINE FFLOAD - LOAD THE FFP MICROCODE
|
| CALL FFLOAD(UCODE) - "UCODE" IS MICROCODE BUFFER
|
| FOR UNIX SYSTEM 5 - SKYFFP IS STRAPPED TO 0x8000 ADDRESS
|
	.long	0x267c5dec	| "ffload" rad50
	.long	0
	.word	0
	.word	0
	.word	4

comreg = 0x80040
stcreg = 0x80042
mc1reg = 0x80048

	.globl	_ffload
_ffload:
	movl	d2,sp@-
	movl	sp@(8),a0	|point to microcode buffer
	movl	#4096,d0	|number of microcode words to load
	movl	#0x1000,d1	|starting ffp memory address
	movw	#0,stcreg	|halt the skyffp
loop:	
	movw	d1,comreg	|load memory address into comreg
	movl	a0@+,d2		|get a microcode word
	swap	d2			|swap the high/low words
	movl	d2,mc1reg	|write to skyffp memory
	addql	#1,d1		|bump up the skyffp memory address
	subql	#1,d0		|decrement the count
	bne		loop		|continue if > 0
	movl	sp@+,d2
	rts
