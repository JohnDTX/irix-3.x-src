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

/* fbc.c	<< GL2 >>
 *
 * Support code for the frame-buffer controller.  Handles device
 * intialization and interrupts.
 */

#include "sys/types.h"
#include "shmem.h"
#include "window.h"
#include "gf2.h"
#include "gl2cmds.h"
#include "immed.h"
#include "errno.h"


short	gl_fastfeed;
short	gl_microcodeloaded;

#undef	DEBUG
#undef	DEBUG_INTS

#define	DELAY(x)	{ register y; y = x; while (y--); }

#ifdef V
#define sustuff(oshandle, fbaddr, n, regs)		\
	{ register short i;				\
	  register long ctx;				\
	  register short *addr = fbaddr;		\
		ctx = gr_setshmem(oshandle);		\
		for(i=0;i<n; i++)			\
			*addr++ = *regs;		\
		gr_restoreshmem(ctx);			\
	}
#define sushort(oshandle, fbaddr, x)			\
	{ register long ctx;				\
		ctx = gr_setshmem(oshandle);		\
		*(fbaddr) = x;				\
		gr_restoreshmem(ctx);			\
	}
#endif

StoreStuff(n)
	register short n;
{
	GEflags = gl_gestatus | AUTOCLEAR_BIT;
	if (gl_fastfeed) {
		register short *addr = gl_fbaddr;
		register short *fbcaddr = (short *)&FBCdata;

		while (n) {
			if (n > 7) {
				*addr++ = *fbcaddr;	/* 1 */
				*addr++ = *fbcaddr;	/* 2 */
				*addr++ = *fbcaddr;	/* 3 */
				*addr++ = *fbcaddr;	/* 4 */
				*addr++ = *fbcaddr;	/* 5 */
				*addr++ = *fbcaddr;	/* 6 */
				*addr++ = *fbcaddr;	/* 7 */
				n -= 7;
			}
			*addr++ = *fbcaddr;		/* and baby makes 8 */
			n--;
		}
		gl_fbaddr = addr;
	} else {
		sustuff(gl_gfport->ic_oshandle, gl_fbaddr, n, &FBCdata);
		gl_fbaddr += n;
	}
	GEflags = gl_gestatus;
}

StoreWord(x)
{
	if (gl_fastfeed)
		*gl_fbaddr++ = x;
	else {
		sushort(gl_gfport->ic_oshandle, gl_fbaddr, x);
		gl_fbaddr++;
	}
}

static unsigned short fbccursor;

/*
 * fbc_reset:
 *	- reset the update controller to a known state
 */
#include "gf2init.c"

unsigned short codebuf[128];
extern unsigned short ucode[][4];
extern unsigned short dividetab[];
extern unsigned short swizzletab[];
extern unsigned short leftmask[];

fbcge_reset() {
	fbc_reset();			/* init fbc hardware */
	fbc_reset();			/* init fbc hardware */
	gefind();			/* find the ge's (and reset them) */
	geconfigure();			/* configure them */
	write_scratch(MASKTAB,32,leftmask);	/* includes left and right */
	write_scratch(SWIZZLETAB,256,swizzletab);
	write_scratch(DIVIDETAB,2048,dividetab);
}

/* write microcode and initialize the GF board */
/* try twice */
fbc_reset()
{
	im_setup;
	register short  j;
	register int s;
	int secondtry;

	secondtry = 0;

again:
	s = spl7();
	if (!gl_microcodeloaded) {
		/* only load the microcode the first time */
		if (Micro_Write(ucode,0,4096) >=0) {
			iprintf("fbcreset: micro write error\n");
			goto bad;
		} else
			gl_microcodeloaded = 1;
	}

	j = FBC_Reset();
	if ( (j != 0xfff) && (j != 0x7ff) ) {
		iprintf("fbcreset: can't reset FBC: j=%x\n", j);
		goto bad;
	}
	if (GEflags) {
		iprintf("fbcreset: can't reset GE pipe\n");
		goto bad;
	}

	gl_fbcversion = Get_Micro_Version();
	if ( (gl_fbcversion & 0xff00) != 0x0200) {
		iprintf("fbcreset: bad microcode version\n");
		goto bad;
	}

	FBCflags = gl_fbcstatus = RUNMODE;
	GEflags = gl_gestatus = GERUNMODE
				& ~ENABFIFOINT_BIT_
				& ~ENABTRAPINT_BIT_
				& ~ENABVERTINT_BIT_
				& ~ENABFBCINT_BIT_;
	im_freepipe;
	splx(s);
	return;

bad:
	if (!secondtry) {
		secondtry = 1;
		j = 100000;
		while (--j)
			;
		goto again;
	}
	iprintf("fbc: dead meat\n");
}

gl_microwrite(code,base,nstates)
	unsigned short code[][4];
	int base, nstates;
{
	register statesleft = nstates;
	register stateblok;
	register blok;
	int sav;

	sav = spl7();
	while (statesleft > 0) {
		stateblok = (statesleft<32) ? statesleft : 32;
		blok = (stateblok<<2) * sizeof(short);
		if(copyin(code,codebuf,blok))
			gl_ioerror = EFAULT;
		if (Micro_Write(codebuf,base,stateblok) >= 0) {
			iprintf("unable to write block of ucode");
			return(0);
		}
		code += stateblok;		/* 4 shorts per state */
		base += stateblok;
		statesleft -= stateblok;
	}
	FBC_Reset();
	FBCflags = gl_fbcstatus;
	GEflags = gl_gestatus;
	gl_microcodeloaded = 1;
	splx(sav);
	return(-1);
}

/*
 *  write_scratch:
 *	write into scratch ram starting at adr for nwds with data from *ptr
 *	Uses pipe -- reset & configure first.
 */
write_scratch(adr, nwds, ptr)
	register unsigned short adr, nwds, *ptr;
{
	register amt;
	register j;
	im_setup;

	while (nwds) {
		amt = (nwds<120) ? nwds : 120;
		im_passthru(amt+3);
		im_outshort(FBCloadram);
		im_outshort(adr);
		im_outshort(amt);
		for (j=0; j < amt; j++) {
			im_outshort(*ptr++);
		}
		adr += amt;
		nwds -= amt;
	}
	im_freepipe;
}

/*
 * fbcshmem:
 *	- store results of fbc reset
 *	- intialize shmem data structures
 */
fbcshmem()
{
	register struct shmem *sh = gl_shmemptr;

	sh->ge_mask = gemask;
	sh->ge_found = gefound;
	fbccursor = 1;
}

fbc_intr()
{
    static short doingretrace = 0;

	if (!(FBCflags & NEWVERT_BIT_)) {
	    /* reset vertical interrupt */
		Disabvert(gl_gestatus);
		Enabvert(gl_gestatus);
		gl_shmemptr->EOFpending &= ~VERTPENDINGBIT;
						/* signal to user */
		if(!doingretrace) {
			doingretrace = 1;
			gl_didswap = 0;
			retrace_softintr();
			if (gl_autocursor && fbccursor && !gl_didswap) {
/* undraw/drawing cursor 
 * By raising HOSTFLAG, the FBC will be forced to generate an interrupt
 * when it reaches the command dispatch state.  However, there may be
 * a programmed interrupt in progress, so we must service until we find
 * the cursor interrupt.  If interpreter is not in Pipe Busy Mode, passthrus 
 * are needed to force the FBC to dispatch.
 */
				fbccursor = 0;
				FBCdata = gl_cursorx-gl_cursorxorgin;
				FBCflags=gl_fbcstatus=gl_fbcstatus | HOSTFLAG;
			}
			if (PIPENOTBUSY && !gl_lock)
				GETOKEN = GEnoop;
			doingretrace = 0;
		}
	}
	fbc_progintr();
}

fbc_progintr()
{
register ushort intcode;
short i;

    /* programmed interrupt (cursor, feedback, error) */
	if ((FBCflags & INTERRUPT_BIT_) == 0) {
	    GFenabvert(gl_gestatus, READOUTRUN);
	    intcode = FBCdata;
	    GFenabvert(gl_gestatus, gl_fbcstatus);
	    if (intcode == _INTCURSOR) {
		FBCdata = gl_cursory-gl_cursoryorgin;
		FBCclrint;			/* dismiss cursor interrupt */
		for (i=0; i<10; i++);	/* needed to avoid race w/ ucode !! */
		FBCflags = gl_fbcstatus = gl_fbcstatus & ~HOSTFLAG;
						/* forced into normal mode */
		fbccursor = 1;
	    } else
		fbc_feedint(intcode);
	}
}

/*
 * fbc_feedint:
 *	- implement feedback and error interrupts
 */
fbc_feedint(intcode)
	ushort intcode;
{
	register struct shmem *sh = gl_shmemptr;
	register ushort *ebuf;
	register unsigned short *fbcclearint = &FBCpixel;
	register short intct;
	register ushort discard;
	ushort minibuf[3];

    /* decode interrupt */
#ifdef	DEBUG_INTS
iprintf("fbc_intr: intcode=%d\n", intcode);
#endif

/*
 * Translate a user "fast-feedback" (feedback into shared memory) into a
 * kernel fast-feedback.  This reduces testing overhead later on in the
 * code.
 */
	/*
	 * sh->fastfeed set and !gl_fastfeed means we are starting, for
	 * the first time, some user fast feedback.  sh->fastfeed clear
	 * and gl_fastfeed set means kernel fast feedback.  Both clear
	 * means slow feedback, or none at all.  In the case of user
	 * fast feedback, we need to do an automatic gl_lock, and
	 * an automatic unlock.
	 */
	if (sh->fastfeed && !gl_fastfeed) {
		gl_fbcount = sh->intbuflen;
		gl_fbaddr = (short *)USERTOKERNEL(sh->intbuf);
		if ((gl_fbaddr < (short *)gl_shmemptr) ||
		    (gl_fbaddr >= (short *)(gl_shmemptr+1))) {
		iprintf("fbc_feedint: bad user feed addr=%x newaddr=%x\n",
						  sh->intbuf, gl_fbaddr);
			gl_fbaddr = sh->smallbuf;
			gl_fbcount = 1;		/* force discard */
		}
		gl_fastfeed = 1;
#ifdef NOTDEF
		gl_lock = 1;			/* auto lock */
#endif
	}

	switch (intcode) {
	  case _INTHIT: 
		*fbcclearint = 1;	/* get hit bits */
#ifdef IP2
asm("\tnop");
asm("\tnop");
#endif
		sh->hitbits |= FBCdata;
		*fbcclearint = 1;	/* throw away char count */
#ifdef IP2
asm("\tnop");
asm("\tnop");
#endif
		*fbcclearint = 1;	/* get name stack depth */
#ifdef IP2
asm("\tnop");
asm("\tnop");
#endif
		intct = FBCdata + 1;	/* FBC count is 1 too few */

		if((sh->numhits < 0) || (gl_fbcount < (intct + 1))) {
		    while (intct-- > 0) {
			*fbcclearint = 1; 
#ifdef IP2
asm("\tnop");
asm("\tnop");
#endif
		    }
		    *fbcclearint = 1;
		    if(sh->numhits > 0)
			sh->numhits = -(sh->numhits);
		} else {
		    *fbcclearint = 1;
		    ++sh->numhits;
		    gl_fbcount -= (intct + 1);
		    StoreWord(intct);
		    StoreStuff(intct);
		}
		break;

	  case _INTPIXEL:	/* AB or CD planes */
		*fbcclearint = 1;
		intct = FBCdata;
		*fbcclearint = 1;
		gl_fbcount -= intct;
		StoreStuff(intct);
endfeed:
		if (sh->EOFpending & EOFPENDINGBITS) {
			if (!((--sh->EOFpending) & EOFPENDINGBITS)) {
				gl_fastfeed = 0;
				if (sh->fastfeed) {
#ifdef NOTDEF
					gl_lock = 0;	/* auto unlock */
#endif
					sh->fastfeed = 0;
				}
			}
		} else {
			iprintf("gl_fastfeed=%d EOFpending=%d gfport=%x\n",
						    gl_fastfeed,
						    sh->EOFpending,
						    gl_gfport);
			iprintf("gl_lock=%d gl_fbaddr=%x gl_fbcount=%d\n",
						gl_lock,
						gl_fbaddr,
						gl_fbcount);
			iprintf("sh->fastfeed=%d\n", sh->fastfeed);
			iprintf("wierd EOF stuff for command=%d\n",
					   intcode);
		}
		return;

	  case _INTPIXEL32:	/* AB and CD planes */
		*fbcclearint = 1;
		intct = FBCdata<<1;	/* two words per pixel */
		*fbcclearint = 1;
		gl_fbcount -= intct;
		StoreStuff(intct);
		goto endfeed;
		break;

	  case _INTPIXELRGB:
		*fbcclearint = 1;
		intct = FBCdata<<1;	/* three words per pixel */
		intct += FBCdata;
		*fbcclearint = 1;
		gl_fbcount -= intct;
		StoreStuff(intct);
#ifdef NOTDEF
		intct = FBCdata;	/* no. of pixels */
		gl_fbcount -= intct;
		gl_fbcount -= intct;
		gl_fbcount -= intct;
		*fbcclearint = 1;
		GEflags = gl_gestatus | AUTOCLEAR_BIT;
		for (; intct; --intct) {
			discard = FBCdata;
			StoreWord(discard & 0xff);		/* red */
			StoreWord((ushort)(discard>>8));	/* grn */
			StoreWord(FBCdata);			/* blu */
		}
		GEflags = gl_gestatus;
#endif
		goto endfeed;

	  case _INTCHPOSN:
		*fbcclearint = 1;
		StoreStuff(3);
		goto endfeed;

	  case _INTEOF:
		*fbcclearint = 1;
		*fbcclearint = 1;
		goto endfeed;

	  case _INTDUMP:
		*fbcclearint = 1;
		StoreStuff(16);
		break;

	  case _INTXFORMPT:
		*fbcclearint = 1;
		StoreStuff(8);
		break;

	  case _INTFEEDBACK:
		*fbcclearint = 1;
		intct = FBCdata - 3;		/* don't count EOF sequence */
		discard = 0;
		if (gl_fbcount < intct) {	/*  not enough room */
			discard = intct - gl_fbcount;
						/* number to reject */
			intct = gl_fbcount;
		}
		*fbcclearint = 1;		/* skip storing wordcount */
		if(intct<0) {
		    iprintf("data is %x\n",FBCdata);
	            *fbcclearint = 1;		/* skip storing wordcount */
		    break;
		}
		if (intct)
			StoreStuff(intct);	/* store as many as will fit */
		if(discard) {
			register j;

			GEflags = gl_gestatus | AUTOCLEAR_BIT;
			while (discard--)
				j = FBCdata;
			GEflags = gl_gestatus;
		}
		gl_fbcount -= intct;		/* decr words left */

	    /* get possible EOF sequence */
		ebuf = minibuf;
		GEflags = gl_gestatus | AUTOCLEAR_BIT;
		*ebuf++ = FBCdata;
		*ebuf++ = FBCdata;
		*ebuf++ = FBCdata;
		GEflags = gl_gestatus;

	    /* see if EOF happened */
		if ((minibuf[0] == FBCEOF1) && (minibuf[1] == FBCEOF2) &&
		    (minibuf[2] == FBCEOF3))
			goto endfeed;

		ebuf = minibuf;
		switch(gl_fbcount) {
		  case 0:
			break;
		  default:
		  all3:
		  case 3:
			StoreWord(*ebuf++);
			gl_fbcount--;
		  case 2:
			StoreWord(*ebuf++);
			gl_fbcount--;
		  case 1:
			StoreWord(*ebuf++);
			gl_fbcount--;
			break;
		}
		break;

	  case _INTUNIMPL:
	  case _INTBPCBUS:
		iprintf("fbc: unwanted int, code=0x%04x\n", intcode);
		grkillproc();
		break;

	  case _INTILLEGAL:
		*fbcclearint = 1;
		iprintf("fbc: bad command, FBCdata=0x%04x\n", intcode);
		grkillproc();
		*fbcclearint = 1;
		break;

	  case _INTDIVIDE:
		iprintf("fbc: division trap\n");
		grkillproc();
		*fbcclearint = 1;
		break;

	  default: 
		iprintf("fbc: unexpected interrupt, intcode=0x%04x\n",
			      intcode);
		grkillproc();
		*fbcclearint = 1;
		break;
	}
}

grkillproc()
{
    if(gl_gfport) {
	gr_kill(gl_gfport->ic_oshandle);
	gr_free(gl_gfport->ic_oshandle);
    }
}

/*
 * fbc_setpipeint:
 *	- enable pipe interrupt IF NOT DOING FEEDBACK
 */
fbc_setpipeint()
{
	if (!gl_lock)
		GEflags = gl_gestatus = gl_gestatus & ~ENABTOKENINT_BIT_;
}

/*
 * fbc_pipeisbusy:
 *	- used externally (from the kgl that is) to see if the
 *	  current graphics process can get stopped
 */
fbc_pipeisbusy()
{
	return (PIPEISBUSY) || gl_lock;
}
