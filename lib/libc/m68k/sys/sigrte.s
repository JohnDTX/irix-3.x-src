| C library -- part of signal

	.text
	.globl	_sighnd, _sigfunc

_sighnd:			| 32 signal handler entries follow
	bsrw	sigrte		| 0
	bsrw	sigrte
	bsrw	sigrte
	bsrw	sigrte
	bsrw	sigrte
	bsrw	sigrte		| 5
	bsrw	sigrte
	bsrw	sigrte
	bsrw	sigrte
	bsrw	sigrte
	bsrw	sigrte		| 10
	bsrw	sigrte
	bsrw	sigrte
	bsrw	sigrte
	bsrw	sigrte
	bsrw	sigrte		| 15
	bsrw	sigrte
	bsrw	sigrte
	bsrw	sigrte
	bsrw	sigrte
	bsrw	sigrte		| 20
	bsrw	sigrte
	bsrw	sigrte
	bsrw	sigrte
	bsrw	sigrte
	bsrw	sigrte		| 25
	bsrw	sigrte
	bsrw	sigrte
	bsrw	sigrte
	bsrw	sigrte
	bsrw	sigrte		| 30
	bsrw	sigrte		| 31

sigrte:
	moveml	#0xFFFE,sp@-		|save all regs
	movl	sp@(60),d0		|pc after jsr
	subl	#_sighnd+4,d0		| signal number * 4
	movl	d0,a0
	asrl	#2,d0			| signal number
	movl	d0,sp@-			|argument to function
	addl	#_sigfunc,a0		|tbl of signal functions
	movl	a0@,a0			|fetch pointer to function
	jsr	a0@			|in C terms: (*_sigfunc[sig])(sig)
	addql	#4,sp			|pop argument
	moveml	sp@+,#0x7FFF		|restore regs
	addql	#4,sp			|clean up pc from jsr in handler
	movw	sp@+,cc			|restore status register
	rts				|return
