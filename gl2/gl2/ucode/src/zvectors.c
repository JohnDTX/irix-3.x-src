/* zvectors.c
 *
 *	Save X2, Y2, Z2 to free up registers
 *	Input [and clip] Z2
 *	Calculate delta X,Y,Z
 *	Based on |deltaX|-|deltaY|, set up incremental deltas for half-DDA
 *		DZ = DZ / |Dlong|
 *		Dshort = Dshort / |Dlong|
 *	Bias:   Short1 += 1/2	( note: |slope| always <= 1 )
 *		Z += 1/2
 *	Draw the line once in each screenmask
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "fbc.mic.h"
#include "consts.h"

zvectors()
{
newfile("zvectors.c");

#define _XLO		_X2	/* save X2, Y2, Z2 before drawing */
#define _YLO		_Y2
#define _ZLO		_Z2
/*	_DELTAX		6	*/
#define _COLR		7
#define _BIAS		8
#define _DELTAY		9
#define _DELTAZ		10
#define _DX_LO		11
#define _DY_LO		12
#define _DZ_LO		13
#define _COUNT		14

label(Z_VEC_DRAW)

/* save X2 Y2 Z2 til end of drawing (at same time, discard x stereo */

	_NS LOADMAR(_SAVE2+5); GEGET; CONST(_SAVE2+5); _ES

	_NS RAM(RAMWR,_X2,INC); _ES

	_NS RAM(RAMWR,_Y2,INC); GEGET; _ES	/* get Z */

	_NS LOADREG(_Z2,ALL16,NOMORE); _ES	/* stick Z into reg */

	_NS RAM(RAMWR,_Z2,HOLD); _ES		/* save in scratch */

#ifdef CLIPZVALUES
/* clip Z2 */

	_NS LOADMAR(_DEPTH_PARAMS+4); CONST(_DEPTH_PARAMS+4); _ES

	_NS REGRAMCOMP(GE,_Z2,INC); _ES		/* check lower Z */

	_NS REGRAMCOMP(GT,_Z2,DEC); COND(IFGE); DOJUMP(VTSTMINZ); _ES
					/* point to max if limit needed */

label(LIMITZ2)
	_NS RAM(RAMRD,_Z2,HOLD); DOJUMP(ZCHECKED); _ES

label(VTSTMINZ)
	_NS REGHOLD; INCMAR; COND(IFGT); DOJUMP(LIMITZ2); _ES
#endif CLIPZVALUES

label(ZCHECKED)

/* calculate deltas and determine "xline" vs. "yline" */

	_NS REGREG(MOVE,_Z1,_DELTAZ); _ES

	_NS REGREG(SUBRSC,_Z2,_DELTAZ); _ES	/* DZ = Z2 - Z2  */

	_NS LOADMAR(_SAVE1); LDOUT; CONST(_SAVE1); _ES

	_NS RAM(RAMWR,_DELTAZ,INC); _ES
				/* store DZ in param space as dividend */

	_NS LOADIMM(_BIAS,0x8000); CONST(0x8000); _ES
						/* constant 1/2 for biasing */

	_NS REGREG(MOVE,_Y1,_DELTAY); _ES

	_NS REGREG(SUBRSC,_Y2,_DELTAY); _ES		/* deltaY = Y2 - Y1 */

	_NS REGREG(MOVE,_X1,_DELTAX); _ES

	_NS REGREG(SUBRSC,_X2,_DELTAX); _ES		/* deltaX = X2 - X1 */

	_NS REGREG(MOVE,_DELTAY,_DELTAY); COND(IFNEG); DOJUMP(ZV_NEGDX); _ES
				/* examine deltaY -- if DX neg, jump */

	_NS REGCOMP(GE,_DELTAX,_DELTAY); COND(IFNNEG); DOJUMP(DXDYTEST); _ES
			/* DX pos; compare DX > DY; if DY pos, go compare */

	_NS SETROP(_DELTAX,NONE); SETSOP(NONQOP,_DELTAY,RAMNOP);
	 ALUOP(ADD); FTODO; DOJUMP(DXDYTEST); _ES
					/* pos DX, neg DY - must add */

label(ZV_NEGDX)
	_NS REGCOMP(GE,_DELTAY,_DELTAX); COND(IFNEG); DOJUMP(DXDYTEST); _ES
			/* DX neg; compare -DY > -DX; if DY neg, go compare */

	_NS SETROP(_DELTAX,NONE); SETSOP(NONQOP,_DELTAY,RAMNOP);
	ALUOP(ADD); FTODO; _ES		/* neg DX, pos DY - add to compare */

	_NS REGHOLD; COND(IFNEG); DOJUMP(XLINE); _ES

	_NS REGHOLD; DOJUMP(YLINE); _ES

label(DXDYTEST)
	_NS REGHOLD; COND(IFNNEG); DOJUMP(XLINE); _ES
				/* if result of compare pos, DX was bigger */

label(YLINE)
	_NS RAM(RAMWR,_DELTAY,HOLD); _ES  /* save delta long as divisor */

	_NS REGREG(RONLYOP, P0, _DELTAY, _DELTAY); 
	    COND(IFNEG); DOJUMP(NEGY_LINE); _ES
					/* go handle negative case */
/* positive Y */
	_NS REGREG(FLOWOP, P0, _COUNT, _COUNT); 
	    COND(IFZ); DOJUMP(ZV_ITERATE); _ES
			/* if long axis 0, deltas irrelevant - go draw */

	_NS REGHOLD; DOJSUB(DIVIDE16); _ES

	_NS RAM(RAMRD,_DZ_LO,DEC); DOJSUB(ZV_YSETUP); _ES

	_NS REGREG(INCR,_ZERO,_DELTAY); DOJUMP(ZV_ITERATE); _ES
						/* incremental DY is one */

label(NEGY_LINE)
	_NS REGREG(SUBRSC,_ZERO,_DELTAY); _ES	/* make DY positive */

	_NS RAM(RAMWR,_DELTAY,HOLD); DOJSUB(DIVIDE16); _ES

	_NS RAM(RAMRD,_DZ_LO,DEC); DOJSUB(ZV_YSETUP); _ES
			/* fetch incremental DZ; go do body of setup */

	_NS REGREG(ONES,0,_DELTAY); DOJUMP(ZV_ITERATE); _ES
						/* incremental DY is -1 */

label(XLINE)
	_NS RAM(RAMWR,_DELTAX,HOLD); _ES	/* deltax is long axis */

	_NS REGREG(MOVE,_DELTAX,_DELTAX); COND(IFNEG); DOJUMP(NEGX_LINE); _ES
					/* go handle negative case */

	_NS REGREG(FLOWOP, P0, _COUNT, _COUNT); 
	    COND(IFZ); DOJUMP(ZV_ITERATE); _ES

	_NS REGHOLD; DOJSUB(DIVIDE16); _ES

	_NS RAM(RAMRD,_DZ_LO,DEC); DOJSUB(ZV_XSETUP); _ES

	_NS REGREG(INCR,_ZERO,_DELTAX); DOJUMP(ZV_ITERATE); _ES
						/* incremental DX is one */

label(NEGX_LINE)
	_NS REGREG(SUBRSC,_ZERO,_DELTAX); _ES	/* make DX pos */

	_NS RAM(RAMWR,_DELTAX,HOLD); DOJSUB(DIVIDE16); _ES

	_NS RAM(RAMRD,_DZ_LO,DEC); DOJSUB(ZV_XSETUP); _ES

	_NS REGREG(ONES,0,_DELTAX);  _ES	/* incremental dX is -1 */

label(ZV_ITERATE)
	_NS REGHOLD; DOJSUB(RESET_MASKS); _ES

	_NS REGREG(RONLYOP, P0, _COUNT, _COUNT); DOTOOUTREG; _ES

	_NS LOADMAR(_COLOR+1); CONST(_COLOR+1); _ES

	_NS RAM(RAMRD,_COLR,HOLD); _ES
				/* fetch current color; decrement count */

/* save X1 Y1 (including biases) and Z1 for restoring for each viewport */
label(VPNXT)
	_NS LOADMAR(_SAVE2); CONST(_SAVE2); _ES

	_NS RAM(RAMWR,_X1,INC); SEQ(LDCT); LOADDI(OUTPUTCOUNT); _ES
	_NS RAM(RAMWR,_Y1,INC); _ES
	_NS RAM(RAMWR,_Z1,INC); _ES
	_NS RAM(RAMWR,_XLO,INC); _ES
	_NS RAM(RAMWR,_YLO,HOLD); DOJUMP(ZV_EACHVP); _ES

label(ZV_VPLOOP)
	_NS LOADMAR(_SAVE2); CONST(_SAVE2); _ES	/* restore starting point */

	_NS RAM(RAMRD,_X1,INC); _ES
	_NS RAM(RAMRD,_Y1,INC); _ES
	_NS RAM(RAMRD,_Z1,INC); _ES
	_NS RAM(RAMRD,_XLO,INC); _ES	/* clear fract accumulators */
	_NS RAM(RAMRD,_YLO,HOLD); SEQ(LDCT); LOADDI(OUTPUTCOUNT); _ES

label(ZV_EACHVP)
	_NS REGREG(MOVE,_BIAS,_ZLO); _ES	/* bias Z by 1/2 */

	_NS REGREG(MOVE,_X1,_X1); BPC(LOADXS); _ES

	_NS REGREG(MOVE,_Y1,_Y1); BPC(LOADYS); _ES

	_NS LOADMAR(_CONFIG); CONST(_CONFIG); _ES

	_NS /* test the double bit */
	    ALUOP(ANDOP, P0);
	    SETROP(0, ALL16); LOADDI(UCONST); CONST(2);
	    SETSOP(NONQOP, 0, RAMRD);
	    FTOYANDQ(FF, OLDQ, REGWRD);
	_ES

	_NS REGHOLD; COND(IFNZ); SEQ(JUMP); NEXT(DBZ_LINE); DOTOMAR(INC); _ES

label(ZV_PIXELLP)
	_NS REGHOLD; BPC(SETADDRS); _ES

	_NS REGHOLD; BPC(READPIXELCD); _ES		/* request Z value */

	_NS LOADDI(BPCDATA); SETROP(0,ALL16); SETSOP(NONQOP,_Z1,RAMNOP);
	 ALUOP(SUBSRC); YQ(FF,LDQ,REGWRD); READBPCBUS; LDOUT;
	 COND(IFFALSE); SEQ(CJPP); _ES		/* do comparison, save in Q */

	_NS SETROP(0,NONE); SETSOP(QOPERAND,0,RAMNOP); ALUOP(SONLYOP,P0);
	 FTODO; COND(IFOVF); DOJUMP(ZOPP_TEST); _ES
	     /* re-lookat compar; if overflow, check OPPOSITE of neg test */

	_NS REGHOLD; COND(IFNNEG); DOJUMP(NOZDRAW); _ES
				/* if new Z less than stored, replace */

label(ZDRAWIT)
	_NS REGREG(MOVE,_Z1,_Z1); BPC(DRAWPIXELCD); _ES

	_NS REGREG(ADD,_DZ_LO,_ZLO); PROPOUT16; _ES
						/* update Z while UC busy */

	_NS REGREG(ADD,_DELTAZ,_Z1); PROPIN; _ES

	_NS REGREG(MOVE,_COLR,_COLR); BPC(DRAWPIXELAB); DOJUMP(ZNEXTPIX); _ES
					/* draw in pixel in current shade */

label(ZOPP_TEST)
	_NS REGHOLD; COND(IFNNEG); DOJUMP(ZDRAWIT); _ES

label(NOZDRAW)				/* just calc next new Z */
	_NS REGREG(ADD,_DZ_LO,_ZLO); PROPOUT16; _ES

	_NS REGREG(ADD,_DELTAZ,_Z1); PROPIN; _ES

label(ZNEXTPIX)
	_NS REGREG(ADD,_DX_LO,_XLO); PROPOUT16; _ES

	_NS REGREG(ADD,_DELTAX,_X1); BPC(LOADXS); PROPIN; _ES	/* new X */

	_NS REGREG(ADD,_DY_LO,_YLO); PROPOUT16; _ES

	_NS REGREG(ADD,_DELTAY,_Y1); BPC(LOADYS); PROPIN;
	 SEQ(RPCT); NEXT(ZV_PIXELLP); _ES		/* new Y; next pixel*/

	_NS REGHOLD; DOJSUB(NEW_MASK); _ES

	_NS 
	    REGREG(RONLYOP, P0, _COUNT, _COUNT); DOTOOUTREG; 
	    COND(IFNZ); DOJUMP(ZV_VPLOOP); 
	_ES

	_NS LOADMAR(_SAVE2+5); CONST(_SAVE2+5); _ES

	_NS RAM(RAMRD,_X1,INC); _ES	/* move pt. 2 to pt. 1 */
	_NS RAM(RAMRD,_Y1,INC); _ES
	_NS RAM(RAMRD,_Z1,HOLD); GEGET; DOJUMP(DISPATCH); _ES

/*================================================================*/

label(ZV_YSETUP)		/* body of Yline setup */
	_NS RAM(RAMRD,_DELTAZ,HOLD); _ES	/* fetch rest of DZ */

	_NS LOADMAR(_SAVE1); LDOUT; CONST(_SAVE1); _ES

	_NS RAM(RAMWR,_DELTAX,HOLD); DOJSUB(DIVIDE16); _ES /* delta short */

	_NS RAM(RAMRD,_DX_LO,DEC); _ES

	_NS RAM(RAMRD,_DELTAX,HOLD); _ES	/* read msw */

	_NS REGREG(MOVE,_ZERO,_DY_LO); _ES

	_NS REGREG(MOVE,_BIAS,_XLO); _ES	/* bias short axis posn */

	_NS REGREG(MOVE, _ZERO,_YLO); _ES	/* long axis posn exact */

	_NS REGREG(MOVE, _DELTAY, _COUNT); SEQ(RETN); _ES
							/* for counter */

label(ZV_XSETUP)		/* ditto for Xlines */
	_NS RAM(RAMRD,_DELTAZ,HOLD); _ES

	_NS LOADMAR(_SAVE1); LDOUT; CONST(_SAVE1); _ES

	_NS RAM(RAMWR,_DELTAY,HOLD); DOJSUB(DIVIDE16); _ES /* delta short */

	_NS RAM(RAMRD,_DY_LO,DEC); _ES

	_NS RAM(RAMRD,_DELTAY,HOLD); _ES

	_NS REGREG(MOVE,_ZERO,_DX_LO); _ES

	_NS REGREG(MOVE,_BIAS,_YLO); _ES	/* bias short axis posn */

	_NS REGREG(MOVE,_ZERO,_XLO); _ES

	_NS REGREG(MOVE,_DELTAX,_COUNT); SEQ(RETN); _ES
}
