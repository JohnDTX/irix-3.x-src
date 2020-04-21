/* block.mic  -- block fill	<< GL2 >>
 *
 *	BLOCK_FILL(xll,yll,xur,yur)
 *
 *	HITMODE test implemented
 *	BOUNDARIES IGNORED in multi-viewport mode -- always clear to vp
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"

block()
{
newfile("block.c");

label(BLOCK_FILL)
/*  uses the "fillrect" function of the UC4  for fast area fill
 *  accepts integer coordinates for Xll,Yll,Xur,Yur
 *  checks for hitmode -- if true, always hits.
 *  uses existing color/config, poly stipple
 *	SINGLE VIEWPORT
 */
#define _WK	5
#define _WK2	4

	_NS LOADREG(_X1,ALL16,MORE); _ES

	_NS LOADREG(_Y1,ALL16,MORE); _ES

	_NS LOADREG(_X2,ALL16,MORE); _ES

	_NS LOADREG(_Y2,ALL16,NOMORE); _ES

	_NS LOADMAR(_HITMODE); CONST(_HITMODE); _ES

	_NS RAM(RAMRD,_WK,HOLD); _ES

	_NS REGHOLD; COND(IFNZ); DOJUMP(RECORDHIT); _ES

	_NS LOADMAR(_POLYSTIPADR); CONST(_POLYSTIPADR);_ES

	_NS RAM(RAMRD,_WK,HOLD); BPC(LOADFA); _ES
					/* load font adr of poly stip */
	_NS LOADMAR(_MULTIVIEW); CONST(_MULTIVIEW); _ES

	_NS RAM(RAMRD,_WK,HOLD); _ES

	_NS REGHOLD; COND(IFZ); DOJUMP(SGLBLOCK); _ES

	_NS REGHOLD; DOJSUB(RESET_MASKS); _ES

label(BLOCK_LOOP)

	_NS REGHOLD; BPC(FILLRECT); DOJSUB(NEW_MASK); _ES
		/* note -- loading next mask will set draw limits! */

	_NS REGHOLD; COND(IFNZ); DOJUMP(BLOCK_LOOP); _ES

	_NS REGHOLD; GEGET; DOJUMP(DISPATCH);_ES

label(SGLBLOCK)
	_NS REGREG(MOVE,_X1,_X1); BPC(LOADXS); _ES

	_NS REGREG(MOVE,_Y1,_Y1); BPC(LOADYS); _ES

	_NS REGREG(MOVE,_X2,_X2); BPC(LOADXE); _ES

	_NS REGREG(MOVE,_Y2,_Y2); BPC(LOADYE); _ES

	_NS REGHOLD; BPC(FILLRECT); _ES		/* draw it	*/

	_NS REGHOLD; GEGET; DOJUMP(DISPATCH); _ES

label(CLEAR) /* clear everything to black */
	_NS LOADMAR(_POLYSTIPADR); CONST(_POLYSTIPADR); _ES
	_NS RAM(RAMRD, _WK2, HOLD); BPC(LOADFA); _ES
	_NS LOADMAR(_CONFIG+1);CONST(_CONFIG+1); _ES
	_NS IMMREG(RONLYOP, P0, 0xc, _WK2); CONST(0xc); BPC(LOADCONFIG); _ES
	_NS IMMREG(RONLYOP, P0, 0, _WK2); CONST(0); BPC(LOADXS); _ES
	_NS IMMREG(RONLYOP, P0, 0, _WK2); CONST(0); BPC(LOADYS); _ES
	_NS IMMREG(RONLYOP, P0, 0x3ff, _WK2); CONST(0x3ff); BPC(LOADXE); _ES
	_NS IMMREG(RONLYOP, P0, 0x2ff, _WK2); CONST(0x2ff); BPC(LOADYE); _ES
	_NS REGREG(FLOWOP, P0, _WK2, _WK2); BPC(SETCOLORAB); _ES
	_NS REGREG(FLOWOP, P0, _WK2, _WK2); BPC(SETCOLORCD); _ES
	_NS REGREG(FHIGHOP, P0, _WK2, _WK2); BPC(SETWEAB); _ES
	_NS REGREG(FHIGHOP, P0, _WK2, _WK2); BPC(SETWECD); _ES
	_NS REGHOLD; BPC(FILLRECT); _ES		/* draw it	*/
	_NS REGHOLD; BPC(NOOP); SEQ(RETN); _ES
}
