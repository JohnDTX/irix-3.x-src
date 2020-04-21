/*	
 * Routines to save/restore the context of the Sky ffp for
 * multiuser support.
 *
 * Written by: Greg Boyd
 *
 * $Source: /d2/3.7/src/sys/multibus/RCS/sky.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:31:39 $
 */

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/setjmp.h"
#include "../h/systm.h"
#include "machine/cpureg.h"
#include "../multibus/fpregs.h"
#include "../multibus/fpopcodes.h"

/* count down loop maximum to determine if the board is hung */
#define MAXTIME 50

short	havefpa;

/*
 * skyprobe:
 *	- reset the sky board
 */
skyprobe()
{
	extern int *nofault;
	extern short beprint;
	int *saved_jb;
	jmp_buf jb;

	beprint = 0;
	saved_jb = nofault;
	if (setjmp(jb) == 0) {
		nofault = jb;
		*SKYSTATREG = RESET;
		*SKYCOMREG = HW_INIT;
		*SKYCOMREG = HW_INIT;
		*SKYCOMREG = HW_SPADD;
		*SKYSTATREG = RUN;
		havefpa = 1;
		printf("sky0 at mbio 0x0040\n");
	} else
		printf("sky0 not installed\n");
	nofault = saved_jb;
	beprint = 1;
}

/*
 * fpinit:
 *	- init the sky board for a user
 */
fpinit()
{
	u.u_pcb.pcb_fps.f_comreg = HW_NOP;
	*SKYSTATREG = RESET;
	*SKYCOMREG = HW_INIT;
	*SKYCOMREG = HW_INIT;
	*SKYCOMREG = HW_SPADD;
	*SKYSTATREG = RUN;
}

/*
 * fpsave:
 *	- save the current sky context in the users u_pcb.pcb_fps structure
 *	- the board is left in a reset state
 */
fpsave()
{
	register short junk;
	register unsigned long *lptr;
	register short clock;

    /* if ffp is idle, dont have to interrupt it */
	if (*SKYSTATREG & SKYIDLE) {
		/* simply store NOP in command code in save area */
		u.u_pcb.pcb_fps.f_comreg = HW_NOP;
	} else {
	    /* wait until ffp is ready for I/O */
		clock = MAXTIME;
		while (!(*SKYSTATREG & SKYIORDY)) {
		    /* if ffp is now idle, store NOP in command code and
		       proceed with saving the registers */
			if (*SKYSTATREG & SKYIDLE) {
				u.u_pcb.pcb_fps.f_comreg = HW_NOP;
				goto save_regs;
			}
			if (!clock--)
				goto timeout;
		}
	    /*
	     * ffp is IORDY, but not IDLE.  have to save total state
	     * satisfy the boards i/o request with dummy data
	     */
		*SKYSTATREG = SINGLE_STEP|RUN;
		if (*SKYSTATREG & SKYIODIR)
			junk = *SKYDT1REG;
		else 
			*SKYDT1REG = junk;

		if (*SKYSTATREG & SKYIORDY) {
			if (*SKYSTATREG & SKYIODIR)
				junk = *SKYDT1REG;
			else
				*SKYDT1REG = junk;
		}

	    /*
	     * read contents of command register (microcode address)
	     * decrement it for restart and save the address in the save area
	     */
		junk = *SKYCOMREG;
		u.u_pcb.pcb_fps.f_comreg = --junk;
		
		/*  reset, init sequence, and run */
		*SKYSTATREG = RESET;
		*SKYCOMREG = HW_INIT;
		*SKYCOMREG = HW_INIT;
		*SKYCOMREG = HW_SPADD;
		*SKYSTATREG = RUN;
	}

save_regs:
    /* NOP, CTXSV to command register; read the Sky internal registers */
	*SKYCOMREG = HW_NOP;
	*SKYCOMREG = HW_CTXSV;

	lptr = u.u_pcb.pcb_fps.f_reg;
	*lptr++ = *SKYDTREG;
	*lptr++ = *SKYDTREG;
	*lptr++ = *SKYDTREG;
	*lptr++ = *SKYDTREG;

	*lptr++ = *SKYDTREG;
	*lptr++ = *SKYDTREG;
	*lptr++ = *SKYDTREG;
	*lptr++ = *SKYDTREG;
	return;

timeout: ;
    /* the board is not responding */
	printf("sky board hung, killing process %d\n", u.u_procp->p_pid);
	psignal(u.u_procp, SIGKILL);
}	


/*
 * fprestore:
 *	- restore the sky context from the users u_pcb.pcb_fps
 */
fprestore()
{
	register unsigned long *lptr = u.u_pcb.pcb_fps.f_reg;

	*SKYCOMREG = HW_CTXRSTR;	/* send context restore command */

	*SKYDTREG = *lptr++;		/* load in registers */
	*SKYDTREG = *lptr++;
	*SKYDTREG = *lptr++;
	*SKYDTREG = *lptr++;
	*SKYDTREG = *lptr++;
	*SKYDTREG = *lptr++;
	*SKYDTREG = *lptr++;
	*SKYDTREG = *lptr++;

	*SKYCOMREG = u.u_pcb.pcb_fps.f_comreg;	/* reload last command in-progress */
	u.u_pcb.pcb_fpsaved = 0;
}
