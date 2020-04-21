/*  readrun.c		<<  GF2/UC4  >>
 *
 *	READ_RUNLENGTH (count)
 *
 *  specify # pixels to read & encode.  Probably should limit to 500/cmd.
 *  Deals with horizontal spans of pixels only.
 *  HOST must set up PFIColumn,PFIXDown,PFIYDown but FBC will handle
 *	PFICD and PFIRead
 *  No. of planes set up via PIXEL_SETUP in attributes.c
 *
 *	subroutines:
 *		NEXTMA
 *		PIXGETXY
 *
 *	data: word if planes<3; 2 words if planes=3; 3 words if planes=7
 *		"planes" comes from scratch:
 *			1=BA
 *			2=DC
 *			3=DC,BA
 *			7=R,G,B
 *	Read/write start from charposn if valid
 *	Host sets up configuration/mode for increment, patterning, etc.
 *	Microcode FORCES BOTH UPDATE config BITS ON while reading.
 *	Reads (count) pixels back to host; updates char position
 *	Use PIXEL PARAMS to set up end of scan area for PFI.
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"

readrun()
{
newfile("readrun.c");

#define _FLAG	4
#define _PX	5
#define _NPIX	6
#define _NADR	8
#define _CURAD	9
#define _CURN	10
#define _PY	11
#define _OLD	12

label(READ_RUNLENGTH)
	_NS REGREG(ONES,0,_NPIX); GEGET; DOJSUB(PIXGETXY);_ES
		  /* -1 to NPIX; go ld charposn, point at PASSCHARS */

	_NS REGHOLD; COND(IFNZ); DOJUMP(OK_TO_READ); _ES
						/* check posn valid */

	_NS LOADIMM(_TEMP,_INTPIXEL); LDOUT;
	 INTERRUPTHOST; CONST(_INTPIXEL);_ES	/* "pixel" host interrupt  */

	_NS REGREG(ZERO,0,_NPIX); LDOUT; INTERRUPTHOST; DOJSUB(OUTSEND); _ES
						/* say zero pixels */

	_NS REGHOLD; GEGET; DOJUMP(DISPATCH); _ES	/* quit */

label(OK_TO_READ)
	_NS LOADDI(INRJUST); SETROP(0,ALL16); SETSOP(NONQOP,_NPIX,RAMNOP);
	 ALUOP(ADD); YQ(FF,OLDQ,REGWRE); BPC(SETADDRS); _ES
					/* count - 1 to NPIX, outreg */

	_NS LOADDI(OUTPUTCOUNT); REGREG(MOVE,_FLAG,_FLAG); SEQ(LDCT); _ES
				/* prepare loop count; relookat planes code */

	_NS REGREG(INCR,_FLAG,_FLAG); COND(IFNNEG); DOJUMP(RUNREAD_ABCD); _ES

	_NS REGHOLD; COND(IFZ); DOJUMP(RUNREAD_CD); _ES

label(RUNREAD_ABCD)
label(RUNREAD_CD)
/* read AB */
	_NS LOADMAR(_CONFIG+1); CONST(_CONFIG+1); _ES

	_NS IMMRAM(ANDRS,_PFIMASK,_TEMP,HOLD); CONST(_PFIMASK); _ES

	_NS IMMREG(ORRS,_PFIREAD+_UPDATEBITS,_TEMP); BPC(LOADCONFIG);
	 CONST(_PFIREAD+_UPDATEBITS); _ES
				/* tweak config reg for autoinc on AB read */

	_NS LOADIMM(_NADR,_STARTLIST); LDMAR; CONST(_STARTLIST); _ES
				/* init next useable address for N ct */

	_NS REGREG(INCR,_NADR,_CURAD); BPC(READPIXELAB); _ES
				/* init cur addr; do read of 1st pixel */

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_OLD,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE);
	 READBPCBUS; LDOUT;
	 COND(IFFALSE); SEQ(CJPP); _ES	/* wait for ack; spy on bus */

	_NS REGREG(INCR,_ZERO,_CURN); INCMAR; SEQ(RPCT); NEXT(RDRUN_INIT); _ES
					/* N = 1; point to ist data */

/* note -- assume never read <3 pixels ... */

label(RDRUN_INIT)
	_NS RAM(RAMWR,_OLD,HOLD); _ES	/* save 1st pixel */

	_NS REGREG(INCR,_CURAD,_CURAD); LDMAR; _ES	/* pt beyond data */

	_NS REGHOLD; BPC(READPIXELAB); _ES	/* read 2nd pixel */

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE);
	 READBPCBUS; LDOUT;
	 COND(IFFALSE); SEQ(CJPP); _ES	/* wait for ack; spy on bus */

	_NS REGCOMP(EQ,_OLD,_TEMP); SEQ(RPCT); NEXT(R2INIT); _ES
						/* compare; decr counter */

label(R2INIT)
	_NS REGREG(MOVE,_TEMP,_OLD); COND(IFNQ); DOJUMP(READRUN_DIFF); _ES
				/* copy new to old; test for state to enter */

	_NS REGREG(INCR,_CURN,_CURN); DOJUMP(READRUN_SAME); _ES
					/* 1st 2 pixels are the same */

label(READRUN_DIFF)
	_NS REGHOLD; BPC(READPIXELAB); _ES	/* do the read */

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE);
	 READBPCBUS; LDOUT;
	 COND(IFFALSE); SEQ(CJPP); _ES	/* wait for ack; spy on bus */

	_NS REGCOMP(EQ,_OLD,_TEMP); _ES

	_NS REGHOLD; COND(IFEQ); DOJUMP(DIFF_TO_SAME); _ES

label(DIFF_TO_DIFF)
	_NS RAM(RAMWR,_OLD,HOLD); _ES	/* write out old pixel */

	_NS REGREG(INCR,_CURAD,_CURAD); INCMAR; _ES	/* next scratch */

	_NS REGREG(INCR,_CURN,_CURN); _ES		/* inc N */

	_NS REGREG(MOVE,_TEMP,_OLD); SEQ(RPCT); NEXT(READRUN_DIFF); _ES
				/* old gets new; still in DIFF state */

	_NS REGHOLD; DOJUMP(RUNREAD_DIFFDONE); _ES

label(DIFF_TO_SAME)
	_NS REGREG(COM2,_CURN,_CURN); _ES	/* negate N */

	_NS REGREG(MOVE,_NADR,_NADR); LDMAR; COND(IFZ); DOJUMP(NO_DIFFS); _ES
		/* point to N addr; if changing same to same, don't use */

	_NS RAM(RAMWR,_CURN,HOLD); _ES		/* write N */

	_NS REGREG(MOVE,_CURAD,_NADR); LDMAR; _ES  /* new N adr - go there */

label(D_T_S)
	_NS LOADIMM(_CURN,2); INCMAR; CONST(2); _ES	/* N=2; pt to N+1 */

	_NS RAM(RAMWR,_TEMP,HOLD); _ES		/* save new data */

	_NS IMMREG(ADD,2,_CURAD); LDMAR; CONST(2); _ES	/* new cur adr */

	_NS REGREG(MOVE,_TEMP,_OLD); SEQ(RPCT); NEXT(READRUN_SAME); _ES
						/* old gets new; now SAME st*/

	_NS REGHOLD; DOJUMP(RUNREAD_SAMEDONE); _ES

label(NO_DIFFS)
	_NS REGREG(SUBSR,_ZERO,_CURAD); DOJUMP(D_T_S); _ES
			/* adjust cur adr and set up for SAME state */

label(READRUN_SAME)
	_NS REGHOLD; BPC(READPIXELAB); _ES	/* do the read */

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE);
	 READBPCBUS; LDOUT;
	 COND(IFFALSE); SEQ(CJPP); _ES	/* wait for ack; spy on bus */

	_NS REGCOMP(EQ,_OLD,_TEMP); _ES

	_NS REGHOLD; COND(IFNQ); DOJUMP(SAME_TO_DIFF); _ES

label(SAME_TO_SAME)
	_NS REGREG(INCR,_CURN,_CURN); SEQ(RPCT); NEXT(READRUN_SAME); _ES
						/* just increment N count */

	_NS REGHOLD; DOJUMP(RUNREAD_SAMEDONE); _ES

label(SAME_TO_DIFF)
	_NS REGREG(MOVE,_NADR,_NADR); LDMAR; _ES

	_NS RAM(RAMWR,_CURN,HOLD); _ES		/* write out N */

	_NS REGREG(MOVE,_CURAD,_NADR); _ES	/* new N location */

label(SAME_TO_DIFF+2)
	_NS REGREG(INCR,_CURAD,_CURAD); LDMAR; _ES	/* next adr for data*/

	_NS REGREG(ZERO,0,_CURN); _ES		/* N = 0 */

	_NS REGREG(MOVE,_TEMP,_OLD); SEQ(RPCT); NEXT(READRUN_DIFF); _ES
				/* old = new; new state is DIFF */

label(RUNREAD_DIFFDONE)
	_NS RAM(RAMWR,_OLD,HOLD); _ES	/* write out last val */

	_NS REGREG(COMPROP,P0,_CURN,_CURN); _ES	/* inc N, then 2's comp */

	_NS REGREG(MOVE,_NADR,_NADR); LDMAR; _ES	/* point to N */

	_NS RAM(RAMWR,_CURN,HOLD); _ES		/* write it */

	_NS REGREG(INCR,_CURAD,_CURAD); DOJUMP(RDRUN_DONE); _ES
							/* inc cur adr */

label(RUNREAD_SAMEDONE)
	_NS REGREG(MOVE,_NADR,_NADR); LDMAR; _ES	/* point to N */

	_NS RAM(RAMWR,_CURN,HOLD); _ES		/* write it */

label(RDRUN_DONE)
	_NS LOADIMM(_TEMP,_INTPIXEL); LDOUT;INTERRUPTHOST;CONST(_INTPIXEL);_ES

	_NS IMMREG(SUBSRC,_STARTLIST,_CURAD); LDOUT;
	 INTERRUPTHOST; CONST(_STARTLIST); _ES

	_NS REGREG(SUBSR,_ZERO,_CURAD); LDOUT;
	 LOADDI(OUTPUTREG); COND(IFFALSE);  SEQ(CJPP); _ES
			/* while showing host wd count, decr for loopct */

	_NS LOADMAR(_STARTLIST); CONST(_STARTLIST); _ES

	_NS REGHOLD; LOADDI(OUTPUTCOUNT); SEQ(PUSH); _ES

/* word-loop */
	_NS RAM(RAMRD,_TEMP,HOLD); LDOUT; INTERRUPTHOST; DOJSUB(OUTSEND); _ES

	_NS REGHOLD; SEQ(LOUP); _ES

	_NS REGHOLD; GEGET; DOJUMP(DISPATCH); _ES

}
