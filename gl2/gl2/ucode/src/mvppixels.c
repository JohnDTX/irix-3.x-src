/*  mvppixels.c		<<  GF2/UC4  >>
 *
 *	multi-viewport mode for DRAW PIXELS
 *
 *	subroutines:
 *		NEXTMA
 *		PIXGETXY
 *
 *	data: word if planes<3; 2 words if planes=3
 *		"planes" comes from scratch:
 *			1=BA
 *			2=DC
 *			3=DC,BA
 ******>>>		7=RG,B
 *	Read/write start from charposn if valid
 *	Host sets up configuration/mode for increment, patterning, etc.
 *	DRAW draws no. of pixels determined by passthru ncount.
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"

mvppixels()
{
newfile("mvppixels.c");

/*
 *  Checks PASSCHARS for abort and for loading CHARPOSN:
 *	if  0, abort
 *	if -1, read current char posn from scratch
 *	if -2, continue from current X,Y
 *  Inputs 1 to 3 words per pixel, depending on "planes" parameter
 *  LDYS (y)
 *  LDXS (x)
 *  SETADDRS		loads start regs.
 *
 *  DRAW AB or CD or both depending on "planes"
 */

#define _FLAG	4
#define _PX	5
#define _NDRAWN	6
#define _TALLY  10
#define _PY	11
#define _WK	12
#define _ADR	13


label(MULTI_PIX)
	_NS REGHOLD; DOJSUB(RESET_MASKS); _ES

/* input N words to scratch */

	_NS REGREG(MOVE,_PASSN,_PASSN); LDOUT; _ES

	_NS REGHOLD; LOADDI(OUTPUTCOUNT); SEQ(LDCT); _ES

	_NS LOADIMM(_ADR,_STARTLIST); LDMAR; CONST(_STARTLIST); _ES

label(MPIX_RD_LOOP)
	_NS LOADREG(_TEMP,ALL16,MORE); _ES

	_NS RAM(RAMWR,_TEMP,INC); SEQ(RPCT); NEXT(MPIX_RD_LOOP); _ES

/* determine which kind of pixels */

	_NS REGREG(MOVE,_FLAG,_FLAG); _ES

	_NS REGREG(INCR,_FLAG,_FLAG); COND(IFNNEG); DOJUMP(MPIX_ABCD); _ES

	_NS REGHOLD; COND(IFZ); DOJUMP(MPIX_CD); _ES

/* MPIX_AB */
	_NS LOADMAR(_CONFIG+1); CONST(_CONFIG+1); _ES

label(MAB_SETUP)
	_NS IMMRAM(ANDRS,_PFIMASK,_TEMP,HOLD); CONST(_PFIMASK);
	BPC(LOADCONFIG); _ES	/* tweak config reg for autoinc on AB wrt */

label(MLP_AB)
	_NS REGREG(MOVE,_PX,_PX); BPC(LOADXS); DOJSUB(PIXPOSN); _ES
			/* go set up position, MAR , counter */

label(MPIX_AB_LOOP)
	_NS RAM(RAMRD,_TEMP,HOLD); BPC(DRAWPIXELAB); _ES

	_NS REGHOLD; INCMAR; SEQ(RPCT); NEXT(MPIX_AB_LOOP); _ES

	_NS REGHOLD; DOJSUB(NEW_MASK); _ES

	_NS REGREG(INCR,_PASSN,_NDRAWN); COND(IFNZ); DOJUMP(MLP_AB); _ES

label(MPIXEND)
	_NS LOADMAR(_CONFIG+1); CONST(_CONFIG+1); _ES

label(MPIXEXIT)
	_NS RAM(RAMRD,_TEMP,HOLD); BPC(LOADCONFIG); _ES
						/* restore true config */

	_NS REGREG(ADD,_NDRAWN,_PX); _ES
					/* update posn by # pixels drawn */

	_NS LOADMAR(_PIXELPARAMS+1); CONST(_PIXELPARAMS+1); _ES
						/* look at absolute xright */

	_NS REGRAMCOMP(GT,_PX,HOLD); _ES	/* compare new position */

	_NS REGREG(ZERO,0,_TEMP); COND(IFGT); DOJUMP(INVALID_POSN); _ES

	_NS LOADMAR(_CHARPOSN); CONST(_CHARPOSN); _ES

	_NS RAM(RAMWR,_PX,HOLD); DOJUMP(DISPATCH); _ES


label(MPIX_CD)
	_NS LOADMAR(_CONFIG); CONST(_CONFIG); _ES

	_NS IMMRAM(ANDRS,_DOUBLEBIT,_TEMP,INC); CONST(_DOUBLEBIT); _ES
					/* test double bit in mode reg */

	_NS REGHOLD; COND(IFNZ); DOJUMP(MAB_SETUP); _ES
					/* if set, do AB write instead */

	_NS IMMRAM(ANDRS,_PFIMASK,_TEMP,HOLD); CONST(_PFIMASK); _ES

	_NS IMMREG(ORRS,_PFICD,_TEMP); CONST(_PFICD);
 	 BPC(LOADCONFIG); _ES	/* tweak config reg for autoinc on CD wrt */

label(MLP_CD)
	_NS REGREG(MOVE,_PX,_PX); BPC(LOADXS); DOJSUB(PIXPOSN); _ES
			/* go set up position, MAR , counter */

label(MPIX_CD_LOOP)
	_NS RAM(RAMRD,_TEMP,HOLD); BPC(DRAWPIXELCD); _ES

	_NS REGHOLD; INCMAR; SEQ(RPCT); NEXT(MPIX_CD_LOOP); _ES

	_NS REGHOLD; DOJSUB(NEW_MASK); _ES

	_NS REGREG(INCR,_PASSN,_NDRAWN); COND(IFNZ); DOJUMP(MLP_CD); _ES

	_NS REGHOLD; DOJUMP(MPIXEND); _ES

label(MPIX_ABCD)
	_NS LOADMAR(_CONFIG+1); CONST(_CONFIG+1); _ES

	_NS IMMREG(SUBSRC,5,_FLAG); CONST(5); _ES  /* test for RGB mode */

	_NS REGREG(ONES,0,_TALLY); COND(IFNNEG); DOJUMP(MPIX_RGB); _ES
					/* initialize pixel-counting flag */

	_NS IMMRAM(ANDRS,_PFIMASK,_TEMP,HOLD); CONST(_PFIMASK);
 	 BPC(LOADCONFIG); _ES	/* tweak config reg for autoinc on AB wrt */

label(MLP_ABCD)
	_NS REGREG(MOVE,_PX,_PX); BPC(LOADXS); DOJSUB(PIXPOSNABCD); _ES
				/* modifies PASSN (/2) */

label(MPIX_ABCDLOOP)
	_NS RAM(RAMRD,_TEMP,HOLD); BPC(DRAWPIXELCD); DOJSUB(NEXTMA); _ES

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(DRAWPIXELAB); _ES

	_NS REGHOLD; INCMAR; SEQ(RPCT); NEXT(MPIX_ABCDLOOP); _ES

	_NS REGHOLD; DOJSUB(NEW_MASK); _ES

	_NS REGREG(INCR,_PASSN,_NDRAWN); COND(IFNZ); DOJUMP(MLP_ABCD); _ES

	_NS REGHOLD; DOJUMP(MPIXEND); _ES

label(MPIX_RGB)
	_NS LOADMAR(_CONFIG+1); CONST(_CONFIG+1); _ES

	_NS IMMRAM(ANDRS,_PFIMASK,_TEMP,DEC); CONST(_PFIMASK); _ES
						/* point to mode reg */

	_NS IMMREG(ORRS,_PFICD,_TEMP); CONST(_PFICD);
 	 BPC(LOADCONFIG); _ES	/* tweak config reg for autoinc on CD wrt */

	_NS RAM(RAMRD,_FLAG,HOLD); _ES	/* grab mode bits for loop */

label(MLP_RGB)
	_NS REGREG(MOVE,_PX,_PX); BPC(LOADXS); DOJSUB(PIXPOSN); _ES

label(MPIX_RGBLOOP)
	_NS IMMREG(ANDRS,~_SWIZZLEBIT,_FLAG); BPC(LOADMODE);
	CONST(~_SWIZZLEBIT); _ES	/* make sure swizzle clear for R,G */

	_NS RAM(RAMRD,_TEMP,INC); _ES 	/* get red */

	_NS RAM(RAMRD,_WK,INC); LDOUT; YTODO(SWAP); _ES /* grn to outreg */

	_NS LOADDI(OUTPUTREG); SETROP(_TEMP,HI8); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE); BPC(DRAWPIXELAB);
	 COND(IFFALSE); SEQ(CJPP); _ES
				/* composite red and green and draw em */

	_NS IMMREG(ORRS,_SWIZZLEBIT,_FLAG); BPC(LOADMODE);
	 CONST(_SWIZZLEBIT); _ES	/* make sure swizzle is set */

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(DRAWPIXELCD); _ES
						/* get & draw blue */

	_NS REGREG(MOVE,_TALLY,_TALLY); INCMAR; _ES /* look at tally flag */

	_NS REGHOLD; COND(IFNNEG); DOJUMP(RGBTEST); _ES

	_NS REGREG(INCR,_NDRAWN,_NDRAWN); _ES	/* only count pixels once */

label(RGBTEST)
	_NS IMMREG(SUBSRC,3,_PASSN); CONST(3); _ES

	_NS REGHOLD; COND(IFNNEG); SEQ(JUMP); NEXT(MPIX_RGBLOOP); _ES

	_NS REGREG(ZERO,0,_TALLY); DOJSUB(NEW_MASK); _ES
			/* finished (first) pass, turn off tallying */

	_NS REGHOLD; COND(IFNZ); DOJUMP(MLP_RGB); _ES

	_NS LOADMAR(_CONFIG); CONST(_CONFIG); _ES

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(LOADMODE); _ES

	_NS REGHOLD; INCMAR; DOJUMP(MPIXEXIT); _ES

/*================================================================*/

label(PIXPOSN)		/* like PIXGETXY but different */
			/* enter having set XS */

	_NS REGREG(MOVE,_PY,_PY); BPC(LOADYS); _ES

	_NS LOADIMM(_TEMP,0x7ff); BPC(LOADXE); CONST(0x7ff); _ES

	_NS REGREG(MOVE,_PASSN,_PASSN); LDOUT; BPC(SETADDRS); _ES

	_NS REGHOLD; LOADDI(OUTPUTCOUNT); SEQ(LDCT); _ES

	_NS REGREG(MOVE,_ADR,_ADR); LDMAR; SEQ(RETN); _ES
					/* point to pixel data list */

label(PIXPOSNABCD)		/* like PIXPOSN for ABCD versions */
	_NS REGREG(MOVE,_PY,_PY); BPC(LOADYS); _ES

	_NS LOADIMM(_TEMP,0x7ff); BPC(LOADXE); CONST(0x7ff); _ES

	_NS SETROP(_PASSN,NONE); SETSOP(NONQOP,_PASSN,RAMNOP);
	 ALUOP(MOVE); YQ(FLR,QR,REGWRE); LDOUT; BPC(SETADDRS); _ES
					/* N/2 to outreg; reset adr regs */

	_NS REGHOLD; LOADDI(OUTPUTCOUNT); SEQ(LDCT); _ES

	_NS REGREG(MOVE,_ADR,_ADR); LDMAR; SEQ(RETN); _ES
					/* point to pixel data list */
}
