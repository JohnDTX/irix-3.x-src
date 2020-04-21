/**************************************************************************
 *									  *
 * 		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/
#include "sys/types.h"
#include "shmem.h"
#include "window.h"
#include "gf2.h"
#include "grioctl.h"
#include <setjmp.h>

#ifdef PM2
#	define	DELAY(x)	{ register y; y = x; while (y--); }
#endif
#ifdef IP2
#	define	DELAY(x)	{ register y; y = 10*x; while (y--); }
#endif

/*
 * gefind:
 *	- find number of ge chips on the gf board
 *	- do this by configuring the previous n-1 ge's as wires, then
 *	  send down an illegal command to the nth ge, and look at the
 *	  trap bits to see which ge traped
 *	NOTE -- config will need to be changed for GA chips
 */
gefind()
{
	register short i, j, trapmask;
	register short headGA = 0;
	int s;

	s = spl7();
	gemask = 0;			/* mask of chips present */
	gefound = 0;			/* count of chips present */

	GEflags = GERESET1;		/* jam the pipe reset line */
	if (FBCflags & FITRAP_BIT_) {
		gemask = 2;
		headGA = 1;
	}

    /* which trap bit expected if GE not there */
	trapmask = GETRAP_BIT(1);
	for (i = 1; i < 13; i++) {
		(void) gereset();

	    /* config intfifo, no lowater allowed */
		GEOUT(0x3A00);
		DELAY(100);

		if (headGA) {
			GEOUT(0xff01);
			DELAY(100);
		}

	    /* configure known GE's as wires */
		for (j = gefound; j; --j) {
			GEOUT(0xFF38);
			DELAY(100);
		}

	    /* configure GE to probe */
		GEOUT(0xFF09);
		DELAY(100);

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
	if (headGA) ++gefound;		/* account for head GA */

/* test for tail GA */
	GEflags = GERESET1;
	if (FBCflags & FOTRAP_BIT_) {
		gemask |= 1;
		++gefound;
	}
	(void) gereset();	
	splx(s);
}

/*
 * gereset:
 *	- reset the ge. assumes that fbcreset has already been done
 */
gereset()
{
	GEflags = GERESET1;
	DELAY(200);
	GEflags = GERESET3;
	GEflags = gl_gestatus;
}

/*
 * ge_intr:
 *	- process a ge interrupt or mouse interrupt
 */
ge_intr()
{
	register struct shmem *sh = gl_shmemptr;
	register long countdown;
	register short *geflags = (short *)&GEflags;

#ifdef	PM2
	mouse_intr();
#endif
	if (*geflags & TRAPINT_BIT) {
		*geflags = gl_gestatus | ENABTRAPINT_BIT_;
		*geflags = gl_gestatus;
		iprintf("ge_intr: ge trap, flags=0x%04x\n", *geflags);
		grkillproc();
	}

	if (*geflags & FIFOINT_BIT) {
#ifdef PM2
		countdown = 1000000;
#endif
#ifdef IP2
		countdown = 10000000;
#endif
		while (*geflags & HIWATER_BIT) {
			if ((FBCflags & INTERRUPT_BIT_) == 0) {
				fbc_progintr();
			}
			if (--countdown == 0) {
				iprintf("ge: ge & fbc hung\n");
				grkillproc();
			}
		}

		*geflags = gl_gestatus | ENABFIFOINT_BIT_;
		*geflags = gl_gestatus;
	}

	if (!(gl_gestatus & ENABTOKENINT_BIT_)) {
		*geflags = gl_gestatus | ENABTOKENINT_BIT_;
		if (PIPENOTBUSY) {
			gl_gestatus |= ENABTOKENINT_BIT_;
			gr_os_stopproc();
		}
		*geflags = gl_gestatus;
	}
}
