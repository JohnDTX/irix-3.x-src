	.comm	__dbargs,512
	.text
	.globl	__dbsubc
__dbsubc:
	movl	__dbargs+4,sp@-
	movl	__dbargs,a0
	jbsr	a0@

	.globl	__dbsubn
__dbsubn:
	stop	#0
