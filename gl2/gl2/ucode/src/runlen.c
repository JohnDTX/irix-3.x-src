/*  runlen.c		<<  GF2/UC4  >>
 *
 *	RUN_LENGTH ([data]*ncount)
 *
 *  Deals with horizontal spans of pixels only. no PFI wrap allowed.

 * >>> NOTE <<< poly stipple address must be set up (e.g. solid fill)

 *  HOST must set up PFIColumn,PFIXDown,PFIYDown but FBC will handle
 *	PFICD and PFIRead
 *  No. of planes set up via PIXEL_SETUP in attributes.c
 *
 *	Accepts N pixel count followed by pixel value for N pixels;
 *	       -N pixel count followed by N explicit pixel values.
 *	Passthru count determines when current command is really done.
 *
 *	data:	1 word /pixel if planes<3;
 *		2 words if planes=3
 *		3 words if planes=7
 *		"planes" comes from scratch:
 *			1=BA
 *			2=DC	(***NOTE -- performs DRAWAB in double mode )
 *			3=DC,BA
 *			7=RG,B
 *	Write start from charposn if valid
 *	Host sets up configuration/mode for increment, patterning, etc.
 *	Next DRAW continues to draw (within boundaries) from previous end posn
 *	Char position updated.  PIXEL PARAM used for right edge check.
 *	Use PIXEL PARAMS to set up end of scan area for PFI.
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"

runlen()
{
newfile("runlen.c");

label(RUN_LENGTH)
/*
 *  Checks PASSCHARS for abort and for loading CHARPOSN:
 *	if  0, abort
 *	if -1, read current char posn from scratch
 *  Inputs 1 to 3 words per pixel, depending on "planes" parameter
 */

#define _FLAG	4
#define _PX	5
#define _NDRAWN 6
#define _DX	8
#define _TEMP2	9
#define _TALLY	10
#define _PY	11
#define _WK	12
#define _ADR	13

	_NS REGREG(SUBSR,_ZERO,_PASSN);	_ES	/*  set up N count-1    */

	_NS REGREG(ZERO,0,_NDRAWN); COND(IFNEG); DOJUMP(GETCMD);_ES
				/*  clear pixel count; test for N done  */

	_NS LOADMAR(_POLYSTIPADR); CONST(_POLYSTIPADR); _ES

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(LOADFA); DOJSUB(RESET_MASKS); _ES

	_NS REGHOLD; GEGET; DOJSUB(PIXGETXY); _ES
			/* go get current charposn and pt to PASSCHARS */

/* input N words to scratch */

	_NS REGREG(MOVE,_PY,_PY); BPC(LOADYE);
	 COND(IFZ); DOJUMP(PIX_REJECT); _ES	/* if flag 0 go reject */

	_NS REGREG(MOVE,_PY,_PY); BPC(LOADYS); _ES   /* set up Y for line */

	_NS REGREG(MOVE,_PASSN,_PASSN); LDOUT; _ES

	_NS LOADIMM(_ADR,_STARTLIST); LDMAR; CONST(_STARTLIST); _ES

	_NS RAM(RAMWR,_PASSN,INC);	/* save N count for each pass */
	 LOADDI(OUTPUTCOUNT); SEQ(LDCT); _ES	/* load loop counter */

label(RUN_RD_LOOP)
	_NS LOADREG(_TEMP,ALL16,MORE); _ES

	_NS RAM(RAMWR,_TEMP,INC); SEQ(RPCT); NEXT(RUN_RD_LOOP); _ES

/* determine which kind of pixels */

	_NS REGREG(MOVE,_FLAG,_FLAG); _ES	/* re-examine planes code */

	_NS REGREG(INCR,_FLAG,_FLAG); COND(IFNNEG); DOJUMP(RUN_ABCD); _ES

	_NS REGHOLD; COND(IFZ); DOJUMP(RUN_CD); _ES

/* RUN_AB */
	_NS LOADMAR(_CONFIG+1); CONST(_CONFIG+1); _ES

label(RUN_AB_SETUP)
	_NS IMMRAM(ANDRS,_PFIMASK,_TEMP,HOLD); CONST(_PFIMASK);
	 BPC(LOADCONFIG); _ES	/* tweak config reg for autoinc on AB wrt */

label(MLP_RUN_AB)
	_NS REGREG(MOVE,_PX,_PX); BPC(LOADXS); DOJSUB(RUNPOSN); _ES
						/* go set up position, MAR */

label(AB_NEXT_RUN)
	_NS RAM(RAMRD,_NDRAWN,INC); _ES  /* get N for next run */

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(SETCOLORAB);	/* get pixel data */
	 COND(IFNEG); DOJUMP(AB_EXPLICIT); _ES		/* if -N, sgl pix */

	_NS REGREG(SUBSR,_ZERO,_NDRAWN); INCMAR; _ES	/* N-1 for Xend */

	_NS SETROP(_NDRAWN,NONE); SETSOP(NONQOP,_DX,RAMNOP);
	 ALUOP(ADD); FTODO;  BPC(LOADXE); _ES		/* end of span */

	_NS REGREG(INCR,_NDRAWN,_NDRAWN); BPC(FILLRECT); _ES
					/* draw the span; incr back to N */

	_NS IMMREG(SUBSRC,2,_PASSN); CONST(2); _ES   /* sub. 2 from total */

label(AB_ENDTEST)
	_NS REGREG(ADD,_NDRAWN,_DX); BPC(LOADXS);
	 COND(IFNNEG); DOJUMP(AB_NEXT_RUN); _ES
		/* add actual # pixels to moving X; test for more runs */

	_NS REGHOLD; DOJSUB(NEW_MASK); _ES

	_NS REGHOLD; COND(IFNZ); DOJUMP(MLP_RUN_AB); _ES

label(RUNEND)
	_NS LOADMAR(_CONFIG+1); CONST(_CONFIG+1); _ES

label(RUNRESTORE)
	_NS RAM(RAMRD,_TEMP,HOLD); BPC(LOADCONFIG); _ES
						/* restore true config */

	_NS REGREG(MOVE,_DX,_PX); DOJUMP(PIXQUIT); _ES
					/* update to next pixel position */

label(AB_EXPLICIT)
	_NS REGREG(MOVE,_DX,_DX); BPC(LOADXS); _ES

	_NS LOADIMM(_TEMP,0x7ff); BPC(LOADXE); CONST(0x7ff); _ES

	_NS REGREG(COMPROP,P0,_NDRAWN,_NDRAWN); BPC(SETADDRS); _ES
					/* 1's comp gives loop count */

	_NS REGREG(INCR,_NDRAWN,_NDRAWN); LOADDI(OUTPUTCOUNT); SEQ(PUSH); _ES
				/* increment back to actual pixel count */

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(DRAWPIXELAB); _ES

	_NS INCMAR; REGHOLD; SEQ(LOUP); _ES	/* read and draw pixels */

	_NS REGREG(SUBSR,_NDRAWN,_PASSN); DOJUMP(AB_ENDTEST); _ES
			/* sub. N+1 from total; go test for done */


label(RUN_CD)
	_NS LOADMAR(_CONFIG); CONST(_CONFIG); _ES

	_NS IMMRAM(ANDRS,_DOUBLEBIT,_TEMP,INC); CONST(_DOUBLEBIT); _ES
					/* test double bit in mode reg */

	_NS REGHOLD; COND(IFNZ); DOJUMP(RUN_AB_SETUP); _ES
					/* if set, do AB write instead */

	_NS IMMRAM(ANDRS,_PFIMASK,_TEMP,HOLD); CONST(_PFIMASK); _ES

	_NS IMMREG(ORRS,_PFICD,_TEMP); CONST(_PFICD);
 	 BPC(LOADCONFIG); _ES	/* tweak config reg for autoinc on CD wrt */

label(MLP_RUN_CD)
	_NS REGREG(MOVE,_PX,_PX); BPC(LOADXS); DOJSUB(RUNPOSN); _ES

label(CD_NEXT_RUN)
	_NS RAM(RAMRD,_NDRAWN,INC); _ES  /* get N for next run */

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(SETCOLORCD);	/* get pixel data */
	 COND(IFNEG); DOJUMP(CD_EXPLICIT); _ES		/* if -N, sgl pix */

	_NS REGREG(SUBSR,_ZERO,_NDRAWN); INCMAR; _ES	/* N-1 for Xend */

	_NS SETROP(_NDRAWN,NONE); SETSOP(NONQOP,_DX,RAMNOP);
	 ALUOP(ADD); FTODO;  BPC(LOADXE); _ES		/* end of span */

	_NS REGREG(INCR,_NDRAWN,_NDRAWN); BPC(FILLRECT); _ES
					/* draw the span; incr back to N */

	_NS IMMREG(SUBSRC,2,_PASSN); CONST(2); _ES   /* sub. 2 from total */

label(CD_ENDTEST)
	_NS REGREG(ADD,_NDRAWN,_DX); BPC(LOADXS);
	 COND(IFNNEG); DOJUMP(CD_NEXT_RUN); _ES
		/* add actual # pixels to moving X; test for more runs */

	_NS REGHOLD; DOJSUB(NEW_MASK); _ES

	_NS REGHOLD; COND(IFNZ); DOJUMP(MLP_RUN_CD); _ES

	_NS REGHOLD; DOJUMP(RUNEND); _ES	/* go to restore status */

label(CD_EXPLICIT)
	_NS REGREG(MOVE,_DX,_DX); BPC(LOADXS); _ES

	_NS LOADIMM(_TEMP,0x7ff); BPC(LOADXE); CONST(0x7ff); _ES

	_NS REGREG(COMPROP,P0,_NDRAWN,_NDRAWN); BPC(SETADDRS); _ES
					/* 1's comp gives loop count */

	_NS REGREG(INCR,_NDRAWN,_NDRAWN); LOADDI(OUTPUTCOUNT); SEQ(PUSH); _ES
				/* increment back to actual pixel count */

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(DRAWPIXELCD); _ES

	_NS INCMAR; REGHOLD; SEQ(LOUP); _ES	/* read and draw pixels */

	_NS REGREG(SUBSR,_NDRAWN,_PASSN); DOJUMP(CD_ENDTEST); _ES
			/* sub. N+1 from total; go test for done */


label(RUN_ABCD)
	_NS LOADMAR(_CONFIG+1); CONST(_CONFIG+1); _ES

	_NS IMMREG(SUBSRC,5,_FLAG); CONST(5); _ES  /* test for RGB mode */

	_NS REGHOLD; COND(IFNNEG); DOJUMP(RUN_RGB); _ES

	_NS IMMRAM(ANDRS,_PFIMASK,_TEMP,HOLD); CONST(_PFIMASK);
 	 BPC(LOADCONFIG); _ES	/* tweak config reg for autoinc on AB wrt */

label(MLP_RUN_ABCD)
	_NS REGREG(MOVE,_PX,_PX); BPC(LOADXS); DOJSUB(RUNPOSN); _ES

label(ABCD_NEXT_RUN)
	_NS RAM(RAMRD,_NDRAWN,INC); _ES  /* get N pix for next run */

	_NS REGHOLD; COND(IFNEG); DOJUMP(ABCD_EXPLICIT); _ES

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(SETCOLORCD); _ES	/* get pixel CDdata */

	_NS REGREG(SUBSR,_ZERO,_NDRAWN); INCMAR; _ES	/* N-1 for Xend */

	_NS RAM(RAMRD,_TEMP2,HOLD); BPC(SETCOLORAB); DOJSUB(NEXTMA); _ES
							/* ditto AB planes */

	_NS SETROP(_NDRAWN,NONE); SETSOP(NONQOP,_DX,RAMNOP);
	 ALUOP(ADD); FTODO;  BPC(LOADXE); _ES		/* end of span */

	_NS REGREG(INCR,_NDRAWN,_NDRAWN); BPC(FILLRECT); _ES
					/* draw the span; incr back to N */

	_NS IMMREG(SUBSRC,3,_PASSN); CONST(3); _ES   /* sub. 3 from total */

label(ABCD_ENDTEST)
	_NS REGREG(ADD,_NDRAWN,_DX); BPC(LOADXS);
	 COND(IFNNEG); DOJUMP(ABCD_NEXT_RUN); _ES
		/* add actual # pixels to moving X; test for more runs */

	_NS REGHOLD; DOJSUB(NEW_MASK); _ES

	_NS REGHOLD; COND(IFNZ); DOJUMP(MLP_RUN_ABCD); _ES

	_NS REGHOLD; DOJUMP(RUNEND); _ES	/* go to restore status */

label(ABCD_EXPLICIT)
	_NS REGREG(MOVE,_DX,_DX); BPC(LOADXS); _ES

	_NS LOADIMM(_TEMP,0x7ff); BPC(LOADXE); CONST(0x7ff); _ES

	_NS REGREG(COMPROP,P0,_NDRAWN,_NDRAWN); BPC(SETADDRS); _ES
					/* 1's comp gives loop count */

	_NS REGREG(INCR,_NDRAWN,_NDRAWN); LOADDI(OUTPUTCOUNT); SEQ(PUSH); _ES
				/* increment back to actual pixel count */

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(DRAWPIXELCD); DOJSUB(NEXTMA); _ES

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(DRAWPIXELAB); _ES

	_NS INCMAR; REGHOLD; SEQ(LOUP); _ES	/* read and draw pixels */

	_NS REGREG(SUBSRC,_NDRAWN,_PASSN); _ES
	_NS REGREG(SUBSR,_NDRAWN,_PASSN); DOJUMP(CD_ENDTEST); _ES
			/* sub. 2N+1 from total; go test for done */


label(RUN_RGB)
	_NS LOADMAR(_CONFIG+1); CONST(_CONFIG+1); _ES

	_NS IMMRAM(ANDRS,_PFIMASK,_TEMP,DEC); CONST(_PFIMASK); _ES
					/* point to mode reg for later */

	_NS IMMREG(ORRS,_PFICD,_TEMP); CONST(_PFICD);
 	 BPC(LOADCONFIG); _ES	/* tweak config reg for autoinc on CD wrt */

	_NS RAM(RAMRD,_FLAG,HOLD); _ES	 /* grab mode flags for inside lp */

label(MLP_RUN_RGB)
	_NS REGREG(MOVE,_PX,_PX); BPC(LOADXS); DOJSUB(RUNPOSN); _ES

label(RGB_NEXT_RUN)
	_NS RAM(RAMRD,_NDRAWN,INC); _ES		/* get N pix for next run */

	_NS REGHOLD; COND(IFNEG); DOJUMP(RGB_EXPLICIT); _ES

	_NS IMMREG(ANDRS,~_SWIZZLEBIT,_FLAG); BPC(LOADMODE);
	 CONST(~_SWIZZLEBIT); _ES	/* make sure swizzle clear for R,G */

	_NS RAM(RAMRD,_TEMP,INC); _ES	/* get red */

	_NS RAM(RAMRD,_TEMP2,INC); LDOUT; YTODO(SWAP); _ES /* grn to outreg */

	_NS LOADDI(OUTPUTREG); SETROP(_TEMP,HI8); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE); BPC(SETCOLORAB);
	 COND(IFFALSE); SEQ(CJPP); _ES		/* composite red and green */

	_NS IMMREG(ORRS,_SWIZZLEBIT,_FLAG); BPC(LOADMODE);
	 CONST(_SWIZZLEBIT); _ES	/* make sure swizzle is set */

	_NS RAM(RAMRD,_TEMP2,HOLD); BPC(SETCOLORCD); _ES	/* get blu */

	_NS REGREG(SUBSR,_ZERO,_NDRAWN); INCMAR; _ES	/* N-1 for Xend */

	_NS SETROP(_NDRAWN,NONE); SETSOP(NONQOP,_DX,RAMNOP);
	 ALUOP(ADD); FTODO;  BPC(LOADXE); _ES		/* end of span */

	_NS REGREG(INCR,_NDRAWN,_NDRAWN); BPC(FILLRECT); _ES
					/* draw the span; incr back to N */

	_NS IMMREG(SUBSRC,4,_PASSN); CONST(4); _ES
					/* subtract 4 from total word count */

label(RGB_ENDTEST)
	_NS REGREG(ADD,_NDRAWN,_DX); BPC(LOADXS);
	 COND(IFNNEG); DOJUMP(RGB_NEXT_RUN); _ES
		/* add actual # pixels to delta X; test for more runs */

	_NS REGHOLD; DOJSUB(NEW_MASK); _ES

	_NS REGHOLD; COND(IFNZ); DOJUMP(MLP_RUN_RGB); _ES

	_NS LOADMAR(_CONFIG); CONST(_CONFIG); _ES

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(LOADMODE); _ES

	_NS REGHOLD; INCMAR; DOJUMP(RUNRESTORE); _ES
					/* restore true mode; go finish */

label(RGB_EXPLICIT)
	_NS REGREG(MOVE,_DX,_DX); BPC(LOADXS); _ES

	_NS LOADIMM(_TEMP,0x7ff); BPC(LOADXE); CONST(0x7ff); _ES

	_NS REGREG(COMPROP,P0,_NDRAWN,_NDRAWN); BPC(SETADDRS); _ES
					/* 1's comp gives loop count */

	_NS REGREG(INCR,_NDRAWN,_NDRAWN); LOADDI(OUTPUTCOUNT); SEQ(PUSH); _ES
				/* increment back to actual pixel count */

	_NS IMMREG(ANDRS,~_SWIZZLEBIT,_FLAG); BPC(LOADMODE);
	 CONST(~_SWIZZLEBIT); _ES	/* make sure swizzle clear for R,G */

	_NS RAM(RAMRD,_TEMP,INC); _ES	/* get red */

	_NS RAM(RAMRD,_TEMP2,INC); LDOUT; YTODO(SWAP); _ES /* grn to outreg */

	_NS LOADDI(OUTPUTREG); SETROP(_TEMP,HI8); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE); BPC(DRAWPIXELAB);
	 COND(IFFALSE); SEQ(CJPP); _ES
				/* composite red and green and draw em */

	_NS IMMRAM(ORRS,_SWIZZLEBIT,_FLAG,HOLD); BPC(LOADMODE);
	 CONST(_SWIZZLEBIT); _ES	/* make sure swizzle is set */

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(DRAWPIXELCD); _ES	/* draw blu */

	_NS INCMAR; REGHOLD; SEQ(LOUP); _ES	/* read and draw pixels */

	_NS REGREG(SUBSRC,_NDRAWN,_PASSN); _ES
	_NS REGREG(SUBSRC,_NDRAWN,_PASSN); _ES
	_NS REGREG(SUBSR,_NDRAWN,_PASSN); DOJUMP(RGB_ENDTEST); _ES
			/* sub. 2N+1 from total; go test for done */


label(RUNPOSN)	/* enter having set XS */

	_NS REGREG(MOVE,_PX,_DX); BPC(SETADDRS); _ES
			/* starting position to working register */

	_NS REGREG(MOVE,_ADR,_ADR); LDMAR; _ES

	_NS RAM(RAMRD,_PASSN,INC); SEQ(RETN); _ES	/* fetch N count */

}
