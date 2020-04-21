/*  pixels.c		<<  GF2/UC4  >>
 *
 *	DRAW_PIXELS ([data]*ncount)
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
 *	data: word if planes<3; 2 words if planes=3
 *		"planes" comes from scratch:
 *			1=BA
 *			2=DC	(***NOTE -- performs DRAWAB in double mode )
 *			3=DC,BA
 ******>>>		7=RG,B
 *	Read/write start from charposn if valid
 *	Host sets up configuration/mode for increment, patterning, etc.
 *	DRAW draws no. of pixels determined by passthru ncount.
 *	Next DRAW continues to draw (within boundaries) from previous end posn
 *	Char position updated.  PIXEL PARAM used for right edge check.
 *	Use PIXEL PARAMS to set up end of scan area for PFI.
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"

pixels()
{
newfile("pixels.c");

label(DRAW_PIXELS)
/*
 *  Checks PASSCHARS for abort and for loading CHARPOSN:
 *	if  0, abort
 *	if -1, read current char posn from scratch
 *  Inputs 1 to 3 words per pixel, depending on "planes" parameter
 *  LDYS (y)
 *  LDXS (x)
 *  SETADDRS		loads start regs.
 *
 *  DRAW AB or CD or both depending on "planes"
 */

#define _FLAG	4
#define _PX	5
#define _NDRAWN 6
#define _PY	11
#define _WK	12

	_NS REGREG(SUBSR,_ZERO,_PASSN);	_ES	/*  set up N count    */

	_NS REGREG(ZERO,0,_NDRAWN); COND(IFNEG); DOJUMP(GETCMD);_ES
				/*  clear pixel count; test for N done  */

	_NS REGHOLD; DOJSUB(PIXGETXY); _ES
			/* go get current charposn and pt to PASSCHARS */

	_NS REGHOLD; BPC(SETADDRS); COND(IFZ); DOJUMP(PIX_REJECT); _ES
					/* if flag is 0, go reject data */

	_NS LOADMAR(_MULTIVIEW); CONST(_MULTIVIEW); _ES

	_NS RAM(RAMRD,_TEMP,HOLD); _ES

	_NS REGREG(MOVE,_FLAG,_FLAG); COND(IFNZ); DOJUMP(MULTI_PIX); _ES
					/* re-examine planes code;
					 * jump if multi-view	*/

	_NS REGREG(INCR,_FLAG,_FLAG); COND(IFNNEG); DOJUMP(PIX_ABCD); _ES

	_NS REGREG(MOVE,_PASSN,_PASSN); LDOUT; COND(IFZ); DOJUMP(PIX_CD); _ES
				/* prepare to load nct-1 as loop count */

/* PIX_AB */
	_NS REGHOLD; LOADDI(OUTPUTCOUNT); SEQ(LDCT); _ES
					/* load loopcount for AB, CD cases */

	_NS LOADMAR(_CONFIG+1); CONST(_CONFIG+1); _ES

label(AB_SETUP)
	_NS IMMRAM(ANDRS,_PFIMASK,_TEMP,HOLD); CONST(_PFIMASK);
	BPC(LOADCONFIG); _ES	/* tweak config reg for autoinc on AB wrt */

label(PIX_AB_LOOP)
	_NS LOADREG(_TEMP,ALL16,MORE); BPC(DRAWPIXELAB); _ES

	_NS REGREG(INCR,_NDRAWN,_NDRAWN); SEQ(RPCT); NEXT(PIX_AB_LOOP) _ES

label(PIXEND)
	_NS RAM(RAMRD,_TEMP,HOLD); BPC(LOADCONFIG); _ES
						/* restore true config */

label(PIXEXIT)
	_NS REGREG(ADD,_NDRAWN,_PX); _ES  /* update by no. pixels drawn */

label(PIXQUIT)
	_NS LOADMAR(_PIXELPARAMS+1); CONST(_PIXELPARAMS+1); _ES
					/* point to viewport Xright */

	_NS REGRAMCOMP(GT,_PX,HOLD); _ES	/* compare new position */

	_NS REGREG(ZERO,0,_TEMP); COND(IFGT); DOJUMP(INVALID_POSN); _ES
					/* make a zero in case posn invalid */

	_NS LOADMAR(_CHARPOSN); CONST(_CHARPOSN); _ES
						/* point to current X posn */

	_NS RAM(RAMWR,_PX,HOLD); DOJUMP(DISPATCH); _ES
							/* update and exit */

label(INVALID_POSN)
	_NS LOADMAR(_PASSCHARS); CONST(_PASSCHARS); _ES

	_NS RAM(RAMWR,_TEMP,HOLD); DOJUMP(DISPATCH); _ES
						/* invalidate flag and exit */

label(PIX_CD)
	_NS REGHOLD; LOADDI(OUTPUTCOUNT); SEQ(LDCT); _ES
					/* load loopcount for AB, CD cases */

	_NS LOADMAR(_CONFIG); CONST(_CONFIG); _ES

	_NS IMMRAM(ANDRS,_DOUBLEBIT,_TEMP,INC); CONST(_DOUBLEBIT); _ES
					/* test double bit in mode reg */

	_NS REGHOLD; COND(IFNZ); DOJUMP(AB_SETUP); _ES
					/* if set, do AB write instead */

	_NS IMMRAM(ANDRS,_PFIMASK,_TEMP,HOLD); CONST(_PFIMASK); _ES

	_NS IMMREG(ORRS,_PFICD,_TEMP); CONST(_PFICD);
 	 BPC(LOADCONFIG); _ES	/* tweak config reg for autoinc on CD wrt */

label(PIX_CD_LOOP)
	_NS LOADREG(_TEMP,ALL16,MORE); BPC(DRAWPIXELCD); _ES

	_NS REGREG(INCR,_NDRAWN,_NDRAWN); SEQ(RPCT); NEXT(PIX_CD_LOOP) _ES

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(LOADCONFIG); DOJUMP(PIXEXIT); _ES

label(PIX_ABCD)
	_NS LOADMAR(_CONFIG+1); CONST(_CONFIG+1); _ES

	_NS IMMREG(SUBSRC,5,_FLAG); CONST(5); _ES  /* test for RGB mode */

	_NS REGHOLD; COND(IFNNEG); DOJUMP(PIX_RGB); _ES

	_NS IMMRAM(ANDRS,_PFIMASK,_TEMP,HOLD); CONST(_PFIMASK);
 	 BPC(LOADCONFIG); _ES	/* tweak config reg for autoinc on AB wrt */

	_NS SETROP(_PASSN,NONE); SETSOP(NONQOP,_PASSN,RAMNOP);
	 ALUOP(MOVE); YQ(FLR,QR,REGWRE); LDOUT; _ES	/* N/2 to outreg */

	_NS REGHOLD; LOADDI(OUTPUTCOUNT); SEQ(LDCT); _ES

label(PIX_ABCDLOOP)
	_NS LOADREG(_TEMP,ALL16,MORE); BPC(DRAWPIXELCD); _ES

	_NS LOADREG(_TEMP,ALL16,MORE); BPC(DRAWPIXELAB); _ES

	_NS REGREG(INCR,_NDRAWN,_NDRAWN); SEQ(RPCT); NEXT(PIX_ABCDLOOP); _ES

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(LOADCONFIG); DOJUMP(PIXEXIT); _ES

label(PIX_RGB)
	_NS LOADMAR(_CONFIG+1); CONST(_CONFIG+1); _ES

	_NS IMMRAM(ANDRS,_PFIMASK,_TEMP,DEC); CONST(_PFIMASK); _ES
					/* point to mode reg for later */

	_NS IMMREG(ORRS,_PFICD,_TEMP); CONST(_PFICD);
 	 BPC(LOADCONFIG); _ES	/* tweak config reg for autoinc on CD wrt */

label(PIX_RGBLOOP)
	_NS IMMRAM(ANDRS,~_SWIZZLEBIT,_FLAG,HOLD); BPC(LOADMODE);
	CONST(~_SWIZZLEBIT); _ES	/* make sure swizzle clear for R,G */

	_NS LOADREG(_TEMP,ALL16,MORE); _ES	/* get red */

	_NS LOADREG(_TEMP,HI8,MORE); BPC(DRAWPIXELAB); _ES /* add green */

	_NS IMMRAM(ORRS,_SWIZZLEBIT,_FLAG,HOLD); BPC(LOADMODE);
	 CONST(_SWIZZLEBIT); _ES	/* make sure swizzle is set */

	_NS LOADREG(_TEMP,ALL16,MORE); BPC(DRAWPIXELCD); _ES
						/* get & draw blue */

	_NS IMMREG(SUBSRC,3,_PASSN); CONST(3); _ES

	_NS REGREG(INCR,_NDRAWN,_NDRAWN);
	 COND(IFNNEG); SEQ(JUMP); NEXT(PIX_RGBLOOP); _ES

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(LOADMODE); _ES
					/* restore true mode bits */

	_NS REGHOLD; INCMAR; DOJUMP(PIXEND); _ES	/* go finish */

label(PIX_REJECT)
	_NS REGREG(MOVE,_PASSN,_PASSN); LDOUT; _ES

	_NS REGHOLD; LOADDI(OUTPUTCOUNT); SEQ(PUSH); _ES

	_NS REGHOLD; GEGET; SEQ(LOUP); _ES
					/* gobble up all of inputs */

	_NS REGHOLD; DOJUMP(DISPATCH); _ES	/* no GEGET - already got */
}

