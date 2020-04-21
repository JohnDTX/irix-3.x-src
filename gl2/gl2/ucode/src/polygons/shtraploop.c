/* traploop.c
/*	start_trapezoid_loop
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
		_BIASED_DONE
		_LEFTCOUNT
		_RIGHTCOUNT
		_LEFT
		_RIGHT
		_YVALUE
		_DEL_HI
		_DEL_LO
		_XLEFT_HI
		_XLEFT_LO
		_XRIGHT_HI
		_XRIGHT_LO

	constants used

	scratch locations used
		_SCR_DEL  (locations)
	routines called
		SHADE_TRAP
		START_LEFT_SIDE_S2D
		FINISH_LEFT_SIDE_S2D
		START_RIGHT_SIDE_S2D
		FINISH_RIGHT_SIDE_S2D
		ADVANCE_S2D

	exits to
		POLY_EXIT
*/

shaded_trap_loop()
{
newfile("shtraploop.c");

label(START_TRAPEZOID_LOOP_S2D)
    _NS /* load the MAR with the address of the ZBUFFERing flag */
	LOADMAR(_ZBUFFER);
	CONST(_ZBUFFER);
    _ES

    _NS 
	REGCOMP(EQ, _LEFTCOUNT, _RIGHTCOUNT);
    _ES

    _NS /* do the conditional branch */
	ALUOP(RONLYOP, P0);
	SETROP(_LEFTCOUNT, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	SEQ(JUMP);
	NEXT(LEFT_AND_RIGHT_NOT_EQUAL_S2D);
	COND(IFNQ);
    _ES

    _NS /* test the ZBUFFER flag */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* branch to non-zbuffered shade */
	SEQ(JUMP);
#ifndef NOZPOLY
	COND(IFZ);
#endif
	NEXT(NON_Z_1);
    _ES

    _NS
	SEQ(JSUB);
	NEXT(ZFILL_TRAP);
    _ES

    _NS 
	/* restore YVALUE from the Q register */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, _YVALUE, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	SEQ(JUMP);
	NEXT(SKIP_NON_Z_1);
    _ES

label(NON_Z_1)
    _NS /* call FILL_TRAPEZOID, also load the low left delta */
	SEQ(JSUB);
	NEXT(SHADE_TRAP);
    _ES

label(SKIP_NON_Z_1)
    _NS /* load the address of the flag */
	LOADMAR(_SCR_BIASED_DONE);
	CONST(_SCR_BIASED_DONE);
    _ES
    
    _NS /* test the BIASED_DONE flag for being done */
	RAM(RAMRD, _BIASED_DONE, HOLD);
    _ES

    _NS /* test done, if neg then the current polygon is complete */
	SEQ(JUMP);
	NEXT(POLY_EXIT);
	COND(IFNEG);
    _ES

    _NS /* call start_left_side, also load the MAR with left, to allow
	testing in this routine */
	SEQ(JSUB);
	NEXT(START_LEFT_SIDE_S2D);
	REGREG(RONLYOP, P0, _LEFT, _LEFT);
	DOTOMAR(LOAD);
    _ES

    _NS /* test done, this was setup by START_LEFT_SIDE_S2D, if true
	then the current polygon is complete, NOTE: this is optimized
	toward screen aligned rectangles, it costs others a state */
	SEQ(JUMP);
	NEXT(POLY_EXIT);
	COND(IFNEG);
    _ES

    _NS /* call start_right_side, also load the MAR with right, to allow
	testing in this routine */
	SEQ(JSUB);
	NEXT(START_RIGHT_SIDE_S2D);
	REGREG(RONLYOP, P0, _RIGHT, _RIGHT);
	DOTOMAR(LOAD);
    _ES

    _NS /* load the MAR with the address of DEL_XLEFT_HI */
	LOADMAR(_SCR_DEL_XLEFT_HI);
	CONST(_SCR_DEL_XLEFT_HI);
    _ES

    _NS /* retrieve HI and then LO */
	RAM(RAMRD, _DEL_HI, INC);
    _ES

    _NS /* retrieve HI and then LO */
	RAM(RAMRD, _DEL_LO, INC);
    _ES

    _NS /* now call finish_left_side, again load MAR with left */
	SEQ(JSUB);
	NEXT(FINISH_LEFT_SIDE_S2D);
	REGREG(RONLYOP, P0, _LEFT, _LEFT);
	DOTOMAR(LOAD);
    _ES

    _NS /* load the MAR with the address of DEL_XRIGHT_HI */
	LOADMAR(_SCR_DEL_XRIGHT_HI);
	CONST(_SCR_DEL_XRIGHT_HI);
    _ES

    _NS /* retrieve HI and then LO */
	RAM(RAMRD, _DEL_HI, INC);
    _ES

    _NS /* retrieve HI and then LO */
	RAM(RAMRD, _DEL_LO, INC);
    _ES

    _NS /* now call finish_right_side, again load MAR with right */
	SEQ(JSUB);
	NEXT(FINISH_RIGHT_SIDE_S2D);
	REGREG(RONLYOP, P0, _RIGHT, _RIGHT);
	DOTOMAR(LOAD);
    _ES

    _NS /* load the MAR with the address of the scratch location */
	LOADMAR(_SCR_DEL_COLOR_RIGHT_LO);
	CONST(_SCR_DEL_COLOR_RIGHT_LO);
    _ES

    _NS /* now call advance left and right to avoid filling the same
    	scan line twice, start the increment here */
	SEQ(JSUB);
	NEXT(ADVANCE_S2D);
	REGRAM(ADDOP, P0, _COLOR_RIGHT_LO, _COLOR_RIGHT_LO, DEC);
	PROPOUT16;
    _ES

    _NS /* decrement left and right counts */
	IMMREG(SUBSROP, P1, 1, _LEFTCOUNT);
	CONST(1);
    _ES

    _NS /* decrement left and right counts */
	IMMREG(SUBSROP, P1, 1, _RIGHTCOUNT);
	CONST(1);
    _ES

    _NS /* lastly increment yvalue and return to the top of the loop */
       REGREG(RONLYOP, P1, _YVALUE, _YVALUE);
       SEQ(JUMP);
       NEXT(START_TRAPEZOID_LOOP_S2D);
    _ES

label(LEFT_AND_RIGHT_NOT_EQUAL_S2D)
    _NS	/* test leftcount vs. rightcount */
	REGCOMP(GT, _LEFTCOUNT, _RIGHTCOUNT);
    _ES

    _NS /* now do the conditional jump to the third case */
	SEQ(JUMP);
	NEXT(LEFT_GREATER_RIGHT_S2D);
	COND(IFGT);
    _ES

    _NS /* test the ZBUFFER flag */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* branch to non-zbuffered shade */
	SEQ(JUMP);
#ifndef NOZPOLY
	COND(IFZ);
#endif
	NEXT(NON_Z_2);
    _ES

    _NS
	REGREG(SUBSROP, P1, _LEFTCOUNT, _RIGHTCOUNT);
	SEQ(JSUB);
	NEXT(ZFILL_TRAP);
    _ES

    _NS
	/* restore YVALUE from the Q register */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, _YVALUE, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	SEQ(JUMP);
	NEXT(SKIP_NON_Z_2);
    _ES

label(NON_Z_2)
    _NS /* call FILL_TRAPEZOID, also load the low left delta */
	REGREG(SUBSROP, P1, _LEFTCOUNT, _RIGHTCOUNT);
	SEQ(JSUB);
	NEXT(SHADE_TRAP);
    _ES

label(SKIP_NON_Z_2)
    _NS /* load the address of the flag */
	LOADMAR(_SCR_BIASED_DONE);
	CONST(_SCR_BIASED_DONE);
    _ES
    
    _NS /* test BIASED_DONE */
	RAM(RAMRD, _BIASED_DONE, HOLD);
    _ES

    _NS /* test done, this was setup by FILL_TRAPEZOID, if true
	then the current polygon is complete */
	SEQ(JUMP);
	NEXT(POLY_EXIT);
	COND(IFNEG);
    _ES

    _NS /* call start_left_side, also load the MAR with left, to allow
	testing in this routine */
	SEQ(JSUB);
	NEXT(START_LEFT_SIDE_S2D);
	REGREG(RONLYOP, P0, _LEFT, _LEFT);
	DOTOMAR(LOAD);
    _ES

    _NS /* load the MAR with the address of DEL_XLEFT_HI */
	LOADMAR(_SCR_DEL_XLEFT_HI);
	CONST(_SCR_DEL_XLEFT_HI);
    _ES

    _NS /* retrieve HI and then LO */
	RAM(RAMRD, _DEL_HI, INC);
    _ES

    _NS /* retrieve HI and then LO */
	RAM(RAMRD, _DEL_LO, INC);
    _ES

    _NS /* now call finish_left_side, again load MAR with left */
	SEQ(JSUB);
	NEXT(FINISH_LEFT_SIDE_S2D);
	REGREG(RONLYOP, P0, _LEFT, _LEFT);
	DOTOMAR(LOAD);
    _ES

    _NS /* load the MAR */
	LOADMAR(_SCR_DEL_COLOR_RIGHT_LO);
	CONST(_SCR_DEL_COLOR_RIGHT_LO);
    _ES

    _NS /* now call advance to avoid filling the same
    	scan line twice, start the increment here */
	SEQ(JSUB);
	NEXT(ADVANCE_S2D);
	REGRAM(ADDOP, P0, _COLOR_RIGHT_LO, _COLOR_RIGHT_LO, DEC);
	PROPOUT16;
    _ES

    _NS /* decrement left and right counts */
	IMMREG(SUBSROP, P1, 1, _LEFTCOUNT);
	CONST(1);
    _ES

    _NS /* decrement left and right counts */
	IMMREG(SUBSROP, P1, 1, _RIGHTCOUNT);
	CONST(1);
    _ES

    _NS
       REGREG(RONLYOP, P1, _YVALUE, _YVALUE);
       SEQ(JUMP);
       NEXT(START_TRAPEZOID_LOOP_S2D);
    _ES

label(LEFT_GREATER_RIGHT_S2D)
    _NS /* test the ZBUFFER flag */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* branch to non-zbuffered shade */
	SEQ(JUMP);
#ifndef NOZPOLY
	COND(IFZ);
#endif
	NEXT(NON_Z_3);
    _ES

    _NS
	ALUOP(RONLYOP, P0);
	SETROP(_RIGHTCOUNT, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	SEQ(JSUB);
	NEXT(ZFILL_TRAP);
    _ES

    _NS
	/* restore YVALUE from the Q register */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, _YVALUE, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	SEQ(JUMP);
	NEXT(SKIP_NON_Z_3);
    _ES

label(NON_Z_3)
    _NS /* call FILL_TRAPEZOID, also load the low left delta */
	ALUOP(RONLYOP, P0);
	SETROP(_RIGHTCOUNT, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	SEQ(JSUB);
	NEXT(SHADE_TRAP);
    _ES

label(SKIP_NON_Z_3)
    _NS /* load the address of the flag */
	LOADMAR(_SCR_BIASED_DONE);
	CONST(_SCR_BIASED_DONE);
    _ES
    
    _NS /* test and restore done */
	RAM(RAMRD, _BIASED_DONE, HOLD);
    _ES

    _NS /* test done, this was setup by FILL_TRAPEZOID, if true
	then the current polygon is complete */
	SEQ(JUMP);
	NEXT(POLY_EXIT);
	COND(IFNEG);
    	/* subtract rightcount from leftcount */
	REGREG(SUBSROP,  P1, _RIGHTCOUNT, _LEFTCOUNT);
    _ES

    _NS /* call start_right_side, also load the MAR with right, to allow
	testing in this routine */
	SEQ(JSUB);
	NEXT(START_RIGHT_SIDE_S2D);
	REGREG(RONLYOP, P0, _RIGHT, _RIGHT);
	DOTOMAR(LOAD);
    _ES

    _NS /* load the MAR with the address of DEL_XRIGHT_HI */
	LOADMAR(_SCR_DEL_XRIGHT_HI);
	CONST(_SCR_DEL_XRIGHT_HI);
    _ES

    _NS /* retrieve HI and then LO */
	RAM(RAMRD, _DEL_HI, INC);
    _ES

    _NS /* retrieve HI and then LO */
	RAM(RAMRD, _DEL_LO, INC);
    _ES

    _NS /* now call finish_right_side, again load MAR with right */
	SEQ(JSUB);
	NEXT(FINISH_RIGHT_SIDE_S2D);
	REGREG(RONLYOP, P0, _RIGHT, _RIGHT);
	DOTOMAR(LOAD);
    _ES

    _NS /* load the MAR */
	LOADMAR(_SCR_DEL_COLOR_RIGHT_LO);
	CONST(_SCR_DEL_COLOR_RIGHT_LO);
    _ES

    _NS /* now call advance to avoid filling the same
    	scan line twice, start the increment here */
	SEQ(JSUB);
	NEXT(ADVANCE_S2D);
	REGRAM(ADDOP, P0, _COLOR_RIGHT_LO, _COLOR_RIGHT_LO, DEC);
	PROPOUT16;
    _ES

    _NS /* decrement left and right counts */
	IMMREG(SUBSROP, P1, 1, _LEFTCOUNT);
	CONST(1);
    _ES

    _NS /* decrement left and right counts */
	IMMREG(SUBSROP, P1, 1, _RIGHTCOUNT);
	CONST(1);
    _ES

    _NS
       REGREG(RONLYOP, P1, _YVALUE, _YVALUE);
       SEQ(JUMP);
       NEXT(START_TRAPEZOID_LOOP_S2D);
    _ES
}
