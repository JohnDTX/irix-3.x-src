|	Kurt Akeley
|	18 June 1985
|	Stuff to handle bus errors on the IP2 68020 board
|
	.data
	.comm	beva,4		| points to the buserror vector location
	.comm	savebev,4	| saves old value of buserror vector
	.comm	gotbe,4		| boolean to remember bus error
	.comm	temp,4		| temporary storage

	.text
	.globl setbeva
|-----------------------------------------------------------------------------
|
|	sets the local pointer to the buserror vector to the long argument
|
setbeva:
	movl	sp@(4),beva
	rts



	.globl catchberr
|-----------------------------------------------------------------------------
|
|	when called, saves the current buserror vector in savebev,
|	then loads the local buserror handler address into the
|	buserror vector location.  clears gotbe.
|	assumes that beva points to the buserror vector.
|
catchberr:
	movl	a0,temp
	movl	beva,a0
	movl	a0@,savebev
	movl	#handleberr,a0@
	movl	#0,gotbe			| clear gotbe
	movl	temp,a0
	rts



	.globl gotberr
|-----------------------------------------------------------------------------
|
|	restores the saved buserror vector (in savebev) to the buserror
|	vector location.  returns true if a bus error was
|	processed since catchberr was called, false otherwise.
|
gotberr:
	movl	a0,temp
	movl	beva,a0
	movl	savebev,a0@
	movl	temp,a0
	movl	gotbe,d0		| return gotbe
	rts



	.globl	handleberr
|-----------------------------------------------------------------------------
|
|	Clear the data rerun bit and return
|	Set gotbe
|
handleberr:
	andw	#0xFEFF,sp@(10)		| clear data rerun bit
	movl	#1,gotbe		| set gotbe
	rte



	.globl	getvbr
|-----------------------------------------------------------------------------
|
|	returns the vector base register contents
|
getvbr:
	.word 0x4E7A
	.word 0x0801
	rts



	.globl	setvbr
|-----------------------------------------------------------------------------
|
|	sets the vector base register to its long argument
|
setvbr:
	movl sp@(4),d0
	.word 0x4E7B
	.word 0x0801
	rts
