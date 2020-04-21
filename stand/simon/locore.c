/*
* $Source: /d2/3.7/src/stand/simon/RCS/locore.c,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:20:51 $
*/

#include	"cpureg.h"
#include	"sr.h"

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
TMU_RATE	= 0x38		| the timeout rate upper byte => ctru
TML_RATE	= 0		| timeout rate lower byte => crtl

|
| These 3 defines represent offsets into the common structure
|
#include	"offset.h"

|
| locore.c
|   This file is composed of 4 parts:
|	start	startup code
|	Xtrap	execption handling code
|       confid	confidence testing code
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

#include	"evec.h"
	
start:
	clrl	d1		| holds initialization level

restart:
	movw	#SR_SIPM7,sr	| set the 68020 status register: Supervisor
				| mode/interrupt priority level 7

	movl	#PROM_BASE,d0	| setup the vbr
	movec	d0,vbr

	clrl	d2		| we use enuf 0, that 0 in a register helps
	movec	d2,cacr		| turn off the cache

|
| Start the refresh
|
| set acr: baud rate set 2, crystal or external clock (X1/CLK), and
|          lots of evil and nasty stuff about what ip3-ip0 should do.
| set counter timer upper register 
| set counter timer lower register
| set opcr: counter timer output, TxRDYB, TxRDYA, RxRDY/FFULLB, RxRDY/FFULLA
|
	movb	#0xEB,DUART0_BASE+ACR_OFF
	movb	#RFU_RATE,DUART0_BASE+CTUR_OFF
	movb	#RFL_RATE,DUART0_BASE+CTLR_OFF
	movb	#0xF4,DUART0_BASE+OPCR_OFF

| For revision A boards we must startup the clock for the timeout circuit
| Revision B boards use OP3 on Duart1 as the watchdog timeout clock
| let the OS set this up
	movb	M_BUT,d3
	andb	#0x10,d3
	beq	notreva

| Rev A board
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

notreva:

| in programming the configuration registers determine if we are a master
| or slave.  If we are a master we want to drive the Pipe and Duarts
|
|
| Now we program the onboard configuration registers
|
	movw	d2,STATUS_REG		| initialize status register
	movb	d2,PARCTL_REG		| no parity checking for now
	movw	SWTCH_REG,d3
	andw	#SW_MASTRSLV,d3		| bit is 0 for master, 1 for slave
	bne	slave

| Master
| init status and parity register for master case
| zero in the leds, allow accesses from multibus
	orw	#0x000f+ST_IP2ACC,STATUS_REG
	bra	initdone

| Slave
| init status and parity register for slave case
| zero in the leds, allow accesses from multibus, disable access to pipe,
| and disable access to duarts
slave:
	orw	#0x000f+ST_IP2ACC+ST_GEMASTER_,STATUS_REG
	orb	#PAR_DIS0+PAR_DIS1,PARCTL_REG


initdone:
	movb	d2,MBP_REG		| initialize multibus protection
					|  no user access to nothin'

	movl	#SRAM_BASE+SRAM_SZ,sp	| reset the initial stack pointer
					|  (this must be done if called
					|   from the kernel)
					| should be a movc XXX,isp

|
| We clear the area after our common area in the private ram
| it won't hurt if we overlap a bit into that common area while clearing
| we might overlap into the scratch space - big deal.
|
	movl	#SRAM_BASE+SRAM_SZ,a1	| end of the static ram

clrloop:
	movl	d2,a1@-			| zero out a long
	cmpl	#SRAM_BASE+COMM_SZ,a1
	bge	clrloop			| one down, any more to go?

|
| do the initializations necessary
|   initialize the common area, determine tty type
|   size memory and clear it
|

	movl	d1,sp@-		| initialization level
	jbsr	_init
	addql	#4,sp		| remove it from the stack

	andw	#0xfff0,STATUS_REG
	orw	#0x000e,STATUS_REG	|   one in the leds.
|
| run confidence tests (as a side effect prints power-up message)
|
	jbsr	confid
	andw	#0xfff0,STATUS_REG
	orw	#0x0009,STATUS_REG	|   six in the leds.

| let's for now assume that the cache works and turn it on
	movl	#1,d0
	movec	d0,cacr

	tstl	SRAM_BASE	| test if any memory found?
	beq	skpbss		|  no!

|
| We have memory, so we allocate the bss area,
| turn on parity and away we go
|
	movb	#PAR_UR+PAR_UW+PAR_KR+PAR_KW+PAR_MBR+PAR_MBW,PARCTL_REG

| someday allocate bss

	orw	#ST_SYSSEG_,STATUS_REG	| clear boot mode


#ifdef LATER
	jsr	_initdevs		| initialize whatever (devices mostly)
#endif

	andw	#0xfff0,STATUS_REG
	orw	#0x0008,STATUS_REG	|   seven in the leds.

skpbss:
	movl	#SRAM_BASE+SRAM_SZ,sp
	jbsr	_main			| main monitor loop

	tstb	d0
	beq	exit

	andw	#0xfff0,STATUS_REG
	orw	#0x0003,STATUS_REG	|   twelve in the leds.


|
| return back here if program was loaded or the go command was issued.
| If go command issued then there will be a stack value in the c_gostk
| field and we should use that for the stack
|
	movl	SRAM_BASE+ENTRY_OFF,a1	| get the entry point
	movl	SRAM_BASE+GOSTK_OFF,d0	| get go cmd stack value

	tstl	d0			| if stack value is 0, don't touch stk
	beq	skpstk

	movl	d0,sp			| must be go cmd - set the stack

skpstk:
	tstw	M_QUAD			| clear possible mousey intrs
	movl	#start,d0		| don't forget the reboot address

|	setup any parameters on the stack
	movl	#SRAM_BASE+ARGV_OFF,sp@-	| argv
	movl	#SRAM_BASE+ARGC_OFF,a0		| argc
	movl	a0@,sp@-

	jsr	a1@		| begin execution somewhere else

| reset the status register - just in case
exit:
	andw	#0x0080,STATUS_REG
	orw	#ST_IP2ACC+ST_SYSSEG_,STATUS_REG

	movw	#SR_SIPM7,sr	| set the 68020 status register: Supervisor
				| mode/interrupt priority level 7

	movl	#PROM_BASE,d0	| setup the vbr
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

|
| confid
|	This section of code contains the code necessary to run
|	a limited set of confidence tests.  This set is:
|
confid:

	rts

	.text

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


zeros:	.long	0,0,0,0,0,0,0,0,0,0,0,0,0,0	|14 long  of zeros
