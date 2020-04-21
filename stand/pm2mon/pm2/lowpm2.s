|
|
|	lowpm2.s  -  	low-level interrupt routines for quirk proms
|

	.globl	_level6int
	.globl	_level4int
	.globl	_level2int
	.globl	_trapF

|
|	level6int is the level 6 interrupt handler.  It catches level
|	6 interrupts, saves the stack frame in one piece, and passes
|	a pointer to this saved stack frame to serialHandler (C).
|
|	serialHandler either returns, in which case the registers
|	are restorred and an rte done, or it calls RestoreStackandGO,
|	passing it the pointer to the saved stack frame. RestoreStackandGO
|	restores the stack pointer to the passed value and jumps to the
|	stack unwinding part of level6int.
|
	.globl	_level6int,_serialHandler
_level6int:
	movw	#0,sp@-		| stack one word for alignment
	moveml	#0xffff,sp@-	| save all registers
	jbsr	_serialHandler
	jra	offintospace

|
|	int 4 -- catch the level 4 interrupt and call squeek()
|
	.globl	_level4int,_squeek
_level4int:
	movw	#0,sp@-		| stack one word for alignment
	moveml	#0xffff,sp@-	| save all registers
	jbsr	_squeek
	jra	offintospace

|
|	level2int is the interrupt receivor for the ethernet 
|
	.globl	_level2int,_nxintr
_level2int:
	movw	#0,sp@-		| stack one word for alignment
	moveml	#0xffff,sp@-	| save all registers
	jbsr	_nxintr
	jra	offintospace

|
|	trapF is the user trap F receivor.
|
	.globl _trapF,_gomonitor
_trapF:
	movw	#0x2700,sr	| spl7
	movw	#0,sp@-		| stack one word for alignment
	moveml	#0xffff,sp@-	| save all registers
	movl	sp,d7		| save the stack
|||	movl	_bstart,sp	| switch to prom stack
	movl	d7,sp@-		| pass frame ptr
	jbsr	_gomonitor
	movl	d7,sp		| restore the stack
|	jra 	offintospace	| fall through

offintospace:
|
|	offintospace trusts that the sp points to a full register set
|	and interrupt stack frame as if level6int
|	had simply returned.
|
	moveml	sp@+,#0xffff	| restore all the registers
	tstw	sp@+		| unstack the alignment word
	rte			| and return from exception
