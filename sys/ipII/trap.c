/*
 * $Source: /d2/3.7/src/sys/ipII/RCS/trap.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:31:04 $
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/seg.h"
#include "../h/acct.h"
#include "../vm/vm.h"
#include "../h/sysent.h"
#include "../h/sysinfo.h"
#include "../h/printf.h"
#include "../ipII/trap.h"
#include "../ipII/cpureg.h"
#include "../ipII/frame.h"
#include "../ipII/cx.h"
#include "../ipII/psr.h"
#include "../ipII/reg.h"
#include "../ipII/pte.h"

#define calcppri(p)	(((p)->p_cpu >> 1) +  (p)->p_nice + (PUSER - NZERO))

/*
 * Offsets of the user's registers relative to
 * the saved r0. See reg.h
 */
char	regloc[8+8+1+1] = {
	R0, R1, R2, R3, R4, R5, R6, R7,
	AR0, AR1, AR2, AR3, AR4, AR5, AR6, SP, PC,
	RPS
};
extern	short nsyscalls;
extern	short beprint;

/*
 * Called from the trap handler when a processor trap occurs.
 */
trap(code, frame)
	register u_short code;
	struct frame frame;
{
	register int *locr0 = (int *)&frame.fr_regs[0];
	register struct proc *p;
	register short i;
	register long aaddr;
	time_t syst;
	extern int *nofault;
	extern int *parityFault;
	short besignal;

	syst = u.u_stime;
	u.u_pcb.pcb_fpsaved = 0;
	u.u_ar0 = locr0;
	p = u.u_procp;
	besignal = 0;

	invalidatehwregs();

	if (!USERMODE(locr0[RPS]))
		code |= KRNL;

	/*
	 * Note kind of frame for signal disposition.  Since this
	 * code may be entered from a runrun trap (T_RESCHED) we
	 * can't juse look at the fault type.
	 */
	switch(code) {
	  case T_EXTERN:
	  case T_EXTERN+KRNL:
	  default:				/* something bad happened */
		printf("trap type=%d pid=%d pc=%x ps=%x\n",
			     code & ~KRNL, code & KRNL ? 0 : p->p_pid,
			     frame.fr_pc, frame.fr_sr);
		panic("trap");
		/* NOTREACHED */

	  case T_PARERR:
	  case T_PARERR+KRNL:
		if (parityFault)
			longjmp(parityFault, 1);
		else {
			parity(frame.fr_sr);
			goto out;
		}
		/*NOTREACHED*/

	  case T_BUSERR:			/* bus error / page fault */
		/*
		 * 4 reasons for a bus error:
		 *	1. timeout (multibus or ge port)
		 *	2. illegal segment access
		 *	3. access dis-allowed by map or limit
		 *	4. fpa error
		 */

		/*
		 * If address is within currently allocated range then
		 * assume its a page fault
		 */
		aaddr = frame.fr_dcfa;

		/*
		** If only a data fault occured we first check if the
		** segment accessed was illegal. (Multibus memory,
		** multibus io, ge or the fpa)
		*/
		if ( ! ( frame.fr_ssw & 0xc000 ) )
		{
			u_long seg;
			seg = aaddr & SEG_MSK;
			if ( seg == SEG_MBMEM || seg == SEG_MBIO ||
				 seg == SEG_GE )
			{
				besignal = i = SIGSEGV;
				break;
			}
			if ( seg == (unsigned long)SEG_FPA )
			{
				/*
				** reset the error mask register to ignore
				** errors.
				*/
				fpreset();
				if ( havefpa )
					besignal = i = SIGFPE;
				else
					besignal = i = SIGSEGV;
				break;
			}
		}

		/*
		** the following code is used to determine where that actual
		** faulting address is.  We assume if either of the instruction
		** stream fault bits are on - it is an instruction fault and
		** calc the faulting address dependent on the frame type.
		** Otherwise it must be a data fault and we can use the dcfa
		** field.
		** NOTE: It may be necessary that this code be present in
		**       the fixmsk() routine - currently it is NOT.
		*/
		if ( ( frame.fr_ssw & 0xc000 ) )
		{
		    /*
		    ** if a long frame we calc the faulting address one way
		    ** otherwise we do it another
		    */
		    if ( frame.fr_vecoff & 0x1000 )
		    {
			    aaddr = frame.fr_beframe.lbefr.lfr_sBaddr;
			    /*
			    ** if stage C faulted - adjust the faulting address
			    */
			    if ( frame.fr_ssw & 0x8000 )
				    aaddr -= 2;
		    }
		    else
		    {
			    /*
			    ** if stage C faulted calc the address one way,
			    ** if it was stage B we do it another way.
			    */
			    if ( frame.fr_ssw & 0x8000 )
				    aaddr = frame.fr_pc + 2;
			    else
				    aaddr = frame.fr_pc + 4;
		    }
		}
		/*
		** normal bus error actions
		*/
		if (((aaddr >= ctob(p->p_loadc)) &&
		     (aaddr < ctob(p->p_loadc + p->p_tsize + p->p_dsize))) ||
		    ((aaddr < USRSTACK) &&
		     (aaddr >= USRSTACK - ctob(p->p_ssize)))) {
			i = u.u_error;
			if (!pagein(aaddr, 0)) {
				u.u_error = i;
				goto out;
			}
			/*
			 * OOPS.  User took a page fault for (currently)
			 * no obvious reason.  See if the reference was a
			 * write on a read-only page.
			 */
			if ((frame.fr_ssw & 0x0080) ||
			    ((frame.fr_ssw & 0x0040) == 0)) {
				register struct pte *pte;
				/*
				 * Cycle was trying to either write, or
				 * do a read-modify-write.  Check page table
				 * protections.  If page was writable, then
				 * ignore this bus fault, and return letting
				 * the chip finish re-running the instruction.
				 */
				pte = vtopte(p, btop(aaddr));
				if (!pte->pg_v)
					panic("trap");
				if ((*(long *)pte & PTE_PROTMSK) != PTE_RACC) {
					goto out;
				}
			} else {
				/*
				 * Since the fault is not a write fault,
				 * and the address is valid, and pagein
				 * didn't want to get a page for us, we
				 * assume here that the chip screwed up
				 * and needs to rerun a fault, for no good
				 * reason.  Just return, letting the chip
				 * try again.
				 */
				goto out;
			}
		} else
		/*
		 * if address within possible stack range, try to
		 * grow the stack
		 */
		if ((aaddr < USRSTACK) &&
		    (aaddr >= USRSTACK - ctob(MAXSSIZ - HIGHPAGES)) &&
		    grow(aaddr))
			goto out;
		besignal = i = SIGSEGV;
		break;
	  case T_ADDRERR:			/* address error */
		u.u_pcb.pcb_aaddr = frame.fr_dcfa;
		besignal = i = SIGBUS;
		break;
	  case T_RESCHED:			/* rescheduling trap */
		/*
		 * Since we are u.u_procp, clock will normally just change
		 * our priority without moving us from one queue to another
		 * (since the running process is not on a queue.)
		 * If that happened after we setrq ourselves but before we
		 * swtch()'ed, we might not be on the queue indicated by
		 * our priority.
		 *
		 * Recompute the currently running processes priority before
		 * calling swtch to account for the cpu it has used during
		 * its time slice.
		 */
		(void) spl6();
		curpri = p->p_pri = calcppri(p);
		setrq(p);
		swtch();
		(void) spl0();
		goto out;
	  case T_DIVZERO:			/* division by zero */
		i = SIGFPE;
		break;
	  case T_TRCTRAP:			/* trace trap */
		locr0[RPS] &= ~PS_T;
		if (u.u_pcb.pcb_faketrap) {
			/*
			 * Clear out faketrap state.  If process is not
			 * being traced, then it shouldn't get a trace
			 * trap fault, so don't send it a SIGNAL signal.
			 */
			u.u_pcb.pcb_faketrap = 0;
			if (!(p->p_flag & STRC))
				goto out;
		}
		i = SIGTRAP;
		break;
	  case T_TRAP1:				/* bpt simulator */
		locr0[RPS] &= ~PS_T;
		i = SIGTRAP;
		break;
	  case T_TRAP2:				/* iot simulator */
		i = SIGIOT;
		break;
	  case T_TRAP3:				/* emt simulator */
		i = SIGEMT;
		break;
	  case T_PRIVVIO:
	  case T_ILLINST:			/* illegal instructions */
	  case T_CHK:
	  case T_TRAPV:
	  case T_L1010:
	  case T_L1111:
	  case T_TRAP4:
	  case T_TRAP5: case T_TRAP6: case T_TRAP7: case T_TRAP8:
	  case T_TRAP9: case T_TRAP10: case T_TRAP11: case T_TRAP12:
	  case T_TRAP13: case T_TRAP14: case T_TRAP15:
		i = SIGILL;
		break;

	/*
	 * Remainder of traps are kernel only...
	 */
	  case T_BUSERR+KRNL:
	  case T_ADDRERR+KRNL:
		u.u_pcb.pcb_aaddr = frame.fr_dcfa;
		if (beprint) {
			printf("kernel trap: type=%d pc=%x ps=%x aaddr=%x\n",
				       code & ~KRNL, frame.fr_pc, frame.fr_sr,
				       frame.fr_dcfa);
		}
		if (nofault) {
			longjmp(nofault, code);
			/* NOTREACHED */
		}
		if (beprint)
			panic("trap(print)");
		else
			panic("trap(noprint)");
		/* NOTREACHED */
	  case T_TRCTRAP+KRNL:
		/*
		 * Trace out of kernel mode apparently happens when a trap
		 * instruction is executed with the trace bit set
		 */
		return;
	}
#ifdef	DEBUG
	if ((kdebug > 1) && ((i == SIGSEGV) || (i == SIGBUS) || (i == SIGILL)))
		debug("about to segv/bus error proc");
#endif
	psignal(p, i);

out:
	/*
	 * Attempt to process a signal.  If there is a signal pending, and
	 * this is not a bus error frame, then send the signal.  If this is
	 * a bus error frame, and the signal to be delivered was sent above
	 * (from a bus error or an address error) attempt to kill the process
	 * normally be calling psig().  If this doesn't work, then mangle
	 * the stack frame of the kernel to indicate that this bus error
	 * frame should be truncated to a short frame.
	 * Otherwise, the signal can be delivered later, so just ignore it
	 * for now.  This assumes that no ``high-priority'' signal can arrive
	 * between calling issig() and psig().
	 */
	(void) spl6();
	if (p->p_sig && (i = issig())) {
		if (frame.fr_vecoff & VECOFF_BIGFRAME) {
			if (i == besignal) {
				psig();
				(void) spl0();
				/*
				 * If we get here, then the user wasn't core
				 * dumped and REALLY wants to get the signal.
				 * We have to let the locore code know this,
				 * so tag the fault frame by setting a unused
				 * bit in the vec format.  This destroys the
				 * buserror frame, and thus the user will be
				 * unable to restart the faulted instruction.
				 */
				frame.fr_vecoff |= VECOFF_MUNGE;
			} else {
				/*
				 * User has a signal pending and we can't
				 * deliver it now on this bus error frame.
				 * Tag the users sr with a trace trap bit, so
				 * that as soon as the instruction re-run's,
				 * the process will take a trace trap fault.
				 * Note in the pcb that this is a fake trace
				 * trap, so that we can disambiguate an
				 * expected trace trap versus an erroneous one.
				 */
				(void) spl0();
				u.u_pcb.pcb_faketrap = 1;
				locr0[RPS] |= PS_T;
			}
		} else {
			(void) spl0();
			psig();
		}
	}

	/* re-adjust process priority in case system did a sleep */
	curpri = p->p_pri = calcppri(p);

	/* add up a profiling tick */
	if (u.u_prof.pr_scale)
		addupc((unsigned)u.u_ar0[PC], &u.u_prof, (int)(u.u_stime-syst));

	/* reload process state (if needed) */
	if (u.u_pcb.pcb_fpinuse && u.u_pcb.pcb_fpsaved)
		fprestore();			/* restore floating point */
	if (p->p_flag & (SPTECHG|SLOSTCX))
		sureg(1);			/* restore page map */

	cx.cx_tduser = p->p_cxtdnum;
	cx.cx_suser = p->p_cxsnum;
	cx.cx_tdsize = p->p_cxbsize;
	cx.cx_ssize = p->p_cxssize;
}

/*
 * process a system call
 */
syscall(frame)
	struct frame frame;
{
	register struct proc *p;
	register int *regp, *argp;
	register short code;
	extern short nsyscalls;

	sysinfo.syscall++;

	invalidatehwregs();

	u.u_error = 0;
	u.u_pcb.pcb_fpsaved = 0;
	u.u_ar0 = regp = (int *)&frame.fr_regs[0];
	u.u_ap = argp = u.u_arg;
	code = regp[R0] & 0377;
	if (code >= nsyscalls)
		code = 0;

	/* copy 7 arguments from user register set into linear memory */
	argp[0] = regp[AR0];
	argp[1] = regp[R1];
	argp[2] = regp[AR1];
	argp[3] = regp[R2];
	argp[4] = regp[AR2];
	argp[5] = regp[R3];
	argp[6] = regp[AR3];

	u.u_dirp = (caddr_t)argp[0];
	u.u_rval1 = 0;
	u.u_rval2 = regp[R1];
	if (qsave(u.u_qsave)) {
		/*
		 * IMPORTANT: restore registers not saved by qsave
		 */
		regp = (int *)&frame.fr_regs[0];
		argp = u.u_arg;
		if (u.u_error == 0)
			u.u_error = EINTR;
	} else
		(*(sysent[code].sy_call))();

	if (u.u_error) {
		regp[RPS] |= PS_C;		/* set carry bit */
		regp[R0] = (unsigned)u.u_error;
	} else {
		regp[RPS] &= ~PS_C;		/* clear carry bit */
		regp[R0] = u.u_rval1;
		regp[R1] = u.u_rval2;
	}

	/*
	 * Test if the trap instruction was executed with the
	 * trace bit set (the trace trap was already ignored)
	 * and set the trace signal to avoid missing the trace
	 * on the trap instruction.
	 */
	p = u.u_procp;
	if (regp[RPS] & PS_T)
		psignal(p, SIGTRAP);
	if (p->p_sig && issig()) {
		psig();
	}

	/* re-adjust process priority in case system did a sleep */
	curpri = p->p_pri = calcppri(p);

	/* reload process state (if needed) */
	if (u.u_pcb.pcb_fpinuse && u.u_pcb.pcb_fpsaved)
		fprestore();			/* restore floating point */
	if (p->p_flag & (SPTECHG|SLOSTCX))
		sureg(1);			/* restore page map */

	cx.cx_tduser = p->p_cxtdnum;
	cx.cx_suser = p->p_cxsnum;
	cx.cx_tdsize = p->p_cxbsize;
	cx.cx_ssize = p->p_cxssize;
}

/*
 * invalidatehwregs:
 *	- invalidate the base and limit registers for the text/data and
 *	  stack regions
 *	- this keeps the kernel honest
 */
invalidatehwregs()
{
	/* make text+data based at 0 in the page map, of 0 length */
	*TDBASE_REG = 0;
	*TDLMT_REG = 0xFFFF;

	/* make stack based at 0 in the page map, of 0 length */
	*STKBASE_REG = 0;
	*STKLMT_REG = 0;
}
