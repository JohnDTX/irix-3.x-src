/*
* $Source: /d2/3.7/src/stand/mon/RCS/locore_mem.c,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:15:40 $
*/

#include	"cpureg.h"
#include	"sr.h"

	.globl	__reset
	.globl	_bzero
	.globl	_getprsr
	.globl	_getprvbr
	.globl	_getprcacr
	.globl	_getprcaar
	.globl	_getprsfc
	.globl	_getprdfc
	.globl	_setprsr
	.globl	_setprvbr
	.globl	_setprcacr
	.globl	_setprcaar
	.globl	_setprsfc
	.globl	_setprdfc
	.globl	_Inprom
	.globl	start

|
| Some misc defines
|
SR_SIPM7	= SR_SU+SR_IPM7	| initial status register (cpu) mode
SR_SIPM0	= SR_SU+SR_IPM0	| allow cpu interrupts

ACR_OFF		= 4		| offsets to various duart registers we
				| must program to get refresh going
CTUR_OFF	= 6
CTLR_OFF	= 7
OPCR_OFF	= 13
RFU_RATE	= 0		| the refresh rate upper byte => ctru
RFL_RATE	= 0x0e		| refresh rate lower byte => crtl
TMU_RATE	= 0x0		| the timeout rate upper byte => ctru
TML_RATE	= 0x0e		| the timeout rate upper byte => ctru

|
| These 3 defines represent offsets into the common structure
|
#include	"offset.h"

|
| locore_mem.c
|   This file is composed of 4 parts:
|	start	startup code when the prom is just another bootable program.
|	Xtrap	execption handling code
|	misc	misc assembly code
|
| start
|	This section contains the startup code needed to get processor
|	in a rational state before the PROM code in C can be invoked.
|	Virtual addresses for the PROMs are as follows:
|		30000000		PROM text
|		contiguous after text	PROM data
|		02000000		Multibus accessable memory (1mb)
|		02100000		PROM bss

|	Physical memory is organized as follows:
|		The last megabyte of physical memory is used for the
|		multibus accessable memory.
|		The bss is allocated starting at the multibus memory going
|		backwards.
|
|	At 33000000 there exists a 2kb chunk of memory. We place our
|	global data there as well as the stack.

|	In the interests of being nice, we try to allow the proms to
|	run (in a limited sense) if no memory is present by using this
|	2kb chunk of memory.

|
| The execption vectors.  They reside at location 30000000 in the PROMS.
| When SYSSEG bit is asserted in the STATUS register it is also maps
| PROM addresses to virtual address 0.
	.text

	.globl	evecstart
#include	"evec.h"
	
start:
	movl	#0,_Inprom		| set flag

restart:
	clrl	d1		| holds initialization level

	movl	#evecstart,d0	| setup the vbr
	movec	d0,vbr

	movl	sp,savesp	| save the stack pointer
|
| Start the timeout
|
| set acr: baud rate set 2, crystal or external clock (X1/CLK), and
|          lots of evil and nasty stuff about what ip3-ip0 should do.
| set counter timer upper register 
| set counter timer lower register
| set opcr: counter timer output, TxRDYB, TxRDYA, RxRDY/FFULLB, RxRDY/FFULLA
|
	movb	#0xEB,DUART1_BASE+ACR_OFF
	movb	#TMU_RATE,DUART1_BASE+CTUR_OFF
	movb	#TML_RATE,DUART1_BASE+CTLR_OFF
	movb	#0xF4,DUART1_BASE+OPCR_OFF

	movl	d1,sp@-		| initialization level
	jbsr	_init
	addql	#4,sp		| remove it from the stack

__reset:
	movl	savesp,sp	| reset the stack. should be a
				| movc XXX,ips
	jbsr	_main

|
| return back here if program was loaded or the go command was issued.
| If go command issued then there will be a stack value in the c_gostk
| field and we should use that for the stack
|
	tstl	d0
	beq	exit

	movl	SRAM_BASE+ENTRY_OFF,a1	| get the entry point
	movl	SRAM_BASE+GOSTK_OFF,d0	| get go cmd stack value

	tstl	d0			| if stack value is 0, don't touch stk
	beq	skpstk

	movl	d0,sp			| must be go cmd - set the stack

skpstk:
	tstw	M_QUAD			| clear possible mousey intrs
	movl	#start,d0		| don't forget the reboot address

	jsr	a1@		| begin execution somewhere else

| reset the status register - just in case

exit:
	movl	evecstart,d0	| setup the vbr
	movec	d0,vbr

	bra	start		| ha-ha we don't die!!!!

|
| trap code section
|	This section of code contains the code necessary to handle
|	execptions.
Xtraps:
	movl	sp,sp@-			| arg1 - the stack pointer
					| arg2 - the frame is already there
	jsr	_trap			| pretty print the shit
	bra	restart			| start over

_getprsr:
	movw	sr,d0
	rts

_getprvbr:
	movec	vbr,d0
	rts

_getprcacr:
	movec	cacr,d0
	rts

_getprcaar:
	movec	caar,d0
	rts

_getprsfc:
	movec	sfc,d0
	rts

_getprdfc:
	movec	dfc,d0
	rts

_setprsr:
	movl	sp@(0x4),d0
	rts

_setprvbr:
	movl	sp@(0x4),d0
	movec	d0,vbr
	rts

_setprcacr:
	movl	sp@(0x4),d0
	movec	d0,cacr
	rts

_setprcaar:
	movl	sp@(0x4),d0
	movec	d0,caar
	rts

_setprsfc:
	movl	sp@(0x4),d0
	movec	d0,sfc
	rts

_setprdfc:
	movl	sp@(0x4),d0
	movec	d0,dfc
	rts

_bzero:
	movl	sp@(4),d1	|starting address
	movl	sp@(8),d0	|count
	beq	7$		|dumb enuf to call with 0 count
	addl	d0,d1		|calc ending address
	movl	d1,a0		|save it
	andl	#1,d1		|word aligned? Who cares on 68020????
	jeq	1$		|yes, potentially long moves
	clrb	a0@-		|clear up to word boundry
	subql	#1,d0		|one less byte to clear
	jeq	7$		|nothing left

1$:
	movl	d0,d1		|copy n
	andl	#0xffffff00,d1	|m = number of 256 byte blocks left * 256
	jeq	3$		|none

	subl	d1,d0		|we will do this many bytes in next loop
	asrl	#8,d1		|number of blocks left
	moveml	#0xFF7E,sp@-	|save registers
	movl	d1,sp@-		|number of blocks goes on top of stack
	movl	#zeros,a1
	moveml	a1@,#0x7CFF	|clear out a bunch of registers
	movl	d0,a1		|and this one too

2$:
	moveml	#0xFF7E,a0@-	|clear out 14 longs worth
	moveml	#0xFF7E,a0@-	|clear out 14 longs worth
	moveml	#0xFF7E,a0@-	|clear out 14 longs worth
	moveml	#0xFF7E,a0@-	|clear out 14 longs worth
	moveml	#0xFF00,a0@-	|clear out 8 longs worth, total of 256 bytes
	subql	#1,sp@		|one more block, any left?
	jgt	2$		|yes, do another pass

	movl	sp@+,d1		|just pop stack
	moveml	sp@+,#0x7EFF	|give me back the registers

3$:
	movl	d0,d1		|copy n left
	andl	#0xfffffffc,d1	|this many longs left
	jeq	5$		|none
	subl	d1,d0		|do this many in next loop

4$:
	clrl	a0@-		|clear a long's worth
	subql	#4,d1		|this many bytes in a long
	jgt	4$		|if there are more

5$:
	tstl	d0		|anything left?
	jeq	7$		|no, just stop here

6$:
	clrb	a0@-		|clear 1 byte's worth
	subql	#1,d0		|one less byte to do
	jgt	6$		|if any more

7$:
	rts			|that's it

	.globl	_spl7, _spl6, _spl5, _spl4, _spl3, _spl2, _spl1, _spl0, _splx
_spl7:
_spl6:
_spl5:
_spl4:
_spl3:
_spl2:
_spl1:
_spl0:
_splx:
	rts

	.data

zeros:	.long	0,0,0,0,0,0,0,0,0,0,0,0,0,0	|14 long  of zeros
savesp:	.long	0
