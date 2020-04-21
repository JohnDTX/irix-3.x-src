|
|	general startup routine for pm2.1 standalone programs
|
|	performs the following functions:
|	   leaves most trap and interrupt vectors alone!
|	   sets stack to begin at start (growing downwards).
| 	   clears bss (from _bstart to _end).
|	   calls _c_startup
|	   does trap #0xE (standalone exit) on return.
|	

COMMONSTART	= 0x200
COMMONEND	= 0x300
COMMONLEN	= 0x100
MOUSE		= 0xFCC000

VBUSERR		= 0x08
VADDERR		= 0x0C
VILLINST	= 0x10

|
|	IMPORTS:
	.globl	__c_startup
	.globl	_edata
	.globl	_end

|
|	EXPORTS:
	.globl	start
	.globl	_halt
	.globl	_exit
	.globl	_common_area
_common_area = COMMONSTART

start:
	jra	start2

	.asciz	"WE ARE HAVING FUN"
	.even

start2:
				| set up stack
	movl	#start,sp	| just below start
	movl	#0,sp@-		| slop word
	movl	sp,a6		| frame pointer too

				| clear bss
	movl	#_bstart,a0	| from bstart
	clrl	d0		| zero = 0
clr:				| 
	movl	d0,a0@+		| *ip++ = zero
	cmpl	#_end,a0	| to end
	jlt	clr		|

| Removed to use stuff from KIPP
|				| set up error handlers
|	movl	#xberr,d0	| bus error
|	movl	d0,VBUSERR	|
|	movl	#xaerr,d0	| address error
|	movl	d0,VADDERR	|
|	movl	#xierr,d0	| illegal instruction
|	movl	d0,VILLINST	|
|
|
| Build up interrupt dispatch table
|	- note that each vector in the interrupt vector page contains
|	  a pointer into the interrupt vector page.  The code for
|	  processing interrupts is doubly mapped at the base of the
|	  kernel (KERN_VBASE) and in the interrupt vector page (IVEC_VBASE).
|	  Note that the code for processing interrupts MUST live in the
|	  first page of the kernel.
|	- the code that runs immediately after an interrupt must run
|	  pc relative, without any kernel data structures, until it
|		(1) switches into the kernel context (KCX), and
|		(2) jumps into the ``normal'' virtual location for the
|		    code
|	  For example: lfault is located somewhere in the first page of
|	  the kernel.  When an interrupt happens, the vector(s) for lfault
|	  cause the processor to run at &lfault - KERN_VBASE + IVEC_VBASE.
|	  The lfault code sets the kernel context, then uses a ``jmp'' to
|	  force the pc to a normal kernel location (KERN_VBASE + offset).
|
| Setup all vectors to point to lfault, except for the mouse, which we
| point to to level7
|
initvectors:
	moveq	#64-1, d0		| # of vectors to setup
	moveq	#0, d1			| vector number in progress
	movl	#0, a0			| base of interrupt vectors
1$:
	cmpb	#46, d1			| Trap 14 saved for prom restart
	jeq	3$
	cmpb	#31, d1			| mouse interrupt vector?
	jne	2$			|   nope
	movl	#ignoremouse, a0@	| point to mouse interrupt
	jra	3$
2$:	movl	#xfault, a0@		| point to fault vector
3$:	addql	#1, d1
	addql	#4, a0			| Increment counter
	dbra	d0, 1$			| loop through all vectors

	movl	#xberr,0x8		| Bus Error
	movl	#xaerr,0xc		| Address Error
	movl	#xierr,0x10		| Instruction Error
|
|	set up the interrupts for the system
|	for a special test -- Chase --
|
	movl	#dkintr,0x64	| Interrupt Level 1
|	movl	#lvtwo,0x68	| Interrupt Level 2
|	movl	#lvthree,0x6c	| Interrupt Level 3
|	movl	#lvfour,0x70	| Interrupt Level 4
|	movl	#lvfive,0x74	| Interrupt Level 5
	movl	#ckintr,0x78	| Interrupt Level 6
|
|
				| call c startup
	jbsr	__c_startup	|
				| on return,
|	jra	_exit		| fall through to exit
_exit:
_halt:
	trap 	#0xE		| back to the proms
	stop	#0x2700		| paranoia
	jra	_halt		| double paranoia


|
| ERROR HANDLERS
	.globl	_longjmp

|
| bus error handler
				| extern jmp_buf bejmp;
				| extern berr();
	.globl	_bejmp,_berr
xberr:	
	tstl	_bejmp
	jeq	xb1

	movl	#1,sp@-		| longjmp(bejmp,1)
	movl	#_bejmp,sp@-	|
	jbsr	_longjmp	|
				| fall through to standalone handler
xb1:
	movw	d0,sp@-		| Stack one word for alignment
	jbsr	_berr
	jra	_halt

|
| address error handler
				| extern jmp_buf aejmp;
				| extern aerr();
	.globl	_aejmp,_aerr
xaerr:
	tstl	_aejmp
	jeq	xa1

	movl	#1,sp@-		| longjmp(aejmp,1)
	movl	#_aejmp,sp@-	|
	jbsr	_longjmp	|
				| fall through to standalone handler
xa1:
	movw	d0,sp@-
	jbsr	_aerr
	jra	_halt

|
| illegal instruction handler
				| extern jmp_buf iejmp;
				| extern ierr();
	.globl	_iejmp,_ierr
xierr:	
	tstl	_iejmp
	jeq	xi1

	movl	#1,sp@-		| longjmp(iejmp,1)
	movl	#_iejmp,sp@-	|
	jbsr	_longjmp	|
				| fall through to standalone handler
xi1:	
	jbsr	_ierr
	jra	_halt
|
| Special interrupt routines
|
	.globl	_diskintr, _clkintr

| Disk interrupt
dkintr:
	moveml	#0xFFFE, sp@-
	jbsr	_diskintr
	moveml	sp@+, #0x7FFF
	rte

| Clock interrupt
ckintr:
	moveml	#0xFFFE, sp@-
	jbsr	_clkintr
	moveml	sp@+, #0x7FFF
	rte

ignoremouse:
	tstw	MOUSE		| Read mouse bits to clear interrupt
	rte
xfault:
	jbsr	_cfault
	jra	_halt
|
