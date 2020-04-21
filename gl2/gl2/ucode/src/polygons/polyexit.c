
/* polyshade		<< GF2/UC4 >>
/*
/*  Register assignments for this routine follow:
*/

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "fbc.mic.h"
#include "polydefs.h"
#include "consts.h"

#include "gl2/gl2cmds.h"


/*
/*  RAM address assignments used are listed in consts.mic
*/

/*
	registers used
		none

	constants used
		none

	scratch locations used
		_CONFIG

	routines called
		none

	exits to 
		DISPATCH
*/

poly_exit()
{
newfile("poly_exit.c");

label(POLY_EXIT)
    _NS
	ALUOP(ANDOP, P0);
	SETROP(0, ALL16);
	LOADDI(UCONST); CONST(7);
	SETSOP(NONQOP, _BIASED_DONE, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	GEOMENGDATA;
    _ES

    _NS	/* get the next command code into the Q register */
    	ALUOP(RONLYOP, P0);
    	SETROP(0, ALL16);
    	SETSOP(NONQOP, 0, RAMNOP);
	LOADDI(INRJUST);
    	FTOYANDQ(FF, LDQ, REGWRD);
    _ES

    _NS /* compare the command with the command for move poly */
	ALUOP(SUBSROP, P1);
	SETROP(0, ALL16);
	LOADDI(UCONST);
	CONST(GEmovepoly);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* if equal to movepoly call it */
	REGREG(RONLYOP, P0, _BIASED_DONE, _BIASED_DONE);
	SEQ(JUMP);
	NEXT(TO_POLY_MOVE);
	COND(IFEQ);
    _ES

    _NS
	SEQ(JSUB);
	COND(IFNZ);
	NEXT(RESET_MASKS);
	BPCCMD(NOOP);
	DOTOOUTREG;
    _ES

    _NS /* restore the configuration and mode registers */
	LOADMAR(_CONFIG);
	CONST(_CONFIG);
    _ES

    _NS /* read the mode */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADMODE);
    _ES

    _NS /* bump the MAR, and mask _BIASED_DONE */
	IMMREG(ANDOP, P0, 1, _BIASED_DONE); CONST(1);
	DOTOMAR(INC);
    _ES

    _NS 
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADCONFIG);
    _ES

    _NS
	LOADMAR(_MULTIVIEW);
	CONST(_MULTIVIEW);
    _ES

    _NS
	RAM(RAMWR, _BIASED_DONE, HOLD);
	SEQ(JUMP);
	NEXT(DISPATCH);
    _ES

label(TO_POLY_MOVE)
    _NS 
	COND(IFZ);
	SEQ(JUMP);
	NEXT(POLY_MOVE);
    _ES

    _NS
	LOADMAR(_MULTIVIEW);
	CONST(_MULTIVIEW);
    _ES

    _NS 
	IMMREG(IOROP, P0, 0x8, _BIASED_DONE); CONST(0x8);
    _ES

    _NS 
	RAM(RAMWR, _BIASED_DONE, HOLD);
	SEQ(JUMP);
	NEXT(POLY_MOVE);
    _ES
}
