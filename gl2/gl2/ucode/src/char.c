/* char.c -- character-related commands for FBC
 *
 *	contains:
 *		CHARPOSN_ABS (x, y)	viewport format
 *		CHARPOSN_REL (x, y)	"  "
 *		LOAD_MASKS   (fontadr, mask, mask ... mask)
 *
 *		uses stm.mic routines, OUTSEND subr
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"

chars()
{
newfile("char.c");

label(CHARPOSN_ABS)
/*
/*	load character position absolute
/*	command word is followed by:
/*		PassThru	(only if Point command "faked")
/*		Point		(GE command code)
/*		X		(32 bits)
/*		Y		(32 bits)
/*	If the Point command does not appear within 2 words, the PASSCHARS
/*	flag is cleared, inhibiting any DRAW_CHARS commands until the next
/*	valid CHARPOSN/Point combination comes through.
/*	If the Point command is clipped by the GE's, the sequence may be:
/*		PassThru	( real, for next command )
/*		<Cmd>		( real passed-thru command )
*/
#define _ONES	13
#define _CMD	12

	_NS REGREG(ZERO,0,_PASSN); DOJSUB(CHAR_POSN);_ES
		/* prepare to record passthru N in case real Point cmd clipped
		 * and good passthru cmd follows;
		 * go get command following POSN cmd. and test for Point
		 * only return if we have to write new X,Y
		 */

label(CH_POSN_END)
	_NS RAM(RAMWR,_X2,INC);_ES	/* save X coord	*/

	_NS RAM(RAMWR,_Y2,HOLD); GEGET; DOJUMP(DISPATCH);_ES
						/* save Y & exit	*/



label(CHARPOSN_REL)	/* set character position relative	*/
	_NS REGREG(ZERO,0,_PASSN); DOJSUB(CHAR_POSN);_ES

	_NS REGRAM(ADD,_X2,_X2,INC); _ES  /* add ram value to new value	*/

	_NS REGRAM(ADD,_Y2,_Y2,DEC); DOJUMP(CH_POSN_END);_ES

/*================================================================*/

label(CHAR_POSN)
		/* body of routine
		 * returns pointing to CHARPOSN if valid point next
		 * First detect point command.  If not there invalidate & exit
		 *			   seq is	1a  nextcmd
		 * Otherwise validate. Then test hitmode.
		 * If hitmode, invalidate, record hit, & exit
		 *			 - seq is	1a  12  nextcmd
		 * Else valid string -     seq is	1a  12  x  y  nextcmd
		 */
	_NS LOADREG(_CMD,ALL16,NOMORE);_ES	/* get word after CHARPOS */

	_NS LOADREG(_PASSN,LO8,NOMORE);_ES
					/* in case good passthru cmd input  */

	_NS IMMREG(ANDRS,_CMDMASK,_CMD); CONST(_CMDMASK);_ES
						/* just command field */

	_NS LOADMAR(_PASSCHARS); CONST(_PASSCHARS); _ES
						/* prepare to set flag */

	_NS IMMCOMP(EQ,_POINTCODE,_CMD); CONST(_POINTCODE);_ES

	_NS REGREG(ONES,0,_ONES); COND(IFNQ); DOJUMP(REJECT_CHARS);_ES
		/* prepare ones to set passchars; if not Point, go clear it */

label(VALIDATE_CHARPOSN)
	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_ONES,RAMWR);
	 ALUOP(MOVE); FTODO; LDMAR; CONST(_ALTVECMODE); _ES
		/* while setting valid flag, address mode flags */

	_NS IMMRAM(ANDRS,_ALTHITBIT,_TEMP,HOLD); CONST(_ALTHITBIT); _ES
						/* examine hitmode flag */

	_NS RAM(RAMRD,_TEMP,HOLD); COND(IFNZ); DOJUMP(HITPOSN); _ES
			/* read flags for later; if hit, take special exit */

	_NS REGHOLD; GEGET; DOJSUB(GET_X2Y2);_ES
				/* non-hit - go get coordinates */

	_NS IMMREG(ANDRS,0,_TEMP); CONST(_ALTZBUBIT+_ALTDEPBIT); _ES
					/* look at Zbuf or Depth mode */

	_NS REGREG(ZERO,0,_ZERO); COND(IFZ); DOJUMP(RETN_FROM_Z); _ES
					/* if neither set, just return */

	_NS IMMREG(ANDRS,0,_TEMP); CONST(_ALTDEPBIT); GEGET; _ES	 
					/* look at Depth mode */

	_NS REGREG(ZERO,0,_ZERO); COND(IFNZ); DOJUMP(DEPTH_CHARPOSN); _ES
					/* go if depthcue mode */

	_NS REGHOLD; GEGET; DOJUMP(RETN_FROM_Z);_ES
				/* zbuffer, discard z and return */

label(DEPTH_CHARPOSN)
	_NS LOADMAR(_DEPTH_PARAMS); CONST(_DEPTH_PARAMS); _ES
				/* else, assume depthcue and do a*z+b */

	_NS SETSOP(NONQOP,0,RAMRD); ALUOP(SONLYOP,P0);
	 YQ(FF,LDQ,REGWRD); INCMAR; DOJSUB(GET_Z_INT); _ES
				/* "a" to Q reg; go get Z value */
				/* and calc I2 = a * Z + b */

	_NS REGREG(MOVE,_I2,_I2); BPC(SETCOLORAB); _ES
				/* set as color of characters */

	_NS LOADMAR(_COLOR); CONST(_COLOR); _ES

	_NS RAM(RAMWR,_I2,INC);_ES	/* save color	*/

	_NS RAM(RAMWR,_I2,HOLD); _ES	 /* in both places */

label(RETN_FROM_Z)
	_NS LOADMAR(_CHARPOSN); CONST(_CHARPOSN); _ES
				/* return pointing to character position */

	_NS SEQ(RETN); _ES

label(REJECT_CHARS)
	_NS RAM(RAMWR,_ZERO,HOLD); SEQ(TEST); _ES	/* pop stack */
	     /* magic -- pop stack and branch to dispatch; clear passchars */
	     /* 2910 counter must be guaranteed 0 */

	_NS REGHOLD; DOJUMP(DISPATCH); _ES


label(HITPOSN)
	_NS LOADMAR(_PASSCHARS); CONST(_PASSCHARS); _ES

	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_ZERO,RAMWR);
	 ALUOP(MOVE); FTODO; LDMAR; CONST(_HITBITS); _ES
		/* surprise! string is really invalid.  point to hit bits */

	_NS REGREG(INCR,_ZERO,_ZERO); _ES	/* make a one */

	_NS RAM(RAMWR,_ZERO,HOLD); GEGET; SEQ(TEST); _ES

	_NS REGHOLD; DOJUMP(DISPATCH); _ES
		/* record the hit and exit, skipping point command */

/*================================================================*/


label(LOAD_MASKS)
/*
/*	load masks into Update Control's font ram
/*	INPUT:
/*		baseadr, mask, mask, ...
/*	note that masks are not packed, for host speed.
/*	uses Passthru N count to count masks.
*/
#define _ADR	4

	_NS LOADMAR(_CHARBASE); CONST(_CHARBASE); _ES

	_NS LOADDI(INRJUST); SETROP(0,ALL16); SETSOP(NONQOP,_ADR,RAMRD);
	 ALUOP(ADD); YQ(FF,OLDQ,REGWRE); BPC(LOADFA); _ES
			/* font addr is relative to stored base adr */

	_NS IMMREG(SUBSRC,2,_PASSN); GEGET; CONST(2); BPC(SETADDRS); _ES
					/* force loading of adr reg */
					/* reduce N by (wds read -1) */

label(MASKLOOP)
	 _NS REGHOLD; COND(IFNEG); DOJUMP(DISPATCH);_ES
						/* quit if N goes neg*/

	_NS LOADREG(_TEMP,ALL16,NOMORE); BPC(WRITEFONT);_ES
					   /* xfer byte to fontram */

	_NS REGREG(INCR,_ADR,_ADR); GEGET; BPC(LOADFA);_ES
						/* next font adr */

	_NS REGREG(SUBSR,_ZERO,_PASSN); DOJUMP(MASKLOOP);_ES
						  /*decrement N & loop  */

}
