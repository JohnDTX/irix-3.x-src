/* swap.c
/*	swap_sides
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
		_LEFT
		_RIGHT
		_DEL_XLEFT_HI
		_DEL_XRIGHT_HI
		_LEFTCOUNT
		_RIGHTCOUNT

	constants used
		none

	scratch locations used
		_LEFT_INC
		_RIGHT_INC

	routines called
		none

	exits to
		caller

*/
swap()
{
newfile("swap.c");

label(SWAP_SIDES)
    _NS /* load the MAR with the address of _BACKFACING */
	LOADMAR(_BACKFACING);
	CONST(_BACKFACING);
    _ES

    /* THIS IS DONE AT THE CALLING POINT 
    _NS /* swap left and right, the pointers into the vertex list 
	load left into Q 
	ALUOP(RONLYOP, P0);
	SETROP(_LEFT, NONE);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
    _ES */

    _NS
	IMMREG(RONLYOP, P0, 4, _LEFT);
	CONST(4);
    _ES

    _NS /* test BACKFACING flag */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(INC);
    _ES

    _NS /* now load _LEFT_INC with the value, and complement it */
	ALUOP(COMPROP, P1);
	SETROP(_LEFT, NONE);
	SETSOP(NONQOP, _LEFT, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(INC);
	/* if the flag WAS set branch to BACKPOLYEXIT */
	SEQ(JUMP);
	COND(IFNZ);
	NEXT(BACKPOLYEXIT);
    _ES

    _NS /* load right into left, also load _RIGHT_INC with the -value of
	_LEFT_INC */
	ALUOP(RONLYOP, P0);
	SETROP(_RIGHT, NONE);
	SETSOP(NONQOP, _LEFT, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
    _ES

    _NS /* load Q into right */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, _RIGHT, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(DEC);
    _ES

    _NS /* swap del_xleft_hi and del_xright_hi
	load del_xleft_hi into Q */
	ALUOP(RONLYOP, P0);
	SETROP(_DEL_XLEFT_HI, NONE);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
    _ES

    _NS /* load right into left */
	REGREG(RONLYOP, P0, _DEL_XRIGHT_HI, _DEL_XLEFT_HI);
    _ES

    _NS /* load Q into right */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, _DEL_XRIGHT_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
    _ES

    _NS /* swap leftcount and rightcount
	load leftcount into Q */
	ALUOP(RONLYOP, P0);
	SETROP(_LEFTCOUNT, NONE);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
    _ES

    _NS /* load right into leftcount */
	REGREG(RONLYOP, P0, _RIGHTCOUNT, _LEFTCOUNT);
    _ES

    _NS /* load Q into rightcount */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, _RIGHTCOUNT, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	SEQ(RETN);
    _ES

label(BACKPOLYEXIT)
    _NS
	LOADMAR(_MULTIVIEW);
	CONST(_MULTIVIEW);
    _ES

    _NS /* pop the return address */
	COND(IFTRUE);
	SEQ(TEST);
    _ES

    _NS
	RAM(RAMRD, _BIASED_DONE, HOLD);
	SEQ(JUMP);
	NEXT(POLY_EXIT);
    _ES
}
