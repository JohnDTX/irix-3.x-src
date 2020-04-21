/* fixchar.c	-- fixed-font character routines
 *		for simple terminal applications:
 *			no gross clipping
 *			no hit mode
 *			fixed height, width, spacing preset
 *			no X or Y offset
 *			draw command provides font ram address only
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"

#define _YEND   3	/*						*/
#define _WIDTH  4	/* copy of scratch param			*/
#define _NCT	5	/* copy of PASSN				*/
/*	_ZRO    6	   defined in consts.h				*/
#define _SPACE	8	/* copy of scratch param			*/
#define _WK     9	/*						*/
#define _WINDL  10	/* viewport left edge		\		*/
#define _WINDB  11	/* 	bottom			|		*/
#define _WINDR  12	/* 	right			|		*/
#define _WINDT  13	/* 	top			 > unsaved from ram */
#define _POSNX  14	/* char. posn.			|		*/
#define _POSNY  15	/* 				/		*/


fixchar()
{
newfile("fixchar.c");

label(FIXCHAR_LOAD)
/*
 *  save parameters for drawing chars:
 *	height
 *	width
 *	spacing
 */

	_NS LOADMAR(_FIXPARAMS); CONST(_FIXPARAMS);_ES
					/*ld adr of params  */

	_NS LOADIMM(_TEMP,2); GEGET; SEQ(LDCT); CONST(2); _ES

	_NS REGHOLD; DOJUMP(INTO_RAM); _ES	/* see attributes.c */


label(FIXCHAR_DRAW)
/*
 *  draws a single character per command
 *  input:  font ram address of masks
 *  uses height, width, spacing from FIXDRAW_LOAD
 */

	_NS LOADMAR(_CHARVIEW); GEGET; CONST(_CHARVIEW);_ES
					/* point to viewport coords  */

	_NS REGREG(MOVE,_PASSN,_NCT); DOJSUB(LM10UP); _ES
					/* save N count-1; go get viewport */

	_NS REGHOLD; INCMAR; _ES

	_NS REGREG(SUBSR,_ZRO,_NCT); LDOUT; INCMAR; DOJSUB(LM14UP);_ES
							/* unsave char posn */

	_NS LOADIMM(_WK,_FIXPARAMS); LDMAR; CONST(_FIXPARAMS); _ES
				 /* point to params; save address */

	_NS REGREG(MOVE,_POSNY,_YEND); BPC(LOADYS);
	 LOADDI(OUTPUTCOUNT); SEQ(LDCT); _ES	/* send/copy starting Y;  */
			/* set up loop count before outreg messed up */

	_NS REGRAM(ADD,_YEND,_YEND,HOLD); BPC(LOADYE); _ES
					/* Y + height to Y end reg */

	_NS LOADMAR(_MULTIVIEW); CONST(_MULTIVIEW); _ES

	_NS RAM(RAMRD,_TEMP,HOLD); _ES		/* look at multiview flag */

	_NS REGREG(INCR,_WK,_WK); LDMAR; COND(IFNZ); DOJUMP(FIX_MULTI); _ES
					/* point to width (FIXPARAM+1) */

	_NS RAM(RAMRD,_WIDTH,INC); _ES		/* get width, pt to spacing */

label(FIXDRAW_LOOP)
	_NS LOADREG(_WK,ALL16,NOMORE); BPC(LOADFA); _ES
					/* get/send base addr	*/

	_NS REGREG(MOVE,_POSNX,_WK); BPC(LOADXS); _ES
					/* send/copy starting X	*/

	_NS REGREG(ADD,_WIDTH,_WK); BPC(LOADXE); _ES
						/* X + width to X end */

#ifdef UC3
	_NS REGRAM(ADD,_POSNX,_POSNX,HOLD); BPC(CHDRAW);
#endif
#ifdef UC4
	_NS REGRAM(ADD,_POSNX,_POSNX,HOLD); BPC(DRAWCHAR);
#endif
	 GEGET; SEQ(RPCT); NEXT(FIXDRAW_LOOP); _ES
			/* draw the char; add spacing to X posn; repeat */

label(FIXCHAR_EXIT)
	_NS LOADMAR(_CHARPOSN); CONST(_CHARPOSN); _ES

	_NS RAM(RAMWR,_POSNX,HOLD); DOJUMP(DISPATCH); _ES
				/* save new char X and exit */

/* multi-viewport version */

label(FIX_MULTI)
	_NS RAM(RAMRD,_WIDTH,INC); _ES		/* get width; pt to spacg */

	_NS RAM(RAMRD,_SPACE,HOLD); _ES		/* get spacing- free up MAR */

label(FIX_CHARLOOP)				/* loop for each char */
	_NS LOADREG(_WK,ALL16,NOMORE); BPC(LOADFA); _ES
					/* get/send base addr	*/

	_NS REGHOLD; DOJSUB(RESET_MASKS); _ES

label(FIXLOOP_VP)
	_NS REGREG(MOVE,_POSNX,_WK); BPC(LOADXS); _ES
					/* send/copy starting X	*/

	_NS REGREG(ADD,_WIDTH,_WK); BPC(LOADXE); _ES
						/* X + width to X end */

	_NS REGREG(MOVE,_POSNY,_POSNY); BPC(LOADYS); _ES	/* reload Y */

	_NS REGREG(MOVE,_YEND,_YEND); BPC(LOADYE); _ES		/* reload Y */

#ifdef UC3
	_NS REGHOLD; BPC(CHDRAW); DOJSUB(NEW_MASK); _ES
#endif
#ifdef UC4
	_NS REGHOLD; BPC(DRAWCHAR); DOJSUB(NEW_MASK); _ES
#endif
			/* draw the char; add spacing to X posn; next VP */

	_NS REGHOLD; COND(IFNZ); DOJUMP(FIXLOOP_VP); _ES  /* loop on VP */

	_NS REGREG(SUBSR,_ZRO,_NCT); _ES		/* decr char count */

	_NS REGREG(ADD,_SPACE,_POSNX,HOLD); GEGET;	/* update w/ spacing*/
	 COND(IFNNEG); DOJUMP(FIX_CHARLOOP); _ES
				/* iterate for each char in passthru */

	_NS REGHOLD; DOJUMP(FIXCHAR_EXIT); _ES
}
