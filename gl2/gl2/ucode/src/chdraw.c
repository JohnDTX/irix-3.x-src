/* chdraw.c			<< GL2  >>
 *
 *	multiwindow support implemented
 *	uses LMnUP from stm.mic
 *
 *	hitmode change: if any char in this string hit, "1" reported
 * 	as _HITCHARNO (and in _HITBITS)
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"

/*  register definitions 	*/

#define _WK	0	/* 0 and 1 are graphics position */
#define _WK2	1
#define _WK3    2	/*						*/
#define _WK4    3	/*						*/
#define _TRUEX  3	/* POSNX + x offset				*/
#define _TRUEY  2	/* POSNY + y offset				*/
#define _NCT    4	/* copied from PASSN register			*/
#define _NEGS   5	/* all ones for sign smear			*/
#define _ZRO    6	/* local 0					*/
/*		7	   _TEMP					*/
#define _HITFLG 8	/* hit mode flag				*/
#define _HITCHR 9	/* whether a char hit				*/
#define _WINDL  10	/* viewport left edge		\		*/
#define _WINDB  11	/* 	bottom			|		*/
#define _WINDR  12	/* 	right			|		*/
#define _WINDT  13	/* 	top			 > unsaved from ram */
#define _POSNX  14	/* char. posn.			|		*/
#define _POSNY  15	/* 				/		*/

chdraw()
{
newfile("chdraw.c");

label(DRAW_CHARS)
/*	first tests PASSCHARS flag to see if valid position
/*	then unsaves viewport, charposn
/*	then tests HITMODE flag
/*	saves graphics posn cursor
*/
	_NS LOADMAR(_PASSCHARS); CONST(_PASSCHARS);_ES

	_NS RAM(RAMRD,_TEMP,DEC);_ES
				/* read flag, address HITMODE flag	*/

 	_NS RAM(RAMRD,_HITFLG,HOLD); COND(IFZ); DOJUMP(CHAR_REJECT);_ES
			/* read hitmode; branch on passchars = 0	*/

	_NS REGREG(MOVE,_PASSN,_NCT); _ES  /* move N count to safe place */

	_NS REGREG(SUBSR,_ZRO,_NCT); _ES	/*  make NCT-1 for testing */

	_NS REGREG(ZERO,0,_HITCHR); _ES		/*  clear "char was hit" */

	_NS LOADMAR(_CHARVIEW); GEGET; CONST(_CHARVIEW);_ES
					/* point to viewport coords  */

	_NS RAM(RAMRD,_WINDL,INC); _ES	/* fetch viewport for sgl-vp case */
	_NS RAM(RAMRD,_WINDB,INC); _ES
	_NS RAM(RAMRD,_WINDR,INC); _ES
	_NS RAM(RAMRD,_WINDT,INC); _ES	/* point to save area */

	_NS RAM(RAMWR,0,INC); _ES	/* save graphics position	*/
	_NS RAM(RAMWR,1,INC); _ES
	_NS RAM(RAMWR,2,INC); DOJSUB(LM14UP); _ES
				/* and intensity; pt to CHARPOSN; go unsave */

label(CHDRAW_LOOP)
/*  outline:
/*	input & unpack parameters:
/*		INPUT:  baseadr		(output as font ram address)
/*			width | height	(unsigned; 0 WIDTH means no draw )
/*			X ofs | Y ofs	(2's comp ; sign smeared by ucode)
/*			X spacing	(16 bit, 2's comp)
/*
/*	modify current char position by offsets
/*	perform gross clipping
/*	if inside viewport, load bounding box & draw
/*	get next input, test for N count done
*/
	_NS LOADMAR(_CHARBASE); CONST(_CHARBASE); _ES
						/* this is SAVE1-1 */

	_NS LOADDI(INRJUST); SETROP(0,ALL16); SETSOP(NONQOP,0,RAMRD);
	 ALUOP(ADD); FTODO; BPC(LOADFA); _ES
			/* send base adr = stored base + input offset */

	_NS REGHOLD; INCMAR; GEGET; _ES
			/* area for unpacking params; input width|height */

	_NS LOADDI(INRJUST); SETROP(_ZRO,LO8); SETSOP(NONQOP,_WK3,RAMNOP);
	 ALUOP(RONLYOP,P0); YQ(FF,OLDQ,REGWRE);_ES
					/* 0 | height to WK3	     */

	_NS REGREG(SUBSR,_ZRO,_WK3);		/* subtract one for BPC	*/
	 COND(IFZ); DOJUMP(ZEROHT); _ES		/* if zero, skip drawing */

	_NS LOADDI(INLJUST); SETROP(_ZRO,LO8); SETSOP(NONQOP,_WK3,RAMWR);
	 ALUOP(RONLYOP,P0); YQ(FF,OLDQ,REGWRE); INCMAR;;_ES
			/* while writing to scratch, 0 | width to WK3	*/

	_NS REGREG(SUBSR,_ZRO,_WK3); _ES	/* subtract one for BPC	*/

	_NS RAM(RAMWR,_WK3,INC); GEGET; COND(IFNEG); DOJUMP(SPACEONLY);_ES
			/* write width to scratch; if <=0, skip drawing	*/

	_NS REGREG(ONES,0,_NEGS);_ES

/*  unpack offsets into 2 words
 *  leaves 0|Yoffset in WK3, 0|Xoffset in WK4
 */
	_NS LOADDI(INRJUST); SETROP(_ZRO,LO8); SETSOP(NONQOP,_WK4,RAMNOP);
	 ALUOP(RONLYOP,P0); YQ(FF,OLDQ,REGWRE);_ES
						/* 0 | Yoffset to WK4	*/

	_NS LOADDI(INLJUST); SETROP(0,ALL16); ALUOP(RONLYOP,P0); FTODO;;_ES
						/* test msb of Yoffset	*/

	_NS REGREG(MOVE,_WK4,_WK3); COND(IFNEG); DOJSUB(SSMEAR_LOWER);_ES
			   /* save (pos) results; if neg, go smear & resave */

	_NS LOADDI(INLJUST); SETROP(_ZRO,LO8); SETSOP(NONQOP,_WK4,RAMWR);
	 ALUOP(RONLYOP,P0); YQ(FF,OLDQ,REGWRE); INCMAR;;_ES
			/* while writing to scratch, 0 | Xoffset to WK4	    */

	_NS LOADDI(INRJUST); SETROP(0,ALL16); ALUOP(RONLYOP,P0); FTODO;;_ES
						/* test msb of Xoffset	*/

	_NS REGREG(ADD,_POSNY,_TRUEY);BPC(LOADYS); /* calc true Y=POSN+OFS  */
	 COND(IFNEG); DOJSUB(SSMEAR_UPPER);_ES  /* if 8 bits neg, go smear  */

	_NS SETROP(0,ALL16); SETSOP(NONQOP,_WK4,RAMWR);
	 ALUOP(MOVE); FTODO; GEGET;		/* save Xoffset in ram	   */
	 LOADDI(UCONST); LDMAR; CONST(_MULTIVIEW); _ES	/* look at vp flag */

/* now SAVE1 area contains:
 *	height
 *	width
 *	Y ofs	also in _WK3 (became _TRUEY)
 *	X ofs   also in _WK4 (became _TRUEX)
 *
 * test multiviewport flag here for (optimum?) speed
 */
	_NS SETROP(0,NONE); SETSOP(NONQOP,0,RAMRD);
	 ALUOP(SONLYOP,P0); FTODO; _ES		/* non-writing test */

	_NS REGREG(ADD,_POSNX,_TRUEX); BPC(LOADXS);
	 COND(IFZ); DOJUMP(1VP_CHARS); _ES
			/* add position+offset for true X; jump on vp mode */

/* multi-viewport characters
 * we have to reload start/end regs for each view
 */
	_NS REGHOLD; DOJSUB(RESET_MASKS); _ES	/* initialize viewport list */

label(CHLOOP_VP)
	_NS LOADMAR(_CHARVIEW); CONST(_CHARVIEW); _ES	/* copy vp to regs */

	_NS RAM(RAMRD,_WINDL,INC); _ES
	_NS RAM(RAMRD,_WINDB,INC); _ES
	_NS RAM(RAMRD,_WINDR,INC); _ES
	_NS RAM(RAMRD,_WINDT,HOLD); _ES

	_NS LOADMAR(_SAVE1); CONST(_SAVE1);_ES		/* point to height  */

	_NS SETROP(_TRUEX,NONE); SETSOP(NONQOP,_WINDR,RAMNOP);
	 ALUOP(SUBRSOP,P0); FTODO; INCMAR; DOJSUB(GROSS_CLIP);_ES
			/* point to width; go clip & load END coords	*/

	_NS REGREG(MOVE,_HITFLG,_HITFLG); COND(IFNEG); DOJUMP(SKIPIT);_ES
				/* read hit flag; if char clipped, branch  */

	_NS REGREG(MOVE,_TRUEX,_TRUEX); BPC(LOADXS);	/* reload XS */
	 COND(IFNZ); DOJUMP(CHAR_HIT); _ES
					/* if in hit mode, go record hit */

	_NS REGREG(MOVE,_TRUEY,_TRUEY); BPC(LOADYS); _ES    /* reload YS */

#ifdef UC3
	_NS REGHOLD; BPC(CHDRAW); DOJUMP(SKIPIT);_ES	/* draw char!!	    */
#endif
#ifdef UC4
	_NS REGHOLD; BPC(DRAWCHAR); DOJUMP(SKIPIT);_ES	/* draw char!!	    */
#endif

label(CHAR_HIT)
	_NS REGREG(INCR,_ZRO,_HITCHR); _ES    /* record hit locally  */

label(SKIPIT)
	_NS REGHOLD; DOJSUB(NEW_MASK); _ES	/* get next mask, if any */

	_NS REGHOLD; COND(IFNZ); DOJUMP(CHLOOP_VP); _ES    /* if more, loop */

label(CHDONE)
	_NS LOADREG(_WK,ALL16,NOMORE);_ES	/* read X spacing	*/

	_NS IMMREG(SUBSRC,4,_NCT); GEGET; CONST(4);_ES
					/* decr. Nct, get next    */

	_NS REGREG(ADD,_WK,_POSNX); COND(IFNNEG); DOJUMP(CHDRAW_LOOP);_ES
		/* update char posn cursor with X spacing;
		/* if not done, loop  */

label(CHARS_DONE)
	_NS LOADMAR(_HITCHARCT); CONST(_HITCHARCT);_ES
				/* whether in hit mode or not.... */

	_NS RAM(RAMWR,_HITCHR,DEC);_ES
			/* record whether char hit; address HITBITS  */

	_NS RAM(RAMWR,_HITCHR,HOLD); _ES
				/* set lsb of word; exit	    */

label(CHAR_EXIT)
	_NS LOADMAR(_CHARVSAVE); CONST(_CHARVSAVE);_ES

	_NS RAM(RAMRD,0,INC); _ES	/* restore graphics position */

	_NS RAM(RAMRD,1,INC); _ES	/* then point to CHARPOSN	*/

	_NS RAM(RAMRD,2,INC); _ES

	_NS RAM(RAMWR,_POSNX,HOLD); DOJUMP(DISPATCH);_ES
						/* update char X and exit  */

label(ZEROHT)
	_NS REGHOLD; GEGET; _ES		/* input & discard offsets	*/

label(SPACEONLY)
	_NS REGHOLD; GEGET; DOJUMP(CHDONE); _ES		/* input X spacing */

label(1VP_CHARS)	/* inner loop for single viewport mode */
	_NS LOADMAR(_SAVE1); CONST(_SAVE1); _ES

	_NS SETROP(_TRUEX,NONE); SETSOP(NONQOP,_WINDR,RAMNOP);
	 ALUOP(SUBRSOP,P0); FTODO; INCMAR; DOJSUB(GROSS_CLIP);_ES
			/* point to width; go clip & load END coords	*/

	_NS REGREG(MOVE,_HITFLG,_HITFLG); COND(IFNEG); DOJUMP(CHDONE);_ES
				/* read hit flag; if char clipped, branch  */

	_NS REGREG(MOVE,_TRUEX,_TRUEX); BPC(LOADXS);	/* reload XS */
	 COND(IFNZ); DOJUMP(1VP_CHHIT); _ES
					/* if in hit mode, go record hit */

#ifdef UC3
	_NS REGHOLD; BPC(CHDRAW); DOJUMP(CHDONE);_ES	/* draw char!!	    */
#endif
#ifdef UC4
	_NS REGHOLD; BPC(DRAWCHAR); DOJUMP(CHDONE);_ES	/* draw char!!	    */
#endif

label(1VP_CHHIT)
	_NS REGREG(INCR,_ZRO,_HITCHR); DOJUMP(CHDONE); _ES

/*================================================================*/

/* if the preceding CHAR_POSN command had its coordinates clipped, just
/* eat up inputs until N count exhausted.
*/
label(CHAR_REJECT)
	_NS REGREG(SUBSR,_ZRO,_PASSN); GEGET; _ES    /* set up N count	*/

label(REJ_LOOP)
	_NS REGHOLD; COND(IFNEG); DOJUMP(DISPATCH);_ES

	_NS LOADDI(UCOUNT); REGHOLD; DOPUSH(3);_ES
						/* skip next 4 inputs	*/

	_NS REGREG(SUBSR,_ZRO,_PASSN); GEGET; _ES
					/* subtract parameter count  */
	_NS REGHOLD; SEQ(LOUP);_ES

	_NS REGREG(MOVE,_PASSN,_PASSN); DOJUMP(REJ_LOOP);_ES


label(SSMEAR_LOWER)
	_NS LOADDI(INRJUST); SETROP(_NEGS,LO8); SETSOP(NONQOP,_WK4,RAMNOP);
	 ALUOP(RONLYOP,P0); YQ(FF,OLDQ,REGWRE);_ES
				/* 11111111 | Yoffset to WK4	     */

	_NS REGREG(MOVE,_WK4,_WK3); SEQ(RETN);_ES   /* save in WK3	*/

label(SSMEAR_UPPER)
	_NS LOADDI(INLJUST); SETROP(_NEGS,LO8); SETSOP(NONQOP,_WK4,RAMNOP);
	 ALUOP(RONLYOP,P0); YQ(FF,OLDQ,REGWRE); SEQ(RETN);_ES
				/* 11111111 | Xoffset to WK4	    */


label(GROSS_CLIP)

/*	tests for intersection of character bounds and viewport bounds.
/*	If any part intersects, character passes.
/*	loads X+height to XEnd, Y+height to Yend
/*	Uses scratch table of params:
/*		height
/*		width
/*		???
/*		???
/*	Uses WK and WK2; Enter pointing to width, exits pointing to width+1
*/
	_NS REGRAM(ADD,_TRUEX,_WK,HOLD); BPC(LOADXE);
	 COND(IFNNEG); DOJUMP(GROSS_REJ);_ES
				/* if X>WINDR, go reject; calc X+W	*/

	_NS SETROP(_WK,NONE); SETSOP(NONQOP,_WINDL,RAMNOP);
	 ALUOP(SUBRSOP,P1); FTODO; DECMAR;;_ES	/* compare X+W >= WINDL	*/

	_NS SETROP(_TRUEY,NONE); SETSOP(NONQOP,_WINDT,RAMNOP);
	 ALUOP(SUBRSOP,P0); FTODO; COND(IFNEG); DOJUMP(G_REJ1);_ES
			/* if X+W fails, reject; compare Y <= WINDT	*/

label(GRCLIP_Y)		/* enter here pointing to height		*/
	_NS REGRAM(ADD,_TRUEY,_WK2,INC); COND(IFNNEG); DOJUMP(GROSS_REJ);_ES
				/* if Y fails, reject; calc Y+H		*/

	_NS SETROP(_WK2,NONE); SETSOP(NONQOP,_WINDB,RAMNOP);
	 ALUOP(SUBRSOP,P1); FTODO;;_ES		/* compare Y+H >= WINDB	*/

	_NS REGREG(MOVE,_WK2,_WK2); BPC(LOADYE);
	 COND(IFNEG); DOJUMP(GROSS_REJ);_ES
					/* send Y+H to YEND reg		*/

label(GROSS_PASS)
	_NS ALUOP(FLOWOP,P0); YQ(FF,OLDQ,REGWRD); INCMAR; SEQ(RETN);_ES
						/* pass: return 0	*/

label(G_REJ1)
	_NS REGHOLD; INCMAR;;_ES	/* just inc mar & continue	*/

label(GROSS_REJ)  _NS ALUOP(FHIGHOP,P0); YQ(FF,OLDQ,REGWRD); INCMAR; SEQ(RETN);_ES
						/* fail: return -1	*/
}
