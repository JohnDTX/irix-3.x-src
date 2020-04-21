/* zstartside.c
/*	start_left_side_S_Z and start_right_side_S_Z
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
	    START_LEFT_SIDE_S_Z
		_XLEFT_HI
		_COLOR_LEFT_HI
		_DEL_HI
		_YVALUE
		_LEFT
		_LEFTCOUNT
		_BIASED_DONE

	    START_RIGHT_SIDE_S_Z
		_XRIGHT_HI
		_COLOR_RIGHT_HI
		_DEL_HI
		_YVALUE
		_RIGHT
		_RIGHTCOUNT
		_BIASED_DONE

	constants used
		none

	scratch locations used
		_LEFT_INC
		_SCR_DEL_XLEFT_HI
		_RIGHT_INC
		_SCR_DEL_XRIGHT_HI

	routines  called
	    START_LEFT_SIDE_S_Z
		BUMP_LEFT_INDEX
	    START_RIGHT_SIDE_S_Z
		BUMP_RIGHT_INDEX

	exits to
		caller
*/

zstart_side()
{
newfile("zstartside.c");

label(START_LEFT_SIDE_S_Z)
    _NS /* load MIN_mask and XLEFT with a new left value */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, LDQ, REGWRD);
	DOTOMAR(INC);
	DOTOOUTREG;
    _ES

label(START_LEFT_SIDE_P1_S_Z)
    _NS
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, _XLEFT_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
    _ES

    _NS /* get the new yvalue */
	RAM(RAMRD, _YVALUE, INC);
    _ES

    _NS /* get the new yvalue */
	RAM(RAMRD, _DEL_HI, INC);
    _ES

    _NS /* get the new yvalue */
	RAM(RAMRD, _COLOR_LEFT_HI, INC);
    _ES

label(UPDATE_LEFT_INDEX_S_Z)
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
	NEXT(EXIT_LEFT_ST_LOOP_S_Z);
	COND(IFNEG);
	REGREG(RONLYOP, P0, _BIASED_DONE, _BIASED_DONE);
    _ES
       
    _NS /* exit if done is true */ 
	SEQ(JUMP);
	COND(IFNEG);
	NEXT(EXIT_LEFT_ST_LOOP_S_Z);
    _ES

    _NS /* test the next left coord
	REGRAMCOMP(LE, _MIN_mask, INC); */

	ALUOP(SUBSROP, P1);
	SETROP(0, ALL16);
	LOADDI(OUTPUTREG);
	COND(IFFALSE);
	SEQ(CJPP);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(INC);
    _ES

    _NS /* conditional branch to UPDATE_LEFT_INDEX */
	SEQ(JUMP);
	NEXT(UPDATE_LEFT_INDEX_S_Z);
	COND(IFNNEG);
	REGRAMCOMP(GT, _YVALUE, DEC);
    _ES

    _NS /* conditional branch to UPDATE_LEFT_INDEX */
	SEQ(JUMP);
	NEXT(UPDATE_LEFT_INDEX_S_Z);
	COND(IFGT);
    _ES

    _NS /* load MIN_mask and XLEFT with a new left value, jump to the 
        top of this section
	RAM(RAMRD, _MIN_mask, INC); */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, LDQ, REGWRD);
	DOTOMAR(INC);
	DOTOOUTREG;
	/* jump back */
        SEQ(JUMP);
        NEXT(START_LEFT_SIDE_P1_S_Z);
    _ES

label(EXIT_LEFT_ST_LOOP_S_Z)
    _NS /* calculate DEL_X_HI, the MAR still holds the right addr */
	ALUOP(SUBSROP, P1);
	SETROP(_XLEFT_HI, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, LDQ, REGWRD);
	DOTOMAR(INC);
    _ES

    _NS 
	REGREG(FLOWOP, P0, _COLOR_LEFT_LO, _COLOR_LEFT_LO);
	DOTOMAR(INC);
    _ES

    _NS /* calculate DEL_Z_HI, the MAR is pointing to the
        right place */
	ALUOP(SUBSROP, P1);
	SETROP(_DEL_HI, NONE);
	SETSOP(NONQOP, _DEL_LO, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(INC);
    _ES

    _NS /* calculate DEL_COLOR_HI, the MAR is pointing to the
        right place */
	ALUOP(SUBSROP, P1);
	SETROP(_COLOR_LEFT_HI, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
    _ES

    _NS /* get the address of the LEFT Zs */
	LOADMAR(_SCR_Z_LEFT_LO_ZS);
	CONST(_SCR_Z_LEFT_LO_ZS);
    _ES

    _NS /* zero the low half */
	RAM(RAMWR, _COLOR_LEFT_LO, INC);
    _ES

    _NS /* write DEL_HI to scratch and load it with the output register */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16);
	LOADDI(OUTPUTREG); COND(IFFALSE); SEQ(CJPP);
	SETSOP(NONQOP, _DEL_HI, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
    _ES

    _NS
        /* do the divide here */
	REGREG(RONLYOP, P0, _LEFTCOUNT, _LEFTCOUNT);
	SEQ(JSUB);
	NEXT(COMPUTE_LEFT_SLOPE_S_Z);
    _ES

    _NS
	LOADMAR(_SCR_BIASED_DONE);
	CONST(_SCR_BIASED_DONE);
    _ES

    _NS
	RAM(RAMWR, _BIASED_DONE, HOLD);
    	SEQ(RETN); 
    _ES

/***************************************************************/
/***************************************************************/
/***************************************************************/

label(START_RIGHT_SIDE_S_Z)
    _NS /* load MAX_mask and XRIGHT with a new left value */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, LDQ, REGWRD);
	DOTOMAR(INC);
	DOTOOUTREG;
    _ES

label(START_RIGHT_SIDE_P1_S_Z)
    _NS
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, _XRIGHT_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
    _ES

    _NS /* get the new yvalue */
	RAM(RAMRD, _YVALUE, INC);
    _ES

    _NS /* get the new yvalue */
	RAM(RAMRD, _DEL_HI, INC);
    _ES

    _NS /* get the new yvalue */
	RAM(RAMRD, _COLOR_RIGHT_HI, INC);
    _ES

label(UPDATE_RIGHT_INDEX_S_Z)
    _NS /* load the MAR with the address of _RIGHT_INC */
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
	NEXT(EXIT_RIGHT_ST_LOOP_S_Z);
	COND(IFNEG);
	REGREG(RONLYOP, P0, _BIASED_DONE, _BIASED_DONE);
    _ES
       
    _NS /* exit if done is true */ 
	SEQ(JUMP);
	NEXT(EXIT_RIGHT_ST_LOOP_S_Z);
	COND(IFNEG);
    _ES

    _NS /* test the next left coord
	REGRAMCOMP(LE, _MAX_mask, INC); */

	ALUOP(SUBRSOP, P1);
	SETROP(0, ALL16);
	LOADDI(OUTPUTREG);
	COND(IFFALSE);
	SEQ(CJPP);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(INC);
    _ES

    _NS /* conditional branch to UPDATE_RIGHT_INDEX */
	SEQ(JUMP);
	NEXT(UPDATE_RIGHT_INDEX_S_Z);
	COND(IFNNEG);
	REGRAMCOMP(GT, _YVALUE, DEC);
    _ES

    _NS /* conditional branch to UPDATE_RIGHT_INDEX */
	SEQ(JUMP);
	NEXT(UPDATE_RIGHT_INDEX_S_Z);
	COND(IFGT);
    _ES

    _NS /* load MAX_mask and XRIGHT with a new left value, jump to the 
        top of this section
	RAM(RAMRD, _MAX_mask, INC); */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, LDQ, REGWRD);
	DOTOMAR(INC);
	DOTOOUTREG;
	/* jump back */
        SEQ(JUMP);
        NEXT(START_RIGHT_SIDE_P1_S_Z);
    _ES

label(EXIT_RIGHT_ST_LOOP_S_Z)
    _NS /* calculate DEL_X_HI, the MAR still holds the right addr */
	ALUOP(SUBSROP, P1);
	SETROP(_XRIGHT_HI, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, LDQ, REGWRD);
	DOTOMAR(INC);
    _ES

    _NS
	REGREG(FLOWOP, P0, _COLOR_RIGHT_LO, _COLOR_RIGHT_LO);
	DOTOMAR(INC);
    _ES

    _NS /* calculate DEL_Z_HI, the MAR is pointing to the
        right place */
	ALUOP(SUBSROP, P1);
	SETROP(_DEL_HI, NONE);
	SETSOP(NONQOP, _DEL_LO, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(INC);
    _ES

    _NS /* calculate DEL_COLOR_HI, the MAR is pointing to the
        right place */
	ALUOP(SUBSROP, P1);
	SETROP(_COLOR_RIGHT_HI, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
    _ES

    _NS /* load the MAR with the address of the RIGHT Zs */
	LOADMAR(_SCR_Z_RIGHT_LO_ZS);
	CONST(_SCR_Z_RIGHT_LO_ZS);
    _ES

    _NS /* zero the low half */
	RAM(RAMWR, _COLOR_RIGHT_LO, INC);
    _ES

    _NS /* write the hi color and load DEL_HI from output */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16);
	LOADDI(OUTPUTREG); COND(IFFALSE); SEQ(CJPP);
	SETSOP(NONQOP, _DEL_HI, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
    _ES

    _NS
        /* do the divide here */
	REGREG(RONLYOP, P0, _RIGHTCOUNT, _RIGHTCOUNT);
	SEQ(JSUB);
	NEXT(COMPUTE_RIGHT_SLOPE_S_Z);
    _ES

    _NS 
	LOADMAR(_SCR_BIASED_DONE);
	CONST(_SCR_BIASED_DONE);
    _ES

    _NS
	RAM(RAMWR, _BIASED_DONE, HOLD);
    	SEQ(RETN); 
    _ES

}
