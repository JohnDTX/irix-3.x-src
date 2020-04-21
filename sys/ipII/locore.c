|
| locore.c
|	Machine dependent start up code for the IP2 processor.
|	If we were consistent with the 68020 and our usage, all
|	 references to the sp would be changed to the movc instruction
|	 referencing the isp.
|

#include	"../ipII/psr.h"
#include	"../ipII/cpureg.h"
#include	"../ipII/pte.h"
#include	"../ipII/trap.h"
#include	"../ipII/vmparam.h"
#include	"../ipII/evec.h"
#include	"debug.h"

|
| masks for affecting the board status register
|
ST_INTROFF	= 0xffcf	| mask to disable intrs in the board status reg
ST_INTRON	= 0x0030	| mask to enable intrs in the board status reg

|
| Status register values for changing interrupt priority level
|
SR_SIPM7	= SR_SU + SR_IPM7	| supervisor mode, priority 7
SR_SIPM0	= SR_SU			| supervisor mode, priority 0
SPL0		= SR_SU

|
| bit definitions various CACR bits
|
CACHE_CLR	= 0x0008
CACHE_ENB	= 0x0001
CACHE_CE	= CACHE_CLR + CACHE_ENB

|
| these values represent the various places in the page table map that
| things live at.  They are by convention only.
|
BTSTARTPG	= 0x3e00	| starting page index that booted pgms start at
				|  represents 62megabytes.
BTSTARTBYTE	= 0xf800	| starting offset in page map that booted pgms
				|  start at (BTSTARTPG * sizeof (pte))
KSTARTPG	= 0x3400	| starting page index that kernel begins at
				|  represents 52megabytes.
KSTARTBYTE	= 0xd000	| starting offset in page map that kernel
				|  begins at (KSTARTPG * sizeof (pte))
KBASEPG		= 0x34		| page base of the kernel (for OS_BASE)

UDOT_PTMAP_OFF	= PTMAP_BASE + KSTARTBYTE + UDOT_OFF

|
| Address's of some of the duart registers
|
DUISR	= DUART1_BASE+0xA	| interrupt status register
DUCCSTP	= DUART1_BASE+0x1E	| say what?

|
| and of course, the misc defines.
|
ONEMEGPG	= 0x100		| number of pages needed to represent 1mb
UDOTPTE		= PTE_SACC	| pte value for proc 0 udot.

|
| includes size of sched page table and udot.  Used to round the end of
| the kernel up to a click boundary
|
RNDUP		= NBPG + USIZE + USIZE - 1

	.data

|
| Allocate the mouse quadrature support info space.
|
	.comm	__mousex,2	| mouse x-axis location
	.comm	__mousey,2	| mouse y-axis location

	.globl	_rebootvec,	_ivectors

#if NDEBUG > 0
	.globl	_savessp
#endif

#ifdef	GL2
|	.globl	__dcrflags, __dcrmodes
#endif

|
| Definition of the virtual address's of all mapped goodies.
| See cpureg.h for defintions of the virtual space.
|
	.globl	_usrpt,		_forkutl,	_xswaputl,	_xswap2utl
	.globl	_swaputl,	_pushutl,	_vfutl,		_vmmap
	.globl	_mbutl,		_u

_usrpt		= USRPT_VBASE
_forkutl	= FORKUTL_VBASE
_xswaputl	= XSWAPUTL_VBASE
_xswap2utl	= XSWAP2UTL_VBASE
_swaputl	= SWAPUTL_VBASE
_pushutl	= PUSHUTL_VBASE
_vfutl		= VFUTL_VBASE
_vmmap		= DEVMEM_VBASE
_mbutl		= MBUF_VBASE
_u		= UDOT_VBASE

#ifdef	PROF
	.globl	_user
_user		= KERN_VBASE
#endif

	.text

	.globl	start,		_end,		_edata,		_main
	.globl	_trap,		_syscall,	_halt,		_debug
	.globl	_doboot,	_bzeroPAGE,	_bcopy,		_bzero
	.globl	_save,		_qsave,		_resume,	_sureg
	.globl	_spl0,		_spl1,		_spl2,		_spl3
	.globl	_spl4,		_spl5,		_spl6,		_spl7
	.globl	_splx,		_spltty,	_splmax
	.globl	_hardclock,	_todintrclr,	_mouseintr

	.globl	Xtraps,		Xsyscall,	Xclock,		Xduart0
	.globl	Xduart1,	Xmbintr01,	Xmbintr2
	.globl	Xmbintr3,	Xmbintr4,	Xmbintr5,	Xmbintr6
	.globl	Xmbintr7
	.globl	call,		backtosystem,	backtouser
	.globl	Xaddrerr,	Xbuserr
	.globl	Xparity

	.text
|
| and away we go.....
| the PROMs set up information in the following manner:
|   Registers:
|	d0		PROM address for reboots
|
|   The common area in the static ram contains the remaining information.
|
start:
	andw	#ST_INTROFF,STATUS_REG	| turn off interrupts for the board
	movw	#SR_SIPM7,sr		| turn off interrupts for the cpu

	moveq	#CACHE_CE,d1		| Turn cache on and clear it
	movec	d1,cacr

	movl	d0,_rebootvec		| where in PROMs to jump to on reboot

|
| now pgm the vbr
|
	movl	#evecstart,d0
	movec	d0,vbr

|
| relocate ourself in the page map to where the kernel
| should live
|
	movl	#PTMAP_BASE+BTSTARTBYTE,a0	| from location
	movl	#PTMAP_BASE+KSTARTBYTE,a1	| to location
	movl	#ONEMEGPG - 1,d0		| number of entries to move

0$:
	movl	a0@+,a1@+		| move 1 entry
	dbra	d0,0$			| one down, do another?

|
| update the kernel base register.  Following this we will be running
| in the portion of the page table map we copied ourself to.
|
	movb	#KBASEPG,OS_BASE

|
| clean the page map, zero out all unused entries.
| we start where we left off after moving ourself.... then we zero out
| from the beginning of the page map to where the kernel begins.
| Clear the 11mb following the kernel.
|
	movl	#0xb00,d0	| number of entries to zero minus one
	moveq	#0,d1		| the zero to store

2$:
	movl	d1,a1@+		| zero an entry
	dbra	d0,2$		| one down, do another?

|
| now from the beginning to where we live
|
	movl	#PTMAP_BASE,a0	| starting pte to zero
	movl	#KSTARTPG-1,d0	| number of entries to zero, minus 1

4$:
	movl	d1,a0@+		| zero an entry
	dbra	d0,4$		| one down, do another?

|
| Clear out bss region (plus sched page table and udot)
|
	movl	#_end+RNDUP,d0		| calc end of UNIX
	andl	#-USIZE,d0		| Round to nearest click
	movl	#_edata,a0		| Start clearing here
6$:
	movl	d1,a0@+			| Clear a long word
	cmpl	d0,a0			| check if went beyond the end
	jcs	6$			|  nope, do another

| check the revision level of board.  Rev A boards need to set kernel size
| to 4 Meg.  Doing this to Rev B boards will turn off the pipe!!!
|

	movb	M_BUT,d0
	andb	#0x10,d0
	movb	d0,_rev_A		| save machine revision level
	beq	notreva

| A rev A board:
| currently the PROMs set the board status register to a 2mb kernel
| area.  We want the 4mb area
|
	orw	#ST_OSSIZE,STATUS_REG

notreva:

|
| pgm the pte for the udot and establish the real stack
| NOTE: this assumes that the sched page table and udot reside
|	in the first 1mb of memory in the 1st memory board.
|	No memory board to physical page mapping is done here!!!!!!
|
	movl	#_end+NBPG-1,d7		| round up to nearest page
	moveq	#PGSHIFT,d0
	lsrl	d0,d7			| Convert address to page
	movl	d7,d1			| make a copy of the page number
	orl	#UDOTPTE,d1		| or in the protection info

	movl	d1,UDOT_PTMAP_OFF	| now pgm the correct pte

	movl	#UDOT_VBASE+USIZE,sp	| Set stack at top of U area

|
| enough is initialized, now we can do the rest in C
|
	movl	d7,sp@-		| arg is the first page after the kernel
				| (sched page table area)
	jbsr	_mlsetup
	addql	#4,sp		| done with the arg

|
| Call Unix main: we return once to startup the init code (icode)
|
	jbsr	_main

	clrl	sp@-
	jbsr	_sureg			| setup pagemap for init
	addql	#4,sp

|
| Pgm the base and limit register for the text/data area
|
	movw	_cx+CX_TDSIZE,d0	| get the text/data size (power of 2)
	moveq	#PTEPCX,d1		| minimum number of pages in a context
	aslw	d0,d1			| convert size to number of pages
	negw	d1			| convert to correct format for limit
	movw	_cx+CX_TDUSER,d0	| get the users context number
	aslw	#PPCXLOG2,d0		| convert to index into page map

	movw	d0,TDBASE_REG		| set the base register
	movw	d1,TDLMT_REG		| set the limit register

|
| Pgm the base and limit register for the stack area
| NOTE: icode has no associated stack area: currently we just
|       overlap the text/stack area.
|
	movw	_cx+CX_TDSIZE,d0	| get the stack size (power of 2)
	moveq	#PTEPCX,d1		| minimum number of pages in a context
	aslw	d0,d1			| convert size to number of pages

	movw	_cx+CX_TDUSER,d0		| get the users context number
	aslw	#PPCXLOG2,d0		| convert to index into page map
	subqw	#1,d1			| calc stack size for limit reg.
	addw	d1,d0			| calc last pte index number

	movw	d0,STKBASE_REG		| pgm the stack base register
	movw	d1,STKLMT_REG		| pgm the stack limit register


	moveq	#CACHE_CE,d0
	movec	d0,cacr

|
| Start icode going.  Build a noraml 4 word exception frame so the rte will
| get the icode going.
|
	clrw	sp@-			| set frame format to normal for 68020
	movl	#USRTEXT,sp@-		| user start address
	clrw	sp@-			| new sr value
	movw	#SR_SIPM7,sr		| block interrupts through rte

	rte				| call init

Xparity:
|
| Disable all interrupts to avoid recursion.
| Adjust the processor priority to level 7, masking out all maskable
| interrupts.
|
	andw	#0xffdf,STATUS_REG	| disable interrupts
	movw	#SR_SIPM7,sr		| block interrupts through rte

Xaddrerr:
Xbuserr:

|
| General exception handler
|
Xtraps:
	addql	#1,_cnt+V_TRAP		| incr trap counter

	clrw	sp@-			| long extend status register
	clrl	sp@-			| address of intr handler (not used)

	moveml	#0xFFFF,sp@-		| save all registers
	movec	usp,a0			| get user stack pointer
	movl	a0,sp@(FRAME_SP)	|  and save it in saved area

trapcom:
	moveq	#0,d0			| zero out the high word
	movw	sp@(FRAME_VECOFFSET),d0	| get the vector offset from the frame
	andw	#0xFFF,d0		| trim out the format
	asrw	#2,d0			| and calc the vector number (trap type)

|
| Processor reschedule
|   This is an emulated exception rather than hardware generated
|
Xresched:
	movl	d0,sp@-			| argument to trap
	jbsr	_trap			| go handle the trap
	addql	#4,sp			| all done with the argument

	btst	#5,sp@(FRAME_SR+2)	| did we come from user mode?
	jne	backtosystem		|  no, return to system
	jra	backtouser		|  yes, return to user

|
| System calls vector here (trap #0)
|
Xsyscall:
	addql	#1,_cnt+V_SYSCALL	| incr syscall counter

	clrw	sp@-			| long extend status register
	clrl	sp@-			| address of intr handler (not used)

	moveml	#0xFFFF,sp@-		| save all registers
	movec	usp,a0			| get user stack pointer
	movl	a0,sp@(FRAME_SP)	|  and save it in saved area

	btst	#5,sp@(FRAME_SR+2)	| did we come from user mode?
	jne	trapcom			|   nope, kernel system call!
	jbsr	_syscall		| Process users system call

|
| Return to the user
|   This portion of code performs 7 actions:
|	1. if runrun is set, reschedule the processor.
|	2. if any timeouts are pending, run them.
|		then, run streams queues
|	3. re-pgm the text and stack base and limit registers.
|	4. restore user registers.
|	5. futz
|	6. return to user via rte.
|
backtouser:
	tstb	_runrun			| Reschedule needed?
	jeq	0$			|  nope

	moveq	#T_RESCHED,d0		| pretend a resched trap occured
	jra	Xresched		|  and let trap() handle it

0$:	movw	#SR_SIPM7,sr		| block interrupts

1$:	tstb	_wantsoftclock		| test if busy or not time to run
	jle	2$			|  yup! so do nothing
	movb	#-1,_wantsoftclock	| mark as busy
	movw	#SR_SIPM0,sr		| now interrupts can occur
	jbsr	_softclock		| go do the work....
	clrb	_wantsoftclock		| mark as not busy and not time to run
	jmp	0$			| try again lest interrupt happened

2$:	tstb	_qrunflag		| do we need to do some streams?
	jle	3$
	tstb	_queueflag		| and are we not busy?
	jne	3$			| no, try again later
	movb	#1,_queueflag		| yes, so make ourself busy
	movw	#SR_SIPM0,sr		| interrupts are ok now
	jbsr	_queuerun		| do it
	clrb	_queueflag
	jmp	0$			| back to check for clock again

3$:
#ifdef	INET
	tstb	_netisrflag		| do we need to run some networks?
	jle	4$			|   nope, its busy or not needed...
	movb	#-1, _netisrflag	| mark as busy
	movw	#SPL0, sr		| Now interrupts can occur
	jbsr	_service_net		| Run network service routines
	clrb	_netisrflag		| Mark as not busy
	jmp	0$			| Start over from the top...
#endif
4$:

|
| Pgm the base and limit register for the text/data area
|
	movw	_cx+CX_TDSIZE,d0	| get the text/data size (power of 2)
	moveq	#PTEPCX,d1		| minimum number of pages in a context
	aslw	d0,d1			| convert size to number of pages
	negw	d1			| convert to correct format for limit
	movw	_cx+CX_TDUSER,d0	| get the users context number
	aslw	#PPCXLOG2,d0		| convert to index into page map

	movw	d0,TDBASE_REG		| set the base register
	movw	d1,TDLMT_REG		| set the limit register

|
| Pgm the base and limit register for the stack area
|
	movw	_cx+CX_SSIZE,d0		| get the stack size (power of 2)
	moveq	#PTEPCX,d1		| minimum number of pages in a context
	aslw	d0,d1			| convert size to number of pages

	movw	_cx+CX_SUSER,d0		| get the users context number
	aslw	#PPCXLOG2,d0		| convert to index into page map
	subqw	#1,d1			| calc stack size for limit reg.
	addw	d1,d0			| calc last pte index number

	movw	d0,STKBASE_REG		| pgm the stack base register
	movw	d1,STKLMT_REG		| pgm the stack limit register

	movl	sp@(FRAME_SP),a0	| get the user stack pointer
	movec	a0,usp			|  and restore it
	moveml	sp@+,#0x7FFF		| restore all the other registers
	addw	#10,sp			| pop off fake-usp, intr handler
					|  address and sr extend word
|
| Now figure out if bus error frame was mangled by trap().  If so, then we
| need to copy the short-frame information to the bottom of the long frame,
| as well as adjust our stack pointer and to repair the vector-offset so
| that the chip won't puke.  Once this is done, the chip will be rte'ing
| through a normal 4 word frame to the users signal catcher.
| Note that because of this, the user will be unable to restart the
| failing instruction.
|
	btst	#6, sp@(6)		| Funny return?
	beq	6$			|   Nope, easy return

	btst	#4,sp@(6)		| short frame?
	bne	7$			|   nope, do long frame offsets!

	movw	sp@,sp@(0x18)		| copy users sr
	movl	sp@(2),sp@(0x1a)	| copy users new pc
	clrw	sp@(0x1e)		| setup a normal 4 word frame format
	addw	#0x18,sp		| remove unused part of former short
					|  frame
	rte
7$:
	movw	sp@,sp@(0x54)		| copy users sr
	movl	sp@(2),sp@(0x56)	| copy users new pc
	clrw	sp@(0x5a)		| setup a normal 4 word frame format
	addw	#0x54,sp		| remove unused part of former long
					|  frame

6$:
	rte				| return from whence called

|
| Return to the system
|   This portion of code performs 3 actions:
|	1. If the O/S was interrupted while priority was 0,
|	   and if any timeouts are pending: run them.
|	2. restore registers
|	3. return to previous system location via rte.
|
backtosystem:
	movw	sp@(FRAME_SR+2),d0	| get the saved sr
	andw	#SR_IPM,d0		| Any priority level bits set?
	jne	4$			|   Yes, skip ast's until later

0$:	movw	#SR_SIPM7,sr		| block clock interrupts...

1$:	tstb	_wantsoftclock		| test if busy or not time to run
	jle	2$			|   yup! so do nothinh
	movb	#-1,_wantsoftclock	| mark as busy
	movw	#SR_SIPM0,sr		| now interrupts can occur
	jsr	_softclock		| go do the work....
	clrb	_wantsoftclock		| mark as busy and not time to run
	jmp	0$

2$:	tstb	_qrunflag		| do we need to do something?
	jle	3$
	tstb	_queueflag		| and are we not busy?
	jne	3$			| no, try again later
	movb	#1,_queueflag		| yes, so make ourself busy
	movw	#SR_SIPM0,sr		| interrupts are ok now
	jbsr	_queuerun		| do it
	clrb	_queueflag
	jmp	0$			| back to check for clock again

3$:
#ifdef	INET
	tstb	_netisrflag		| do we need to run some networks?
	jle	4$			|   nope, its busy or not needed...
	movb	#-1, _netisrflag	| mark as busy
	movw	#SPL0, sr		| Now interrupts can occur
	jbsr	_service_net		| Run network service routines
	clrb	_netisrflag		| Mark as not busy
	jmp	0$			| Start over from the top...
#endif

4$:	movl	sp@(FRAME_SP),a0	| get the user stack pointer
	movec	a0,usp			|  and restore it
	moveml	sp@+,#0x7FFF		| restore all other registers
	addw	#10,sp			| pop off fake-usp, intr handler
					|  address and sr extend word
	rte				| return from whence called

|
| multibus interrupts vector to here. (levels 0 to 7)
| the array _ivectors contain the address of the service routine.  This array
| is set up during autoconfiguration time.
|
Xmbintr01:
	clrw	sp@-			| long extend status register
	movl	_ivectors+4,sp@-	| address of interrupt handler
	jra	call

Xmbintr2:
	clrw	sp@-			| long extend status register
	movl	_ivectors+8,sp@-	| address of interrupt handler
	jra	call

Xmbintr3:
	clrw	sp@-			| long extend status register
	movl	_ivectors+12,sp@-	| address of interrupt handler
	jra	call

Xmbintr4:
	clrw	sp@-			| long extend status register
	movl	_ivectors+16,sp@-	| address of interrupt handler
	jra	call

Xmbintr5:
	clrw	sp@-			| long extend status register
	movl	_ivectors+20,sp@-	| address of interrupt handler
	jra	call

Xmbintr6:
	clrw	sp@-			| long extend status register
	movl	_ivectors+24,sp@-	| address of interrupt handler
	jra	call

Xmbintr7:
	clrw	sp@-			| long extend status register
	movl	_ivectors+28,sp@-	| address of interrupt handler

|
| Common interrupt dispatch
|
call:
	addql	#1,_cnt+V_INTR		| incr interrupt counter

	moveml	#0xFFFF,sp@-		| save all registers
	movec	usp,a0			| get user stack pointer
	movl	a0,sp@(FRAME_SP)	|  and save it in saved area
	movl	sp@(FRAME_INTR),a0	| fetch interrupt handler address
	jsr	a0@			|   and jump to actual interrupt handler
	btst	#5,sp@(FRAME_SR+2)	| did we come from user mode?
	jne	backtosystem		|  no, return to system
	jra	backtouser		|  yes, return to user

|
| Mouse quadrature interrupt service routine
|
	.globl	_mouseintr
_mouseintr:
	movl	d0,sp@- 	| Save d0, we need to use it
	addql	#1,_cnt+V_INTR	| incr interrupt counter

	movw	M_QUAD,d0	| Read mouse quadrature to clear interrupt
|
| first the x translation...
|
	btst	#MOUSE_XFIREBIT_,d0	| X fire bit on? (active low)
	jne	2$			|   nope
	btst	#MOUSE_XBIT,d0		| Positive movement?
	jeq	0$			|   nope
	addqw	#1,__mousex		| Advance mouse 1 tick in x
	jra	2$
0$:	subqw	#1,__mousex		| Retreat mouse 1 tick in x
|
| Now the y translation...
|
2$:
	btst	#MOUSE_YFIREBIT_,d0	| Y fire bit on? (active low)
	jne	8$			|   nope, all done
	btst	#MOUSE_YBIT,d0		| Positive movement?
	jeq	4$			|   nope
	addqw	#1,__mousey		| Advance mouse 1 tick in y
	jra	8$
4$:	subqw	#1,__mousey		| Retreat mouse 1 tick in y

8$:	movl	sp@+,d0			| Restore d0
	rte
|
| Duart 0, ports 0 and 1 vector to here
|
Xduart0:
	clrw	sp@-			| long extend status register
	movl	#0,sp@-			| save duart number, in place of
					|  intr handler address

ducom:
	addql	#1,_cnt+V_INTR		| incr interrupt counter

	moveml	#0xFFFF,sp@-		| save all registers
	movec	usp,a0			| get user stack pointer
	movl	a0,sp@(FRAME_SP)	|  and save it in saved area

	movl	sp@(FRAME_INTR),sp@-	| arg is duart number
	jsr	_duintr			| now jump to the duart handler
	addql	#4,sp			| done with the arg

	btst	#5,sp@(FRAME_SR+2)	| did we come from user mode?
	jne	backtosystem		|  no, return to system
	jra	backtouser		|  yes, return to user

|
| Duart 1, ports 2 and 3 vector to here
|
Xduart1:
	clrw	sp@-			| long extend status register
	movl	#2,sp@-			| save duart number, in place of
					|  intr handler address
	jra	ducom			| go to the common code

|
| Clock vectors to here (MC146818A)
|
Xclock:
	clrw	sp@-			| long extend status register
	clrl	sp@-			| address of intr handler (not used)

	addql	#1,_cnt+V_INTR		| incr interrupt counter

	moveml	#0xFFFF,sp@-		| save all registers
	movec	usp,a0			| get user stack pointer
	movl	a0,sp@(FRAME_SP)	|  and save it in saved area

	movl	sp@(FRAME_PC), sp@-	| Push pc to top of frame
	movl	sp@(FRAME_SR+4), sp@-	| Push sr to top of frame
	jsr	_hardclock		| handle clock intr- machine independent
	addql	#8, sp

	jsr	_todintrclr		| reset intr latch - machine dependent

	btst	#5,sp@(FRAME_SR+2)	| did we come from user mode?
	jne	backtosystem		|  no, return to system
	jra	backtouser		|  yes, return to user


|
| context switching routines
|

| save( savebuf )
| label_t	*savebuf;
|
|   save the registers d2-d7, a2-a7 and the return address.
|   This function always returns a ZERO.
|
_save:
	movl	sp@+,a1		| get the return address
	movl	sp@,a0		| get the supplied arg
	moveml	#0xFCFC,a0@	| save d2-d7, a2-a7 in the save area
	movl	a1,a0@(48)	| save return address in the save area
				|  (the offset 48 should be generated by
				|  genassym)
	moveq	#0,d0		| always return a ZERO
	jmp	a1@		| return

|
| qsave( savebuf )
| label_t	*savebuf;
|
|   Like save but only saves enough to implement a non-local goto
|   NOTE: if you return to a routine that has register variables,
|         it will NOT work.
|
_qsave:
	movl	sp@+,a1		| get the return address
	movl	sp@,a0		| get the supplied arg
	addw	#40,a0		| get to correct location in save area
				|  (the offset 40 should be generated by
				|  genassym)
	movl	a6,a0@+		| save a6
	movl	a7,a0@+		| save a7
	movl	a1,a0@+		| save return address
	moveq	#0,d0		| always return a ZERO.
	jmp	a1@		| return

|
| resume( udotpage, savebuf )
| ushort	udotpage;
| label_t	*savebuf;
|
|   restore the registers and stack and map to the given udot.
|   This function returns to where the save()/qsave() associated
|   with the supplied save buffer was issued.  This function
|   always returns a ONE.
|
_resume:
	moveq	#CACHE_CE,d0		| Clear out cache, since we are
	movec	d0,cacr			|   running a new user process
	clrl	d0			| start with a clean register
	movw	sp@(6),d0		| page of the new udot
	orl	#UDOTPTE,d0		| insert the page permissions
	movl	sp@(8),a0		| ptr to label_t
	movw	#SR_SIPM7,sr		| no intrs - cpu
	andw	#ST_INTROFF,STATUS_REG	| no intrs - board

	movl	d0,UDOT_PTMAP_OFF	| set up new udot page table entry
	moveml	a0@+,#0xFCFC		| restore the registers

	movw	#SR_SIPM0,sr		| intrs are now okay - cpu
	orw	#ST_INTRON,STATUS_REG	| 		     - board

	movl	a0@,a1			| fetch the original pc
	moveq	#1,d0			| return 1
	jmp	a1@			| and away we go

|
| Copy a page of memory:
|	void bcopyPAGE(from, to);
|	char *from, *to;
|
	.text
	.globl	_bcopyPAGE
_bcopyPAGE:
#ifdef	PROF
	movl	#.LbcopyPAGE, a0
	jsr	mcount
	.bss
.LbcopyPAGE:
	.space	4
	.text
#endif
	movl	sp@(4), a0	| a0 := from
	movl	sp@(8), a1	| a1 := to
	movw	#16-1, d0	| # of 256 byte chunks to move
	addw	#4096, a0	| a0 := end of region to copy from
	addw	#4096, a1	| a1 := end of region to copy to
1$:
	movl	a0@-, a1@-	| 000-015
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 016-031
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 032-047
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 048-063
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 064-079
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 080-095
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 096-111
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 112-127
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 128-143
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 144-159
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 160-175
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 176-191
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 192-207
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 208-223
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 224-239
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 240-255
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	dbra	d0, 1$
	rts
|
| Clear out one page of memory, as fast as we can
|
	.text
_bzeroPAGE:
#ifdef	PROF
	movl	#.LbzeroPAGE, a0
	jsr	mcount
	.bss
.LbzeroPAGE:
	.space	4
	.text
#endif
	movl	sp@(4), a0	| Address of region to clear
	addw	#4096, a0	| Point to last byte + 1

	moveml	#0x3F3E, sp@-	| Save registers
	moveml	zeros, #0x7EFF	| Get a bunch of zeros into the registers

	moveml	#0xFF7E, a0@-	| 56 bytes done
	moveml	#0xFF7E, a0@-	| ...112 bytes
	moveml	#0xFF7E, a0@-	| ...168 bytes
	moveml	#0xFF7E, a0@-	| ...224 bytes
	moveml	#0xFF7E, a0@-	| ...280 bytes
	moveml	#0xFF7E, a0@-	| ...336 bytes
	moveml	#0xFF7E, a0@-	| ...392 bytes
	moveml	#0xFF7E, a0@-	| ...448 bytes
	moveml	#0xFF7E, a0@-	| ...504 bytes
	moveml	#0xFF7E, a0@-	| ...560 bytes
	moveml	#0xFF7E, a0@-	| ...616 bytes
	moveml	#0xFF7E, a0@-	| ...672 bytes
	moveml	#0xFF7E, a0@-	| ...728 bytes
	moveml	#0xFF7E, a0@-	| ...784 bytes
	moveml	#0xFF7E, a0@-	| ...840 bytes
	moveml	#0xFF7E, a0@-	| ...896 bytes
	moveml	#0xFF7E, a0@-	| ...952 bytes
	moveml	#0xFF7E, a0@-	| ...1008 bytes

	moveml	#0xFF7E, a0@-	| 56 bytes done
	moveml	#0xFF7E, a0@-	| ...112 bytes
	moveml	#0xFF7E, a0@-	| ...168 bytes
	moveml	#0xFF7E, a0@-	| ...224 bytes
	moveml	#0xFF7E, a0@-	| ...280 bytes
	moveml	#0xFF7E, a0@-	| ...336 bytes
	moveml	#0xFF7E, a0@-	| ...392 bytes
	moveml	#0xFF7E, a0@-	| ...448 bytes
	moveml	#0xFF7E, a0@-	| ...504 bytes
	moveml	#0xFF7E, a0@-	| ...560 bytes
	moveml	#0xFF7E, a0@-	| ...616 bytes
	moveml	#0xFF7E, a0@-	| ...672 bytes
	moveml	#0xFF7E, a0@-	| ...728 bytes
	moveml	#0xFF7E, a0@-	| ...784 bytes
	moveml	#0xFF7E, a0@-	| ...840 bytes
	moveml	#0xFF7E, a0@-	| ...896 bytes
	moveml	#0xFF7E, a0@-	| ...952 bytes
	moveml	#0xFF7E, a0@-	| ...1008 bytes

	moveml	#0xFF7E, a0@-	| 56 bytes done
	moveml	#0xFF7E, a0@-	| ...112 bytes
	moveml	#0xFF7E, a0@-	| ...168 bytes
	moveml	#0xFF7E, a0@-	| ...224 bytes
	moveml	#0xFF7E, a0@-	| ...280 bytes
	moveml	#0xFF7E, a0@-	| ...336 bytes
	moveml	#0xFF7E, a0@-	| ...392 bytes
	moveml	#0xFF7E, a0@-	| ...448 bytes
	moveml	#0xFF7E, a0@-	| ...504 bytes
	moveml	#0xFF7E, a0@-	| ...560 bytes
	moveml	#0xFF7E, a0@-	| ...616 bytes
	moveml	#0xFF7E, a0@-	| ...672 bytes
	moveml	#0xFF7E, a0@-	| ...728 bytes
	moveml	#0xFF7E, a0@-	| ...784 bytes
	moveml	#0xFF7E, a0@-	| ...840 bytes
	moveml	#0xFF7E, a0@-	| ...896 bytes
	moveml	#0xFF7E, a0@-	| ...952 bytes
	moveml	#0xFF7E, a0@-	| ...1008 bytes

	moveml	#0xFF7E, a0@-	| 56 bytes done
	moveml	#0xFF7E, a0@-	| ...112 bytes
	moveml	#0xFF7E, a0@-	| ...168 bytes
	moveml	#0xFF7E, a0@-	| ...224 bytes
	moveml	#0xFF7E, a0@-	| ...280 bytes
	moveml	#0xFF7E, a0@-	| ...336 bytes
	moveml	#0xFF7E, a0@-	| ...392 bytes
	moveml	#0xFF7E, a0@-	| ...448 bytes
	moveml	#0xFF7E, a0@-	| ...504 bytes
	moveml	#0xFF7E, a0@-	| ...560 bytes
	moveml	#0xFF7E, a0@-	| ...616 bytes
	moveml	#0xFF7E, a0@-	| ...672 bytes
	moveml	#0xFF7E, a0@-	| ...728 bytes
	moveml	#0xFF7E, a0@-	| ...784 bytes
	moveml	#0xFF7E, a0@-	| ...840 bytes
	moveml	#0xFF7E, a0@-	| ...896 bytes
	moveml	#0xFF7E, a0@-	| ...952 bytes
	moveml	#0xFF7E, a0@-	| ...1008 bytes

	moveml	#0xFF7E, a0@-	| Pick up remaining bytes (56 here)
	moveml	#0xC000, a0@-	|   and last 8 (total of 4096)

	moveml	sp@+,#0x7CFC	|give me back the registers

	rts			|that's it
zeros:	.long	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0
|
| Clear out some number of bb's worth of memory, as fast as we can
|
	.text
	.globl	_bzeroBBS
_bzeroBBS:
#ifdef	PROF
	movl	#.LbzeroBBS, a0
	jsr	mcount
	.bss
.LbzeroBBS:
	.space	4
	.text
#endif
	movl	sp@(4), a0	| Address of region to clear
	tstl	sp@(8)		| Any bb's to do?
	jeq	99$		| Oops, nothing to do
	moveml	#0x3F3E, sp@-	| Save registers
	moveml	zeros, #0x7EFF	| Get a bunch of zeros into the registers

2$:	addw	#512, a0	| Point to last byte + 1

	moveml	#0xFF7E, a0@-	| 56 bytes done
	moveml	#0xFF7E, a0@-	| ...112 bytes
	moveml	#0xFF7E, a0@-	| ...168 bytes
	moveml	#0xFF7E, a0@-	| ...224 bytes
	moveml	#0xFF7E, a0@-	| ...280 bytes
	moveml	#0xFF7E, a0@-	| ...336 bytes
	moveml	#0xFF7E, a0@-	| ...392 bytes
	moveml	#0xFF7E, a0@-	| ...448 bytes
	moveml	#0xFF7E, a0@-	| ...504 bytes
	moveml	#0xC000, a0@-	| pick up last 8 bytes (total of 512)
	addw	#512, a0	| Advance past cleared data
	subql	#1, sp@(52)	| Reduce count by 1
	jne	2$		| If non-zero, do next bb

	moveml	sp@+,#0x7CFC	|give me back the registers
99$:	rts			|that's it
|
| Adjust processor priority routines
|
| spl0()
|   set interrupt priority mask to 0. Returns old status register value.
|
_spl0:
	movw	sr,d0
	movw	#SR_SU,sr
	rts

|
| spl1()
|   set interrupt priority mask to 1. Returns old status register value.
|
_spl1:
	movw	sr,d0
	movw	#SR_SU+SR_IPM1,sr
	rts

|
| spl2()
|   set interrupt priority mask to 2. Returns old status register value.
|
_spl2:
	movw	sr,d0
	movw	#SR_SU+SR_IPM2,sr
	rts

|
| spl3()
|   set interrupt priority mask to 3. Returns old status register value.
|
_spl3:
	movw	sr,d0
	movw	#SR_SU+SR_IPM3,sr
	rts

|
| spl4()
|   set interrupt priority mask to 4. Returns old status register value.
|
_spl4:
	movw	sr,d0
	movw	#SR_SU+SR_IPM4,sr
	rts

|
| spl5()
|   set interrupt priority mask to 5. Returns old status register value.
|
_spl5:
	movw	sr,d0
	movw	#SR_SU+SR_IPM5,sr
	rts

|
| spl6()
| spltty()
| spl7()
|   set interrupt priority mask to 6. Returns old status register value.
|
_spl6:
_spltty:
_spl7:
	movw	sr,d0
	movw	#SR_SU+SR_IPM6,sr
	rts

|
| splmax()
|   set interrupt priority mask to highest possible value (7). Returns
|   old status register value.
|
_splmax:
	movw	sr,d0
	movw	#SR_SU+SR_IPM7,sr
	rts

|
| splx( sr )
| ushort	sr;
|
|   set processor status register to supplied value.  Normally
|   used for setting the interrupt priority mask.
|
_splx:
	movl	sp@(4),d0
	movw	d0,sr
	rts

|
| misc routines
|

|
| debug()
|   Entry point for kernel debugger.  Sets up debug stack, bumps up
|   priority then calls the routine that does the real work.
|   NOTE: the debugger stack starts at the end of the static ram
|
_debug:	link	a6, #0
#if NDEBUG > 0
	movl	sp,_savessp		| Save system sp before switching
	movl	a6,_savesfp		| Save system fp before switching
	movl	#SRAM_BASE+SRAM_SZ,sp	| Point to base of stack
	moveml	#0xFFFF,sp@-		| Save kernel regs on new stack
	moveq	#0,d0			| Zero extend the
	movw	sr,d0			|   status register
	movl	d0,sp@-			| Save status register on stack
	movl	_savessp,a0
	movl	a0@(8),sp@-

	movw	#SR_SIPM7,sr		| Bump priority up...

| should board interrupts be disabled????

	jsr	_kdb			| Call kernal debugger

	addql	#4,sp			| remove the arg
	movl	sp@+,d0			| Pop priority
	addql	#4,sp
	moveml	sp@+,#0xFFFE		| Restore all other registers
	movl	_savessp,sp		| Restore stack
	movw	d0,sr			| lastly, restore priority
#endif
	unlk	a6
	rts				| Back from whence we came

|
| doboot()
|    assist code for the reboot system call.
|
_doboot:
	movl	_rebootvec,a0		| Get prom reboot location
	movw	#SR_SIPM7,sr		| no intrs - cpu
	andw	#ST_INTROFF,STATUS_REG	| no intrs - board

	moveq	#CACHE_CLR,d0		| clear the cache but disable it
	.word	0x4e7b
	.word	0x0002

	jmp	a0@

|
| halt()
|   Halt the system.  68000's don't have a real halt instruction (besides a
|   double bus error), so we loop by stopping at the hightest priority
|   in supervisor mode.
|
_halt:
	stop	#SR_SIPM7
	jra	_halt

#ifdef	PROF
|
| Profiling support code
|
	.globl	_profiling, _proflevel7
	.text
_proflevel7:
	tstw	M_QUAD		| Clear interrupt
	tstb	_profiling	| Profiling?
	jeq	0$		|   nope, return
	btst	#5, sp@		| Were we in kernel mode?
	jne	1$		|   yes, do normal thing
|
| In user mode.  Increment profiling pseudo address "_user"
|
	movl	a0, sp@-
	movl	_profbuf, a0
	addql	#1, a0@		| Increment "user" counter
	movl	sp@+, a0
	rte

1$:	movl	d0, sp@-
	movl	a0, sp@-	| Save these registers
	movl	sp@(10), d0	| Get pc
	andl	#PCMASK, d0	| Mask out uninteresting bits
	lsrl	#PROFSHIFT-2, d0| Reduce to buffer slot # (*4 for long slots)
	movl	_profbuf, a0	| Get pointer to buffer
	addl	d0, a0		| Advance to slot
	addql	#1, a0@		| Bump counter
	movl	sp@+, a0
	movl	sp@+, d0	| Restore these registers
0$:	rte
|
| Kernel version of mcount and smcount
|
	.globl	mcount, smcount

mcount:	movl	a0@,d0		| fetch contents of LABEL
	jne	1$		| something there
	movl	_mcountbuf,d1	| See if monitoring is set up yet
	jeq	3$		| nope, its zero still
	movl	d1,a1		| ptr to next available cnt structure
	addl	#8,_mcountbuf	| bump cntbase to followng structure
	movl	sp@,a1@+	| save ptr to function
	movl	a1,a0@		| save ptr to cnt in structure in LABEL
	jra	2$

1$:	movl	d0,a1		| so we can use it as a ptr
2$:	addql	#1,a1@		| bump the function count
3$:	rts
|
| Version of mcount which takes its address argument from the stack AND
| saves all registers (including temporaries)
|
smcount:
	moveml	#0xC0C0,sp@-	| Save d0,d1,a0, and a1
	movl	sp@(20),a0

	movl	a0@,d0		| fetch contents of LABEL
	jne	1$		| something there
	movl	_mcountbuf,d1	| See if monitoring is set up yet
	jeq	3$		| nope, its zero still
	movl	d1,a1		| ptr to next available cnt structure
	addl	#8,_mcountbuf	| bump cntbase to followng structure
	movl	sp@(16),a1@+	| save ptr to function
	movl	a1,a0@		| save ptr to cnt in structure in LABEL
	jra	2$

1$:	movl	d0,a1		| so we can use it as a ptr
2$:	addql	#1,a1@		| bump the function count

3$:	moveml	sp@+,#0x0303	| Restore d0,d1,a0, and a1
	rts
#endif
|
| Bit field operators
|

|
| bset:
|	- Set a bit
|
	.globl	_bset
_bset:
	movl	sp@(4), a0	| get base of bitmap
	movl	sp@(8), d0	| get bit number
	movl	d0, d1
	asrl	#3, d1		| Convert d1 to a byte offset
	bset	d0, a0@(0,d1:l)	| Set the bit
	rts
|
| bclr:
|	- Clear a bit.
|
	.globl	_bclr
_bclr:
	movl	sp@(4), a0	| get base of bitmap
	movl	sp@(8), d0	| get bit number
	movl	d0, d1
	asrl	#3, d1		| Convert d1 to a byte offset
	bclr	d0, a0@(0,d1:l)	| Clear the bit
	rts
|
| btst:
|	- Test a bit.  Return 1 if the bit is set, 0 otherwise.
|
	.globl	_btst
_btst:
	movl	sp@(4), a0	| get base of bitmap
	movl	sp@(8), d0	| get bit number
	movl	d0, d1
	asrl	#3, d1		| Convert d1 to a byte offset
	btst	d0, a0@(0,d1:l)	| Is the bit set?
	jne	1$		|  yes
	moveq	#0, d0		| Bit is clear, so return a 0
	rts
1$:	moveq	#1, d0		| Bit is set, so return a 1
	rts
|
| bfset:
|	- Set a bit field
|
	.globl	_bfset
_bfset:
#ifdef	PROF
	movl	#.Lbfset, a0
	jsr	mcount
	.bss
.Lbfset:
	.space	4
	.text
#endif
	movl	sp@(4), a0	| get base of bitmap
	movl	sp@(8), d0	| get bit number
	movl	sp@(12), d1	| get number of bits to set
	tstl	d1		| Any bits to set at all?
	jeq	3$		| nope

	movl	d2, sp@-	| save d2

1$:	movl	d0, d2		| Copy bit number
	asrl	#3, d2		| Convert d2 to a byte offset
	bset	d0, a0@(0,d2:l)	| Set the bit
	addql	#1, d0		| Advance bit position
	subql	#1, d1		| Reduce bit count by 1
	jne	1$		| Loop if there are more bits left

2$:	movl	sp@+, d2	
3$:	rts
|
| bfclr:
|	- Clear a bit field
|
	.globl	_bfclr
_bfclr:
#ifdef	PROF
	movl	#.Lbfclr, a0
	jsr	mcount
	.bss
.Lbfclr:
	.space	4
	.text
#endif
	movl	sp@(4), a0	| get base of bitmap
	movl	sp@(8), d0	| get bit number
	movl	sp@(12), d1	| get number of bits to clear
	tstl	d1		| Any bits to clear at all?
	jeq	3$		| nope

	movl	d2, sp@-	| save d2

1$:	movl	d0, d2		| Copy bit number
	asrl	#3, d2		| Convert d2 to a byte offset
	bclr	d0, a0@(0,d2:l)	| Set the bit
	addql	#1, d0		| Advance bit position
	subql	#1, d1		| Reduce bit count by 1
	jne	1$		| Loop if there are more bits left

2$:	movl	sp@+, d2	
3$:	rts
|
| bftstset:
|	- test a bit field of length len in bitmap bp starting at b
|	  to be all clear
|	- return a count of the number of set bits found
|
	.globl	_bftstset
_bftstset:
#ifdef	PROF
	movl	#.Lbftstset, a0
	jsr	mcount
	.bss
.Lbftstset:
	.space	4
	.text
#endif
	movl	sp@(4), a0	| get base of bitmap
	movl	sp@(8), d0	| get bit number
	movl	sp@(12), d1	| get number of bits to test
	tstl	d1		| Any bits to test at all?
	jeq	3$		| nope

	moveml	#0x3800, sp@-	| push registers (d2, d3, d4)
	moveq	#0, d3		| d3 will count the number of set bits we hit
	moveq	#1, d4		| d4 holds a constant 1

	cmpl	#16, d1		| enough to do big loop?
	jge	5$		| yes

4$:	movl	d0, d2		| Copy bit number
	asrl	#3, d2		| Convert d2 to a byte offset
	btst	d0, a0@(0,d2:l)	| Test the bit
	jeq	2$		| Bit is clear, stop loop
	addl	d4, d3		| Advance count of set bits
	addl	d4, d0		| Advance bit position
	subl	d4, d1		| Reduce bit count by 1
	jne	4$		| Loop if there are more bits left

2$:	movl	d3, d0
	moveml	sp@+, #0x001C	| pop registers (d4, d3, d2)
3$:	rts
|
| Special version of loop.  Check for various bit aligments to do mass
| checking.  We only bother to do this if the number of bits we are
| examining is fairly large (so that the extra overhead becomes noise).
|
5$:	movl	d0, d2		| Get bit number
	andb	#0x1F, d2	| see if low 5 bits are clear
	jeq	32$		| yes they are - do a long at a time
	andb	#0xF, d2	| See if low 4 bits are clear
	jeq	16$		| yes they are - do a short at a time
	andb	#7, d2		| See if low 3 bits are clear
	jeq	8$		| yes they are - do a byte at a time
|
| bit at a time loop
|
1$:	movl	d0, d2		| Copy bit number
	asrl	#3, d2		| Convert d2 to a byte offset
	btst	d0, a0@(0,d2:l)	| Test the bit
	jeq	2$		| Bit is clear, stop loop
	addl	d4, d3		| Advance count of set bits
	addl	d4, d0		| Advance bit position
	subl	d4, d1		| Reduce bit count by 1
	jne	5$		| Loop if there are more bits left
	jra	2$
|
| byte at a time loop
|
8$:	cmpl	#8, d1		| More than 8 bits left to check?
	jlt	1$		| nope
	movl	d0, d2		| Get bit number
	asrl	#3, d2		| Convert d2 to a byte offset
	cmpb	#0xFF, a0@(0,d2:l)	| Check entire byte
	jne	1$		| Somethings clear, do things the hard way
	addql	#8, d3		| 8 more set bits
	addql	#8, d0		| Advance 8 bits
	subql	#8, d1		| Reduce bit count by 8
	jne	8$		| Check for 8 more bits
	jra	2$
|
| short at a time loop
|
16$:	cmpl	#16, d1		| More than 16 bits left to check?
	jlt	8$		| nope - check for 8 left
	movl	d0, d2		| Get bit number
	asrl	#3, d2		| Convert d2 to a byte offset
	cmpw	#0xFFFF, a0@(0,d2:l)	| Check entire word
	jne	8$		| Somethings clear, do things the hard way
	moveq	#16, d4		| temporarily set d4 to 16
	addl	d4, d3		| 16 more set bits
	addl	d4, d0		| Advance 16 bits
	subl	d4, d1		| Reduce bit count by 16
	moveq	#1, d4		| Restore d4 to 1
	tstl	d1
	jne	16$		| Check for 16 more bits
	jra	2$
|
| long at a time loop
|
32$:	cmpl	#32, d1		| More than 32 bits left to check?
	jlt	16$		| nope - check for 16 left
	movl	d0, d2		| Get bit number
	asrl	#3, d2		| Convert d2 to a byte offset
	cmpl	#0xFFFFFFFF, a0@(0,d2:l)	| Check entire long
	jne	16$		| Somethings clear, do things the hard way
	moveq	#32, d4		| temporarily set d4 to 32
	addl	d4, d3		| 32 more set bits
	addl	d4, d0		| Advance 32 bits
	subl	d4, d1		| Reduce bit count by 32
	moveq	#1, d4		| Restore d4 to 1
	tstl	d1
	jne	32$		| Check for 32 more bits
	jra	2$
|
| bftstclr:
|	- test a bit field of length len in bitmap bp starting at b
|	  to be all clear
|	- return a count of the number of set bits found
|
	.globl	_bftstclr
_bftstclr:
#ifdef	PROF
	movl	#.Lbftstclr, a0
	jsr	mcount
	.bss
.Lbftstclr:
	.space	4
	.text
#endif
	movl	sp@(4), a0	| get base of bitmap
	movl	sp@(8), d0	| get bit number
	movl	sp@(12), d1	| get number of bits to test
	tstl	d1		| Any bits to test at all?
	jeq	3$		| nope

	moveml	#0x3800, sp@-	| push registers (d2, d3, d4)
	moveq	#0, d3		| d3 will count the number of clear bits we hit
	moveq	#1, d4		| d4 holds a constant 1

	cmpl	#16, d1		| enough to do big loop?
	jge	5$		| yes

4$:	movl	d0, d2		| Copy bit number
	asrl	#3, d2		| Convert d2 to a byte offset
	btst	d0, a0@(0,d2:l)	| Test the bit
	jne	2$		| Bit is set, stop loop
	addl	d4, d3		| Advance count of clear bits
	addl	d4, d0		| Advance bit position
	subl	d4, d1		| Reduce bit count by 1
	jne	4$		| Loop if there are more bits left

2$:	movl	d3, d0
	moveml	sp@+, #0x001C	| pop registers (d4, d3, d2)
3$:	rts
|
| Special version of loop.  Check for various bit aligments to do mass
| checking.  We only bother to do this if the number of bits we are
| examining is fairly large (so that the extra overhead becomes noise).
|
5$:	movl	d0, d2		| Get bit number
	andb	#0x1F, d2	| see if low 5 bits are clear
	jeq	32$		| yes they are - do a long at a time
	andb	#0xF, d2	| See if low 4 bits are clear
	jeq	16$		| yes they are - do a short at a time
	andb	#7, d2		| See if low 3 bits are clear
	jeq	8$		| yes they are - do a byte at a time
|
| bit at a time loop
|
1$:	movl	d0, d2		| Copy bit number
	asrl	#3, d2		| Convert d2 to a byte offset
	btst	d0, a0@(0,d2:l)	| Test the bit
	jne	2$		| Bit is set, stop loop
	addl	d4, d3		| Advance count of clear bits
	addl	d4, d0		| Advance bit position
	subl	d4, d1		| Reduce bit count by 1
	jne	5$		| Loop if there are more bits left
	jra	2$
|
| byte at a time loop
|
8$:	cmpl	#8, d1		| More than 8 bits left to check?
	jlt	1$		| nope
	movl	d0, d2		| Get bit number
	asrl	#3, d2		| Convert d2 to a byte offset
	tstb	a0@(0,d2:l)	| Check entire byte
	jne	1$		| Somethings set, do things the hard way
	addql	#8, d3		| 8 more clear bits
	addql	#8, d0		| Advance 8 bits
	subql	#8, d1		| Reduce bit count by 8
	jne	8$		| Check for 8 more bits
	jra	2$
|
| short at a time loop
|
16$:	cmpl	#16, d1		| More than 16 bits left to check?
	jlt	8$		| nope - check for 8 left
	movl	d0, d2		| Get bit number
	asrl	#3, d2		| Convert d2 to a byte offset
	tstw	a0@(0,d2:l)	| Check entire word
	jne	8$		| Somethings set, do things the hard way
	moveq	#16, d4		| temporarily set d4 to 16
	addl	d4, d3		| 16 more clear bits
	addl	d4, d0		| Advance 16 bits
	subl	d4, d1		| Reduce bit count by 16
	moveq	#1, d4		| Restore d4 to 1
	tstl	d1
	jne	16$		| Check for 16 more bits
	jra	2$
|
| long at a time loop
|
32$:	cmpl	#32, d1		| More than 32 bits left to check?
	jlt	16$		| nope - check for 16 left
	movl	d0, d2		| Get bit number
	asrl	#3, d2		| Convert d2 to a byte offset
	tstl	a0@(0,d2:l)	| Check entire long
	jne	16$		| Somethings set, do things the hard way
	moveq	#32, d4		| temporarily set d4 to 32
	addl	d4, d3		| 32 more clear bits
	addl	d4, d0		| Advance 32 bits
	subl	d4, d1		| Reduce bit count by 32
	moveq	#1, d4		| Restore d4 to 1
	tstl	d1
	jne	32$		| Check for 32 more bits
	jra	2$
#ifdef	INET
|
| __insque:
|	- insert ``entry'' onto queue following ``pred''
|
	.globl	__insque, __remque
	.text
__insque:
	movl	sp@(4), a0	| entry
	movl	sp@(8), a1	| pred
	movl	a2, sp@-	| Save a2

	movl	a1@, a2		| save forward link of predecessor
	movl	a1, a0@(4)	| set backward link of entry
	movl	a0, a2@(4)	| set backward link of successor
	movl	a2, a0@		| set forward link of entry
	movl	a0, a1@		| set forward link of predecessor

	movl	sp@+, a2	| Restore a2
	rts
|
| __remque:
|	- remove ``entry'' from its queue
|
__remque:
	movl	sp@(4), a0	| entry

	movl	a0@(4), a1	| a1 points to predecessor
	movl	a0@, a1@	| set forward link of predecessor
	movl	a0@, a1		| a1 points to successor
	movl	a0@(4), a1@(4)	| set backward link of successor
	rts
#endif
