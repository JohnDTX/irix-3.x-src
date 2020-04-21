/*  copypixels.c
 *			COPY_SCREEN
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"

copypixels()
{
newfile("copypixels.c");


label(COPY_SCREEN)
/*
 * Copies a horizontal span of pixels.  Exact addresses can be specified.
 * Copying is word-mode if lsb's of source and destination match.
 * If screenmask is turned on, exact clipping of WRITTEN pixels is done.
 *	Pixels can be READ from anywhere!
 *
 *   input:
 *	Xfrom, Yfrom starting point	(integer format)
 *	Xto, Yto starting point 	(integer format)
 *	XfromMax			(integer format)
 *   if from < to, copy backwards from end of span
 *   loop:
 *	load start regs w/ "from"
 *	WDREAD; inc axis
 *	load start regs w/ "to"
 *	WDWR; inc axis
 *   restore X1, Y1; exit
 */
#define _LENGTH		2
#define _XFROM		3
#define _YFROM  	4
#define _XTO    	5
#define _YTO    	6
#define _MAX		8
#define _INCR   	9
#define _SRC		10
#define _DST		11
#define _CT		12
#define _1STMASK	13
#define _LASTMASK	14
#define _TEMP2		8
#define _LOOPCT		13
#define _PFIMODE	14

	_NS LOADREG(_XFROM,ALL16,MORE); _ES	/* input (integer) coords */

	_NS LOADREG(_YFROM,ALL16,MORE); _ES

	_NS LOADREG(_XTO,ALL16,MORE); _ES

	_NS LOADREG(_YTO,ALL16,MORE); _ES

	_NS LOADREG(_MAX,ALL16,NOMORE); _ES	/* read max. (min.) X */

	_NS LOADMAR(_MULTIVIEW); CONST(_MULTIVIEW); _ES

	_NS RAM(RAMRD,_DST,HOLD); _ES	/* fetch flag for later */

/* calculate span length in words while max > from, in case copy aligned */

	_NS REGREG(MOVE,_XFROM,_TEMP); _ES

	_NS IMMREG(ANDRS,0xFFF0,_TEMP); CONST(0xFFF0); _ES

	_NS REGREG(MOVE,_MAX,_LENGTH); _ES

	_NS IMMREG(ANDRS,0xFFF0,_LENGTH); CONST(0xFFF0); _ES

	_NS REGREG(SUBSRC,_TEMP,_LENGTH); _ES	/* len gets 16*(spans-1) */

	_NS REGCOMP(GT,_XFROM,_XTO); _ES	/* compare X's for r-to-l */

	_NS REGREG(MOVE,_MAX,_CT); COND(IFGT); DOJUMP(PLUSX); _ES
					/* prepare to calc true length */

/* negative direction: reverse XFROM, MAX; move XTO to end */

	_NS REGREG(SUBSRC,_XFROM,_CT); LDOUT; _ES  /* ct gets true length */

	_NS LOADIMM(_INCR,-16); CONST(-16); _ES		/* negative dir */

	_NS LOADIMM(_1STMASK,_RIGHTMASK); CONST(_RIGHTMASK); _ES
						/* init ptrs to mask tabs */

	_NS LOADIMM(_LASTMASK,_LEFTMASK); CONST(_LEFTMASK); _ES

	_NS REGREG(MOVE,_XFROM,_MAX); _ES

	_NS REGREG(ADD,_CT,_XFROM); _ES
				/* start copy from opp. end of span */

	_NS REGREG(ADD,_CT,_XTO); DOJUMP(X_COPY); _ES

label(PLUSX)
	_NS REGREG(SUBSRC,_XFROM,_CT); LDOUT; _ES  /* ct gets true length */

	_NS LOADIMM(_INCR,16); CONST(16); _ES	/* positive direction */

	_NS LOADIMM(_1STMASK,_LEFTMASK); CONST(_LEFTMASK); _ES
						/* init ptrs to mask tabs */

	_NS LOADIMM(_LASTMASK,_RIGHTMASK); CONST(_RIGHTMASK); _ES

label(X_COPY)
/* determine whether 4 lsb's of from, to match */

	_NS REGREG(MOVE,_XFROM,_TEMP); _ES

	_NS REGREG(XOR,_XTO,_TEMP); _ES

	_NS IMMREG(ANDRS,0xF,_TEMP); CONST(0xF); _ES

	_NS REGHOLD; COND(IFNZ); DOJUMP(RANDOM); _ES

/* aligned copy -- use word read/write for interior spans */

/* come up with first span's bitmask */

	_NS REGREG(MOVE,_XFROM,_TEMP); _ES

	_NS IMMREG(ANDRS,0xF,_TEMP); CONST(0xF); _ES

	_NS REGREG(ADD,_1STMASK,_TEMP); LDMAR; _ES

	_NS RAM(RAMRD,_1STMASK,HOLD); _ES	/* "permanent" register */

	_NS REGREG(MOVE,_MAX,_TEMP); _ES	/* use ending address */

	_NS IMMREG(ANDRS,0xF,_TEMP); CONST(0xF); _ES

	_NS REGREG(ADD,_LASTMASK,_TEMP); LDMAR; _ES

	_NS RAM(RAMRD,_LASTMASK,HOLD); _ES	/* get permanent lastmask */

	_NS REGREG(MOVE,_DST,_DST); _ES		/* look at multiview flag */

	_NS REGREG(MOVE,_1STMASK,_TEMP); COND(IFNZ); DOJUMP(MULTIX); _ES
					/* make copy of starting mask */

/* single-vp aligned copy (AB or all planes) */

	_NS REGREG(MOVE,_XFROM,_XFROM); BPC(LOADXS); DOJUMP(INTO_1S); _ES
					/* start first span */
/* middle spans */
label(1S_LOOP)
	_NS REGREG(MOVE,_TEMP,_TEMP); BPC(DRAWWORD); _ES

	_NS REGREG(ONES,0,_TEMP); _ES	/* draw all pixels next time */

	_NS REGREG(MOVE,_XFROM,_XFROM); BPC(LOADXS); _ES  /* load xfrom	*/

label(INTO_1S)
	_NS REGREG(MOVE,_YFROM,_YFROM); BPC(LOADYS); _ES  /* load yfrom	*/

	_NS REGREG(ADD,_INCR,_XFROM); BPC(SAVEWORD); _ES
			/* read source word; increment xfrom for next time */

	_NS REGREG(MOVE,_XTO,_XTO); BPC(LOADXS);_ES	/* load xto	*/

	_NS REGREG(MOVE,_YTO,_YTO); BPC(LOADYS);_ES	/* load yto	*/

	_NS IMMREG(SUBSRC,16,_LENGTH); CONST(16); _ES

	_NS REGREG(ADD,_INCR,_XTO); COND(IFNNEG); DOJUMP(1S_LOOP); _ES
				/* write to destination; increment xto	*/
						/* loop if not last word */
/* last span - use LASTMASK to write */

	_NS REGREG(MOVE,_LASTMASK,_LASTMASK); BPC(DRAWWORD);
	 GEGET; DOJUMP(DISPATCH); _ES

/*================================================================*/

label(MULTIX)
	_NS REGHOLD; DOJSUB(RESET_MASKS); _ES

label(MV_XLP)
	_NS REGREG(MOVE,_XFROM,_SRC); _ES	/* init for each vp */

	_NS REGREG(MOVE,_XTO,_DST); _ES

	_NS REGREG(MOVE,_LENGTH,_CT); _ES

	_NS REGREG(MOVE,_1STMASK,_TEMP); _ES

	_NS REGREG(MOVE,_SRC,_SRC); BPC(LOADXS); DOJUMP(INTO_MS); _ES

label(MVX_SPANLP)
	_NS REGREG(MOVE,_TEMP,_TEMP); BPC(DRAWWORD); _ES

	_NS REGREG(ONES,0,_TEMP); _ES	/* draw all pixels next time */

	_NS REGREG(MOVE,_SRC,_SRC); BPC(LOADXS); _ES  /* load xfrom	*/

label(INTO_MS)
	_NS REGREG(MOVE,_YFROM,_YFROM); BPC(LOADYS); _ES  /* load yfrom	*/

	_NS REGREG(ADD,_INCR,_SRC); BPC(SAVEWORD); _ES
			/* read source word; increment xfrom for next time */

	_NS REGREG(MOVE,_DST,_DST); BPC(LOADXS);_ES	/* load xto	*/

	_NS REGREG(MOVE,_YTO,_YTO); BPC(LOADYS);_ES	/* load yto	*/

	_NS IMMREG(SUBSRC,16,_CT); CONST(16); _ES

	_NS REGREG(ADD,_INCR,_DST); COND(IFNNEG); DOJUMP(MVX_SPANLP); _ES
					/* incr xto; loop if not last word */

/* use LASTMASK to write */

	_NS REGREG(MOVE,_LASTMASK,_LASTMASK); BPC(DRAWWORD);
	 DOJSUB(NEW_MASK); _ES

	_NS REGHOLD; COND(IFNZ); DOJUMP(MV_XLP); _ES

	_NS REGHOLD; GEGET; DOJUMP(DISPATCH); _ES

label(RANDOM)
	_NS LOADMAR(_CONFIG); CONST(_CONFIG); _ES

	_NS IMMRAM(ANDRS,_DOUBLEBIT,_SRC,HOLD); CONST(_DOUBLEBIT); _ES
					/* src reg gets doublebuffer flag */

	_NS REGREG(MOVE,_DST,_DST); COND(IFZ); DOJUMP(RAND_ALL); _ES
					/* test multiview flag */

	_NS REGREG(MOVE,_INCR,_INCR); COND(IFNZ); DOJUMP(RAND_MULTI_AB); _ES
					/* test sign of increment */

/* non-aligned, single-vp, AB planes only */

	_NS REGREG(INCR,_ZERO,_INCR); COND(IFNEG); DOJSUB(INCR_IS_-1); _ES
					/* incr is 1 if it was positive */

	_NS LOADDI(OUTPUTCOUNT); REGHOLD; SEQ(PUSH); _ES

/* AB loop */
	_NS REGREG(MOVE,_XFROM,_XFROM); BPC(LOADXS); _ES

	_NS REGREG(MOVE,_YFROM,_YFROM); BPC(LOADYS); _ES

	_NS REGHOLD; BPC(SETADDRS); _ES

	_NS REGREG(ADD,_INCR,_XFROM); BPC(READPIXELAB); _ES

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE);
	 READBPCBUS; LDOUT;
	 COND(IFFALSE); SEQ(CJPP); _ES	/* wait for ack; spy on bus */

	_NS REGREG(MOVE,_XTO,_XTO); BPC(LOADXS); _ES

	_NS REGREG(MOVE,_YTO,_YTO); BPC(LOADYS); _ES

	_NS REGHOLD; BPC(SETADDRS); _ES

	_NS REGREG(MOVE,_TEMP,_TEMP); BPC(DRAWPIXELAB); _ES

	_NS REGREG(ADD,_INCR,_XTO); SEQ(LOUP); _ES

	_NS REGHOLD; GEGET; DOJUMP(DISPATCH); _ES

label(INCR_IS_-1)
	_NS REGREG(ONES,0,_INCR); SEQ(RETN); _ES	/* increment is -1 */

label(RAND_MULTI_AB)
/* non-aligned, multi-vp, AB planes only */

	_NS REGREG(INCR,_ZERO,_INCR); COND(IFNEG); DOJSUB(INCR_IS_-1); _ES
					/* incr is 1 if it was positive */

	_NS REGHOLD; DOJSUB(RESET_MASKS); _ES

label(RM_VPLP)
	_NS REGREG(MOVE,_CT,_CT); LDOUT; _ES	/* reload pixel count */

	_NS REGREG(MOVE,_XTO,_DST); _ES

	_NS REGREG(MOVE,_XFROM,_SRC); LOADDI(OUTPUTCOUNT); SEQ(PUSH); _ES
						/* restore X src adr */
/* AB loop */
	_NS REGREG(MOVE,_SRC,_SRC); BPC(LOADXS); _ES

	_NS REGREG(MOVE,_YFROM,_YFROM); BPC(LOADYS); _ES

	_NS REGHOLD; BPC(SETADDRS); _ES

	_NS REGREG(ADD,_INCR,_SRC); BPC(READPIXELAB); _ES

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE);
	 READBPCBUS; LDOUT;
	 COND(IFFALSE); SEQ(CJPP); _ES	/* wait for ack; spy on bus */

	_NS REGREG(MOVE,_DST,_DST); BPC(LOADXS); _ES

	_NS REGREG(MOVE,_YTO,_YTO); BPC(LOADYS); _ES

	_NS REGHOLD; BPC(SETADDRS); _ES

	_NS REGREG(MOVE,_TEMP,_TEMP); BPC(DRAWPIXELAB); _ES

	_NS REGREG(ADD,_INCR,_DST); SEQ(LOUP); _ES

	_NS REGHOLD; DOJSUB(NEW_MASK); _ES

	_NS REGHOLD; COND(IFNZ); DOJUMP(RM_VPLP); _ES

	_NS REGHOLD; GEGET; DOJUMP(DISPATCH); _ES


label(RAND_ALL)
/* non-aligned, single-vp, all planes */

	_NS REGREG(MOVE,_INCR,_INCR); COND(IFNZ); DOJUMP(RAND_MULTI_ALL); _ES
					/* test sign of increment */

	_NS REGREG(INCR,_ZERO,_INCR); COND(IFNEG); DOJSUB(INCR_IS_-1); _ES
					/* incr is 1 if it was positive */

	_NS LOADDI(OUTPUTCOUNT); REGHOLD; SEQ(PUSH); _ES

/* ABCD loop */
	_NS REGREG(MOVE,_XFROM,_XFROM); BPC(LOADXS); _ES

	_NS REGREG(MOVE,_YFROM,_YFROM); BPC(LOADYS); _ES

	_NS REGHOLD; BPC(SETADDRS); _ES

	_NS REGHOLD; BPC(READPIXELCD); _ES

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE);
	 READBPCBUS; LDOUT;
	 COND(IFFALSE); SEQ(CJPP); _ES	/* wait for ack; spy on bus */

	_NS REGREG(ADD,_INCR,_XFROM); BPC(READPIXELAB); _ES

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP2,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE);
	 READBPCBUS; LDOUT;
	 COND(IFFALSE); SEQ(CJPP); _ES	/* wait for ack; spy on bus */

	_NS REGREG(MOVE,_XTO,_XTO); BPC(LOADXS); _ES

	_NS REGREG(MOVE,_YTO,_YTO); BPC(LOADYS); _ES

	_NS REGHOLD; BPC(SETADDRS); _ES

	_NS REGREG(MOVE,_TEMP,_TEMP); BPC(DRAWPIXELCD); _ES

	_NS REGREG(MOVE,_TEMP2,_TEMP2); BPC(DRAWPIXELAB); _ES

	_NS REGREG(ADD,_INCR,_XTO); SEQ(LOUP); _ES

	_NS REGHOLD; GEGET; DOJUMP(DISPATCH); _ES

label(RAND_MULTI_ALL)

	_NS REGREG(INCR,_ZERO,_INCR); COND(IFNEG); DOJSUB(INCR_IS_-1); _ES
					/* incr is 1 if it was positive */

	_NS REGHOLD; DOJSUB(RESET_MASKS); _ES

label(RMA_VPLP)
	_NS REGREG(MOVE,_CT,_CT); LDOUT; _ES	/* reload pixel count */

	_NS REGREG(MOVE,_XFROM,_SRC); _ES

	_NS REGREG(MOVE,_XTO,_DST); LOADDI(OUTPUTCOUNT); SEQ(PUSH); _ES

/* ABCD loop */
	_NS REGREG(MOVE,_SRC,_SRC); BPC(LOADXS); _ES

	_NS REGREG(MOVE,_YFROM,_YFROM); BPC(LOADYS); _ES

	_NS REGHOLD; BPC(SETADDRS); _ES

	_NS REGHOLD; BPC(READPIXELCD); _ES

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE);
	 READBPCBUS; LDOUT;
	 COND(IFFALSE); SEQ(CJPP); _ES	/* wait for ack; spy on bus */

	_NS REGREG(ADD,_INCR,_SRC); BPC(READPIXELAB); _ES

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP2,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE);
	 READBPCBUS; LDOUT;
	 COND(IFFALSE); SEQ(CJPP); _ES	/* wait for ack; spy on bus */

	_NS REGREG(MOVE,_DST,_DST); BPC(LOADXS); _ES

	_NS REGREG(MOVE,_YTO,_YTO); BPC(LOADYS); _ES

	_NS REGHOLD; BPC(SETADDRS); _ES

	_NS REGREG(MOVE,_TEMP,_TEMP); BPC(DRAWPIXELCD); _ES

	_NS REGREG(MOVE,_TEMP2,_TEMP2); BPC(DRAWPIXELAB); _ES

	_NS REGREG(ADD,_INCR,_DST); SEQ(LOUP); _ES

	_NS REGHOLD; DOJSUB(NEW_MASK); _ES

	_NS REGHOLD; COND(IFNZ); DOJUMP(RMA_VPLP); _ES

	_NS REGHOLD; GEGET; DOJUMP(DISPATCH); _ES
}

