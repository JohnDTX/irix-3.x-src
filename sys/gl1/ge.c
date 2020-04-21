/*
 * Pseudo device supporting the graphics engine:
 *	- note that this driver is closely coupled with the fbc driver,
 *	  and that both must be config'd into a system for the system
 *	  to run as well as link
 *	- the mouse driver is also requisite if the fbc driver is used...
 *
 * $Source: /d2/3.7/src/sys/gl1/RCS/ge.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:28:20 $
 */

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/systm.h"
#include "../h/setjmp.h"
#include "machine/cpureg.h"
#include "../gl1/kgl.h"
#include "../gl1/shmem.h"
#include "../gl1/gfdev.h"
#include "../gl1/grioctl.h"

extern	jmp_buf txenv;

/*
 * gefind:
 *	- find number of ge chips on the gf board
 *	- do this by configuring the previous n-1 ge's as wires, then
 *	  send down an illegal command to the nth ge, and look at the
 *	  trap bits to see which ge traped
 *	- we assume caller is at correct spl
 */
gefind()
{
	register short i, j, trapmask;
	register short head = 0;

	gemask = 0;			/* mask of chips present */
	gefound = 0;			/* count of chips present */

    /* flag presence of TTL head FIFO */
	if (gereset() && ((FBCflags & FITRAP_BIT_) == 0))
		gemask = head = 2;

    /* which trap bit expected if GE not there */
	trapmask = GETRAP_BIT(1);
	for (i = 1; i < 13; i++) {
		(void) gereset();

	    /* config intfifo, no lowater allowed */
		if (!head)
			GEOUT(0x3D00);
		DELAY(200);

	    /* configure known GE's as wires */
		for (j = gefound; j; --j) {
			GEOUT(0xFF38);
			DELAY(200);
		}

	    /* configure GE to probe */
		GEOUT(0xFF09);
		DELAY(200);

	    /* send an illegal command */
		GEOUT(2);
		DELAY(200);
		if (GEflags & trapmask) {
			gemask++;
			gefound++;
		}
		gemask <<= 1;
		trapmask <<= 1;
	}
	(void) gereset();		/* make pipe undefined */
}

/*
 * gereset:
 *	- reset the ge
 */
gereset()
{
	short head;

	GEflags = GERESET1;
	DELAY(200);
	head = FBCflags & FITRAP_BIT_;
	GEflags = GERESET3;
	return head;
}

/*
 * ge_intr:
 *	- process a ge interrupt or mouse interrupt
 */
ge_intr()
{
	register struct shmem *sh = (struct shmem *)SHMEM_VBASE;
	register long countdown;
	short dummy;

    /* see if interrupt came from mouse/mailbox/parallel port */
	if (~(*(short *)EREG) & (ER_MOUSE|ER_MAILBOX|ER_RCV|ER_XMIT)) {
		dummy = *(short *)MBUT;		/* clear interrupt */
		return;
	}

	if (GEflags & TRAPINT_BIT) {
		GEflags = sh->ge_status | ENABTRAPINT_BIT_;
		GEflags = sh->ge_status;
		dprintf("geintr: ge trap, flags=0x%04x\n", GEflags);
		grkill();
	}

	if (GEflags & FIFOINT_BIT) {
		if (GEflags & HIWATER_BIT) {
			countdown = 100000;
			while (GEflags & HIWATER_BIT) {
				if ((FBCflags & INTERRUPT_BIT_) == 0)
					fbcprogint(fbcstatus);
				if (--countdown == 0) {
					dprintf("geintr: hardware is hung\n");
					grkill();
				}
			}
		}
		GEflags = sh->ge_status | ENABFIFOINT_BIT_;
		GEflags = sh->ge_status;
	}
}

/*
 * grkill:
 *	- this is called during a nasty error from the hardware
 *	  to destroy the currently running graphic process
 *	  and to reset the hardware
 *	- if we were running in tx_redisplay(), then force a reset of
 *	  the hardware, and longjmp() back to the redisplay code
 */
grkill()
{
	register struct shmem *sh = (struct shmem *)SHMEM_VBASE;
	int s;

    /* kill currently running graphics process */
	if (grproc) {
		dprintf("grkill: killing process #%d\n", grproc->p_pid);
		psignal(grproc, SIGKILL);
		runrun++;		/* force reschedule, just in case */
	}

    /* see if we are recursing because of brain damaged hardware */
	if (kgr & KGR_DIDRESET) {
		panic("kernel gr reset failed");
	}

    /* see if we were running in tx_redisplay() */
	s = spl4();
	if (kgr & KGR_REPAINTING) {
		sh->autocursor = 0;
		sh->IdleMode = 1;
		kgreset(1);			/* reset without repaint */
		splx(s);
		kgr |= KGR_DIDRESET;
		longjmp(txenv, 1);		/* try once more */
		/* NOTREACHED */
	}

    /* reset hardware (fully!) */
	kgr |= KGR_BUSY;
	sh->autocursor = 0;
	sh->IdleMode = 1;
	kgreset(3);			/* do hard reset + repaint */
	kgr &= ~KGR_BUSY;
	splx(s);
}

/*
 * gr_free:
 *	- release usage of the graphics pipe
 *	- do a hard reset of the hardware state, if the textport is on
 */
gr_free()
{
	extern short blinkcount;
	extern int (*softintr[])();

	if (u.u_procp == grproc) {
		grproc = NULL;
		u.u_procp->p_flag &= ~SGR;
		cxput(u.u_procp, 1);
		if (gl_dialboxinuse) {
			softintr[3] = NULL;
			dial_ints(0);
		}
		blinkcount = 0;			/* peter 8/22/84 */
		if (!gr_tpblank)
			greset();
	}
}
