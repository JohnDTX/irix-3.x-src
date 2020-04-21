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
|
|	THIS SPECIAL VERSION OF SETJMP/LONGJMP WAS ALTERED FOR
|	SPECIAL USE WITH THE PM2 STARTUP.  IT CLEARS THE FIRST WORD
|	OF THE JUMP BUFFER AFTER A LONGJMP HAS BEEN DONE.  
|
|	(this re-invokes the startup's default handling of bus errors)
|	GB 12/2/83
|
	.globl _setjmp, _longjmp
	.text

_setjmp:
	movl	sp@(4.),a0	|pointer to jmp_buf
	movl	sp@,a0@		|pc
	moveml	#0xFCFC,a0@(4.)	|d2-d7, a2-a7
	clrl	d0		|return 0
	rts

_longjmp:
	movl	sp@(4.),a0	|pointer to jmp_buf
	movl	sp@(8.),d0	|value returned
	moveml	a0@(4.),#0xFCFC	|restore d2-d7, a2-a7
	movl	a0@,sp@		|restore pc of call to setjmp to stack
	movl	#0,a0@		| and clear the jump buffer pc (GB)
	rts
