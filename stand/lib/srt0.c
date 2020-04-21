/*
 * $Source: /d2/3.7/src/stand/lib/RCS/srt0.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:12 $
 */

#include	"cpureg.h"

	.globl	_exit
	.globl	__exit
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
	.globl	start

|
| Some misc defines
|
ACR_OFF		= 4		| offsets to various duart registers we
				| must program to get refresh going
CTUR_OFF	= 6
CTLR_OFF	= 7
OPCR_OFF	= 13
RFU_RATE	= 0		| the refresh rate upper byte => ctru
RFL_RATE	= 14		| refresh rate lower byte => crtl

|
| These 3 defines represent offsets into the common structure
|
COMM_SZ		= 0x160		| # of bytes in common structure
ENTRY_OFF	= 0x34		| offset into common area for entrypt
GOSTK_OFF	= 0x38		| offset into common area for go cmd stack

NFILES		= 4		| max open files, see stand.h

|
| srt0.c
|   This is the standalone runtime startoff.
|   This code makes many assumptions, the largest of which is that
|   when the proms load a pgm, it clears the bss and the page table
|   and stack are setup correctly.
|

	.text

#include	"evec.h"

ARGC_OFF	= 0x3c
ARGV_OFF	= 0x40
	
start:
#ifdef LATER
	movl	#evecstart,d0	| setup the vbr
	movec	d0,vbr
#endif

	movl	sp,savesp	| save the stack pointer

	jbsr	__init		| whatever????

	movl	#SRAM_BASE+ARGV_OFF,sp@-	| argv
|	movl	a0@,sp@-
	movl	#SRAM_BASE+ARGC_OFF,a0		| argc
	movl	a0@,sp@-
	jbsr	_main
	jbsr	_exit

__exit:
	movl	#NFILES-1,d5
	subql	#4,sp

eloop:
	movl	d5,sp@
	dbmi	d5,eloop

	movl	savesp,sp
	rts

|
| trap code section
|	This section of code contains the code necessary to handle
|	execptions.
Xtraps:
	movl	sp,sp@-			| arg1 - the stack pointer
					| arg2 - the frame is already there
	jsr	_trap			| pretty print the shit
	movl	savesp,sp
	rts

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

	.data

zeros:	.long	0,0,0,0,0,0,0,0,0,0,0,0,0,0	|14 long  of zeros
savesp:	.long	0
