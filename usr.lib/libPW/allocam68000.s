| alloca (nbytes)  --  fixup stack pointer to that nbytes are is below it
|	and return the old stack pointer.  Effectively this allows local space
|	allocation on the stack which is returned when the caller returns

	.text
	.globl	_alloca
_alloca:
	movl	sp@,a1		| save return addr
	movl	sp@(4),d0	| load nbytes
	negl	d0		| setup to dec stack by that amt
	addl	sp,d0		| calculate new stack pointer
	addql	#1,d0		| allign to long boundary
	andb	#0xFC,d0	| knock out low bits
	movl	d0,sp		| set new stack pointer
	subl	#0x10,sp	| adjust for return
				| 4 each for: low bits just anded, return
				|  address, the parameter, and good measure
	tstb	sp@(-132)	| probe past end on new allocated mem
	movl	a1,sp@		| put ret addr on for rts
	rts
