/* points.c		<<  GF2/UC4  >>
 *
 *    --- handles GE point commands
 *    --- multi-viewport support
 *
 *		POINT		(Xmsw,Xlsw,Ymsw,Ylsw)
 *		XFORM_POINT	(Xmsw,Xlsw,Ymsw,Ylsw,Zmsw,Zlsw,Wmsw,Wlsw)
 *					-- NEW: just gobble coords.
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"

points()
{
newfile("points.c");

#define _Z	8	/* must match depthvec.c */

label(POINT)
/* inputs (x,y) in viewport coordinates and outputs the following to BPC:
 *	Load XS (x)
 *	Load YS (y)
 *	Load YE (y)
 *	Load Config (finishline on)
 *	Load Mode (depthcue off)
 *	DrawLine2
 *	Load Config (normal)
 *	Load Mode (normal)
 *
 * the following decisions are made before drawing:
 *	if (ALTVECMODE==0) drawpoint();
 *	else if (HITMODE) recordhit();
 *	else  ( must be Zbuffer or Depthcue ) getx_z()
 *
 *	uses register definitions in consts.mic:
 *		X1,Y1,X2,Y2,TEMP,HITREG, etc.
 */

	_NS LOADMAR(_ALTVECMODE); CONST(_ALTVECMODE);_ES
					/* are we in special mode?	*/

	_NS RAM(RAMRD,_TEMP,HOLD);_ES

	_NS REGHOLD; COND(IFZ); DOJUMP(DOPOINT);_ES

	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(ANDRS); FTODO; CONST(_ALTHITBIT); _ES
						/* test for hitmode */

	_NS REGHOLD; COND(IFNZ); DOJUMP(RECORDHIT); _ES

	_NS IMMCOMP(EQ,_ALTMVPBIT,_TEMP); CONST(_ALTMVPBIT); _ES
		/* eliinate the case where MULTIVIEW is only special */

	_NS REGHOLD; COND(IFEQ); DOJUMP(DOPOINT); _ES

/* must be Zbuffer or Depthcue mode -- discard x  & input Z */

	_NS REGHOLD; GEGET; DOJSUB(GET_X1Y1); _ES	/* get X, Y normal */

	_NS REGHOLD; BPC(SETADDRS); GEGET; _ES
				  /* load X,Y UC registers; input stereo X */

	_NS REGHOLD; GEGET; _ES		/* discard 1 wd; get Z */

	_NS LOADREG(_Z1,ALL16,NOMORE); _ES

	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP); 
	 ALUOP(ANDRS); FTODO; CONST(_ALTDEPBIT); _ES
				/* test for depthcue mode */

	_NS REGREG(MOVE,_Z1,_Z); COND(IFNZ); DOJUMP(DEPTH_POINT); _ES
			/* move Z where it belongs for later */

/* handle Z buffered case */
#ifdef CLIPZVALUES
	_NS LOADMAR(_DEPTH_PARAMS+4); CONST(_DEPTH_PARAMS+4); _ES

	_NS REGRAMCOMP(GE,_Z1,INC); _ES	/* test lower Z limit */

	_NS REGRAMCOMP(GT,_Z1,HOLD); COND(IFGE); DOJUMP(TSTMAXZ); _ES
				/* test upper limit; branch on lower OK */

	_NS REGHOLD; DECMAR; _ES		/* point to Z min again */

label(MAXZ)
	_NS RAM(RAMRD,_Z1,HOLD); DOJUMP(DOZPT); _ES	/* supply limit */

label(TSTMAXZ)
	_NS REGHOLD; COND(IFGT); DOJUMP(MAXZ); _ES
#endif

label(DOZPT)
	_NS LOADMAR(_CONFIG); CONST(_CONFIG); _ES

	/* test for double buffer mode */
	_NS IMMRAM(ANDOP, P0, 2, _TEMP, HOLD); CONST(2); _ES

	_NS REGHOLD; SEQ(JUMP); NEXT(DBZPT); COND(IFNZ); _ES

	_NS IMMRAM(ANDRS,_DEPTHMASK,_TEMP,INC); CONST(_DEPTHMASK); _ES

	_NS IMMREG(ORRS,_SWIZZLEBIT,_TEMP); BPC(LOADMODE);
	 CONST(_SWIZZLEBIT); _ES	/* mode: no depthcue, yes swizzle */

	_NS IMMRAM(ORRS,_FINISHBIT,_TEMP,HOLD); BPC(LOADCONFIG);
	 CONST(_FINISHBIT); _ES			/* cur config | FinishLine */

	_NS REGHOLD; BPC(READPIXELCD); _ES	/* request Z */

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_Z1,RAMNOP);
	 ALUOP(SUBSRC); YQ(FF,LDQ,REGWRD);
	 READBPCBUS; LDOUT;
	 COND(IFFALSE); SEQ(CJPP); _ES		/* wait for ack; spy on bus */
						/* do Z comparison */

	_NS SETROP(0,NONE); SETSOP(QOPERAND,0,RAMNOP); ALUOP(SONLYOP,P0);
	 FTODO; COND(IFOVF); DOJUMP(ZPT_OPPTEST); _ES
	     /* re-lookat compar; if overflow, check OPPOSITE of neg test */

	_NS REGHOLD; COND(IFNNEG); DOJUMP(PT_EXIT); _ES
				/* if new Z less than stored, replace */

	_NS REGREG(MOVE,_Z1,_Z1); BPC(SETCOLORCD); DOJUMP(MVPPOINT); _ES
						/* Z is CD code; go draw */

label(ZPT_OPPTEST)
	_NS REGHOLD; COND(IFNEG); DOJUMP(PT_EXIT); _ES

	_NS REGREG(MOVE,_Z1,_Z1); BPC(SETCOLORCD); DOJUMP(MVPPOINT); _ES

/*================================================================*/

label(DOPOINT)
	_NS REGHOLD; GEGET; DOJSUB(GET_X1Y1); _ES   /* go get point coords */

label(DRAWPOINT)
	_NS LOADMAR(_CONFIG+1); CONST(_CONFIG+1); _ES

	_NS IMMRAM(ORRS,_FINISHBIT,_TEMP,HOLD); BPC(LOADCONFIG);
	 CONST(_FINISHBIT); _ES			/* cur config | FinishLine */

	_NS REGHOLD; DECMAR; _ES	/* what a waste */

	_NS IMMRAM(ANDRS,_DEPTHMASK,_TEMP,HOLD); BPC(LOADMODE);
	 CONST(_DEPTHMASK); _ES			/* current mode & ~DepthCue */

	_NS LOADMAR(_MULTIVIEW); CONST(_MULTIVIEW); _ES

	_NS RAM(RAMRD,_TEMP,HOLD); _ES		/* look at multiview flag */

	_NS REGREG(MOVE,_Y1,_Y1); BPC(LOADYE);
	 COND(IFZ); DOJUMP(SGL_POINT); _ES	/* load YE to match YS */

/* multi-viewport -- this will be slow. */

label(MVPPOINT)
	_NS REGHOLD; DOJSUB(RESET_MASKS); _ES

label(POINTLOOP)
	_NS REGREG(MOVE,_X1,_X1); BPC(LOADXS); _ES	/* reload point X,Y */

	_NS REGREG(MOVE,_Y1,_Y1); BPC(LOADYS); _ES

	_NS REGREG(MOVE,_Y1,_Y1); BPC(LOADYE); _ES

	_NS REGHOLD; BPC(OCT0VECT); DOJSUB(NEW_MASK); _ES
		/* draw point in current vp; get new vp, test for last	*/

	_NS REGHOLD; COND(IFNZ); DOJUMP(POINTLOOP); _ES    /* if more vp's  */

label(PT_EXIT)				/* restore config/mode */
	_NS LOADMAR(_CONFIG); CONST(_CONFIG); _ES

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(LOADMODE); DOJSUB(NEXTMA); _ES

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(LOADCONFIG);
	  GEGET; DOJUMP(DISPATCH); _ES

label(SGL_POINT)
	_NS REGHOLD; BPC(OCT0VECT); DOJUMP(PT_EXIT); _ES


label(DEPTH_POINT)	/* uses subrs in depthvec.c */

	_NS LOADMAR(_DEPTH_PARAMS); CONST(_DEPTH_PARAMS); _ES
				/* begin setting up to do I1 calculation */

	_NS SETSOP(NONQOP,0,RAMRD); ALUOP(SONLYOP,P0);
	 YQ(FF,LDQ,REGWRD); INCMAR; DOJSUB(AZSUB); _ES
				/* "a" to Q reg; go and calc I2 = a * Z + b */

	_NS REGREG(MOVE,_I2,_I1); _ES			/* really wanted I1 */

	_NS REGRAMCOMP(LT,_I1,INC); _ES		/* check lower limit */

	_NS REGRAMCOMP(LE,_I1,HOLD);		/* check Imax */
	 COND(IFLT); DOJSUB(MINI); _ES		/* branch on too small */

	_NS REGHOLD; COND(IFLE); DOJUMP(DODPNT); _ES	/* br on Imax OK */

	_NS RAM(RAMRD,_I1,HOLD); _ES	/* I was too big - supply max */

label(DODPNT)
	_NS LOADMAR(_CONFIG+1); CONST(_CONFIG+1); _ES

	_NS IMMRAM(ORRS,_FINISHBIT,_TEMP,HOLD); BPC(LOADCONFIG);
	 CONST(_FINISHBIT); _ES			/* turn on finish bit */

	_NS LOADMAR(_MULTIVIEW); CONST(_MULTIVIEW); _ES

	_NS RAM(RAMRD,_TEMP,HOLD); _ES

	_NS REGREG(MOVE,_Y1,_Y1);BPC(LOADYE); COND(IFNZ); DOJUMP(MVP_DPNT);_ES

	_NS REGREG(MOVE,_I1,_I1); BPC(OCT0VECT); _ES

	_NS REGHOLD; BPC(NOOP); DOJUMP(PT_EXIT); _ES
						/* single-vp: draw and exit */

label(MVP_DPNT)
	_NS REGHOLD; DOJSUB(RESET_MASKS); _ES

label(MVP_DPTLOOP)
	_NS REGREG(MOVE,_X1,_X1); BPC(LOADXS); _ES

	_NS REGREG(MOVE,_Y1,_Y1); BPC(LOADYS); _ES

	_NS REGREG(MOVE,_Y1,_Y1); BPC(LOADYE); _ES

	_NS REGREG(MOVE,_I1,_I1); BPC(OCT0VECT); _ES

	_NS REGHOLD; BPC(NOOP); DOJSUB(NEW_MASK); _ES

	_NS REGHOLD; COND(IFNZ); DOJUMP(MVP_DPTLOOP); _ES

	_NS REGHOLD; DOJUMP(PT_EXIT); _ES


label(XFORM_POINT)
/*
 *	Do GEtransformpts in feedback mode!
 */
	_NS REGHOLD; LOADDI(UCOUNT); DOPUSH(7); _ES

	_NS REGHOLD; GEGET; SEQ(LOUP); _ES	/* throw away 8 wds */

	_NS REGHOLD; GEGET; DOJUMP(DISPATCH); _ES


label(DBZPT)
	_NS LOADMAR(_COLOR+1); CONST(_COLOR+1); _ES

	_NS RAM(RAMRD, _TEMP, HOLD); _ES

	/* what buffer am I looking at */
	_NS LOADMAR(_CONFIG+1); CONST(_CONFIG+1); _ES

	_NS 
	    ALUOP(ANDOP, P0);
	    SETROP(0, ALL16); CONST(1); LOADDI(UCONST);
	    SETSOP(NONQOP, 0, RAMRD); 
	    FTOYANDQ(FF, OLDQ, REGWRD);
	    BPC(LOADCONFIG);
	_ES

	_NS REGHOLD; SEQ(JUMP); NEXT(DBZPTA); COND(IFNZ); _ES

	_NS 
	    ALUOP(RONLYOP, P0);
	    SETROP(0, ALL16); CONST(0x6); LOADDI(UCONST);
	    SETSOP(NONQOP, 0, RAMNOP);
	    FTOYANDQ(FF, OLDQ, REGWRD);
	    BPC(LOADCONFIG);
	_ES

	_NS REGHOLD; BPC(READPIXELAB); _ES	/* request Z */

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_Z1,RAMNOP);
	 ALUOP(SUBSRC); YQ(FF,LDQ,REGWRD);
	 READBPCBUS; LDOUT;
	 COND(IFFALSE); SEQ(CJPP); _ES		/* wait for ack; spy on bus */
						/* do Z comparison */

	_NS SETROP(0,NONE); SETSOP(QOPERAND,0,RAMNOP); ALUOP(SONLYOP,P0);
	 FTODO; COND(IFOVF); DOJUMP(ZPT_OPPTEST_B); _ES
	     /* re-lookat compar; if overflow, check OPPOSITE of neg test */

	_NS REGHOLD; COND(IFNNEG); DOJUMP(PT_EXIT); _ES
				/* if new Z less than stored, replace */

	_NS 
	    REGHOLD;
	    DOJUMP(DBZ_MVPPOINT_B);
	_ES

label(ZPT_OPPTEST_B)
	_NS REGHOLD; COND(IFNEG); DOJUMP(PT_EXIT); _ES

label(DBZ_MVPPOINT_B)
	_NS REGHOLD; DOJSUB(RESET_MASKS); _ES

label(DBZ_POINTLOOP_B)
	_NS REGREG(MOVE,_X1,_X1); BPC(LOADXS); _ES

	_NS REGREG(MOVE,_Y1,_Y1); BPC(LOADYS); _ES

	_NS REGHOLD; BPC(SETADDRS); _ES

	_NS ALUOP(RONLYOP, P0);
	    SETROP(0, ALL16); CONST(0x16); LOADDI(UCONST);
	    SETSOP(NONQOP, 0, RAMNOP);
	    FTOYANDQ(FF, OLDQ, REGWRD);
	    BPC(LOADCONFIG);
	_ES

	_NS REGREG(RONLYOP, P0, _Z1, _Z1); BPC(DRAWPIXELAB); _ES

	_NS ALUOP(RONLYOP, P0);
	    SETROP(0, ALL16); CONST(0x1a); LOADDI(UCONST);
	    SETSOP(NONQOP, 0, RAMNOP);
	    FTOYANDQ(FF, OLDQ, REGWRD);
	    BPC(LOADCONFIG);
	_ES

	_NS REGREG(RONLYOP, P0, _TEMP, _TEMP); BPC(DRAWPIXELAB); _ES

	_NS REGHOLD; DOJSUB(NEW_MASK); _ES
		/* draw point in current vp; get new vp, test for last	*/

	_NS REGHOLD; COND(IFNZ); DOJUMP(DBZ_POINTLOOP_B); _ES

	_NS REGHOLD; DOJUMP(PT_EXIT); _ES

/************/
/************/
/************/
label(DBZPTA)
	_NS 
	    ALUOP(RONLYOP, P0);
	    SETROP(0, ALL16); CONST(0x9); LOADDI(UCONST);
	    SETSOP(NONQOP, 0, RAMNOP);
	    FTOYANDQ(FF, OLDQ, REGWRD);
	    BPC(LOADCONFIG);
	_ES

	_NS REGHOLD; BPC(READPIXELCD); _ES	/* request Z */

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_Z1,RAMNOP);
	 ALUOP(SUBSRC); YQ(FF,LDQ,REGWRD);
	 READBPCBUS; LDOUT;
	 COND(IFFALSE); SEQ(CJPP); _ES		/* wait for ack; spy on bus */
						/* do Z comparison */

	_NS SETROP(0,NONE); SETSOP(QOPERAND,0,RAMNOP); ALUOP(SONLYOP,P0);
	 FTODO; COND(IFOVF); DOJUMP(ZPT_OPPTEST_A); _ES
	     /* re-lookat compar; if overflow, check OPPOSITE of neg test */

	_NS REGHOLD; COND(IFNNEG); DOJUMP(PT_EXIT); _ES
				/* if new Z less than stored, replace */

	_NS
	    REGHOLD; DOJUMP(DBZ_MVPPOINT_A);
	_ES

label(ZPT_OPPTEST_A)
	_NS REGHOLD; COND(IFNEG); DOJUMP(PT_EXIT); _ES

label(DBZ_MVPPOINT_A)
	_NS REGHOLD; DOJSUB(RESET_MASKS); _ES

label(DBZ_POINTLOOP_A)
	_NS REGREG(MOVE,_X1,_X1); BPC(LOADXS); _ES	/* reload point X,Y */

	_NS REGREG(MOVE,_Y1,_Y1); BPC(LOADYS); _ES

	_NS REGHOLD; BPC(SETADDRS); _ES

	_NS ALUOP(RONLYOP, P0);
	    SETROP(0, ALL16); CONST(0x19); LOADDI(UCONST);
	    SETSOP(NONQOP, 0, RAMNOP);
	    FTOYANDQ(FF, OLDQ, REGWRD);
	    BPC(LOADCONFIG);
	_ES

	_NS REGREG(RONLYOP, P0, _Z1, _Z1); BPC(DRAWPIXELAB); _ES

	_NS ALUOP(RONLYOP, P0);
	    SETROP(0, ALL16); CONST(0x15); LOADDI(UCONST);
	    SETSOP(NONQOP, 0, RAMNOP);
	    FTOYANDQ(FF, OLDQ, REGWRD);
	    BPC(LOADCONFIG);
	_ES

	_NS REGREG(RONLYOP, P0, _TEMP, _TEMP); BPC(DRAWPIXELAB); _ES

	_NS REGHOLD; DOJSUB(NEW_MASK); _ES
		/* draw point in current vp; get new vp, test for last	*/

	_NS REGHOLD; COND(IFNZ); DOJUMP(DBZ_POINTLOOP_A); _ES  

	_NS REGHOLD; DOJUMP(PT_EXIT); _ES
}
