|C startup, transliteration of crt0.s from unix

	.text
	.globl	_exit,_environ,start,_main,_errno

start:	
	movl	sp@(4),a0	|ptr to arg0
	clrl	a0@(-4)		|make sure it has a 0 word preceding
	movl	sp,a0		|save ptr to nargs
	subql	#8,sp		|allocate space for two words
	movl	a0@,sp@		|copy down nargs
	addql	#4,a0		|bump ptr past nargs to arg0 ptr
	movl	a0,sp@(4)	|save this as second arg to main

1$:	tstl	a0@+		|look for 0 word marking end of args
	jne	1$		|not there yet
	movl	sp@(4),a1	|get ptr to ptr_to_arg0 ie argv
	cmpl	a1@,a0		|have we gone past *argv
	jlt	2$		|no, a0 must point to valid environ (blo?)
	subql	#4,a0		|else make it point to zero word
2$:	movl	a0,sp@(8)	|this becomes third argument to main
	movl	a0,_environ	|save it in global
	jsr	_main
	addql	#8,sp		|pop argc,argv
	movl	d0,sp@-		|value of main is argument to exit
	jsr	_exit
	movw	#1,d0		|exit trap
	trap	#0

	.bss
_environ:
	.space	4
_errno:
	.space	4
