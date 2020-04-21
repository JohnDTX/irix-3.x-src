/*
 * $Source: /d2/3.7/src/sys/pmII/RCS/trap.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:52 $
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
#include "../pmII/trap.h"
#include "../pmII/cpureg.h"
#include "../pmII/frame.h"
#include "../pmII/cx.h"
#include "../pmII/psr.h"
#include "../pmII/reg.h"
#include "../pmII/pte.h"

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
extern	short kdebug;

/*
 * Called from the trap handler when a processor trap occurs.
 */
trap(code, frame)
	register u_short code;
	struct frame frame;
{
	register int *locr0 = (int *)&frame.regs[0];
	register struct proc *p;
	register short i;
	register long aaddr;
	time_t syst;
	extern int *nofault;
	short isbeframe, besignal;
	u_short exception;
	extern int paritysearching;

	syst = u.u_stime;
	u.u_pcb.pcb_fpsaved = 0;
	u.u_ar0 = locr0;
	p = u.u_procp;
	besignal = 0;

	/*
	 * Check for a parity error right away
	 */
	if ((code == T_BUSERR) &&
	    (((exception = *(u_short *)EREG) & ER_PARITY) == 0)) {
		/*
		 * Parity errors are okay IFF nofault is set to catch bus
		 * errors and the faulter is the kernel.  This can happen
		 * when the user is using /dev/{k,}mem and touches
		 * non-existant memory.
		 */
		if (nofault && !USERMODE(locr0[RPS]))
			longjmp(nofault, T_BUSERR);
		parity((short) frame.sr);
	}

	if (!USERMODE(locr0[RPS]))
		code |= KRNL;
	switch(code) {
	  default:				/* something bad happened */
		printf("trap type=%d pid=%d pc=%x ps=%x ereg=%x\n",
			     code, code & KRNL ? 0 : p->p_pid,
			     frame.pc, frame.sr, exception);
		panic("trap");
		/* NOTREACHED */
	  case T_BUSERR:			/* bus error / page fault */
		/*
		 * If address is within currently allocated range then
		 * assume its a page fault
		 */
		aaddr = frame.aaddr;		/* fault address */
		if (((aaddr >= ctob(p->p_loadc)) &&
		     (aaddr < ctob(p->p_loadc + p->p_tsize + p->p_dsize))) ||
		    ((aaddr < USRSTACK) &&
		     (aaddr >= USRSTACK - ctob(p->p_ssize)))) {
			i = u.u_error;
			if (!pagein(aaddr, 0)) {
				u.u_error = i;
				goto out;
			}
		}
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
		u.u_pcb.pcb_aaddr = frame.aaddr;
		besignal = i = SIGBUS;
		break;
	  case T_RESCHED:			/* rescheduling trap */
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
			 * trap fault, so don't send it a SIGTRAP signal.
			 */
			u.u_pcb.pcb_faketrap = 0;
			if (!(p->p_flag & STRC))
				goto out;
		}
		i = SIGTRAP;
		break;
	  case T_TRAP1:				/* bpt simulator */
		i = SIGTRAP;
		locr0[RPS] &= ~PS_T;
		break;
	  case T_TRAP2:				/* iot simulator */
		i = SIGIOT;
		break;
	  case T_TRAP3:				/* emt simulator */
		i = SIGEMT;
		break;
	  case T_ILLINST:			/* illegal instructions */
	  case T_CHK:
	  case T_TRAPV:
	  case T_PRIVVIO:
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
		u.u_pcb.pcb_aaddr = frame.aaddr;
		if (beprint && !nofault)
			printf("kernel trap: type=%d pc=%x ps=%x aaddr=%x\n",
				       code - KRNL, frame.pc, frame.sr,
				       frame.aaddr);
		if (nofault) {
			longjmp(nofault, code);
			/* NOTREACHED */
		}
		panic("trap");
		/* NOTREACHED */
	  case T_TRCTRAP+KRNL:
		/*
		 * Trace out of kernel mode apparently happens when a trap
		 * instruction is executed with the trace bit set
		 */
		return;
	}
	if ((kdebug > 1) && ((i == SIGSEGV) || (i == SIGBUS) || (i == SIGILL)))
		debug("about to segv/bus error proc");
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
		if (frame.vecoffset & VECOFF_BIGFRAME) {
			if (i == besignal) {
				psig();
				(void) spl0();
				/*
				 * If we get here, then the user wasn't core
				 * dumped and REALLY wants to get the signal.
				 * We have to let the locore code know this,
				 * so tag the fault frame by setting the vector
				 * offset to 0xFFFF.  This destroys the
				 * buserror frame, and thus the user will be
				 * unable to restart the faulted instruction.
				 */
				frame.vecoffset = VECOFF_MUNGE;
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

	cx.cx_user = p->p_cxnum;
}

/*
 * process a system call
 */
syscall(frame)
	struct frame frame;
{
	register struct proc *p;
	register int *regp, *argp;
	register struct user *up;
	register short code;
	extern short nsyscalls;

	sysinfo.syscall++;
	up = &u;
	up->u_error = 0;
	up->u_pcb.pcb_fpsaved = 0;
	up->u_ar0 = regp = (int *)&frame.regs[0];
	up->u_ap = argp = up->u_arg;
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

	up->u_dirp = (caddr_t)argp[0];
	up->u_rval1 = 0;
	up->u_rval2 = regp[R1];
	if (qsave(up->u_qsave)) {
		/*
		 * IMPORTANT: restore registers not saved by qsave
		 */
		up = &u;
		regp = (int *)&frame.regs[0];
		argp = up->u_arg;
		if (up->u_error == 0)
			up->u_error = EINTR;
	} else
		(*(sysent[code].sy_call))();

	if (up->u_error) {
		regp[RPS] |= PS_C;		/* set carry bit */
		regp[R0] = (unsigned)up->u_error;
	} else {
		regp[RPS] &= ~PS_C;		/* clear carry bit */
		regp[R0] = up->u_rval1;
		regp[R1] = up->u_rval2;
	}

	/*
	 * Test if the trap instruction was executed with the
	 * trace bit set (the trace trap was already ignored)
	 * and set the trace signal to avoid missing the trace
	 * on the trap instruction.
	 */
	p = up->u_procp;
	if (regp[RPS] & PS_T)
		psignal(p, SIGTRAP);
	if (p->p_sig && issig())
		psig();

	/* re-adjust process priority in case system did a sleep */
	curpri = p->p_pri = calcppri(p);

	/* reload process state (if needed) */
	if (up->u_pcb.pcb_fpinuse && up->u_pcb.pcb_fpsaved)
		fprestore();			/* restore floating point */
	if (p->p_flag & (SPTECHG|SLOSTCX))
		sureg(1);			/* restore page map */

	cx.cx_user = p->p_cxnum;
}
