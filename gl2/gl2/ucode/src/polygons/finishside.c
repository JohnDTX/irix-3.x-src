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
    	FINISH_LEFT_SIDE
		_XLEFT_HI,
		_XLEFT_LO,
		_MIN_MASK,	NOTE: this is also used as a temp register 
		_LEFTCOUNT,
		_DEL_XLEFT_HI,
		_DEL_XLEFT_LO,
		Q,
		_BIASED_DONE

    	FINISH_RIGHT_SIDE
		_XRIGHT_HI,
		_XRIGHT_LO,
		_MAX_MASK,	NOTE: this is also used as a temp register 
		_RIGHTCOUNT,
		_DEL_XRIGHT_HI,
		_DEL_XRIGHT_LO,
		Q,
		_BIASED_DONE

   constants used
	none
*/

finish_side()
{
newfile("finishside.c");

label(FINISH_LEFT_SIDE)
    _NS /* set MIN_MASK to the min of XLEFT_HI and the current scratch
        value */
	REGRAMCOMP(LT, _XLEFT_HI, HOLD);
    _ES

    _NS /* conditional branch to load MIN_MASK with the lesser value */
        SEQ(JUMP);
	NEXT(XLEFT_MIN);
	COND(IFLT);
    _ES

    _NS /* load MIN_MASK with the RAM value, decrement the MAR to point
        at a Z value, which is unused and we'll use it to store the MASK */
	REGRAM(SONLYOP, P0, 0, _MIN_MASK, DEC);
	SEQ(JUMP);
	NEXT(LEFT_BIAS_TEST);
    _ES

label(XLEFT_MIN)
    _NS	/* load MIN_MASK with _XLEFT_HI */
	REGREG(RONLYOP, P0, _XLEFT_HI, _MIN_MASK);
	DOTOMAR(DEC);
    _ES

label(LEFT_BIAS_TEST)
    _NS /* test LEFTCOUNT for a flat side, also save _MIN_MASK in the
    	unused Z location */
	ALUOP(RONLYOP, P0);
	SETROP(_LEFTCOUNT, NONE);
	SETSOP(NONQOP, _MIN_MASK, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* start figuring the abs value of del_xleft */
	REGREG(RONLYOP, P0, _DEL_XLEFT_HI, _DEL_XLEFT_HI);
	SEQ(JUMP);
	NEXT(UNBIASED_LEFT_SIDE);
	COND(IFZ);
    _ES

    _NS /* if its negative the complement must be taken */
	SEQ(JUMP);
	NEXT(POS_LEFT_SLOPE);
	COND(IFNNEG);
	ALUOP(COMPROP, P1);
	SETROP(_DEL_XLEFT_LO, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	PROPOUT12;
    _ES

    _NS /* finish the complement, using MIN_MASK as a temporary */
	ALUOP(COMPROP, P0);
	SETROP(_DEL_XLEFT_HI, NONE);
	SETSOP(NONQOP, _MIN_MASK, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPIN;
    _ES

    _NS /* conditionally branch to UNBIASED_LEFT_SIDE */
	SEQ(JUMP);
	NEXT(UNBIASED_LEFT_SIDE);
	COND(IFZ);
    _ES

    _NS /* jump to bias the left side, also right shift _MIN_MASK and Q*/
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _MIN_MASK, RAMNOP);
	FTOYANDQ(FLR, QR, REGWRE); 
	SEQ(JUMP);
	NEXT(BIAS_LEFT_SIDE);
    _ES

label(POS_LEFT_SLOPE)
    _NS /* now test the high half */
	REGREG(RONLYOP, P0, _DEL_XLEFT_HI, _MIN_MASK);
    _ES

    _NS /* conditionally branch to the UNBIASED section, load Q with
    	the low part in case we want to shift */
	SEQ(JUMP);
	NEXT(UNBIASED_LEFT_SIDE);
	COND(IFZ);
	ALUOP(RONLYOP, P0);
	SETROP(_DEL_XLEFT_LO, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
    _ES

    _NS /* now right shift _MIN_MASK and Q */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _MIN_MASK, RAMNOP);
	FTOYANDQ(FLR, QR, REGWRE); 
    _ES

label(BIAS_LEFT_SIDE)
    _NS /* test Q sign bit */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* branch if positive, this means that no bits have to be set */
	SEQ(JUMP);
	NEXT(NOT_NEG_LEFT);
	COND(IFNNEG);
    _ES

    _NS /* set the 12 bit word sign bit */
	ALUOP(IOROP, P0);
	SETROP(0, ALL16);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	LOADDI(UCONST);
	CONST(0xf800);
    _ES

label(NOT_NEG_LEFT)
    _NS	/* subtract the value now in _MIN_MASK and Q from _XLEFT_HI and LO,
	XLEFT_LO has nothing in it yet, so it can simply be loaded with
	the 2s complment of Q */
	ALUOP(COMPSOP, P1);
	SETROP(_XLEFT_LO, NONE);
	SETSOP(QOPERAND, _XLEFT_LO, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPOUT12;
    _ES

    _NS /* now subtract _MIN_MASK from _XLEFT_HI using the carry from above */
	ALUOP(SUBSROP, P0);
	SETROP(_MIN_MASK, NONE);
	SETSOP(NONQOP, _XLEFT_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPIN;
    _ES

    _NS /* set the BIASED bit, bit 0 */
	IMMREG(IOROP, P0, 2,  _BIASED_DONE);
	CONST(2);
    _ES

    _NS /* restore _MIN_MASK  with the value stored in RAM */
	REGRAM(SONLYOP, P0, 0, _MIN_MASK, HOLD);
    _ES

    _NS	/* add one to the hi half of xleft and return */
	REGREG(RONLYOP, P1, _XLEFT_HI, _XLEFT_HI);
	SEQ(RETN);
    _ES


label(UNBIASED_LEFT_SIDE)
    _NS /* add one half to XLEFT, the low half has no value as yet */
	IMMREG(RONLYOP, P0, 0x800/*ONE_HALF_12*/, _XLEFT_LO); 
	CONST(0x800);
    _ES

    _NS /* clear the BIASED bit and return */
	IMMREG(ANDOP, P0, ~2,  _BIASED_DONE);
	CONST(~2);
    _ES

    _NS /* restore _MIN_MASK  with the value stored in RAM */
	REGRAM(SONLYOP, P0, 0, _MIN_MASK, HOLD);
	SEQ(RETN);
    _ES

/***********************************************************************/
/***********************************************************************/
/***********************************************************************/

label(FINISH_RIGHT_SIDE)
    _NS /* set MAX_MASK to the min of XRIGHT_HI and the current scratch
        value */
	REGRAMCOMP(GT, _XRIGHT_HI, HOLD);
    _ES

    _NS /* conditional branch to load MAX_MASK with the lesser value */
        SEQ(JUMP);
	NEXT(XRIGHT_MAX);
	COND(IFGT);
    _ES

    _NS /* load MAX_MASK with the RAM value, also decrement the MAR to
    	point a place where we can save _MAX_MASK */
	REGRAM(SONLYOP, P0, 0, _MAX_MASK, DEC);
	SEQ(JUMP);
	NEXT(RIGHT_BIAS_TEST);
    _ES

label(XRIGHT_MAX)
    _NS /* load MAX_MASK with _XRIGHT_HI */
	REGREG(RONLYOP, P0, _XRIGHT_HI, _MAX_MASK);
	DOTOMAR(DEC);
    _ES

label(RIGHT_BIAS_TEST)
    _NS /* test RIGHTCOUNT for a flat side, also save _MAX_MASK in the
    	unused Z location */
	ALUOP(RONLYOP, P0);
	SETROP(_RIGHTCOUNT, NONE);
	SETSOP(NONQOP, _MAX_MASK, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* start figuring the abs value of del_xright */
	REGREG(RONLYOP, P0, _DEL_XRIGHT_HI, _DEL_XRIGHT_HI);
	SEQ(JUMP);
	NEXT(UNBIASED_RIGHT_SIDE);
	COND(IFZ);
    _ES

    _NS /* if its negative the complement must be taken */
	SEQ(JUMP);
	NEXT(POS_RIGHT_SLOPE);
	COND(IFNNEG);
	ALUOP(COMPROP, P1);
	SETROP(_DEL_XRIGHT_LO, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	PROPOUT12;
    _ES

    _NS /* finish the complement */
	ALUOP(COMPROP, P0);
	SETROP(_DEL_XRIGHT_HI, NONE);
	SETSOP(NONQOP, _MAX_MASK, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPIN;
    _ES

    _NS /* conditionally branch to UNBIASED_RIGHT_SIDE */
	SEQ(JUMP);
	NEXT(UNBIASED_RIGHT_SIDE);
	COND(IFZ);
    _ES

    _NS /* jump to bias the right side, also right shift _MAX_MASK and Q */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _MAX_MASK, RAMNOP);
	FTOYANDQ(FLR, QR, REGWRE); 
	SEQ(JUMP);
	NEXT(BIAS_RIGHT_SIDE);
    _ES

label(POS_RIGHT_SLOPE)
    _NS /* now test the high half */
	REGREG(RONLYOP, P0, _DEL_XRIGHT_HI, _MAX_MASK);
    _ES

    _NS /* conditionally branch to the UNBIASED section, load Q with
    	the low part in case we want to shift */
	SEQ(JUMP);
	NEXT(UNBIASED_RIGHT_SIDE);
	COND(IFZ);
	ALUOP(RONLYOP, P0);
	SETROP(_DEL_XRIGHT_LO, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
    _ES

    _NS /* now right shift _MAX_MASK and Q */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _MAX_MASK, RAMNOP);
	FTOYANDQ(FLR, QR, REGWRE); 
    _ES

label(BIAS_RIGHT_SIDE)
    _NS /* test Q sign bit */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* branch if positive, this means that no bits have to be set */
	SEQ(JUMP);
	NEXT(NOT_NEG_RIGHT);
	COND(IFNNEG);
    _ES

    _NS /* set the 12 bit word sign bit */
	ALUOP(IOROP, P0);
	SETROP(0, ALL16);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	LOADDI(UCONST);
	CONST(0xf800);
    _ES

label(NOT_NEG_RIGHT)
    _NS /* now add _MAX_MASK to _XRIGHT_HI */
	ALUOP(ADDOP, P0);
	SETROP(_MAX_MASK, NONE);
	SETSOP(NONQOP, _XRIGHT_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
    _ES

    _NS /* set the BIASED bit and return, bit 2 */
	IMMREG(IOROP, P0, 4,  _BIASED_DONE);
	CONST(4);
    _ES

    _NS /* restore _MAX_MASK  with the value stored in RAM */
	REGRAM(SONLYOP, P0, 0, _MAX_MASK, HOLD);
    _ES

    _NS	/* add the value in _MAX_MASK and Q to _XRIGHT_HI and LO,
	XRIGHT_LO */
	ALUOP(SONLYOP, P0);
	SETROP(_XRIGHT_LO, NONE);
	SETSOP(QOPERAND, _XRIGHT_LO, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	SEQ(RETN);
    _ES

label(UNBIASED_RIGHT_SIDE)
    _NS /* add one half to XRIGHT, the low half has no value as yet */
	IMMREG(RONLYOP, P0, 0x800/*ONE_HALF_12*/, _XRIGHT_LO); 
	CONST(0x800);
    _ES

    _NS /* clear the BIASED bit and return */
	IMMREG(ANDOP, P0, ~4,  _BIASED_DONE);
	CONST(~4);
    _ES

    _NS /* restore _MAX_MASK  with the value stored in RAM */
	REGRAM(SONLYOP, P0, 0, _MAX_MASK, HOLD);
	SEQ(RETN);
    _ES
}
