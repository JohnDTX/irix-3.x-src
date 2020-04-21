|
|	PM2.1 VERSION Oct 5,1983.... G.Boyd
|			04feb85 - modified by D. Fong
|
|	pm2start.s - 
|		base startup for the pm2 version of the Quirk proms.
|		This assembler file performs the following very
|		basic operations:
|
|		*  enable refresh
|		*  test the memory used for the page, prot maps and
|		   the context register.
|		*  map the prom space virt->phys
|		*  exit boot state
|		*  stuff all the trap vectors with a default,
|		   and the important ones with a temporary handler.
|		*  clear bss for the main routine
|		*  call the C part of the prom code (_qprom())
|
|	at each step in the process, a number is placed in the leds.  If
|	the prom dies before the initial sign-on message, this number can
|	be used for a partial trace.
|

|
|
|	USEFUL CONSTANTS:
|
MEMPAGE =	0x1000			| size of mem page

PAGEMAP = 	0xfc0000		| leave the first four pages alone
					| unless in prom!
PAGEMAPEND = 	0xfc2000
PROTMAP = 	0xfc2000		| leave the first four pages alone
					| unless in prom!
PROTMAPEND = 	0xfc4000
PROT    = 	0x2200			| prototype protection locmem|rwx

CONTEXT = 	0xfc8001		| context reg

ROM0 = 		0xf80000		| addr of first prom
STATUS  = 	0xfc9000		| status reg; high byte is leds
CONFREG = 	0xfd0000		| config reg

UART0ADDR = 	0xfc4000		| used for refresh address
UART1ADDR = 	0xfc6000		| base addr for DUART 1
MOUSE =		0xfcc000		| mouse reg

|
|	low core constants
|
ZERO =		0x00
LOWVECTORADD = 	0x08			| addr of first of generic vectors
HIVECTORADD = 	0xc0			| addr of last of generic vectors
LOWINTADD = 	0x60
HIINTADD = 	0x78			| start with level 6
ILLINSTVEC =	0x10
DONTTOUCH =	0xfc			| conf reg mask/value if memory
					| should be preserved on this start,
					| and we should immediately drop into
					| the debugger
COMMONSTART = 	0x200			| common data area - not cleared at
COMMONLEN  = 	0x100			| startup
COMMONEND  = 	0x300
SAVEREGLOC = 	0x300

|
|	constants used in setting up stack+bss
|	on startup, one page is mapped for stack+bss
|	at physical ScratPhys; later prom code
|	must map the right amount of stack+bss to
|	the right physical pages.
|
BssAddr =	0xF6F800		| addr of bss; same as _bstart
BssSize =	0x800			| size of bss
StackAddr =	0xF6F000		| _bstart+BssSize-MEMPAGE
StackPAGEMAP =	0xFC1EDE		| PAGEMAP+atop(StackAddr)
StackPROTMAP =	0xFC3EDE		| PROTMAP+atop(StackAddr)
ScratPhys =	0x7F			| physical page for temp stack

|
|	several constants exist for each type of trap to tell
|	where on the stack the status register was saved and where
|	the pc was saved. the trap types are constants below:
|
TRAP =		0			| trap
BUSERR =	1			| buserr
ADDERR =	2			| address error
INTERRUPT =	0x10			| interrupt

INTERRUPTBIT = 4
INTERRUPT7 = 0x17
INTERRUPT6 = 0x16
INTERRUPT5 = 0x15
INTERRUPT4 = 0x14
INTERRUPT3 = 0x13
INTERRUPT2 = 0x12
INTERRUPT1 = 0x11
INTERRUPT0 = 0x10
MC68010  = 0x4	|or this bit in to get the correct offsets if we
		|are on a 68010/20

|
|
|	IMPORTS:
|
	.globl	_qprom
	.globl  _bstart
	.globl	_end
	.globl	_current_intlevel
	.globl  _longjmp

|
|
|	EXPORTS:
|
	.globl	start
	.globl  _halt
	.globl  _exit
	.globl	_common_area
_common_area = COMMONSTART

|
|
|	DATA SEGMENT:
|
|	each type of trap has an offset into the stack at which
|	the status register, access address, ir, and pc at the time 
|	of the trap can be found.  This is given by the table
|	below, indexed by trap type (long).
|
	.data
$stack_offsets:
	.byte	 0,-1,-1, 4	| standard case:SR at zero,pc at four,no aa,ir.
	.byte	 8, 4, 6,12	| buserror case: sr@8,ir@6,aa@4,pc@12
	.byte	 8, 4, 6,12	| address error case: same as bus error 
	.byte	 0,-1,-1, 4	| standard case:SR at zero,pc at four,no aa,ir.
|
|	following are the corresponding offsets for the 68010/20
|
	.byte	 0,-1,-1, 4	| standard case: SR at zero, pc at four, no aa.
	.byte	 0,12,24, 4	| buserror case: sr@0,aa@12,ir@24,pc@4
	.byte	 0,12,24, 4	| address error case: same as bus error 
	.byte	 0,-1,-1, 4	| standard case: SR at zero, pc at four, no aa.

|
|
|	BSS SEGMENT:
|
	.bss
$num_ints:
	.word	0	

|
|
|	TEXT SEGMENT:
|
	.text
|
|
|	RESET VECTOR:
|
|	when not in prom, these *MUST* assemble to
|	harmless legal instructions.
|
$base_vector:
	.long	_bstart
	.long	start

|
|	all starts come here.....
|
|	beginning of the instruction sequence for the pm2start
|	routine.  
|
|		******************************************
|	NOTE: it was discovered that sometimes an interrupt (particuarly
|	a mouse interrupt) may be pending at cold boot.  At this time,
|	the proms are mapped to 0.  Thus, a set of autovectors is mapped
|	from 0x60 to 0x80 which simply branch to start.  The first instructions
|	at start, then, disable interrupts and clear the (non-maskable)
|	mouse interrupt.  These autovectors are POSITION DEPENDANT.  Any
|	work in this first small portion of the startup must cause the 
|	autovectors to be repositioned !!
|		******************************************
|
start:
|
|	clear the mouse and mask what we can.
|
	tstw	MOUSE
	movw	#0x2700,sr
|
|	enable refresh....
|
	movb	#/eb,UART0ADDR+0x8		| Enable Refresh
	movb	#/00,UART0ADDR+0xc
	movb	#28, UART0ADDR+0xe
	movb	#/f4,UART0ADDR+0x1a
|
|	just in case we are rebooting, lets disable parity:
|
	andw	#0xFFEF,STATUS		| turn off parity
|
|	on reboots, we dont want to trash the page, protection maps
|	and the context map.
|
|	TEST CONFIG HERE
|
	andw	#0xFF8F,STATUS		| disable parity, mailbox ints
	movw	CONFREG,d7		| get configuration
	andw	#DONTTOUCH,d7		| mask off hostspeed bits
	cmpw	#DONTTOUCH,d7		| is it the 'drop to debugger' value?
	beq	ctest_return		| yes, dont disturb memory
	movl	#start,d0
	bra	after_autovec		| jump to after the autovectors.
|
|
|	INTERRUPT AUTOVECTORS:
|
|	vector all interrupts to start
|
	.word	0			| alignment to 0x60 (f80060).
	.long   start			| spurious
	.long 	start			| lev 1
	.long	start
	.long 	start
	.long	start
	.long 	start
	.long	start
	.long 	start			| lev 7.
|
|
after_autovec:
	movl	#_bstart,sp
	cmpl	#ROM0,d0		| are we in prom?
	blt	ctest_return		| no, dont do the page/prot map tests
	btst	#7,STATUS		| are we in boot state?
	beq	ctest_return		| no - dont do the page/prot map tests
	movw	#0,PAGEMAP		| log->phys for pgs 0 & 1
	movw	#PROT,PROTMAP
|
|	Do the cold start self test stuff
|
	movl	#ctest_return,a6	| load the return address
	jmp	cold_test
ctest_return:

|
|	map the prom space virt->phys
|
	movl	#_bstart,sp
 	andw	#0xFFF0,STATUS		| clear the leds 
	orw	#7,STATUS		| set the leds to seven
	movw	#0,PAGEMAP		| log->phys for pgs 0 & 1
	movw	#PROT,PROTMAP
	movw	#ScratPhys,StackPAGEMAP
	movw	#PROT,StackPROTMAP
|
|	setup to catch mouse interrupts,
|	read the mouse, set up our context and clear boot state
|
 	movl	#int7catch,d0		| int level 7 catcher
 	movl	d0,/7c
	tstw	MOUSE			| read the mouse
	movb	#0,CONTEXT		| Context <- 0
	orw	#/80,STATUS		| Clear boot state !!
|
|	sp has been set by the restart vector to BssAddr
|
|	now we are ready to clear the stack space:
|
|	    * clear  all of page StackPage
|	    * move the data at 0 to (COMMONSTART + COMMONLEN) to StackAddr
|	    * clear from 0x8 to COMMONSTART.
|
|
	movl	#_bstart+BssSize-MEMPAGE,a0
1$:	movl	#0,a0@+			| clear word
	cmpl	#_bstart+BssSize,a0	| done?
	blt	1$			| nope
|
| 	move the low area to our safe page
|
	movl	#ZERO,a0		| address of area to save
	movl	#_bstart+BssSize-MEMPAGE,a1
2$:	movl	a0@+,a1@+		| move it
	cmpl	#COMMONEND,a0		| check for done
	ble	2$
|
|	okay, go clear the area below common
|
	movl	#ZERO,a0
4$:	movl	#0,a0@+			| clear a longword
	cmpl	#COMMONSTART,a0		| reached the start of common yet?
	blt	4$			| clear more
|
|	general entry point: traps eventually come here:
|
teenter:
|
|	set the trap vectors to a default value.
|	during this phase, the leds should be set to eight
|
 	andw	#0xFFF0,STATUS		|clear the leds 
	orw	#8,STATUS		|set the leds to eight
	movl	#$default_tcatch,d0	|address of default trap handler

	movl	#HIVECTORADD,a0		|address of last trap vector stuff
1$:	movl	d0,a0@-			|stuff the current trap vector
	cmpl	#LOWVECTORADD,a0	|test for finish
	bge	1$
|	
|
|	stuff level 7 interrupts specially
|
	movl	#int7catch,d0		|level 7 ints
	movl	d0,0x7c			|stuff it
	movl	#int6catch,d0		|level 6 ints
	movl	d0,0x78			|stuff it
	movl	#int5catch,d0		|level 5 ints
	movl	d0,0x74			|stuff it
	movl	#int4catch,d0		|level 4 ints
	movl	d0,0x70			|stuff it
	movl	#int3catch,d0		|level 3 ints
	movl	d0,0x6c			|stuff it
	movl	#int2catch,d0		|level 2 ints
	movl	d0,0x68			|stuff it
	movl	#int1catch,d0		|level 1 ints
	movl	d0,0x64			|stuff it
	movl	#int0catch,d0		|spurious interrupts
	movl	d0,0x60
	movl	#$default_icatch,d0	|address of default int handler
	movl	d0,0x80			|spurious interrupts loc
|
|	finally, set some of the important trap vectors
|
	.globl	_trapF
|
	movl	#xberr,d0		| bus error (vector #2)
	movl	d0,/8
	movl	#xaerr,d0		| address error (vector #3)
	movl	d0,/c
	movl	#xierr,d0		| illegal instruction (vector #4)
	movl	d0,/10
	movl	#start,d0		| `trap #14' vector (reboot ) 
	movl	d0,/E*/4+/80		| 0x80+0xE*4
	movl	#_trapF,d0		| `trap #15' vector (s/w break)
	movl	d0,/F*/4+/80		| 0x80+0xF*4
 	movl	#int4catch,d0		| int level 4 catcher
 	movl	d0,/70			| stuffed
 	movl	#int7catch,d0		| int level 7 catcher
 	movl	d0,/7c
|
|	clear bss....
|
	movl	#_bstart,a0		| clear bss
	clrl	d0
clr:
	movl	d0,a0@+
	cmpl	#_end,a0
	blts	clr
|
|	and, lastly, call the _qprom routine.
|
 	andw	#0xFFF0,STATUS		|clear the leds 
	orw	#9,STATUS		|set the leds to nine
	movl	#_bstart,sp		| unwind the stack for safety
	jbsr	_qprom			| Call the quirk C entry


|
|	trap handlers, utility routines, etc for the pm2start
|	module for the pm2 proms.
|
|	GENERAL TRAP/INTERRUPT RECEIVING SEQUENCE:
|	
|	    save registers a0,a1,d0,d1 at SAVEREGLOC
|
|	    set pri level to 7, keeping old in d1.  If we
|	    	find we must return to the user, reset sr using d1.
|		as the only user return is via a longjmp, it is unnecessary
|		to restore the saved registers.
|
|
_exit:
exit:
_halt:
halt:
	trap 	#14			| Closest thing to a halt
|
|	and, if that didn't work, make certain......
|
	nop
	stop	#0

|
|	trap handlers setup earlier.  These consist of the default
|	handler and special handlers. The default handler cleans
|	up the stack and calls a routine to display the error message.
|	A warm boot is initiated.
|
|	Special handlers check the user's jump buffer.  If the pc
|	is set, the state is restored and a longjmp made.  
|
int7catch:
	tstw	MOUSE
	rte
int6catch:
	movb	#INTERRUPT6,d0
	bra	$default_icatch
int5catch:
	movb	#INTERRUPT5,d0
	bra	$default_icatch
int4catch:
	movb	#INTERRUPT4,d0
	bra	$default_icatch
int3catch:
	movb	#INTERRUPT3,d0
	bra	$default_icatch
int2catch:
	movb	#INTERRUPT2,d0
	bra	$default_icatch
int1catch:
	movb	#INTERRUPT1,d0
	bra	$default_icatch
int0catch:
	movb	#INTERRUPT0,d0
	bra	$default_icatch
$default_tcatch:
	movb	#TRAP,d0		| trap indicator
					| fall through
$default_catch:
|
|	make sure a minimal amount of stuff is mapped
|
	movw	#ScratPhys,StackPAGEMAP
	movw	#PROT,StackPROTMAP
	moveml	#0x303,SAVEREGLOC
	movb	d0,_current_intlevel
	bra	$default_catch2

$default_icatch:
|
|	d0 has been set to the trap / intr type
|
	movw	#ScratPhys,StackPAGEMAP
	movw	#PROT,StackPROTMAP
	movb	d0,_current_intlevel
	moveml	#0x8103,SAVEREGLOC
					| fall through
|
|	should we re-enter boot state?
|
$default_catch2:
	movl	sp,d1
	bclr	#0,d1			| force word alignment
	movl	d1,sp
	movw	sr,d1			| disable
	movw	#0x2700,sr
	movw	#0,sp@-			| alignment word
	bra	$display_error_and_reboot |and do a warm start

|
|
$idle_loop:
 	andw	#0xFFF0,STATUS		|clear the leds 
	orw	#0xE,STATUS		|set the leds to 'E' (error)
$idle_loop1:
	bra	$idle_loop1

|
|	special trap catch routines.  There is one of these for
|	each of bus error, address error, illegal instruction,
|	and trap #14.  Their handling is similar:
|
|		* a user can get control of the trap by setting up
|		  a setjmp vector from a C program with a special
|		  name.  These names are 
|			_bejmp - bus error
|			_aejmp - address error
|			_iejmp - illegal instruction error
|			_tejmp - trap #14 error 
|		  if a user handler exists, it is used. If it returns
|		  the default handling (below) is re-joined.
|
|		* default handling consists of calling a routine to 
|		  print an error message, if one exists, and joining
|		  the $default_catch code (checking the conf switch, etc)
|

|
|	BUS ERROR HANDLER
|		user setjmp buffer should be _bejmp,
|		user routine to print error message is berr
|
	.globl	_bejmp

xberr:
|||	movw	#ScratPhys,StackPAGEMAP|||
|||	movw	#PROT,StackPROTMAP|||
	moveml	#0x303,SAVEREGLOC
	movl	sp,d1
	bclr	#0,d1			| force word alignment
	movl	d1,sp
	movw	sr,d1			| disable
	movw	#0x2700,sr
	movl	#_bejmp,a0
	cmpl	#0,a0
	beqs	xb1
1$:	movl	a0@,d0
	beqs	xb1			| no pc set
	movb	#BUSERR,_current_intlevel
	bra	golongjmp

xb1:	movw	d0,sp@-			| Stack one word for alignment
	movb	#BUSERR,_current_intlevel		| bus error type code
	bra	$display_error_and_reboot	| no berr routine.

|
|	ADDRESS ERROR HANDLER
|		user setjmp buffer should be _aejmp,
|		user routine to print error message is aerr
|
	.globl	_aejmp
xaerr:
|||	movw	#ScratPhys,StackPAGEMAP|||
|||	movw	#PROT,StackPROTMAP|||
	moveml	#0x303,SAVEREGLOC
	movl	sp,d1
	bclr	#0,d1			| force word alignment
	movl	d1,sp
|	bra	_trapF
	movw	sr,d1			| disable
	movw	#0x2700,sr
	movl	#_aejmp,a0
	cmpl	#0,a0
	beqs	xa1
1$:	movl	a0@,d0
	beqs	xa1			| no pc set
	movb	#ADDERR,_current_intlevel
	bra	golongjmp

xa1:	movw	d0,sp@-			| Stack one word for alignment
	movb	#ADDERR,_current_intlevel
	bra	$display_error_and_reboot	| no aerr routine.

|
|	ILLEGAL INSTRUCTION ERROR HANDLER
|		user setjmp buffer should be _iejmp,
|		user routine to print error message is ierr
|
	.globl	_iejmp

xierr:
|||	movw	#ScratPhys,StackPAGEMAP|||
|||	movw	#PROT,StackPROTMAP|||
	moveml	#0x303,SAVEREGLOC
	movl	sp,d1
	bclr	#0,d1			| force word alignment
	movl	d1,sp
	movw	sr,d1			| disable
	movw	#0x2700,sr
	movl	#_iejmp,a0
	cmpl	#0,a0
	beqs	xi1
1$:	movl	a0@,d0
	beqs	xi1			| no pc set
	movb	#TRAP,_current_intlevel
	bra	golongjmp

xi1:	movw	d0,sp@-			| Stack one word for alignment
	movb	#TRAP,_current_intlevel
	bra	$display_error_and_reboot	| no ierr routine.
|
|	TRAP #14 ERROR HANDLER
|		user setjmp buffer should be _tejmp,
|		user routine to print error message is terr
|
	.globl	_tejmp
|
xtrape:
|||	movw	#ScratPhys,StackPAGEMAP|||
|||	movw	#PROT,StackPROTMAP|||
	moveml	#0x303,SAVEREGLOC
	movl	sp,d1
	bclr	#0,d1			| force word alignment
	movl	d1,sp
	movw	sr,d1			| disable
	movw	#0x2700,sr
	movl	#_tejmp,a0
	cmpl	#0,a0
	beqs	xe1
1$:	movl	a0@,d0
	beqs	xe1			| no pc set
	jbsr	golongjmp

xe1:	
	movl	#_bstart,sp		| gotta fix up the stack
	movw	d0,sp@-			| Stack one word for alignment
	bra	teenter				| no teerr routine.
|
|
golongjmp:
|
|	The trap type is already in d0.
|	The status register to restore is in d1.
|	The address of the jump buffer is in a0
|	The old a0,a1,d0,d1 are in our safe location.
|
	movl	#_longjmp,a1
	cmpl	#0,a1
	beqs	$display_error_and_reboot
	movw	d1,sr
1$:	movl	#1,sp@-
	movl	a0,sp@-
	jbsr	a1@
	bra	teenter
|
|
$display_error_and_reboot:
|
|	stuff an 'E' into the status register and reboot. The
|	trap type code is in d0.
|
 	andw	#0xFFD0,STATUS		|clear the leds, disable parity
	orw	#0xE,STATUS		|set the leds to 'E' (error)
|
|	at this point, we either:
|
|	    print a message and clear the interrupt if an uncaught interrupt
|	    has occurred and the count of uncaught interrupts has not reached
|	    the allowable maximum. an rte is done.
|
|	    otherwise, the default handler is called (see below) and a 
|	    warmboot is done.
|
	.globl	_intr
 	btst	#INTERRUPTBIT,_current_intlevel	| is this an interrupt?
 	beq	godefault		| nope, go do default handling
	jbsr	_intr			| otherwise, go display the message
	movw	sp@+,d1			| pop the alignment word off the stack
|
|	    if this is a level4 interrupt, we will return from it.
|
	andl	#0xf,_current_intlevel			| get the level only
	cmpl	#4,d2			| level four?
	bne	displayE
	moveml	SAVEREGLOC,#0x8103	| restore sp,a0,d1,d0
	rte
|
|
|
|	the trap type code is in d0.  The old sr is in d1. Each byte of
|	this code specifies the location of a register off
|	the stack pointer, or -1 if none.  We will move
|	this information to the appropriate place and print it
|	if possible.  The call we are emulating is:
|
|	    grave_error(typecode,sr,aa,ir,pc)
|
|	grave_error has the responsibility of printing the error
|	message.
|
godefault:
|
|	first, lets determine if this is a 68010/20 or not.
|
	movl	sp,a1			|save current sp
	movl	ILLINSTVEC,a0		|and the illegal inst vector
	movl	#whoops,ILLINSTVEC	|stuff it
	clrl	d0
	.word	0x4E7B			|	movec
	.word	0x0801			|	d0,vbr
	clrl	d0
	movb	_current_intlevel,d0			|restore d0
	orb	#MC68010,d0		|no trap - set the 'is 68010' bit.
	bras	whoops1
whoops: movb	_current_intlevel,d0
whoops1: movl	a0,ILLINSTVEC		|and replace the vector
	movl	a1,sp			| and the stack pointer.
	movl	#3,d1			|number of pieces of information
	movl	#$stack_offsets,a2	|get the information offset
	movl	d0,d2
	asll	#2,d2
	addl	d2,a2			|a2 now has the address of the
					|table entry (four bytes)
.L4:	movl	a2,a0
	addl	d1,a0			|address of constant which is offset
	movb	a0@,d2			|byte offset into stack of this info
	extw	d2
	extl	d2
	tstl	d2
	bmi	.L2			|no information
	movw	d2,a0
	addl	a1,a0			|make an address
	movl	a0@,sp@-		|push on stack for call
	bra	.L3
.L2:	movl    #-1,sp@-
.L3:
	subql	#1,d1	
	bge	.L4			|go again?
	movl	d0,sp@-			|function code
	movl	#_grave_error,a0
	cmpl	#0,a0
	beqs	.L5
1$:	jbsr	a0@			|jump to routine
displayE:
.L5:
	movl	#0x50000,d0		|display the 'E' for a sec.
.L1:	subql	#1,d0
	bgt	.L1
	bra	teenter	


|==========================================================================
|
|	COLD START SELF TESTS:
|
|	On a cold start, this module is jumped to, with the return jump
|	address in register A6.  Failure data is returned in D4.
|
|	The tests performed in this module are:
|
|		Page map		(status 0)
|		Protection map		(status 1)
|		Context register	(status 2)
|		DUART			(status 3)
|		DUART timings		(status 4 and 5)
|		RAM boot pages		(status 6)
|
|

ENV_RPT_PGTEST	= 0x17 * 0x08	| Repeat page map test on boot
ENV_RPT_PTTEST	= 0x18 * 0x08	| Repeat protection map test on boot
ENV_RPT_CXTEST	= 0x19 * 0x08	| Repeat context reg. test on boot
ENV_RPT_DUARTST	= 0x1a * 0x08	| Repeat DUART test on boot
ENV_RPT_TIMETST	= 0x1b * 0x08	| Repeat DUART timer tests on boot
ENV_RPT_RAMBTST	= 0x1c * 0x08	| Repeat RAM boot page tests on boot

DUART_TIMEOUT	= 0x00		| DUART test error codes
DUART_CHARERR	= 0x01

TIMER0		= 0x00		| DUART timer error codes
TIMER1		= 0x01

RAM_START	= 0x00		| RAM boot start and end
RAM_END		= 0x1000
RAM_HI_ADDR	= 0x7f		| Page address of high tested memory


|
|	Patterns which will be used to test the protection and page
|	maps.
|
	.data
	.even

|
|	PAGE AND PROTECTION MAP PATTERNS
|	The high four bits of the page map are always zero.  Other than
|	this, all bits of both the page map and protection map are r/w
|
pagemap_patterns:	| Page map
	.word	0	| terminator
	.word	0x0aaa
	.word	0x0555
 	.word	0x0a5a
	.word	0x05a5
pgp_end:

protmap_patterns:	| Protection map
	.word	0	| terminator
	.word	0xaaaa
	.word	0x5555
	.word	0xa5a5
	.word	0x5a5a
ptp_end:

|
|	CONTEXT REGISTER PATTERNS
|	The context register is an 8 bit r/w register.
|
context_patterns:	| Context register
	.byte	0	| terminator
	.byte	0x55
	.byte	0xaa
	.byte	0xa5
	.byte	0x5a
cxp_end:

	.even

|
|	DUART PORT LOCATIONS
|
duart_ports:
	.long	UART0ADDR		| DUART 0 port A
	.long	UART0ADDR+0x10		| DUART 0 port B
	.long	UART1ADDR		| DUART 1 port A
	.long	UART1ADDR+0x10		| DUART 1 port B
	.long	0			| Terminator

|
|	DUART TEST PATTERN
|
duart_pattern:
	.asciz	"duart test... "


	.even
	.text
|
|	MODULE ENTRY POINT
|
cold_test:

|
|	PAGE MAP TEST
|	Test the memories used for the page map PAGEMAP-(PROTMAP-1).
|	We use the patterns found in pagemap_patterns, and first write,
|	then read them from all the memory in question.  If an error
|	occurs, an immediate halt will be taken.  At this point,
|	examination of the registers will reveal:
|
|		d0 - write (0) or read (1)
|		d1 - pattern currently being used
|		a0 - address of one word below the memory
|		     address which failed.
|
|	This data is especially useful in conjunction with an ICE.
|
|	When we download a program, the stack is usually set to somewhere
|	between 0x500 and 0x1000.  A parity error occurs when we read from
|	the stack before writing it.  To take care of this, we are going to 
|	clear the first page here, and clear the rest of memory after we do
|	the memory scan.
|
|	During this test, the leds will display a 0.
|
pgtest:
	andw	#0xFFF0,STATUS		| Set the leds to zero
	movl	#pgp_end,a1		| Address of pattern in use
nextpgtest:
	movw	a1@-,d1			|pattern in use
	tstw	d1			|if zero, we are done
	jeq	pgtestdone		
	movl	#PAGEMAPEND,a0		|address in memory being tested
	moveq	#0,d0			|write in progress
1$:	movw	d1,a0@-			|write the pattern
	cmpl	#PAGEMAP,a0		|test for done
	bgt	1$			
|
	moveq	#1,d0			|read in progress
	movl	#PAGEMAPEND,a0		|end location
2$:	cmpw	a0@-,d1			|read the word and compare
	bne	error
	cmpl	#PAGEMAP,a0		|test for done
	bgt	2$
	jra	nextpgtest		|and go do the next test

pgtestdone:
	movw	CONFREG,d2		| Check for endless loop
	andw	#DONTTOUCH,d2		| Mask off non-setup bits
	cmpw	#ENV_RPT_PGTEST,d2	| Compare for looping code
	jeq	pgtest			| If true, go back to beginning


|
|	PROTECTION MAP TEST
|	Test the memories used for the protection map PROTMAP-PROTMAPEND.
|	The patterns used are found in protmap_patterns.  The registers
|	are set up similarly to the pagemap tests.
|
|	During this test, the leds will display a 1.
|
pttest:
 	andw	#0xFFF0,STATUS		| Clear the leds
	orw	#1,STATUS		| Set the leds to one
	movl	#ptp_end,a1		|address of pattern in use
nextpttest:
	movw	a1@-,d1			|pattern in use
					|if zero, we are done
	jeq	pttestdone		
	movl	#PROTMAPEND,a0		|address in memory being tested
	moveq	#0,d0			|write in progress
1$:	movw	d1,a0@-			|write the pattern
	cmpl	#PROTMAP,a0		|test for done
	bgt	1$
|
	moveq	#1,d0			|read in progress
	movl	#PROTMAPEND,a0		|end location
2$:	cmpw	a0@-,d1			|read the word and compare
	bne	error
	cmpl	#PROTMAP,a0		|test for done
	bgt	2$
	jra	nextpttest		|and go do the next pattern

pttestdone:
	movw	CONFREG,d2		| Check for endless loop
	andw	#DONTTOUCH,d2		| Mask off non-setup bits
	cmpw	#ENV_RPT_PTTEST,d2	| Compare for looping code
	jeq	pttest			| If true, go back to beginning


|
|	CONTEXT REGISTER TEST
|	Test the context register by writing and reading random data 
|
|	During this test, the leds will display a 2.
|
cxtest:
 	andw	#0xFFF0,STATUS		| Clear the leds 
	orw	#2,STATUS		| Set the leds to 2
	movl	#cxp_end,a1		|address of pattern in use
nextcxtest:
	movb	a1@-,d1			|pattern in use
					|if zero, we are done
	jeq	cxtestdone		
	movl	#CONTEXT,a0		|address in memory being tested
	moveq	#0,d0			|write in progress
	movb	d1,a0@			|write the pattern
|
	moveq	#1,d0			|read in progress
	cmpb	a0@,d1			|read the word and compare
	bne	error
	jra	nextcxtest		|and go do the next pattern

cxtestdone:
	movw	CONFREG,d2		| Check for endless loop
	andw	#DONTTOUCH,d2		| Mask off non-setup bits
	cmpw	#ENV_RPT_CXTEST,d2	| Compare for looping code
	jeq	cxtest			| If true, go back to beginning



	jmp	cold_return		| DEBUG -------
|
|	DUART TEST
|
|	During this test, the leds will display a 3.
|
duart_test:
 	andw	#0xFFF0,STATUS		| Clear the leds
	orw	#3,STATUS		| Set the leds to 3
|
|	Set up DUARTs for loopback at 19.2K on all channels
|
	movl	#duart_ports,a1		| Start address of port bases
10$:	tstl	a1@			| Check for terminator
	jeq	11$			| If so, go to test loop
	movl	a1@+,a2			| Load port address
	movb	#0x4a,a2@(0x04)		| Reset error bits
	movb	#0x15,a2@(0x04)		| Set MR ptr to 1, enable Tx and Rx
	movb	#0x13,a2@(0x00)		| 8 bits, no parity, no control
	movb	#0x87,a2@(0x00)		| Loopback, no control, 1 stop
	movb	#0xcc,a2@(0x02)		| 19.2K baud
	jra	10$			| Loop
11$:

	movl	#duart_ports,a1		| Start address of port bases
1$:	tstl	a1@			| Check for terminator
	jeq	duart_done		| If so, 'bye kids...
	movl	a1@+,a2			| Load port address
	movl	#duart_pattern,a0	| Load pattern starting address
2$:	tstb	a0@			| Check for end of string
	jeq	1$			| If so, do another port
	movb	a0@,a2@(0x06)		| Output the character
	movl	#0xffff,d1		| Set up the timeout
3$:	tstl	d1			| Check for timeout
	jeq	duart_timeout		| If timed out, go to error
	btst	#0x00,a2@(0x02)		| Check for data in Rx
	dbeq	d1,3$			| If data is not in, spin
	movb	a2@(0x06),d0		| Input character
	cmpb	a0@+,d0			| Check to see if it's the same
	jeq	2$			| If so, continue text loop

	movw	#DUART_CHARERR,d0	| Load character mismatch error code
	jra	duart_error		| and go to error routine

duart_timeout:
	movw	#DUART_TIMEOUT,d0	| Load timeout error code

duart_error:
	movw	CONFREG,d2		| Check for endless loop
	andw	#DONTTOUCH,d2		| Mask off non-setup bits
	cmpw	#ENV_RPT_DUARTST,d2	| Compare for looping code
	jeq	duart_test		| If true, go back to beginning
	jra	error			| If not, go to error routine

duart_done:
	movw	CONFREG,d2		| Check for endless loop
	andw	#DONTTOUCH,d2		| Mask off non-setup bits
	cmpw	#ENV_RPT_DUARTST,d2	| Compare for looping code
	jeq	duart_test		| If true, go back to beginning



	jmp	timing2_done		| DEBUG ----
|
|	DUART TIMING TESTS
|
|	During this test, the leds will display a 4 while testing the
|	DUART 0 timing, then a 5 for DUART 1.
|
timing1:
 	andw	#0xFFF0,STATUS		| Clear the leds
	orw	#4,STATUS		| Set the leds to 4

	movl	#0xffff,d0		| Set up the timeout
1$:	tstl	d0			| Check for timeout
	jeq	time1_timeout		| If timed out, go to error
	btst	#0x03,UART0ADDR+0x0a	| Check timer
	dbeq	d0,1$			| If data is not zero, spin

	movl	#0xffff,d0		| Set up the timeout
2$:	tstl	d0			| Check for timeout
	jeq	time1_timeout		| If timed out, go to error
	btst	#0x03,UART0ADDR+0x0a	| Check timer
	dbne	d0,2$			| If data is not one, spin

timing1_done:

	movb	#0x00,UART1ADDR+0x1a		| Shut off outputs
	movb	#0xeb,UART1ADDR+0x6		| Set up timer on DUART1
	movb	#0x00,UART1ADDR+0xc
	movb	#0x02,UART1ADDR+0xe
timing2:
 	andw	#0xFFF0,STATUS		| Clear the leds
	orw	#5,STATUS		| Set the leds to 5

	movl	#0xffff,d0		| Set up the timeout
1$:	tstl	d0			| Check for timeout
	jeq	time2_timeout		| If timed out, go to error
	btst	#0x03,UART1ADDR+0x0a	| Check timer
	dbeq	d0,1$			| If data is not zero, spin

	movl	#0xffff,d0		| Set up the timeout
2$:	tstl	d0			| Check for timeout
	jeq	time2_timeout		| If timed out, go to error
	btst	#0x03,UART1ADDR+0x0a	| Check timer
	dbne	d0,2$			| If data is not one, spin
	jra	timing2_done

time1_timeout:
	movw	#TIMER0,d0		| Load timer 0 error code
	jra	time_error		| and go to error routine

time2_timeout:
	movw	#TIMER1,d0		| Load timer 1 error code

time_error:
	movw	CONFREG,d2		| Check for endless loop
	andw	#DONTTOUCH,d2		| Mask off non-setup bits
	cmpw	#ENV_RPT_TIMETST,d2	| Compare for looping code
	jeq	timing1			| If true, go back to beginning
	jra	error			| If not, go to error routine

timing2_done:
	movw	CONFREG,d2		| Check for endless loop
	andw	#DONTTOUCH,d2		| Mask off non-setup bits
	cmpw	#ENV_RPT_TIMETST,d2	| Compare for looping code
	jeq	timing1			| If true, go back to beginning



	jmp	cold_return		| DEBUG ----
|
|	RAM BOOT PAGES TEST
|
|	During this test, the leds will display a 6.
|
ram_b_test:
	movb	#0,CONTEXT		| Context 0
	orw	#0x80,STATUS		| Clear boot state
 	andw	#0xFFF0,STATUS		| Clear the leds
	orw	#6,STATUS		| Set the leds to 6

	movw	#0,PAGEMAP		| Set up page and prot maps at on
	movw	#PROT,PROTMAP		|  board phys 0
pass_2:
	movl	#ptp_end,a1		| Address of pattern in use
nextramtest:
	movw	a1@-,d1			| Pattern in use
					| If zero, we are done
	jeq	ram_b_done		
	movl	#RAM_END,a0		| Address in memory being tested
	moveq	#0,d0			| Write in progress
1$:	movw	d1,a0@-			| Write the pattern
	cmpl	#RAM_START,a0		| Test for done
	bgt	1$
|
	moveq	#1,d0			| Read in progress
	movl	#RAM_END,a0		| End location
2$:	cmpw	a0@-,d1			| Read the word and compare
	bne	error
	cmpl	#RAM_START,a0		| Test for done
	bgt	2$
	jra	nextramtest		|  and go do the next pattern

ram_b_done:
	tstw	PAGEMAP			| Is this the second pass?
	bne	1$			| If so, get out of loop

	movw	#RAM_HI_ADDR,PAGEMAP	| Set page map for next block to test
	jra	pass_2			|  and execute next pass

1$:	movw	CONFREG,d2		| Check for endless loop
	andw	#DONTTOUCH,d2		| Mask off non-setup bits
	cmpw	#ENV_RPT_RAMBTST,d2	| Compare for looping code
	jeq	ram_b_test		| If true, go back to beginning


|
|	RETURN
|	Upon successful completion of all the self tests, we get to this
|	point.  Then we use a6 to tell us where to return to.  Duh.
|
cold_return:
	jmp	a6@			| Return to the calling program


|
|	ERROR
|	We come here if an error occurs and a Diagnostics
|	exception switch code is not set.  If the verbose switch is
|	set, the display flashes the test status code for twice as
|	long as the error code in d0.
|	Otherwise, we just stop, choke and die.
|
error:
	btst	#0x02,CONFREG		| Check verbose switch
	jeq	choke_and_die		| If not verbose, stop

	movw	STATUS,d1		| Save old status
1$:	andw	#0xFFF0,STATUS		| Clear the leds
	orw	d0,STATUS		| Set the leds to error code
	movl	#0x50000,d2		| Load timing constant
2$:	subql	#1,d2			| Check for loop done
	jne	2$			| If not, spin

	movw	d1,STATUS		| Set the leds to old status
	movl	#0x100000,d2		| Load timing constant
3$:	subql	#1,d2			| Check for loop done
	jne	3$			| If not, spin

	jra	1$			| Start over

choke_and_die:
	stop	#0			| Where all bad boards come to die

|
|	END SELF TESTS
|
|==========================================================================
