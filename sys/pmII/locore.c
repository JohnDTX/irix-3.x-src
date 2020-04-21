|
| Machine dependent start up code for pmII cpu
|
| Written by: Kipp Hickman
|
| $Source: /d2/3.7/src/sys/pmII/RCS/locore.c,v $
| $Revision: 1.1 $
| $Date: 89/03/27 17:33:40 $
|

#include "../h/kprof.h"
#include "../pmII/cpureg.h"
#include "../pmII/cx.h"
#include "../pmII/trap.h"
#include "../pmII/vmparam.h"
#include "../pmII/frame.h"

	.globl	start, _end, _edata, _main
	.globl	__mousex, __mousey, _mousebusy
	.globl	backtosystem, backtouser
#ifdef	GL2
	.globl	__dcrflags, __dcrmodes
#endif

|
| User structure dependencies
|
PAGESHIFT= 12			| Shift count for converting bytes to pages

|
| Status register values for changing interrupt priority level
|
SPL7	= 0x2700		| High priority supervisor mode
SPL0	= 0x2000		| Low priority, supervisor mode

|
| STACK is the address in virtual memory of where the kernel will put its
| stack for short periods of time, during initialization, AFTER we have
| properly moved the kernel in virtual space from 0 to KERN_VBASE.
|
| TMPSTACK is used prior to stack, while we are moving ourselves to KERN_VBASE.
|
TMPSTACK= 0x0003FE
STACK	= KERN_VBASE + TMPSTACK

|
| Address's of some of the duart registers
|
DUISR	= DUART1_VBASE+0xA
DUCCSTP	= DUART1_VBASE+0x1E

|
| Define virtual address's of all mapped goodies.  See ``cpureg.h'' for
| defintions of the virtual space.  We declare the pte space's in
| machdep.c.
|
	.data
	.globl	_usrpt, _forkutl, _xswaputl, _xswap2utl, _swaputl
	.globl	_pushutl, _vfutl, _vmmap, _u
_usrpt		= USRPT_VBASE
_forkutl	= FORKUTL_VBASE
_xswaputl	= XSWAPUTL_VBASE
_xswap2utl	= XSWAP2UTL_VBASE
_swaputl	= SWAPUTL_VBASE
_pushutl	= PUSHUTL_VBASE
_vfutl		= VFUTL_VBASE
_vmmap		= DEVMEM_VBASE
_u		= UDOT_VBASE

#ifdef	PROF
	.globl	_user
_user		= KERN_VBASE
#endif
	.text
|
| We start things going here...
|
start:	movl	#TMPSTACK, sp		| For the mouse...
	movw	#SPL7, sr		| spl7
	andw	#0xFFDF, STATUS		| Disable parity detection
|
| Clear out all of the page map, except for the first 1 meg (where
| we are currently running.  The pagemap clearing done here and below
| is done purely for making crash dumps easier to debug
|
	movl	#4096 - 256 - 1, d0	| # of pages to clear - 1
	movl	#PAGEBASE+0x200, a0	| First unused word in page map
	movl	#PROTBASE+0x200, a1	| First unused word in protection map
1$:	clrw	a0@+			| Zero out the indirection info
	movw	#PR_INVALID, a1@+	|   as well as the protection info
	dbra	d0, 1$

|
| Next, copy the page table info into the new kernel area.  We actually edit
| the protection information as we copy it, to include the new kernel context
| (KCX).  The idea here is straightforward.  The kernel is addressed to run
| at 0xC004xx (and larger);  but it is loaded via the boot proms at address
| 0x0004xx (and larger).  The kernel also, when its ready to execute C
| code, must be running in context 0x20.  To accomplish this we make two
| copies of the lower 1Mb of the page map;  the first copy is stored at
| the very top of the page map (0xCxxxxx ^ (0x20 << 16) --> 0xExxxxx), and has
| its protection information modified to include the fact that context 20 is
| going to be used;  the other copy is stored at 0xCxxxxx (one to one with the
| kernels address's) so that we can jump from the startup code (which will have
| a low pc (< 0x1000) to a real pc (0xC00400 < pc < 0xC01000). Ick.
|
	movl	#256-1, d0		| # of pages - 1
	movl	#PAGEBASE, a0		| Old base of page map
	movl	#PROTBASE, a1		| Old base of protection map
	movl	#PAGEBASE+KV_KCX, a2	| Base of page map
	movl	#PROTBASE+KV_KCX, a3	| Base of protection map
	movl	#PAGEBASE+KV_CX0, a4	| Copy at Cxx in the page map
	movl	#PROTBASE+KV_CX0, a5	| Copy at Cxx in the protection map

2$:	movw	a0@, a2@+
	movw	a0@+, a4@+		| Copy page indirection info
	movw	a1@+, d2		| Get protection info
	movw	d2, a5@+		|   (copy untouched for copy at Cxx)
	orw	#KCX, d2		| Add in new context bits for
	movw	d2, a3@+		|   the target area
	dbra	d0, 2$
|
| Just before we change kernel context's, save away any prom state that
| we might need
|
#ifdef	GL2
|
| Save away any prom state we might want
|
	movb	0x23e, d4		| Save dcr modes
	movw	0x228, d5		| Save dcr flags
#endif

|
| Now switch contexts to the newly mapped area.  We can now use the kernel
| absolute address's.  This is the tricky part, as far as the mouse is
| concerned.  The window of vulnerability here is small (< 5 instructions)
| but none the less...The goal of this section of the code is to
| make the code run, even if the mouse interrupts.
|
| Next, map the vector info up at its new virutal location (in both the
| temporary copy and the final copy).
|
| Now, switch the stack pointer to point to the real virtual address
| (but still going through the temporary copy).  Also set up the vbr
| register to point to its new base address.
|
	movw	#0, PAGEBASE + VBR_KCX	| Copy vbr pte info into final copy
	movw	#0x2200+KCX, PROTBASE + VBR_KCX
	movw	#0, PAGEBASE + VBR_CX0		| Copy vbr pte info into
	movw	#0x2200, PROTBASE + VBR_CX0	|   into temporary copy (Cxx)
	movl	#STACK, sp		| Setup real stack pointer...
	movl	#IVEC_VBASE, d0		| Setup vbr
	movec	d0, vbr

	jmp	99$			| Leap up to KERN_VBASE area...
99$:	movb	#KCX, CONTEXT		| And away we go...
|
| At this point, we are running in the correct kernel context (KCX) and
| the correct virtual address.
|
| Now clear out the old map area (first 1 meg + copy at 0xCxx)
|
	movl	#256 - 1, d0		| # of pages to clear - 1
	movl	#PAGEBASE, a0		| First unused word in page map
	movl	#PROTBASE, a1		| First unused word in protection map
	movl	#PAGEBASE+KV_CX0, a2
	movl	#PROTBASE+KV_CX0, a3
3$:	clrw	a0@+			| Zero out the indirection info
	movw	#PR_INVALID, a1@+	|   as well as the protection info
	clrw	a2@+			| Also, zero the copy of the
	movw	#PR_INVALID, a3@+	|   page and protection maps
	dbra	d0, 3$
|
| Clear out bss region (plus sched page table and udot)
|
MISC	=	USIZE + USIZE + USIZE - 1
	movl	#_end+MISC, d7		| End of Unix
	andl	#-USIZE, d7		| Round to nearest click
	movl	#_edata, a0		| Start clearing here
	moveq	#0, d1			| A zero to move to memory
4$:	movl	d1, a0@+		| Clear bss
	cmpl	d7, a0
	jcs	4$
#ifdef	GL2
|
| Copy dcr regs saved above into real memory
|
	movb	d4, __dcrmodes		| Save dcr modes
	movw	d5, __dcrflags		| Save dcr flags
#endif
|
| Initialize the duarts so SOMETHING can be printed
| Setup the mmu before con_init(), in case console is off board
|
	jsr	_duinit			| Initialize duarts

|	movl	#_end+USIZE-1, d7	| End of Unix + 1 click
|	andl	#0x0FFFFF, d7		| Strip off 0xC00000 bits
|	moveq	#PAGESHIFT, d0
|	lsrl	d0,d7			| Convert address to clicks
|	movl	d7,sp@-
	jsr	_mmuinit		| Initialize mmu
|	addql	#2, d7			| Skip past proc 0's udot & page table

	movl	#UDOT_VBASE+USIZE,sp	| Set stack at top of U area
|
| Save prom mouse interrupt vector
|
	movl	KERN_VBASE+0x7C, _prom_mouseintr
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
	movl	#KERN_VBASE, a0		| base of kernel
1$:	cmpb	#31, d1			| mouse interrupt vector?
	jne	2$			|   nope
	movl	#level7-KERN_VBASE+IVEC_VBASE, a0@+ | point to mouse interrupt
	jra	3$
2$:	movl	#lfault-KERN_VBASE+IVEC_VBASE, a0@+ | point to fault vector
3$:	addql	#1, d1
	dbra	d0, 1$			| loop through all vectors
|
| Now setup special vectors
|
	movl	#level1-KERN_VBASE+IVEC_VBASE, KERN_VBASE+0x64
	movl	#level2-KERN_VBASE+IVEC_VBASE, KERN_VBASE+0x68
	movl	#level3-KERN_VBASE+IVEC_VBASE, KERN_VBASE+0x6C
	movl	#level4-KERN_VBASE+IVEC_VBASE, KERN_VBASE+0x70
	movl	#level5-KERN_VBASE+IVEC_VBASE, KERN_VBASE+0x74
	movl	#level6-KERN_VBASE+IVEC_VBASE, KERN_VBASE+0x78
	movl	#level7-KERN_VBASE+IVEC_VBASE, KERN_VBASE+0x7C
	movl	#lsyscall-KERN_VBASE+IVEC_VBASE, KERN_VBASE+0x80
|
| Do the remaining setup in C
|
	jsr	_premain		| Pre-main setup code
|
| Call Unix main; when we return, we are about to run init code (icode)
|
	jsr	_main
	clrl	sp@-
	jsr	_sureg			| setup pagemap for init
	addql	#4, sp
|
| Start icode going
|
	clrw	sp@-			| short stack frame for 68010
	movl	#USRTEXT, sp@-		| user start address
	clrw	sp@-			| new sr value
	movw	#SPL7, sr		| block interrupts through rte
	jmp	10$ - KERN_VBASE + IVEC_VBASE
10$:
	movb	_cx+CX_USER, CONTEXT	| user context
	rte				| call init
|
| Adjust process priority
|
	.globl	_spl0, _spl1, _spl2, _spl3, _spl4, _spl5, _spl6, _spl7
	.globl	_spltty, _splx
_spl0:	movw	sr, d0
	movw	#0x2000, sr
	rts
_spl1:	movw	sr, d0
	movw	#0x2100, sr
	rts
_spl2:	movw	sr, d0
	movw	#0x2200, sr
	rts
_spl3:	movw	sr, d0
	movw	#0x2300, sr
	rts
_spl4:	movw	sr, d0
	movw	#0x2400, sr
	rts
_spl5:	movw	sr, d0
	movw	#0x2500, sr
	rts
_spl6:
_spltty:
_spl7:
	movw	sr, d0
	movw	#0x2600, sr
	rts
_splx:	movl	sp@(4), d0
	movw	d0, sr
	rts
|
| save and restore of register sets
|
	.globl	_save, _qsave, _resume
	.text
_save:	movl	sp@+,a1		| return address
	movl	sp@,a0		| ptr to label_t
	moveml	#0xFCFC,a0@	| save d2-d7, a2-a7
	movl	a1,a0@(48)	| save return address
	moveq	#0,d0
	jmp	a1@		| return

_qsave:	movl	sp@+,a1		| return address
	movl	sp@,a0		| ptr to label_t
	addw	#40,a0
	movl	a6,a0@+		| save a6
	movl	a7,a0@+		| save a7
	movl	a1,a0@+		| save return address
	moveq	#0,d0
	jmp	a1@		| return
|
| We switch to the temporary stack, before changing udots, because if we
| get interrupted by the mouse (or something else) between loading in the
| new udot address and reloading the stack registers, the level7 interrupt
| code will be using the old udots stack pointer in the new udot. Bad.
|
_resume:
	movw	sp@(6), d0		| pg_pfnum of new udot
	movl	sp@(8), a0		| ptr to label_t
	movw	#SPL7, sr		| Block out normal interrupts
	movl	#_debug_stack+512, sp	| Switch to temporary stack
	movw	d0, PAGEBASE + UDOT_KCX	| set up new udot page table entry
	moveml	a0@+, #0xFCFC		| restore the registers
	movw	#SPL0, sr		| Enable normal interrupts
	movl	a0@, a1			| fetch the original pc
	moveq	#1, d0			| return 1
	jmp	a1@			| return
|
| Switch off to kernel debugger
|
#include "debug.h"
	.globl	_debug
	.text
_debug:
#if NDEBUG > 0
	link	a6, #0
	movl	sp, _savessp		| Save system sp before switching

	movl	#_debug_stack+512, sp	| Point to base of stack
	moveml	#0xFFFF, sp@-		| Save kernel regs on new stack
	moveq	#0, d0			| Zero extend the
	movw	sr, d0			|   status register
	movl	d0, sp@-		| Save status register on stack
	movw	#SPL7, sr		| Bump priority up...
	movl	_savessp,a0
	movl	a0@(8),sp@-		| Push argument to debug()
	jsr	_kern_debug		| Call kernal debugger
	addql	#4, sp			| Pop argument to debug()
	movl	sp@+, d0		| Pop priority
	addql	#4, sp			| Skip over d0 saved above

	moveml	sp@+, #0xFFFE		| Restore all other registers
	movl	_savessp, sp		| Restore stack
	movw	d0, sr			| lastly, restore priority
	unlk	a6
#endif
	rts				| Back from whence we came
|
| Reboot/halt the system
|	- most of the grot below is because we can get a mouse interrupt at
|	  any old time
|
	.globl	_doboot
	.text
_doboot:
	movl	_rebootvec, a0			| Get reboot vector for later
|
| Restuff level7 interrupt vector with prom handler
|
	movw	#0x2200+KCX, PROTBASE + VBR_KCX	| Make page read/write
	movl	_prom_mouseintr, IVEC_VBASE+0x7C
|
| Remap the kernels vector page, the udot, and this code, in context 0.
| This is for catching mouse interrupts after switching to context 0, but
| before clearing the vector base register.
|
	clrw	PAGEBASE + VBR_CX0		| Make current vector base reg
	movw	#0x2200, PROTBASE + VBR_CX0	|   usable in context 0
	movw	sp@(6), PAGEBASE + VBR_CX0 + 2	| Make sure stack is present in
	movw	#0x2200, PROTBASE + VBR_CX0 + 2	|   context 0
	clrw	PAGEBASE + KV_CX0		| Setup map for 0xC00xxx, this
	movw	#0x2200, PROTBASE + KV_CX0	|   is for this code to exec in
|
| Make another duplicate of the vector page, in context 0, at virtual 0.
| This is where the proms expect the vbr to point to.
|
	clrw	PAGEBASE		| Setup new vector page @ virtual 0
	movw	#0x2200, PROTBASE	|   for mouse interrupts
|
| Everythings set... do it.
|
	movb	#0, CONTEXT		| Switch to context 0 for proms
	moveq	#0, d0			| Switch to 0 based vbr for proms
	movec	d0, vbr
	jmp	a0@
|
| Halt the system in its tracks.  68000's don't have a real halt instruction,
| so stop at high priority, buzzing
|
	.globl	_halt
	.text
_halt:	stop	#0x2700
	jra	_halt
|
| Interrupt handling software
|
	.globl	_ivectors
	.globl	_trap, _syscall
	.globl	_rebootvec
|
| Clock interrupt dispatch (user being interrupted)
|
	.globl	clock
clock:	addql	#1, _cnt+V_INTR
	btst	#5, sp@(6)		| interrupting the kernel
	jne	kclock			| yup
	moveml	#0xFFFF, sp@-		| save all registers
	movl	usp, a0
	movl	a0, sp@(FRAME_SP)	| save usr stack ptr

	movl	sp@(FRAME_PC), sp@-	| Push pc to top of frame
	movl	sp@(FRAME_SR+4), sp@-	| Push sr to top of frame
	jsr	_hardclock
	addql	#8, sp
|
| Return to user...
|
	.globl	backtouser
backtouser:
	tstb	_runrun			| Reschedule needed?
	jeq	0$			|  nope
	moveq	#T_RESCHED, d0		| fake trap out...
	jra	resched

0$:	movw	#SPL7, sr		| Block clock interrupts...
	tstb	_wantsoftclock		| Is softclock ready/busy?
	jle	2$			|   Nope
	movb	#-1, _wantsoftclock	| Don't want softclock any more...
	movw	#SPL0, sr		| spl0()
	jsr	_softclock		| Call softclock code...
	clrb	_wantsoftclock		| Softclock is all done
	jmp	0$			| try again lest interrupt happened

2$:	tstb	_qrunflag		| do we need to do some streams?
	jle	3$
	tstb	_queueflag		| and are we not busy?
	jne	3$			| no, try again later
	movb	#1,_queueflag		| yes, so make ourself busy
	movw	#SPL0, sr		| interrupts are ok now
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
	movw	#SPL7, sr		| Block interrupts during rte
	movl	sp@(FRAME_SP), a0	| Restore the
	movl	a0, usp			|   user stack pointer
	moveml	sp@+, #0x7FFF		| restore all other registers
	addw	#10, sp			| pop off fake-usp, filler and sr
					| alignment word
|
| Now figure out if bus error frame was mangled by trap().  If so, then we
| need to copy the short-frame information to the bottom of the long frame,
| as well as adjust our stack pointer and to repair the vector-offset so
| that the 68010 won't puke.  Once this is done, the 68010 will be rte'ing
| through a short frame to the users signal catcher.  Note that because of
| this, the user will be unable to restart the failing instruction.
|
	cmpw	#VECOFF_MUNGE, sp@(6)	| Funny return?
	jne	5$			|   Nope, easy return
	movw	sp@, sp@(50)		| Copy users sr
	movl	sp@(2), sp@(52)		| Copy users new pc
	clrw	sp@(56)			| Setup a clear vector offset
	addw	#50, sp			| Pop off junk part of frame

5$:	jmp	6$ - KERN_VBASE + IVEC_VBASE	| Get to a vector base address
6$:	movb	_cx+CX_USER,CONTEXT		| user context
	rte				| return from whence called
|
| Clock kernel dispatch (kernel being interrupted)
|
	.globl	kclock
kclock:	moveml	#0xc0c0, sp@-		| save C temporaries

	movl	sp@(24), sp@-		| Push old pc
	movl	sp@(20+4), sp@-		| Push old sr
	jsr	_hardclock		|   and jump to actual interrupt handler
	addql	#8, sp

kbacktosystem:
	movw	#SPL7, sr		| Block clock interrupts during rte
	movw	sp@(22), d0		| See if system was at high priority
	andw	#0x0700, d0		| Any priority level bits set?
	jne	4$			|   Yes, skip ast's until later

0$:	movw	#SPL7, sr		| Block clock interrupts...
	tstb	_wantsoftclock		| Is softclock ready/busy?
	jle	2$			|   Nope

	movb	#-1, _wantsoftclock	| Don't want softclock any more...
	movw	#SPL0, sr		| back to spl0()
	jsr	_softclock		| Call softclock code...
	clrb	_wantsoftclock		| Softclock is all done
	jmp	0$

2$:	tstb	_qrunflag		| do we need to do something?
	jle	3$
	tstb	_queueflag		| and are we not busy?
	jne	3$			| no, try again later
	movb	#1,_queueflag		| yes, so make ourself busy
	movw	#SPL0,sr		| interrupts are ok now
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
	moveml	sp@+, #0x0303		| restore C temporaries
	addw	#6, sp			| pop off handler addr and sr filler
	rte				| return from whence called
|
| Common user interrupt dispatch
|
	.globl	call
call:	addql	#1, _cnt+V_INTR
	btst	#5, sp@(6)		| interrupting the kernel
	jne	kcall			| yup
	moveml	#0xFFFF, sp@-		| save all registers
	movl	usp, a0
	movl	a0, sp@(FRAME_SP)	| save usr stack ptr
	movl	sp@(FRAME_INTR), a0	| fetch interrupt handler address
	jsr	a0@			|   and jump to actual interrupt handler
	jra	backtouser		| return to user
|
| Common kernel-interrupt dispatch (for non clock interrupts)
|
	.globl	kcall
kcall:	moveml	#0xc0c0, sp@-		| save C temporaries
	movl	sp@(16), a0		| fetch interrupt handler address
	jsr	a0@			|   and jump to actual interrupt handler
	jra	kbacktosystem
|
| Process some sort of fault (traps, random faults, etc...)
|
	.globl	fault
fault:	addql	#1, _cnt+V_TRAP
	moveml	#0xFFFF,sp@-		| save all registers
	movl	usp,a0
	movl	a0,sp@(FRAME_SP)	| save usr stack ptr
trapit:
	moveq	#0, d0			| Zero extend the vector offset,
	movw	sp@(FRAME_VECOFFSET), d0
	andw	#0xFFF, d0		|   trim out the frame type, then
	asrw	#2, d0			|    calculate trap type

resched:
	movl	d0, sp@-		| argument to trap
	jsr	_trap			| C handler for traps and faults
	addql	#4, sp
	btst	#5, sp@(FRAME_SR+2)	| did we come from user mode?
	jne	backtosystem		| no, just continue
	jra	backtouser		| no, just return normally
|
| Process a system call (trap #0)
|
	.globl	syscall
syscall:
	addql	#1, _cnt+V_SYSCALL
	moveml	#0xFFFF, sp@-		| save all registers
	movl	usp, a0
	movl	a0, sp@(FRAME_SP)	| save usr stack ptr
	btst	#5, sp@(FRAME_SR+2)	| did we come from user mode?
	jne	trapit			|   nope, trap it
	jsr	_syscall		| Process system call
	jra	backtouser		| no, just return normally
|
| Return to the system...
|	- check processor priority. If its non-zero, then the priority is
|	  considered higher than ast stuff, so just return
|
	.globl	backtosystem
backtosystem:
	movw	sp@(FRAME_SR+2), d0	| See if system was at high priority
	andw	#0x0700, d0		| Any priority level bits set?
	jne	0$			|   Yes, skip ast's until later
	movw	#SPL7, sr		| Block clock interrupts...
	tstb	_wantsoftclock		| Is softclock ready/busy?
	jle	0$			|   Nope
	movb	#-1, _wantsoftclock	| Don't want softclock any more...
	movw	#SPL0, sr		| spl0()
	jsr	_softclock		| Call softclock code...
	clrb	_wantsoftclock		| Softclock is all done

0$:	movw	#SPL7, sr		| Block clock interrupts during rte
	movl	sp@(FRAME_SP), a0	| Restore the
	movl	a0, usp			|   user stack pointer
	moveml	sp@+, #0x7FFF		| restore all other registers
	addw	#10, sp			| pop off fake-usp, filler and sr
					| alignment word
	rte				| return from whence called
|
| Interrupt service staging code:
|	- each of these short routines is used to switch the kernel into its
|	  context before jumping off to a more thorough processing routine.
|	- WARNING: these routines must run in relative-pc addressing mode
|	  until the context is switched (this is actually quite trivial,
|	  but is VERY important).  Thus, any branches must be relative.
|	- the reason for this brane damage is that when an interrupt occurs,
|	  the interrupt vectors located at IVEC_VBASE in the virtual address
|	  space of the user (or the kernel) contain pointers into the kernel
|	  below, which are based at IVEC_VBASE.  That is, for
|	  any fault vector located at IVEC_VBASE+0x??, a pointer to
|	  "lfault" is stored.  The pointer value will be
|	  "lfault - KERN_VBASE + IVEC_VBASE".
|	  This is confusing, but necessary, since when running
|	  in the users context, we have to take faults in her/his virtual
|	  address space
|
	.globl	lfault, lsyscall, level1, level2
	.globl	level3, level4, level5, level6, level7
lfault:
	movb	#KCX, CONTEXT		| switch to kernel context
	clrw	sp@-			| long extend status register
	clrl	sp@-			| filler
	jmp	fault			| Absolute jump to handler

lsyscall:
	movb	#KCX, CONTEXT		| switch to kernel context
	clrw	sp@-			| long extend status register
	clrl	sp@-			| filler
	jmp	syscall

level1:	movb	#KCX, CONTEXT
	clrw	sp@-			| long extend status register
	movl	_ivectors+4, sp@-
	jmp	call

level2:	movb	#KCX, CONTEXT
	clrw	sp@-			| long extend status register
	movl	_ivectors+8, sp@-
	jmp	call

level3:	movb	#KCX, CONTEXT
	clrw	sp@-			| long extend status register
	movl	_ivectors+12, sp@-
	jmp	call

level4:	movb	#KCX, CONTEXT
	clrw	sp@-			| long extend status register
	movl	_ivectors+16, sp@-
	jmp	call

level5:	movb	#KCX, CONTEXT
	clrw	sp@-			| long extend status register
	movl	_ivectors+20, sp@-
	jmp	call

level6:	movb	#KCX, CONTEXT
	btst	#3, DUISR		| Is clock interrupt high?
	jne	5$			|   Yes, do clock code
	clrw	sp@-			| long extend status register
	movl	#_duintr_both, sp@-	| Call duart interrupt code
	jmp	call			| Jump to common interrupt handler

5$:	tstb	DUCCSTP			| Reset clock interrupt
	clrw	sp@-			| long extend status register
	clrl	sp@-			| We don't care what goes here...
	jmp	clock			| Jump to common interrupt handler
|
| Mouse entry point
|
	.data
	.comm	__mousex, 2
	.comm	__mousey, 2
	.comm	_mousebusy, 1
	.text

level7: movl	d0, sp@-	| Save d0
	movw	WCONTEXT,sp@-	| Save current running context
	movb	#KCX, CONTEXT	| Get to kernel context
	addql	#1, _cnt+V_INTR

	movw	MOUSE, d0	| Read mouse bits to clear interrupt

	tstw	_mousebusy	| Is the mouse currently busy?
	jne	10$
|
| Perform mouse quadrature support
|	- first do the x translation...
|
	btst	#MOUSE_XFIREBIT, d0	| X fire bit on? (active low)
	jne	1$			|   nope
	btst	#MOUSE_XBIT, d0		| Positive movement?
	jeq	2$			|   nope
	addqw	#1, __mousex		| Advance mouse 1 tick in x
	jra	1$
2$:	subqw	#1, __mousex		| Retreat mouse 1 tick in x
|
| Now do the y translation
|
1$:	btst	#MOUSE_YFIREBIT, d0	| Y fire bit on? (active low)
	jne	10$			|   nope, all done
	btst	#MOUSE_YBIT, d0		| Positive movement?
	jeq	4$			|   nope
	addqw	#1, __mousey		| Advance mouse 1 tick in y
	jra	10$
4$:	subqw	#1, __mousey		| Retreat mouse 1 tick in y
|
| Mouse is finished...
|	- restore context, and return
|
10$:	movw	sp@+, WCONTEXT	| Restore previous running context
	movl	sp@+,d0		| Restore d0
	rte

#ifdef	PROF
|
| Profiling support code
|

|
| Setup prof interrupt vector (point level 7 handler at proflevel7 routine
| instead of mouse)
|
	.globl	_profiling, _setprofvec

_setprofvec:
	movw	#0x2200+KCX, PROTBASE + VBR_KCX | Make vector page writable
	movl	#proflevel7 - KERN_VBASE + IVEC_VBASE, IVEC_VBASE + 0x7c
	movw	#0x2100+KCX, PROTBASE + VBR_KCX | Make vector page read-only
	rts
|
| Profile interrupt handler
|
proflevel7:
	movw	WCONTEXT, sp@-	| Save current running context
	movb	#KCX, CONTEXT	| Get to kernel context
	tstw	MOUSE		| clear interrupt

	tstb	_profiling	| Profiling?
	jeq	0$		|   nope, return

	btst	#5, sp@(2)	| Were we in kernel mode?
	jne	1$		|   yes, do normal thing
|
| In user mode.  Increment profiling pseudo address "_user"
|
	movl	a0, sp@-	| Save this register
	movl	_profbuf, a0
	addql	#1, a0@		| Increment "user" counter
	movl	sp@+, a0
0$:	movw	sp@+, WCONTEXT	| Restore previous running context
	rte

1$:	movl	d0, sp@-
	movl	a0, sp@-	| Save these registers
	movl	sp@(12), d0	| Get pc
	cmpl	#KERN_VLIMIT, d0| Pc in vector page?
	jle	2$		|   nope, pc is okay
	andl	#0xFFF, d0	| brutally reduce to a vector page address

2$:	andl	#PCMASK, d0	| Strip off KERN_VBASE prefix
	lsrl	#PROFSHIFT-2, d0| Reduce to buffer slot # (*4 for long slots)
	movl	_profbuf, a0	| Get pointer to buffer
	addl	d0, a0		| Advance to slot
	addql	#1, a0@		| Bump counter

3$:	movl	sp@+, a0
	movl	sp@+, d0	| Restore these registers
	movw	sp@+, WCONTEXT	| Restore previous running context
	rte
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
| 14 longs worth of zeros
|
zeros:
	.long	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

|
| Clear out one page of memory, as fast as we can
|
	.text
	.globl	_bzeroPAGE
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
