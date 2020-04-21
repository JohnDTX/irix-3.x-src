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

VBUSERR	= 0x08
VADDERR	= 0x0c
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

				| set up error handlers
	movl	#xberr,d0	| bus error
	movl	d0,VBUSERR	|
	movl	#xaerr,d0	| address error
	movl	d0,VADDERR	|
	movl	#xierr,d0	| illegal instruction
	movl	d0,VILLINST	|

				| call c startup
	jbsr	__c_startup	|
				| on return,
|	jra	_exit		| fall through to exit
_exit:
_halt:
	trap 	#0xe		| back to the proms
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
