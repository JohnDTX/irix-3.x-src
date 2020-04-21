/*  readpixels.c		<<  GF2/UC4  >>
 *
 *	READ_PIXELS (count)
 *
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
 ******>>>		7=R,G,B
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

readpixels()
{
newfile("readpixels.c");

#define _FLAG	4
#define _PX	5
#define _NDRAWN 6
#define _PY	11
#define _WK	12
#define _NWDS	13

label(READ_PIXELS)
/*
 *	read 16- or 32-plane pixel values from n-pixel span
 */

	_NS REGREG(ONES,0,_NDRAWN); GEGET; DOJSUB(PIXGETXY);_ES
		  /* -1 to NDRAWN; go ld charposn, point at PASSCHARS */

	_NS REGHOLD; COND(IFNZ); DOJUMP(OK2READ); _ES	/* check posn valid */

	_NS LOADIMM(_TEMP,_INTPIXEL); LDOUT;
	 INTERRUPTHOST; CONST(_INTPIXEL);_ES	/* "pixel" host interrupt  */

	_NS REGREG(ZERO,0,_NDRAWN); LDOUT; INTERRUPTHOST; DOJSUB(OUTSEND); _ES
						/* say zero pixels */

	_NS REGHOLD; GEGET; DOJUMP(DISPATCH); _ES	/* quit */

label(OK2READ)
	_NS LOADDI(INRJUST); SETROP(0,ALL16); SETSOP(NONQOP,_NDRAWN,RAMNOP);
	 ALUOP(ADD); YQ(FF,OLDQ,REGWRE); BPC(SETADDRS); _ES
					/* count - 1 to NDRAWN, outreg */

	_NS LOADDI(OUTPUTCOUNT); REGREG(MOVE,_FLAG,_FLAG); SEQ(LDCT); _ES
				/* prepare loop count; relookat planes code */

	_NS REGREG(INCR,_FLAG,_FLAG); COND(IFNNEG); DOJUMP(READ_ABCD); _ES

	_NS REGHOLD; COND(IFZ); DOJUMP(READ_CD); _ES

/* first stab: separate into a read loop and a feedback loop, in case reads
 * take a long time, so software never has to busywait on next word.
 */
/* read AB */
	_NS LOADMAR(_CONFIG+1); CONST(_CONFIG+1); _ES

	_NS IMMRAM(ANDRS,_PFIMASK,_TEMP,HOLD); CONST(_PFIMASK); _ES

	_NS IMMREG(ORRS,_PFIREAD+_UPDATEBITS,_TEMP); BPC(LOADCONFIG);
	 CONST(_PFIREAD+_UPDATEBITS); _ES
				/* tweak config reg for autoinc on AB read */

	_NS LOADMAR(_STARTLIST); CONST(_STARTLIST); _ES

label(READ_ABLOOP)
	_NS REGHOLD; BPC(READPIXELAB); _ES	/* do the read */

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE);
	 READBPCBUS; LDOUT;
	 COND(IFFALSE); SEQ(CJPP); _ES	/* wait for ack; spy on bus */

	_NS RAM(RAMWR,_TEMP,INC); SEQ(RPCT); NEXT(READ_ABLOOP); _ES
							/* save and loop */

label(SEND_NWDS)	/* give host NDRAWN words, restore and exit */
	_NS REGREG(MOVE,_NDRAWN,_NWDS); LDOUT; _ES	/* loop count */

label(SEND_N+)
	_NS REGREG(INCR,_NDRAWN,_NDRAWN); LOADDI(OUTPUTCOUNT); SEQ(LDCT); _ES
				/* NDRAWN gets true pixel count */

	_NS LOADIMM(_TEMP,_INTPIXEL); LDOUT;
	 INTERRUPTHOST; CONST(_INTPIXEL);_ES	/* "pixel" host interrupt  */

label(SENDEM)
	_NS REGREG(MOVE,_NDRAWN,_NDRAWN); LDOUT;
	 INTERRUPTHOST; DOJSUB(OUTSEND); _ES	/* send # pixels */

	_NS LOADMAR(_STARTLIST); CONST(_STARTLIST); _ES

label(SENDLP)
	_NS RAM(RAMRD,_TEMP,HOLD); LDOUT; INTERRUPTHOST; DOJSUB(OUTSEND); _ES

	_NS REGHOLD; SEQ(RPCT); NEXT(SENDLP); _ES

label(PIXRESTORE)
	_NS LOADMAR(_CONFIG+1); CONST(_CONFIG+1); _ES

	_NS REGREG(ADD,_NDRAWN,_PX); _ES	/* update x posn */

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(LOADCONFIG);
	 GEGET; DOJUMP(PIXQUIT); _ES		/* restore config and exit */


label(READ_CD)
	_NS LOADMAR(_CONFIG+1); CONST(_CONFIG+1); _ES

	_NS IMMRAM(ANDRS,_PFIMASK,_TEMP,HOLD); CONST(_PFIMASK); _ES

	_NS IMMREG(ORRS,_PFIREAD+_PFICD+_UPDATEBITS,_TEMP); BPC(LOADCONFIG);
	 CONST(_PFIREAD+_PFICD+_UPDATEBITS); _ES
				/* tweak config reg for autoinc on CD read */

	_NS LOADMAR(_STARTLIST); CONST(_STARTLIST); _ES

label(READ_CDLOOP)
	_NS REGHOLD; BPC(READPIXELCD); _ES	/* do the read */

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE);
	 READBPCBUS; LDOUT;
	 COND(IFFALSE); SEQ(CJPP); _ES	/* wait for ack; spy on bus */

	_NS RAM(RAMWR,_TEMP,INC); SEQ(RPCT); NEXT(READ_CDLOOP); _ES
							/* save and loop */

	_NS REGREG(MOVE,_NDRAWN,_NDRAWN); LDOUT; DOJUMP(SEND_N+); _ES
							/* go feed it back */

label(READ_ABCD)
	_NS LOADMAR(_CONFIG+1); CONST(_CONFIG+1); _ES	/* pt to config */

	_NS IMMRAM(ANDRS,_PFIMASK,_TEMP,DEC); CONST(_PFIMASK); _ES
				/* point to mode in case of RGB mode */

	_NS IMMREG(SUBSRC,5,_FLAG); CONST(5); _ES
						/* test for RGB mode */

	_NS REGHOLD; COND(IFNNEG); DOJUMP(READ_RGB); _ES

	_NS IMMREG(ORRS,_PFIREAD+_UPDATEBITS,_TEMP); BPC(LOADCONFIG);
	 CONST(_PFIREAD+_UPDATEBITS); _ES
			/* tweak config reg for autoinc on AB read */

	_NS LOADMAR(_STARTLIST); CONST(_STARTLIST); _ES

label(READ_ABCDLOOP)
	_NS REGHOLD; BPC(READPIXELCD); _ES	/* do the read */

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE);
	 READBPCBUS; LDOUT;
	 COND(IFFALSE); SEQ(CJPP); _ES	/* wait for ack; spy on bus */

	_NS RAM(RAMWR,_TEMP,INC); _ES

	_NS REGREG(MOVE,_NDRAWN,_NWDS); BPC(READPIXELAB); _ES
				  /* do the read and something else useful */

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE);
	 READBPCBUS; LDOUT;
	 COND(IFFALSE); SEQ(CJPP); _ES	/* wait for ack; spy on bus */

	_NS RAM(RAMWR,_TEMP,INC); SEQ(RPCT); NEXT(READ_ABCDLOOP); _ES
							/* save and loop */

	_NS REGREG(ADDC,_NWDS,_NWDS); LDOUT; _ES
					/* (true count*2)-1 to outreg */

	_NS REGREG(INCR,_NDRAWN,_NDRAWN); LOADDI(OUTPUTCOUNT); SEQ(LDCT); _ES
						/* true pixel count */

	_NS LOADIMM(_TEMP,_INTPIXEL32); LDOUT;
	 INTERRUPTHOST; CONST(_INTPIXEL32);_ES	/* "pixel" host interrupt  */

	_NS REGHOLD; DOJUMP(SENDEM); _ES	/* go feed them back */

label(READ_RGB)
	_NS IMMREG(ORRS,_PFIREAD+_PFICD+_UPDATEBITS,_TEMP); BPC(LOADCONFIG);
	 CONST(_PFIREAD+_PFICD+_UPDATEBITS); _ES
					/* autoincrement on CD read */

	_NS RAM(RAMRD,_FLAG,HOLD); _ES			/* fetch mode bits */

	_NS LOADMAR(_STARTLIST); CONST(_STARTLIST); _ES

label(READ_RGBLOOP)
	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_FLAG,RAMNOP);
	 ALUOP(ANDRS); FTODO; BPC(LOADMODE); CONST(~_SWIZZLEBIT); _ES
					/* make sure swizzle bit is clear */

	_NS REGHOLD; BPC(READPIXELAB); _ES  /* do the read */

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE);
	 READBPCBUS; LDOUT;
	 COND(IFFALSE); SEQ(CJPP); _ES	/* wait for ack; spy on bus */

	_NS RAM(RAMWR,_TEMP,INC); _ES	/* store red and green */

	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_FLAG,RAMNOP);
	 ALUOP(ORRS); FTODO; BPC(LOADMODE); CONST(_SWIZZLEBIT); _ES
					/* make sure swizzle bit is set */

	_NS REGREG(MOVE,_NDRAWN,_NWDS); BPC(READPIXELCD); _ES
					/* do the read etc. */

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE);
	 READBPCBUS; LDOUT;
	 COND(IFFALSE); SEQ(CJPP); _ES	/* wait for ack; spy on bus */

	_NS RAM(RAMWR,_TEMP,INC); SEQ(RPCT); NEXT(READ_RGBLOOP); _ES
						/* save blue and loop */

	_NS REGREG(MOVE,_FLAG,_FLAG); BPC(LOADMODE); _ES
						/* restore mode bits */

	_NS REGREG(MOVE,_NWDS,_NWDS); LDOUT; _ES
					/* (true count-1) to outreg */

	_NS REGREG(INCR,_NDRAWN,_NDRAWN); LOADDI(OUTPUTCOUNT); SEQ(LDCT); _ES
							/* true pixel count */

	_NS LOADIMM(_TEMP,_INTPIXELRGB); LDOUT;
	 INTERRUPTHOST; CONST(_INTPIXELRGB);_ES	/* "pixel" host interrupt  */

label(SENDEM_RGB)
	_NS REGREG(MOVE,_NDRAWN,_NDRAWN); LDOUT;
	 INTERRUPTHOST; DOJSUB(OUTSEND); _ES		/* send # pixels */

	_NS LOADMAR(_STARTLIST); CONST(_STARTLIST); _ES

label(RGBSENDLOOP)
	_NS IMMRAM(ANDRS,0xff,_TEMP,HOLD); LDOUT; INTERRUPTHOST; 
	 CONST(0xff); _ES			/* send A planes = red */

	_NS SETSOP(NONQOP,_TEMP,RAMRD); ALUOP(SONLYOP,P0); YQ(FF,OLDQ,REGWRE);
	 SWAPBYTE; LOADDI(OUTPUTREG); COND(IFFALSE); SEQ(CJPP); _ES
					/* while showing host red, read grn */

	_NS IMMREG(ANDRS,0xff,_TEMP); LDOUT; INTERRUPTHOST; CONST(0xff); _ES
					/* mask off just 8 bits of green */

	_NS REGHOLD; INCMAR; LOADDI(OUTPUTREG); COND(IFFALSE); SEQ(CJPP); _ES

	_NS IMMRAM(ANDRS,0xff,_TEMP,HOLD); LDOUT; INTERRUPTHOST;
	 CONST(0xff); _ES			/* send C planes as blu */

	_NS REGHOLD; INCMAR; LOADDI(OUTPUTREG); COND(IFFALSE); SEQ(CJPP); _ES

	_NS REGHOLD; SEQ(RPCT); NEXT(RGBSENDLOOP); _ES

	_NS REGHOLD; DOJUMP(PIXRESTORE); _ES	/* clean up & exit */

/*================================================================*/

label(NEXTMA)
	_NS REGHOLD; INCMAR; SEQ(RETN); _ES


label(PIXGETXY)		/* fetch X,Y start,end from scratch */
	_NS LOADMAR(_CHARPOSN); CONST(_CHARPOSN); _ES

	_NS RAM(RAMRD,_PX,HOLD); BPC(LOADXS); DOJSUB(NEXTMA); _ES

	_NS RAM(RAMRD,_PY,HOLD); BPC(LOADYS); _ES

	_NS LOADIMM(_TEMP,0x7ff); BPC(LOADXE); CONST(0x7ff); _ES
			/* load right edge of PFI box way offscreen */

	_NS LOADMAR(_PIXELPARAMS); CONST(_PIXELPARAMS); _ES

	_NS IMMRAM(SUBSRC,3,_FLAG,HOLD); CONST(3); _ES/* fetch planecode -3 */

	_NS LOADMAR(_PASSCHARS); CONST(_PASSCHARS); _ES
					/* point to pass/reject flag */

	_NS RAM(RAMRD,_TEMP,HOLD); SEQ(RETN); _ES
						/* examine the flag, return */
}
