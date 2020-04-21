|setjmp, longjmp
|
|	longjmp(a, v)
|causes a "return(v)" from the
|last call to
|
|	setjmp(v)
|by restoring all the registers and
|adjusting the stack
|
|jmp_buf is set up as:
|
|	_________________
|	|	pc	|
|	-----------------
|	|	d2	|
|	-----------------
|	|	...	|
|	-----------------
|	|	d7	|
|	-----------------
|	|	a2	|
|	-----------------
|	|	...	|
|	-----------------
|	|	a7	|
|	-----------------

include(../DEFS.m4)

ENTRY(setjmp)
	movl	sp@(4),a0	|pointer to jmp_buf
	movl	sp@,a0@		|pc
	moveml	#0xFCFC,a0@(4)	|d2-d7, a2-a7
	moveq	#0,d0		|return 0
	rts

ENTRY(longjmp)
	movl	sp@(4),a0	|pointer to jmp_buf
	movl	sp@(8),d0	|value returned
	bne	1$		| force d0 to be non zero
	moveq	#1,d0		| force d0 to be non zero
1$:	moveml	a0@(4),#0xFCFC	|restore d2-d7, a2-a7
	movl	a0@,sp@		|restore pc of call to setjmp to stack
	rts
