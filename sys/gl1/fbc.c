/*
 * Pseudo device supporting the frame buffer controller
 *
 * $Source: /d2/3.7/src/sys/gl1/RCS/fbc.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:28:16 $
 */

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/systm.h"
#include "machine/cpureg.h"
#include "../gl1/kgl.h"
#include "../gl1/gfdev.h"
#include "../gl1/shmem.h"

#undef	DEBUG
#undef	DEBUG_INTS

/* dissmiss current interrupt */
#define CLEARINT()	FBCclrint

/* store successive n wds of interrrupt data in a list p */
#define StoreShorts(p, n) { \
	register short count = (n); \
	while (count--) { \
		CLEARINT(); \
		(p) = FBCdata; \
	} \
}

/* interrupt codes from FBC */
#define INTHIT		1
#define INTFEEDDATA	2
#define INTPIXEL	3
#define INTSTOREMM	4
#define INTILLEGAL	5
#define INTSTOREVP	6
#define INTCHARPOSN	7
#define INTFEEDCMD	8
#define INTEOF		9
#define INTBBOXCLIP	10
#define INTBBOXSIZE	11
#define INTDUMP		12
#define INTDBLFEED	13
#define INTXFORMPT	14
#define INTUNIMPL	15
#define INTBPCBUS	16
#define INTRGBPIXEL	17
#define INTRAMERR	18
#define INTCURSOR	19

/*
 * fbcreset:
 *	- reset the update controller to a known state
 */
fbcreset()
{
	register short  i, j;

	GEflags = GERESET3;
	do {
		GEflags = GERESET1;	/* reset GE's */
		FBCflags = 0xFF;	/* hard reset the FBC! */
		DELAY(500);		/* allow FBC to go to state 3ff */
		GEflags = GERESET3;
		FBCflags = STARTDEV;
		CLEARINT();		/* FBC (re-)executes 3ff */
		FBCflags = FORCEREQ_BIT_;
		FBCflags = RUNMODE;
		for (i = 0; i < 20; i++)
			CLEARINT();	/* ensure all startup interrupts gone */
		if ((FBCflags & INTERRUPT_BIT_) == 0)
			printf("fbcreset: can't reset FBC\n");
		if ((FBCflags & GEREQ_BIT_) == 0)
			printf("fbcreset: can't reset outfifo\n");
		FBCflags = READOUTRUN;
		j = FBCdata;		/* get scratch ram size -1 */
		FBCflags = fbcstatus = RUNMODE;
		for (i = 0; i < 20; i++)
			CLEARINT();
	} while ((j != 0xfff && j != 0x7ff) || FBCdata != 0x40);
	fbcshmem();
}

/*
 * fbcshmem:
 *	- store results of fbc reset
 *	- intialize shmem data structures
 */
fbcshmem()
{
	register struct shmem *sh = (struct shmem *)SHMEM_VBASE;

	sh->IdleMode = 1;
	sh->autocursor = 0;
	sh->intbuf = sh->p_intbuf = (short *)(SHMEM_VBASE + NBPG);
	sh->intbuflen = sh->p_intbufsize = ctob(1);
	sh->inttoken = sh->p_inttoken = &sh->inttokenbuf[0];
	sh->p_inttokensize = TOKENBUFSIZE;
	sh->ge_status = GERESET3;
	sh->gr_cfr = CONFIGBOTH;
	strcpy(sh->version, gl_version);
	setmode(MD_SINGLE);
	fbccursor = 1;
	qreset();
}

fbc_intr()
{
	register struct shmem *sh = (struct shmem *)SHMEM_VBASE;
	register ushort fbc_flags;
	extern short blinkcount;

	fbc_flags = FBCflags;
	if (!(fbc_flags & NEWVERT_BIT_)) {
		mouse();
		sh->RetraceReceived++;
		sh->gr_retraces++;
		if(blinkcount > 0)		/* peter 8/22/84 */
			do_blink();

	    /* reset vertical interrupt */
		Disabvert(sh->ge_status);
		Enabvert(sh->ge_status);
		if (sh->autocursor && fbccursor) {
/* undraw/drawing cursor 
 * By raising HOSTFLAG, the FBC will be forced to generate an interrupt
 * when it reaches the command dispatch state.  However, there may be
 * a programmed interrupt in progress, so we must service until we find
 * the cursor interrupt.  If interpreter is in Idle Mode, passthrus are
 * needed to force the FBC to dispatch.
 */
			fbccursor = 0;
			FBCdata = cursorx;
			FBCflags = fbcstatus = RUNMODE | HOSTFLAG;
			if (sh->IdleMode) {
				GEOUT(8);
				GEOUT(8);
			}
		}
	}
	if ((FBCflags & INTERRUPT_BIT_) == 0)
		fbcprogint(fbcstatus);
}

fbcprogint(fbc_flags)
	ushort fbc_flags;
{
	register struct shmem *sh = (struct shmem *)SHMEM_VBASE;
	register ushort fbc_data;
	register ushort intcode;
	register ushort intct;

    /* get interrupt code from fbc after processing retrace interrupt */
	if (sh->RetraceExpected) {
		GFenabvert(sh->ge_status, READOUTRUN);
		intcode = FBCdata;
		GFenabvert(sh->ge_status, fbc_flags);
	} else {
		GFdisabvert(sh->ge_status, READOUTRUN);
		intcode = FBCdata;
		GFdisabvert(sh->ge_status, fbc_flags);
	}

    /* decode interrupt */
#ifdef	DEBUG_INTS
	{ extern duputchar(); }
	if(intcode != INTCURSOR) {
	    fprintf(duputchar,"fbcintr: intcode=%d\n", intcode);
	}
#endif
	switch (intcode) {
	  case INTHIT: 
		CLEARINT();	/* get hit bits */
		sh->hitbits |= FBCdata;
#ifdef	DEBUG_INTS
{ extern duputchar(); }
fprintf(duputchar,"fbcint: sh->hitbits=0x%x\n", sh->hitbits);
#endif
		CLEARINT();	/* throw away char count */
		CLEARINT();	/* get name stack depth */
		intct = FBCdata;
		CLEARINT();	/* throw away bogus name */
		if (sh->intbuflen < intct) {	/* more than fit */
			while (sh->intbuflen > 0) {
				CLEARINT();
				*sh->intbuf++ = FBCdata;
				sh->intbuflen--;
				intct--;
			}
			while (intct-- > 0)
				CLEARINT();
		} else {				/* all of them */
			sh->intbuflen -= intct;
			StoreShorts(*sh->intbuf++, intct);
		}
		CLEARINT();
		break;

	  case INTFEEDDATA: 
	    /* collect 8 coord wds  */
		StoreShorts(*sh->intbuf++, 8);
	    /* account for 4 coord pairs */
		(*sh->inttoken)++;
		CLEARINT();
		break;

	  case INTPIXEL: 
		*sh->intbuf++ = FBCpixel;
		for (intct = 0; intct < 15; intct++) {
			CLEARINT();
			*sh->intbuf++ = FBCpixel;
		}
		CLEARINT();
		break;

	  case INTSTOREMM: 
	  case INTSTOREVP: 
		StoreShorts(*sh->intbuf++, 32);
		CLEARINT();
		break;

	  case INTILLEGAL: 
		CLEARINT();
		dprintf("fbc: bad command, FBCdata=0x%04x\n", FBCdata);
		grkill();
		CLEARINT();
		break;

	  case INTCHARPOSN: 
		StoreShorts(*sh->intbuf++, 3);
		CLEARINT();
		break;

	  case INTFEEDCMD: 
/*
 * inttoken points to the index buffer for the
 * feedback commands.  It points to the previous
 * location in the buffer, so that upon receiving the
 * feedback command token from the FBC, the interrupt
 * routine may record the presence or absence of data
 * for the previous command.  i.e., if data has
 * arrived between the last feedback command and this
 * one, then the data buffer contains valid fedback
 * data for the previous command, otherwise the
 * command was clipped out.
 */
		if (sh->clippnt) {
			if ((!sh->firstfeed) && (*sh->inttoken == 0))
				/* no coords last time */
				sh->intbuf += 8;
			sh->firstfeed = 0;
			sh->inttoken++;
			*sh->inttoken = 0;/* default: no data rcvd  */
		/* record occurence of token in the list */
		} else {
			if (sh->firstfeed != 1) {
		    /* Not the first feedback command. */
				if (*sh->inttoken == 2) {
/* Seen two data points since last feedback command.
    Must be a move and a draw. */
					*(sh->inttoken++) = 1;
					*(sh->inttoken++) = 0;
				} else
				if (*sh->inttoken == 1) {
/* Only seen one data point since last feedback command.
    Must be a draw. */
					if (sh->firstfeed == 0)
						*(sh->inttoken++) = 0;
/* ... unless this is the second feedback command,
    which always follows a move. */
					else
						sh->inttoken++;
				}
				sh->firstfeed = 0;
				*sh->inttoken = 0;
			} else {
				sh->firstfeed = 2;
				sh->inttoken++;
				*sh->inttoken = 0;
			}
		}
		CLEARINT();
		break;

	  case INTEOF: 
	  case INTBBOXCLIP: 
	  case INTBBOXSIZE: 
		sh->EOFpending = 0;
		sh->EOFcode = intcode;
		CLEARINT();
		sh->EOFbits = FBCdata;
		CLEARINT();
		break;

	  case INTDUMP: 
	  case INTDBLFEED: 
	  case INTRAMERR: 
		dprintf("fbc: diagnostic interrupt, intcode=0x%04x\n",
			      intcode);
		grkill();
		CLEARINT();
		break;

	  case INTXFORMPT: 
		StoreShorts(*sh->intbuf++, 8);
		CLEARINT();
		break;

	  case INTUNIMPL: 
		CLEARINT();
		dprintf("fbc: unimplemented command, FBCdata=0x%04x\n",
			      FBCdata);
		grkill();
		CLEARINT();
		break;

	  case INTRGBPIXEL: 
		*sh->intbuf++ = FBCpixel;
		for (intct = 0; intct < 31; intct++) {
			CLEARINT();
			*sh->intbuf++ = FBCpixel;
		}
		CLEARINT();
		break;
	  case INTCURSOR: 
		FBCdata = cursory;
		CLEARINT();			/* dismiss cursor interrupt */
		FBCflags = fbcstatus = RUNMODE;/* forced into normal mode */
		DELAY(500);
		fbccursor = 1;
		break;
	  default: 
		dprintf("fbc: unexpected interrupt, intcode=0x%04x\n",
			      intcode);
		grkill();
		CLEARINT();
		break;
	}
}
