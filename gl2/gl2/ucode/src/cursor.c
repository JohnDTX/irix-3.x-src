/* cursor.c		<< GF2/UC4  >>
 *
 *	cursor draw/undraw microcode
 *	DRAW_CURSOR (x,y)
 *	UNDRAW_CURSOR
 *		LD_CURCOLOR
 *			NEXTMA
 *
 *			X1, Y1 left intact
 *		-- new "side door" cursor draw/undraw
 *		-- old draw/undraw are backward-compatible w/ s/w
 *		-- two-color cursor implemented.  Configure 2nd color using
 *		    LOADRAM at _SECONDCURSOR (7 params)
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"


#define _WK		12
#define _TMP		9
#define _CURSX		10
#define _CURSY		11
#define _ADR		13
#define _YFROM		12	/* Y source param for block  copy	*/
#define _YTO		13	/* Y dest ditto				*/
#define _SIDE_DOOR	14	/* 0 = front door (pipe);  1 = side door */
#define _OLDX		15

cursor()
{
newfile("cursor.c");


label(DRAW_CURSOR)
/*
 *	calls setup subroutine,
 *	reads X,Y from pipe, then calls draw subroutine
 */
	_NS LOADMAR(_CURSAVE); CONST(_CURSAVE); _ES

	_NS RAM(RAMWR,9,INC); DOJSUB(STM10UP); _ES

	_NS REGREG(ZERO,0,_SIDE_DOOR); DOJSUB(LD_CURCOLOR); _ES
			/* clear side-door flag; go set up for cur drawing */
			/* return having tested CURSORDRAWN	*/

	_NS REGHOLD; BPC(NOOP); COND(IFNZ); DOJSUB(UNDRAW_CUR_SUB); _ES
			/* if already drawn, undraw before drawing	*/

	_NS REGHOLD; GEGET; DOJSUB(GET_X2Y2); _ES    /* coords to _CURSX,Y */

	_NS REGREG(MOVE,_X2,_CURSX); _ES

	_NS REGREG(MOVE,_Y2,_CURSY); DOJUMP(DRAW_ONLY); _ES
						    /* go draw & exit */


label(UNDRAW_CURSOR)
/*
 *	just calls setup routine, then performs undraw
 */
	_NS LOADMAR(_CURSAVE); CONST(_CURSAVE); _ES

	_NS RAM(RAMWR,9,INC); DOJSUB(STM10UP); _ES

	_NS REGREG(ZERO,0,_SIDE_DOOR); DOJSUB(LD_CURCOLOR); _ES
		/* clear side-door flag; go set up cursor's color/config */
		/* return having tested CURSORDRAWN flag */

	_NS REGHOLD; COND(IFZ); DOJUMP(CURSOR_EXIT); _ES
				/* if cursor not drawn, don't do anything */

	_NS REGHOLD; BPC(NOOP); DOJSUB(UNDRAW_CUR_SUB); _ES

	_NS REGHOLD; DOJUMP(CURSOR_EXIT); _ES



label(DRAW_CUR_SUB)
/*
/*	saves 2 adjacent 16x16 pixel screen areas
/*	draws previously selected cursor character at new (x,y)
*/

	_NS IMMREG(INCR,_OLDCURSOR,_TMP); LDMAR; CONST(_OLDCURSOR);_ES
				/* load addr for Y coord saving	*/

	_NS LOADDI(UCONST); MICROCONST(16); SETROP(0,ALL16);
	 SETSOP(NONQOP,_CURSX,RAMNOP);
 	 ALUOP(ADDOP,P0); FTODO; BPC(LOADXS); CONST(16);_ES
			/* send X+16 to X start reg		*/

	_NS LOADIMM(_YTO,_YOFFSCREEN); CONST(_YOFFSCREEN);_ES
			/* set up destination of 16x16 block	*/

	_NS REGREG(MOVE,_CURSY,_YFROM);_ES	/* set up source of block  */

	_NS RAM(RAMWR,_CURSY,DEC); DOJSUB(MOVECURSOR);_ES
					/* save Y in scratch; go copy */

	_NS REGREG(MOVE,_CURSY,_YFROM); DECMAR; _ES
			/* restore source address; point at DRAWN flag  */

	_NS RAM(RAMWR,_YTO,INC); _ES	/* write non-zero value, pt to oldX */

	_NS SETROP(_CURSX,NONE); SETSOP(NONQOP,_CURSX,RAMWR);
	 ALUOP(RONLYOP,P0); FTODO; BPC(LOADXS); DOJSUB(MOVECURSOR);_ES
	     /* send X to start reg for leftmost block; save in scratch   */

/* now draw cursor glyph -- first load char bounds	*/
/* X start is already correct	*/
	_NS REGREG(MOVE,_CURSY,_CURSY); BPC(LOADYS);_ES
						/* load proper Y start  */

	_NS IMMREG(ADD,15,_CURSX); BPC(LOADXE); CONST(15);_ES
							/* load X end  */

	_NS IMMREG(ADD,15,_CURSY); BPC(LOADYE); CONST(15);_ES
							/* load Y end  */

	_NS REGHOLD; BPC(DRAWCHAR); _ES  /* draw the glyph; exit */

label(DRAW_CURSOR_2)
	_NS LOADMAR(_SECONDCURSOR); CONST(_SECONDCURSOR); _ES
						    /* alternate parameters */

	_NS RAM(RAMRD,_TMP,HOLD); BPC(LOADFA); _ES   /* send font ram adr */

	_NS REGHOLD; INCMAR; DOJSUB(CUR_COLORWE_ONLY); _ES
			/* point to color, we; go load them */

	_NS REGHOLD; BPC(NOOP); _ES

	_NS LOADMAR(_OLDCURSOR); CONST(_OLDCURSOR);_ES

	_NS SETSOP(NONQOP,_CURSX,RAMRD); ALUOP(SONLYOP,P0);
	 YQ(FF,OLDQ,REGWRE); YTODO(ALL16); BPC(LOADXS);_ES
					/* old X coord to X start reg	*/
	_NS REGHOLD; INCMAR; _ES

	_NS SETSOP(NONQOP,_CURSY,RAMRD); ALUOP(SONLYOP,P0);
	 YQ(FF,OLDQ,REGWRE); YTODO(ALL16); BPC(LOADYS);_ES
					/* old Y coord to Y start reg	*/

	_NS IMMREG(ADD,15,_CURSX); BPC(LOADXE); CONST(15);_ES
							/* load X end  */

	_NS IMMREG(ADD,15,_CURSY); BPC(LOADYE); CONST(15);_ES
							/* load Y end  */

	_NS REGHOLD; BPC(DRAWCHAR); SEQ(RETN); _ES
						/* draw the glyph; exit */


label(UNDRAW_CUR_SUB)
/*
/*  restores screen area where cursor was last displayed.
/*	gets old coords from OLDCURSOR
*/
	_NS LOADMAR(_OLDCURSOR); CONST(_OLDCURSOR);_ES

	_NS SETSOP(NONQOP,_OLDX,RAMRD); ALUOP(SONLYOP,P0);
	 YQ(FF,OLDQ,REGWRE); YTODO(ALL16); BPC(LOADXS);_ES
					/* old X coord to X start reg	*/

	_NS LOADIMM(_YFROM,_YOFFSCREEN); INCMAR; CONST(_YOFFSCREEN);_ES
		/* Y source is offscreen area; address old Y coord	*/

	_NS RAM(RAMRD,_YTO,HOLD); DOJSUB(MOVECURSOR);_ES
		/* read old Y = dest; go unsave block of pixels		*/

	_NS LOADIMM(_YFROM,_YOFFSCREEN); DECMAR; CONST(_YOFFSCREEN);_ES
			/* reload source adr; pt at X	*/

	_NS IMMREG(ADD,16,_OLDX); BPC(LOADXS); CONST(16);_ES
			/* load X+16 into start reg for 2nd block	*/

	_NS REGHOLD; DOJSUB(MOVECURSOR);_ES	/* move 2nd block  */

	_NS REGREG(ZERO,0,_TMP); DECMAR; _ES	/* point at DRAWN flag */

	_NS RAM(RAMWR,_TMP,HOLD); SEQ(RETN); _ES	/* store zero */

/*================================================================*/
/*			common subroutines
 */

label(LD_CURCOLOR)
/*  subr to set up cursor's color and configuration */
/*  NOTE --- cursor font ram adr is absolute! */
/*  returns having tested DRAWN flag */

	_NS LOADMAR(_CURSOR); CONST(_CURSOR);_ES
			/* address cursor params (baseadr,mode,config...) */

label(CONT_LDCURCOLOR)
	_NS RAM(RAMRD,_TMP,HOLD); BPC(LOADFA); _ES   /* send font ram adr */

	_NS REGHOLD; INCMAR; DOJSUB(CUR_COLORWE); _ES
			/* point to mode, config, color, we; go load them */

	_NS LOADMAR(_CURSORDRAWN); CONST(_CURSORDRAWN); _ES

	_NS RAM(RAMRD,_TMP,HOLD); SEQ(RETN); _ES    /* end of LD_CURCOLOR */


label(CUR_COLORWE)	/* just writes mode, config, color, we from scratch */
	_NS SETSOP(NONQOP,0,RAMRD); ALUOP(SONLYOP,P0);
	 FTODO; BPC(LOADMODE); DOJSUB(NEXTMA); _ES
				/* read mode, send to UC, go inc MAR */

	_NS SETSOP(NONQOP,0,RAMRD); ALUOP(SONLYOP,P0);
	 FTODO; BPC(LOADCONFIG); DOJSUB(NEXTMA); _ES
			/* read config from ram, send to config reg	*/

label(CUR_COLORWE_ONLY)
	_NS SETSOP(NONQOP,0,RAMRD); ALUOP(SONLYOP,P0);
	 FTODO; BPC(SETCOLORCD); DOJSUB(NEXTMA); _ES
					/* continue with color, wrt enables */

	_NS SETSOP(NONQOP,0,RAMRD); ALUOP(SONLYOP,P0);
	 FTODO; BPC(SETCOLORAB); DOJSUB(NEXTMA); _ES

	_NS SETSOP(NONQOP,0,RAMRD); ALUOP(SONLYOP,P0);
	 FTODO; BPC(SETWECD); DOJSUB(NEXTMA); _ES

	_NS SETSOP(NONQOP,0,RAMRD); ALUOP(SONLYOP,P0);
	 FTODO; BPC(SETWEAB); SEQ(RETN); _ES


label(REGULAR_RESTORE)
	_NS LOADMAR(_CONFIG); CONST(_CONFIG); _ES

	_NS SEQ(JUMP); NEXT(CUR_COLORWE); _ES

label(RESTORE_COLORWE)	/* just writes mode, config, color, we from scratch */
        _NS /* test for zbuffering */
	    ALUOP(SONLYOP,P0);
	    SETROP(0, NONE);
	    SETSOP(NONQOP,0,RAMRD); 
	    FTOYANDQ(FF, OLDQ, REGWRD);
	_ES

	_NS 
	    COND(IFZ); SEQ(JUMP); NEXT(REGULAR_RESTORE);
	_ES

	_NS LOADMAR(_CONFIG); CONST(_CONFIG); _ES

        _NS /* test for double buffering */
	    ALUOP(ANDOP,P0);
	    SETROP(0, ALL16); LOADDI(UCONST); CONST(2);
	    SETSOP(NONQOP,0,RAMRD); 
	    FTOYANDQ(FF, OLDQ, REGWRD);
	_ES

	_NS 
	    COND(IFZ); SEQ(JUMP); NEXT(CUR_COLORWE) 
	_ES

	_NS SETSOP(NONQOP,0,RAMRD); ALUOP(SONLYOP,P0);
	 FTODO; BPC(LOADMODE); DOJSUB(NEXTMA); _ES
				/* read mode, send to UC, go inc MAR */

	_NS SETSOP(NONQOP,0,RAMRD); ALUOP(SONLYOP,P0);
	 FTODO; BPC(LOADCONFIG); DOJSUB(NEXTMA); _ES
			/* read config from ram, send to config reg	*/

	_NS SETSOP(NONQOP,0,RAMRD); ALUOP(SONLYOP,P0);
	 FTODO; BPC(SETCOLORCD); DOJSUB(NEXTMA); _ES
					/* continue with color, wrt enables */

	_NS SETSOP(NONQOP,0,RAMRD); ALUOP(SONLYOP,P0);
	     FTODO; BPC(SETCOLORAB); DOJSUB(NEXTMA); _ES

	/* here I need to call the special code to load dbz WEs */
	_NS REGHOLD; DOTOMAR(INC); _ES

	_NS 
	    REGREG(FLOWOP, P0, _WK, _WK); BPC(LOADMODE);
	_ES

	_NS
	    RAM(RAMRD, _WK, HOLD); 
	    SEQ(JUMP); NEXT(LOAD_DBZ_WENABLES);
	_ES

label(MOVECURSOR)
/* move a 16x16 block from (current X,YFROM) to (current X, YTO)
 *	destroys YFROM, saves/restores YTO at TEMP
 */
	_NS REGREG(MOVE,_YTO,_TMP); LOADDI(UCOUNT); DOPUSH(15);_ES
	_NS REGREG(MOVE,_YFROM,_YFROM); BPC(LOADYS);_ES
	_NS REGREG(INCR,_YFROM,_YFROM); BPC(SAVEWORD);_ES
	_NS REGREG(MOVE,_YTO,_YTO); BPC(LOADYS);_ES
	_NS ALUOP(ONES); FTODO; BPC(DRAWWORD); _ES	/* all-1's mask */
	_NS REGREG(INCR,_YTO,_YTO); SEQ(LOUP);_ES
	_NS REGREG(MOVE,_TMP,_YTO); SEQ(RETN);_ES


/* entry for cursor routines from DISPATCH
 *	load cursor's color and config
 *	collect new X coord from Multibus reg
 *	interrupt host for Y coord; collect it
 *	undraw old cursor, then draw at new coords.
 */

label(SIDE_CURSOR)
	_NS LOADMAR(_CURSAVE); CONST(_CURSAVE); _ES

	_NS RAM(RAMWR,9,INC); DOJSUB(STM10UP); _ES

	_NS LOADREG(_CURSX,ALL16,NOMORE); COND(IFFALSE); SEQ(VECT); _ES
		/* get X coord out of Multibus reg using VECT microinstruc. */

	_NS LOADIMM(_TMP,_INTCURSOR); LDOUT;
	 INTERRUPTHOST; CONST(_INTCURSOR) _ES	/* signal that Y needed */

	_NS REGREG(ONES,0,_SIDE_DOOR); DOJSUB(LD_CURCOLOR); _ES
			/* set side-door flag; go set up cursor's config */

	_NS LOADREG(_CURSY,ALL16,NOMORE); COND(IFFALSE); SEQ(VECT); _ES
					/* get Y coord as X above	*/

label(CUR_FLAGTEST)
	_NS REGHOLD; COND(IFNFLAG); DOJUMP(SIDE_GO); _ES
		/* wait for host flag to clear to complete handshake */

	_NS REGHOLD; DOJUMP(CUR_FLAGTEST); _ES

label(SIDE_GO)
	_NS REGHOLD; BPC(NOOP); DOJSUB(UNDRAW_CUR_SUB); _ES

label(DRAW_ONLY)
	_NS REGHOLD; BPC(NOOP); DOJSUB(DRAW_CUR_SUB); _ES

label(CURSOR_EXIT)
label(CFIG_RESTORE)		/* restore normal color, config	*/
	_NS LOADMAR(_ZBUFFER); CONST(_ZBUFFER);_ES/* address normal setup */

	_NS REGHOLD; DOJSUB(RESTORE_COLORWE); _ES  /* go send stuff */

	_NS REGREG(MOVE,_SIDE_DOOR,_SIDE_DOOR); _ES /* examine sidedoor flag*/

	_NS REGHOLD; COND(IFNEG); DOJUMP(CURCMD); _ES
			/* side-door flag set; skip get of new command */

	_NS REGHOLD; GEGET; _ES

label(CURCMD)
	_NS LOADMAR(_CURSAVE); CONST(_CURSAVE); _ES

	_NS RAM(RAMRD,9,INC); DOJSUB(LM10UP); _ES

	_NS REGHOLD; DOJUMP(DISPATCH); _ES


label(TESTCURDRAWN)	/* subroutine to force cursor off if drawn	*/
			/* enter having read _CURSORDRAWN		*/

	_NS REGHOLD; COND(IFZ); SEQ(RETN); _ES   /* do nothing if not drawn */

	_NS LOADMAR(_CURSAVE); CONST(_CURSAVE); _ES

	_NS RAM(RAMWR,9,INC); DOJSUB(STM10UP); _ES	/* save regs */

	_NS REGREG(ONES,0,_SIDE_DOOR); DOJSUB(LD_CURCOLOR); _ES
					/* set up to undraw old cursor */

	_NS REGHOLD; BPC(NOOP); DOJSUB(UNDRAW_CUR_SUB); _ES	/* undraw */

	_NS LOADMAR(_ZBUFFER); CONST(_ZBUFFER); _ES

	_NS REGHOLD; DOJSUB(RESTORE_COLORWE); _ES/* restore current config */

	_NS LOADMAR(_CURSAVE); CONST(_CURSAVE); _ES

	_NS RAM(RAMRD,9,INC); DOJSUB(LM10UP); _ES	/* restore regs */

	_NS REGHOLD; SEQ(RETN); _ES
}
