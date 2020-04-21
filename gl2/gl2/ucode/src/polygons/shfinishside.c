/* finishside.c
/*	finish_left_side and finish_right_side
/*
/*
*/
#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "polydefs.h"

/* registers used
    	FINISH_LEFT_SIDE_S2D
		_XLEFT_HI,
		_XLEFT_LO,
		_LOC_TEMP,	
		_LEFTCOUNT,
		_DEL_HI,
		_DEL_LO,
		Q,
		_BIASED_DONE

    	FINISH_RIGHT_SIDE_S2D
		_XRIGHT_HI,
		_XRIGHT_LO,
		_LOC_TEMP,	NOTE: this is also used as a temp register 
		_RIGHTCOUNT,
		_DEL_HI,
		_DEL_LO,
		Q,
		_BIASED_DONE

   constants used
	none
*/

shfinish_side()
{
newfile("shfinishside.c");

label(FINISH_LEFT_SIDE_S2D)
    _NS /* set MIN_MASK to the min of XLEFT_HI and the current scratch
        value */
	REGRAMCOMP(LT, _XLEFT_HI, HOLD);
    _ES

    _NS /* conditional branch to load MIN_MASK with the lesser value */
	REGREG(FLOWOP, P0, _COLOR_LEFT_LO, _COLOR_LEFT_LO);
        SEQ(JUMP);
	NEXT(XLEFT_MIN_S2D);
	COND(IFLT);
    _ES

    _NS /* load MIN_MASK with the RAM value */
	RAM(RAMRD, _LOC_TEMP, HOLD);
	SEQ(JUMP);
	NEXT(LEFT_BIAS_TEST_S2D);
    _ES

label(XLEFT_MIN_S2D)
    _NS	/* load MIN_MASK with _XLEFT_HI */
	REGREG(RONLYOP, P0, _XLEFT_HI, _LOC_TEMP);
    _ES

label(LEFT_BIAS_TEST_S2D)
    _NS /* load the MAR */
	LOADMAR(_SCR_MIN_MASK);
	CONST(_SCR_MIN_MASK);
    _ES

    _NS /* test LEFTCOUNT for a flat side, also save _MIN_MASK in the
    	unused Z location */
	ALUOP(RONLYOP, P0);
	SETROP(_LEFTCOUNT, NONE);
	SETSOP(NONQOP, _LOC_TEMP, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* start figuring the abs value of del_xleft */
	REGREG(RONLYOP, P0, _DEL_HI, _DEL_HI);
	SEQ(JUMP);
	NEXT(UNBIASED_LEFT_SIDE_S2D);
	COND(IFZ);
    _ES

    _NS /* if its negative the complement must be taken */
	SEQ(JUMP);
	NEXT(POS_LEFT_SLOPE_S2D);
	COND(IFNNEG);
	ALUOP(COMPROP, P1);
	SETROP(_DEL_LO, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	PROPOUT16;
    _ES

    _NS /* finish the complement, using _LOC_TEMP as a temporary */
	ALUOP(COMPROP, P0);
	SETROP(_DEL_HI, NONE);
	SETSOP(NONQOP, _LOC_TEMP, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPIN;
    _ES

    _NS /* conditionally branch to UNBIASED_LEFT_SIDE */
	SEQ(JUMP);
	NEXT(UNBIASED_LEFT_SIDE_S2D);
	COND(IFZ);
    _ES

    _NS /* clear the low order bit of Q */
	ALUOP(ANDOP, P0);
	SETROP(0, ALL16); LOADDI(UCONST); CONST(0xfffe);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
    _ES

    _NS /* jump to bias the left side, also right shift _LOC_TEMP and Q*/
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _LOC_TEMP, RAMNOP);
	FTOYANDQ(FLR, QR, REGWRE); 
	SEQ(JUMP);
	NEXT(BIAS_LEFT_SIDE_S2D);
    _ES

label(POS_LEFT_SLOPE_S2D)
    _NS /* now test the high half */
	REGREG(RONLYOP, P0, _DEL_HI, _LOC_TEMP);
    _ES

    _NS /* conditionally branch to the UNBIASED section, load Q with
    	the low part in case we want to shift */
	SEQ(JUMP);
	NEXT(UNBIASED_LEFT_SIDE_S2D);
	COND(IFZ);
	ALUOP(RONLYOP, P0);
	SETROP(_DEL_LO, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
    _ES

    _NS /* clear the low order bit of Q */
	ALUOP(ANDOP, P0);
	SETROP(0, ALL16); LOADDI(UCONST); CONST(0xfffe);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
    _ES

    _NS /* now right shift _LOC_TEMP and Q */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _LOC_TEMP, RAMNOP);
	FTOYANDQ(FLR, QR, REGWRE); 
    _ES

label(BIAS_LEFT_SIDE_S2D)
    _NS	/* subtract the value now in _LOC_TEMP and Q from _XLEFT_HI and LO,
	XLEFT_LO has nothing in it yet, so it can simply be loaded with
	the 2s complment of Q */
	ALUOP(COMPSOP, P1);
	SETROP(_XLEFT_LO, NONE);
	SETSOP(QOPERAND, _XLEFT_LO, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPOUT16;
    _ES

    _NS /* now subtract _LOC_TEMP from _XLEFT_HI using the carry from above */
	ALUOP(SUBSROP, P0);
	SETROP(_LOC_TEMP, NONE);
	SETSOP(NONQOP, _XLEFT_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPIN;
    _ES

    _NS /* load the MAR with the address of the delta color values */
	LOADMAR(_SCR_DEL_COLOR_LEFT_HI);
	CONST(_SCR_DEL_COLOR_LEFT_HI);
    _ES

    _NS /* load _LOC_TEMP and Q with the low and hi */
	RAM(RAMRD, _LOC_TEMP, INC);
	SEQ(JSUB);
	NEXT(DIVIDE_DELTA);
    _ES

    _NS 
	SEQ(JUMP);
	NEXT(NEG_SLOPE_COLOR_BIAS_LEFT_S2D);
	COND(IFNEG);
    _ES

    _NS
	ALUOP(COMPSOP, P1);
	SETROP(0, NONE);
	SETSOP(QOPERAND, _COLOR_LEFT_LO, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPOUT16;
    _ES

    _NS 
	ALUOP(SUBSROP, P0);
	SETROP(_LOC_TEMP, NONE);
	SETSOP(NONQOP, _COLOR_LEFT_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPIN;
	SEQ(JUMP);
	NEXT(SKIP_NEG_SLOPE_BIAS_LEFT_S2D);
    _ES

label(NEG_SLOPE_COLOR_BIAS_LEFT_S2D)
    _NS
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, _COLOR_LEFT_LO, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
    _ES

    _NS 
	ALUOP(ADDOP, P0);
	SETROP(_LOC_TEMP, NONE);
	SETSOP(NONQOP, _COLOR_LEFT_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
    _ES

label(SKIP_NEG_SLOPE_BIAS_LEFT_S2D)
    _NS
	LOADMAR(_SCR_BIASED_DONE);
	CONST(_SCR_BIASED_DONE);
    _ES

    _NS /* set the BIASED bit, bit 1 */
	ALUOP(IOROP, P0);
	SETROP(0, ALL16);
	LOADDI(UCONST);
	CONST(2);
	SETSOP(NONQOP, _BIASED_DONE, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
    _ES

    _NS	/* add one to the hi half of xleft and return */
	REGREG(RONLYOP, P1, _XLEFT_HI, _XLEFT_HI);
    _ES

    _NS 
	RAM(RAMWR, _BIASED_DONE, HOLD);
	SEQ(RETN);
    _ES


label(UNBIASED_LEFT_SIDE_S2D)
    _NS /* add one half to XLEFT, the low half has no value as yet */
	IMMREG(RONLYOP, P0, 0x8000, _XLEFT_LO); 
	CONST(0x8000);
    _ES

    _NS
	LOADMAR(_SCR_BIASED_DONE);
	CONST(_SCR_BIASED_DONE);
    _ES

    _NS /* set the BIASED bit, bit 1 */
	ALUOP(ANDOP, P0);
	SETROP(0, ALL16);
	LOADDI(UCONST);
	CONST(~2);
	SETSOP(NONQOP, _BIASED_DONE, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
    _ES

    _NS
	RAM(RAMWR, _BIASED_DONE, HOLD);
	SEQ(RETN);
    _ES

/***********************************************************************/
/***********************************************************************/
/***********************************************************************/

label(FINISH_RIGHT_SIDE_S2D)
    _NS /* set MAX_MASK to the min of XRIGHT_HI and the current scratch
        value */
	REGRAMCOMP(GT, _XRIGHT_HI, HOLD);
    _ES

    _NS /* conditional branch to load MAX_MASK with the lesser value */
	REGREG(FLOWOP, P0, _COLOR_RIGHT_LO, _COLOR_RIGHT_LO);
        SEQ(JUMP);
	NEXT(XRIGHT_MAX_S2D);
	COND(IFGT);
    _ES

    _NS /* load MAX_MASK with the RAM value, also decrement the MAR to
    	point a place where we can save _MAX_MASK */
	RAM(RAMRD, _LOC_TEMP, HOLD);
	SEQ(JUMP);
	NEXT(RIGHT_BIAS_TEST_S2D);
    _ES

label(XRIGHT_MAX_S2D)
    _NS /* load MAX_MASK with _XRIGHT_HI */
	REGREG(RONLYOP, P0, _XRIGHT_HI, _LOC_TEMP);
	DOTOMAR(DEC);
    _ES

label(RIGHT_BIAS_TEST_S2D)
    _NS 
	LOADMAR(_SCR_MAX_MASK);
	CONST(_SCR_MAX_MASK);
    _ES

    _NS /* test RIGHTCOUNT for a flat side */
	ALUOP(RONLYOP, P0);
	SETROP(_RIGHTCOUNT, NONE);
	SETSOP(NONQOP, _LOC_TEMP, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* start figuring the abs value of del_xright */
	REGREG(RONLYOP, P0, _DEL_HI, _DEL_HI);
	SEQ(JUMP);
	NEXT(UNBIASED_RIGHT_SIDE_S2D);
	COND(IFZ);
    _ES

    _NS /* if its negative the complement must be taken */
	SEQ(JUMP);
	NEXT(POS_RIGHT_SLOPE_S2D);
	COND(IFNNEG);
	ALUOP(COMPROP, P1);
	SETROP(_DEL_LO, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	PROPOUT16;
    _ES

    _NS /* finish the complement */
	ALUOP(COMPROP, P0);
	SETROP(_DEL_HI, NONE);
	SETSOP(NONQOP, _LOC_TEMP, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPIN;
    _ES

    _NS /* conditionally branch to UNBIASED_RIGHT_SIDE */
	SEQ(JUMP);
	NEXT(UNBIASED_RIGHT_SIDE_S2D);
	COND(IFZ);
    _ES

    _NS /* clear the low order bit of Q */
	ALUOP(ANDOP, P0);
	SETROP(0, ALL16); LOADDI(UCONST); CONST(0xfffe);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
    _ES

    _NS /* jump to bias the right side, also right shift _LOC_TEMP and Q */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _LOC_TEMP, RAMNOP);
	FTOYANDQ(FLR, QR, REGWRE); 
	SEQ(JUMP);
	NEXT(BIAS_RIGHT_SIDE_S2D);
    _ES

label(POS_RIGHT_SLOPE_S2D)
    _NS /* now test the high half */
	REGREG(RONLYOP, P0, _DEL_HI, _LOC_TEMP);
    _ES

    _NS /* conditionally branch to the UNBIASED section, load Q with
    	the low part in case we want to shift */
	SEQ(JUMP);
	NEXT(UNBIASED_RIGHT_SIDE_S2D);
	COND(IFZ);
	ALUOP(RONLYOP, P0);
	SETROP(_DEL_LO, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
    _ES

    _NS /* clear the low order bit of Q */
	ALUOP(ANDOP, P0);
	SETROP(0, ALL16); LOADDI(UCONST); CONST(0xfffe);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
    _ES

    _NS /* now right shift _LOC_TEMP and Q */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _LOC_TEMP, RAMNOP);
	FTOYANDQ(FLR, QR, REGWRE); 
    _ES

label(BIAS_RIGHT_SIDE_S2D)
    _NS	/* add the value in _LOC_TEMP and Q to _XRIGHT_HI and LO,
	XRIGHT_LO */
	ALUOP(SONLYOP, P0);
	SETROP(_XRIGHT_LO, NONE);
	SETSOP(QOPERAND, _XRIGHT_LO, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
    _ES

    _NS /* now add _LOC_TEMP to _XRIGHT_HI */
	ALUOP(ADDOP, P0);
	SETROP(_LOC_TEMP, NONE);
	SETSOP(NONQOP, _XRIGHT_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
    _ES

    _NS /* load the MAR with the address of the delta color values */
	LOADMAR(_SCR_DEL_COLOR_RIGHT_HI);
	CONST(_SCR_DEL_COLOR_RIGHT_HI);
    _ES

    _NS /* load _LOC_TEMP and Q with the low and hi */
	RAM(RAMRD, _LOC_TEMP, INC);
	SEQ(JSUB);
	NEXT(DIVIDE_DELTA);
    _ES

    _NS 
	SEQ(JUMP);
	NEXT(NEG_SLOPE_COLOR_BIAS_RIGHT_S2D);
	COND(IFNEG);
    _ES

    _NS
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, _COLOR_RIGHT_LO, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
    _ES

    _NS 
	ALUOP(ADDOP, P0);
	SETROP(_LOC_TEMP, NONE);
	SETSOP(NONQOP, _COLOR_RIGHT_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	SEQ(JUMP);
	NEXT(SKIP_NEG_SLOPE_BIAS_RIGHT_S2D);
    _ES

label(NEG_SLOPE_COLOR_BIAS_RIGHT_S2D)
    _NS
	ALUOP(COMPSOP, P1);
	SETROP(0, NONE);
	SETSOP(QOPERAND, _COLOR_RIGHT_LO, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPOUT16;
    _ES

    _NS 
	ALUOP(SUBSROP, P0);
	SETROP(_LOC_TEMP, NONE);
	SETSOP(NONQOP, _COLOR_RIGHT_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPIN;
    _ES

label(SKIP_NEG_SLOPE_BIAS_RIGHT_S2D)
    _NS 
	LOADMAR(_SCR_BIASED_DONE);
	CONST(_SCR_BIASED_DONE);
    _ES

    _NS /* set the BIASED bit and return, bit 2 */
	ALUOP(IOROP, P0);
	SETROP(0, ALL16); LOADDI(UCONST); CONST(4);
	SETSOP(NONQOP, _BIASED_DONE, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
    _ES

    _NS
	RAM(RAMWR, _BIASED_DONE, HOLD);
	SEQ(RETN);
    _ES

label(UNBIASED_RIGHT_SIDE_S2D)
    _NS /* add one half to XRIGHT, the low half has no value as yet */
	IMMREG(RONLYOP, P0, 0x8000, _XRIGHT_LO); 
	CONST(0x8000);
    _ES

    _NS 
	LOADMAR(_SCR_BIASED_DONE);
	CONST(_SCR_BIASED_DONE);
    _ES

    _NS /* set the BIASED bit and return, bit 2 */
	ALUOP(ANDOP, P0);
	SETROP(0, ALL16);
	LOADDI(UCONST);
	CONST(~4);
	SETSOP(NONQOP, _BIASED_DONE, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
    _ES

    _NS
	RAM(RAMWR, _BIASED_DONE, HOLD);
	SEQ(RETN);
    _ES
}
