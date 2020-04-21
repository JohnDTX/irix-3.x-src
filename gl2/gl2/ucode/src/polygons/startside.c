/* startside.c
/*	start_left_side and start_right_side
/*
/*
*/
#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "polydefs.h"

/*
	registers used
	    START_LEFT_SIDE_F2D
		_MIN_MASK
		_XLEFT_HI
		_YVALUE
		_LEFT
		_LEFTCOUNT
		_BIASED_DONE
		_DEL_XLEFT_HI
	    START_RIGHT_SIDE_F2D
		_MAX_MASK
		_XRIGHT_HI
		_YVALUE
		_RIGHT
		_RIGHTCOUNT
		_BIASED_DONE
		_DEL_XRIGHT_HI

	constants used
		none

	scratch locations used
		_LEFT_INC
		_RIGHT_INC

	routines  called
	    START_LEFT_SIDE_F2D
		BUMP_LEFT_INDEX
	    START_RIGHT_SIDE_F2D
		BUMP_RIGHT_INDEX

	exits to
		caller
*/

start_side()
{
newfile("startside.c");

label(START_LEFT_SIDE_F2D)
    _NS /* load MIN_MASK and XLEFT with a new left value */
	RAM(RAMRD, _MIN_MASK, INC);
    _ES

label(START_LEFT_SIDE_P1)
    _NS
	REGREG(RONLYOP, P0, _MIN_MASK, _XLEFT_HI);
    _ES

    _NS /* get the new yvalue */
	RAM(RAMRD, _YVALUE, INC);
    _ES

label(UPDATE_LEFT_INDEX)
    _NS /* load the MAR with the address of _LEFT_INC */
	LOADMAR(_LEFT_INC);
	CONST(_LEFT_INC);
    _ES

    _NS /* move to the next index */
	SEQ(JSUB);
	NEXT(BUMP_LEFT_INDEX);
	REGRAM(ADDOP, P0, _LEFT, _LEFT, LOAD);
    _ES

    _NS /* calculate a new count, the MAR is set up by BUMP'INDEX */
	REGRAM(SUBSROP, P1, _YVALUE, _LEFTCOUNT, DEC);
    _ES

    _NS /* test _LEFTCOUNT */
	ALUOP(COMPROP, P1);
	SETROP( _LEFTCOUNT, NONE);
	SETSOP( NONQOP, _LEFTCOUNT, RAMNOP);
	FTOYANDQ(FF, OLDQ,  REGWRD);
    _ES

    _NS /* test done and exit if above results in neg */
	SEQ(JUMP);
	NEXT(EXIT_LEFT_ST_LOOP);
	COND(IFNEG);
	REGREG(RONLYOP, P0, _BIASED_DONE, _BIASED_DONE);
    _ES
       
    _NS /* exit if done is true */ 
	SEQ(JUMP);
	NEXT(EXIT_LEFT_ST_LOOP);
	COND(IFNEG);
    _ES

    _NS /* test the next left coord */
	REGRAMCOMP(LE, _MIN_MASK, INC);
    _ES

    _NS /* conditional branch to UPDATE_LEFT_INDEX */
	SEQ(JUMP);
	NEXT(UPDATE_LEFT_INDEX);
	COND(IFLE);
	REGRAMCOMP(GT, _YVALUE, DEC);
    _ES

    _NS /* conditional branch to UPDATE_LEFT_INDEX */
	SEQ(JUMP);
	NEXT(UPDATE_LEFT_INDEX);
	COND(IFGT);
    _ES

    _NS /* load MIN_MASK and XLEFT with a new left value, jump to the 
        top of this section */
	RAM(RAMRD, _MIN_MASK, INC);
        SEQ(JUMP);
        NEXT(START_LEFT_SIDE_P1);
    _ES

label(EXIT_LEFT_ST_LOOP)
    _NS /* test leftcount */
	REGREG(RONLYOP, P0, _LEFTCOUNT, _LEFTCOUNT);
    _ES

    _NS /* calculate DEL_XLEFT_HI, the MAR still holds the right addr,
	load the value into Q, and DEL_XLEFT_HI */
	ALUOP(SUBSROP, P1);
	SETROP(_XLEFT_HI, NONE);
	SETSOP(NONQOP, _DEL_XLEFT_HI, RAMRD);
	FTOYANDQ(FF, LDQ, REGWRE);
        /* do the divide here */
	SEQ(JSUB);
	COND(IFNZ);
	NEXT(COMPUTE_LEFT_SLOPE_F2D);
    _ES

    _NS
	REGREG(RONLYOP, P0, _BIASED_DONE, _BIASED_DONE);
    	SEQ(RETN); 
    _ES

/************************************************************************/
/************************************************************************/
/************************************************************************/


label(START_RIGHT_SIDE_F2D)
    _NS /* load MAX_MASK and XRIGHT with a new right value */
	RAM(RAMRD, _MAX_MASK, INC);
    _ES

label(START_RIGHT_SIDE_P1)
    _NS
	REGREG(RONLYOP, P0, _MAX_MASK, _XRIGHT_HI);
    _ES

    _NS /* get the new yvalue */
	RAM(RAMRD, _YVALUE, INC);
    _ES

label(UPDATE_RIGHT_INDEX)
    _NS /* load the MAR with the address of RIGHT_INC */
	LOADMAR(_RIGHT_INC);
	CONST(_RIGHT_INC);
    _ES

    _NS /* move to the next index */
	SEQ(JSUB);
	NEXT(BUMP_RIGHT_INDEX);
	REGRAM(ADDOP, P0, _RIGHT, _RIGHT, LOAD);
    _ES

    _NS /* calculate a new count, the MAR is set up by BUMP'INDEX */
	REGRAM(SUBSROP, P1, _YVALUE, _RIGHTCOUNT, DEC);
    _ES

    _NS /* test _RIGHTCOUNT */
	ALUOP(COMPROP, P1);
	SETROP( _RIGHTCOUNT, NONE);
	SETSOP( NONQOP, _RIGHTCOUNT, RAMNOP);
	FTOYANDQ(FF, OLDQ,  REGWRD);
    _ES

    _NS /* test done and exit if above results in neg */
	SEQ(JUMP);
	NEXT(EXIT_RIGHT_ST_LOOP);
	COND(IFNEG);
	REGREG(RONLYOP, P0, _BIASED_DONE, _BIASED_DONE);
    _ES
       
    _NS /* exit if done is true */ 
	SEQ(JUMP);
	NEXT(EXIT_RIGHT_ST_LOOP);
	COND(IFNEG);
    _ES

    _NS /* test the next right coord */
	REGRAMCOMP(GE, _MAX_MASK, INC);
    _ES

    _NS /* conditional branch to UPDATE_RIGHT_INDEX */
	SEQ(JUMP);
	NEXT(UPDATE_RIGHT_INDEX);
	COND(IFGE);
	REGRAMCOMP(GT, _YVALUE, DEC);
    _ES

    _NS /* conditional branch to UPDATE_RIGHT_INDEX */
	SEQ(JUMP);
	NEXT(UPDATE_RIGHT_INDEX);
	COND(IFGT);
    _ES

    _NS /* load MAX_MASK and XRIGHT with a new right value, jump to the 
        top of this section */
	RAM(RAMRD, _MAX_MASK, INC);
        SEQ(JUMP);
        NEXT(START_RIGHT_SIDE_P1);
    _ES

label(EXIT_RIGHT_ST_LOOP)
    _NS /* test rightcount */
	REGREG(RONLYOP, P0, _RIGHTCOUNT, _RIGHTCOUNT);
    _ES

    _NS /* calculate DEL_XRIGHT_HI, the MAR still holds the right addr,
	load the value into Q */
	ALUOP(SUBSROP, P1);
	SETROP(_XRIGHT_HI, NONE);
	SETSOP(NONQOP, _DEL_XRIGHT_HI, RAMRD);
	FTOYANDQ(FF, LDQ, REGWRE);
        /* do the divide here, if the count is not zero */
	SEQ(JSUB);
	COND(IFNZ);
	NEXT(COMPUTE_RIGHT_SLOPE_F2D);
    _ES

    _NS
	SEQ(RETN);
    _ES
}
