	.comm	_$a4_save,4
	.comm	_$a5_save,4

	.globl	_hasfpa,_setfpamask,_fpspl0,_fpspl7,_fpclear

_hasfpa:
	movl	#start,d0
	cmpl	#0x2000,d0
	blt	nofpa
	movl	#1,d0
	rts
nofpa:	clrl	d0
	rts
	.data
	.even
fpamask: .byte	0xff
	.even
forthandler:
	.long	0

	.text

_setfpamask:
|
|	single parameter has mask in low-order 8 bits.
|
	movb	0x8800:w,d0
	movb	#0xff,0x8800:w
	movl	sp@(4),d0
	movb	d0,fpamask
	rts
	
	.globl	_setfastmode

_setfastmode:
|
|	set the board into fast mode (rather than IEEE mode)
|
	movb	0x8600:w,d0
	bset	#0,d0
	movb	d0,0x8600:w
	rts
	
	.globl	_setieeemode
_setieeemode:
|
|	set the board into ieee mode (rather than FAST mode)
|
	movb	0x8600:w,d0
	bclr	#0,d0
	movb	d0,0x8600:w
	rts

_fpclear:
	clrl	d0
	movb	0x8800:w,d0
|
| 	we have to remove masked errors, so they are not reported.
|
	notb	d0
	clrl	d1
	movb	fpamask,d1
	notb	d1
	andb	d1,d0		
	notb	d0
|
	movb	#0xff,0x8800:w
	rts

_fpspl0:
	clrl	d0
	movb	0x8800:w,d0
|
| 	we have to remove masked errors, so they are not reported.
|
	notb	d0
	clrl	d1
	movb	fpamask,d1
	notb	d1
	andb	d1,d0		
	notb	d0
|
	movb	#0xff,0x8800:w
	movb	fpamask,0x8900:w
	rts

_fpspl7:
	movb	#0xff,0x8900:w
	rts

|
|	call_fort_user_handler is called with a pointer to the
|	fpe code when it is desired to call the user's handler.
|	The global registers are restored, and the fortran handler
|	is called.  As FORTRAN doesn't save d2,a2,a4 and a5, we
|	need to save and restore them.
|
	.globl	_call_fort_user_handler
_call_fort_user_handler:
	tstl	forthandler
	beq	out
	link	a6,#-16
	moveml	#0x3404,a6@(-16)
	movl	_$a4_save,a4
	movl	_$a5_save,a5
	movl	forthandler,a0
	movl	a6@(8),sp@-
	jsr	a0@
	addql	#4,sp
	moveml	#0x3404,a6@(-16)
	unlk	a6
out:
	rts

|
|	the following routines had to be added so that cycles did not
|	appear in the lorder'ing of libc.a....  There was a cycle since
|	fpsubs called fpsignal and fpsignal called fpsubs.
|
	.globl __save_fortran_registers,__set_forthandler,fpecleanup
__save_fortran_registers:
	movl	a4,_$a4_save
	movl	a5,_$a5_save
	rts

IGN_FORTHANDLERBIT = 11

__set_forthandler:
	movl	sp@(4),d0
	clrl	forthandler
	btst	#IGN_FORTHANDLERBIT,d0
	bne	noforthandler
	movl	#fpecleanup,forthandler
noforthandler:
	rts
