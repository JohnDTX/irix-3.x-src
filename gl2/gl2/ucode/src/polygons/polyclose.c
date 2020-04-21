
/* polyclose.c
 *
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"
#include "polydefs.h"

/* 
	registers used
		_LASTPT
		_XTHIS
		_YTHIS
		_CURSHADE
		_ZTHIS
		_TEMP
		_BIASED_DONE

	constants used
		none

	scratch locations used
		_POLYGONSTYLE, assumes Z_BUFFER flag to be in the next
				scratch location

	routines called
		ADD_SENTINELS
	
	exits to
		GET_FIRST_SIDES_F2D
	    or
		GET_FIRST_SIDES_F_Z
	    or
		GET_FIRST_SIDES_S2D
	    or
		GET_FIRST_SIDES_S_Z
*/

polyclose()
{
newfile("polyclose.c");

label(POLY_CLOSE)
    _NS /* make sure the MAR is loaded correctly */
	REGREG(RONLYOP, P0, _LASTPT, _LASTPT);
	DOTOMAR(LOAD);
    _ES

    _NS /* now load the last vertex into scratch ram */
	RAM(RAMWR,  _XTHIS, INC);
    _ES

    _NS /* now load the last vertex into scratch ram */
	RAM(RAMWR,  _YTHIS, INC);
    _ES

    _NS /* write the shade value */
	RAM(RAMWR,  _PREVSHADE, INC);
    _ES

    _NS /* write the z value dummy or not, also add the sentenals */
	RAM(RAMWR,  _ZTHIS, INC);
	SEQ(JSUB);
	NEXT(ADD_SENTINELS);
    _ES

    _NS /* load the MAR with the address of the polystyle flag */
	LOADMAR(_POLYGONSTYLE);
	CONST(_POLYGONSTYLE);
    _ES

    _NS /* read the value */
	RAM(RAMRD, _TEMP, INC);
    _ES

    _NS /* if the flag is zero jump to the flat shaded case */
	SEQ(JUMP);
	COND(IFZ);
	NEXT(FLAT_POLY);
	/* read the z buffering flag */
	RAM(RAMRD, _TEMP, HOLD);
    _ES

    _NS /* If we've arrived here then this is the shaded case, if the flag 
	is zero jump to the non z buffered case */
	SEQ(JUMP);
	COND(IFZ);
	/* clear the done flag */
	REGREG(FLOWOP, P0, 0,  _BIASED_DONE);
	NEXT(GET_S2D);
    _ES

    _NS /* if we're here I have to rearrange z and shade values,
    	addsent' incremented _LASTPT by one already */
	REGREG(RONLYOP, P1, _LASTPT, _LASTPT);
	DOTOMAR(LOAD);
    _ES

    _NS /* get the SHADE value */
	RAM(RAMRD, _PREVSHADE, HOLD);
    _ES

    _NS /* write the Z value */
	RAM(RAMWR, _ZTHIS, INC);
    _ES

    _NS /* write the shade */
	RAM(RAMWR, _PREVSHADE, INC);
    _ES

    _NS /* load one of the corners with the value of the BOTTOM most
	point */
	REGREG(RONLYOP, P0, _BOTTOMPNT, _CORNER1);
    _ES

    _NS /* pick out the other corner in case this is a single pixel wide
	polygon, this test will  work in all cases but a horizontal line */
	REGCOMP(NQ, _BOTTOMPNT, _TOPPNT);
    _ES

    _NS /* if they aren't equal load the 2nd corner with _TOPPNT */
	REGREG(RONLYOP, P0, _TOPPNT, _CORNER2);
	COND(IFNQ);
	SEQ(JUMP);
	NEXT(GET_S_Z);
    _ES

    _NS /* if we're here then this is a horizontal line */
	REGCOMP(NQ, _LEFTPNT, _BOTTOMPNT);
    _ES

    _NS /* if they aren't equal load the 2nd corner with _LEFTPNT */
	REGREG(RONLYOP, P0, _LEFTPNT, _CORNER2);
	COND(IFNQ);
	SEQ(JUMP);
	NEXT(GET_S_Z);
    _ES

    _NS /* otherwise load the second  corner with RIGHTPNT */
	REGREG(RONLYOP, P0, _RIGHTPNT, _CORNER2);
    _ES

label(GET_S_Z);
    _NS /* if we're here the this is shaded z-buffered */
	SEQ(JUMP);
	REGREG(FLOWOP, P0, 0,  _BIASED_DONE);
	NEXT(GET_FIRST_SIDES_S_Z);
    _ES

label(FLAT_POLY)
    _NS /* If we've arrived here then this is the flat shaded case, if the flag 
	is zero jump to the non z buffered case */
	SEQ(JUMP);
	COND(IFZ);
	NEXT(GET_F2D);
	REGREG(FLOWOP, P0, _BIASED_DONE, _BIASED_DONE);
    _ES

    _NS /* if we're here I have to rearrange z and shade values,
    	addsent' incremented _LASTPT by one already */
	REGREG(RONLYOP, P1, _LASTPT, _LASTPT);
	DOTOMAR(LOAD);
    _ES

    _NS /* write the Z value */
	RAM(RAMWR, _ZTHIS, INC);
    _ES

    _NS /* write the shade */
	RAM(RAMWR, _PREVSHADE, INC);
    _ES

    _NS /* load one of the corners with the value of the BOTTOM most
	point */
	REGREG(RONLYOP, P0, _BOTTOMPNT, _CORNER1);
    _ES

    _NS /* pick out the other corner in case this is a single pixel wide
	polygon, this test will  work in all cases but a horizontal line */
	REGCOMP(NQ, _BOTTOMPNT, _TOPPNT);
    _ES

    _NS /* if they aren't equal load the 2nd corner with _TOPPNT */
	REGREG(RONLYOP, P0, _TOPPNT, _CORNER2);
	COND(IFNQ);
	SEQ(JUMP);
	NEXT(GET_FIRST_SIDES_S2D);
    _ES

    _NS /* if we're here then this is a horizontal line */
	REGCOMP(NQ, _LEFTPNT, _BOTTOMPNT);
    _ES

    _NS /* if they aren't equal load the 2nd corner with _LEFTPNT */
	REGREG(RONLYOP, P0, _LEFTPNT, _CORNER2);
	COND(IFNQ);
	SEQ(JUMP);
	NEXT(GET_FIRST_SIDES_S2D);
    _ES

    _NS /* otherwise load the second  corner with RIGHTPNT */
	REGREG(RONLYOP, P0, _RIGHTPNT, _CORNER2);
	SEQ(JUMP);
	NEXT(GET_FIRST_SIDES_S2D);
    _ES

label(GET_F2D)
    _NS /* load one of the corners with the value of the BOTTOM most
	point */
	REGREG(RONLYOP, P0, _BOTTOMPNT, _CORNER1);
    _ES

    _NS /* pick out the other corner in case this is a single pixel wide
	polygon, this test will  work in all cases but a horizontal line */
	REGCOMP(NQ, _BOTTOMPNT, _TOPPNT);
    _ES

    _NS /* if they aren't equal load the 2nd corner with _TOPPNT */
	REGREG(RONLYOP, P0, _TOPPNT, _CORNER2);
	COND(IFNQ);
	SEQ(JUMP);
	NEXT(GET_FIRST_SIDES_F2D);
    _ES

    _NS /* if we're here then this is a horizontal line */
	REGCOMP(NQ, _LEFTPNT, _BOTTOMPNT);
    _ES

    _NS /* if they aren't equal load the 2nd corner with _LEFTPNT */
	REGREG(RONLYOP, P0, _LEFTPNT, _CORNER2);
	COND(IFNQ);
	SEQ(JUMP);
	NEXT(GET_FIRST_SIDES_F2D);
    _ES

    _NS /* otherwise load the second  corner with RIGHTPNT */
	REGREG(RONLYOP, P0, _RIGHTPNT, _CORNER2);
	SEQ(JUMP);
	NEXT(GET_FIRST_SIDES_F2D);
    _ES

label(GET_S2D)
    _NS /* load one of the corners with the value of the BOTTOM most
	point */
	REGREG(RONLYOP, P0, _BOTTOMPNT, _CORNER1);
    _ES

    _NS /* pick out the other corner in case this is a single pixel wide
	polygon, this test will  work in all cases but a horizontal line */
	REGCOMP(NQ, _BOTTOMPNT, _TOPPNT);
    _ES

    _NS /* if they aren't equal load the 2nd corner with _TOPPNT */
	REGREG(RONLYOP, P0, _TOPPNT, _CORNER2);
	COND(IFNQ);
	SEQ(JUMP);
	NEXT(GET_FIRST_SIDES_S2D);
    _ES

    _NS /* if we're here then this is a horizontal line */
	REGCOMP(NQ, _LEFTPNT, _BOTTOMPNT);
    _ES

    _NS /* if they aren't equal load the 2nd corner with _LEFTPNT */
	REGREG(RONLYOP, P0, _LEFTPNT, _CORNER2);
	COND(IFNQ);
	SEQ(JUMP);
	NEXT(GET_FIRST_SIDES_S2D);
    _ES

    _NS /* otherwise load the second  corner with RIGHTPNT */
	REGREG(RONLYOP, P0, _RIGHTPNT, _CORNER2);
	SEQ(JUMP);
	NEXT(GET_FIRST_SIDES_S2D);
    _ES

#ifdef NOZPOLY
label(ZFILL_TRAP)
label(GET_FIRST_SIDES_S_Z)
	_NS REGHOLD; DOJUMP(UNIMPL_TRAP); _ES
#endif

}
