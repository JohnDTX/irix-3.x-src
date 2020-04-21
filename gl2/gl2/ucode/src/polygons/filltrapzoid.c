/*	fill_trapezoid
 *
 *	fills the current trapezoid for each different screen mask
 *	NOTE - make sure RESET_MASKS called somewhere before NEW_MASK - msg
 *
 */
#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"
#include "polydefs.h"

/* registers used
	FILL_TRAPEZOID
		_DEL_XLEFT_HI
		_DEL_XRIGHT_HI
		_DEL_XRIGHT_LO
		_XRIGHT_LO,
		_XRIGHT_HI,
		_XLEFT_LO,
		_XLEFT_HI,
		_YVALUE,
		_BIASED_DONE 	NOTE: the done bit is the high order bit
					the biasing bits are the two
					low order bits

   constants used
	FILL_TRAPEZOID
		none
*/

fill_trapezoid ()
{
newfile("filltrapezoid.c");

label(FILL_TRAPEZOID)
    _NS	/* load DDAEDF and DDAEDI */
	REGREG(RONLYOP, P0, _DEL_XRIGHT_HI, _DEL_XRIGHT_HI);
	DOTOOUTREG;
	BPCCMD(LOADEDI);
    _ES

    _NS
	REGREG(RONLYOP, P0, _DEL_XRIGHT_LO, _DEL_XRIGHT_LO);
	DOTOOUTREG;
	BPCCMD(LOADEDF);
    _ES

    _NS	/* load the DDA values which have to be reset for each new mask,
	   NOTE: if the code loops, this value is loaded at the bottom */
	REGREG(RONLYOP, P0, _XRIGHT_LO, _XRIGHT_LO);
	DOTOOUTREG;
	BPCCMD(LOADEAF);
    _ES

    _NS
	REGREG(RONLYOP, P0, _XRIGHT_HI, _XRIGHT_HI);
	DOTOOUTREG;
	BPCCMD(LOADEAI);
    _ES

label(MASK_LOOP)
    _NS
	REGREG(RONLYOP, P0, _XRIGHT_HI, _XRIGHT_HI);
	DOTOOUTREG;
	BPCCMD(LOADXE);
    _ES

    _NS
	REGREG(RONLYOP, P0, _XLEFT_LO, _XLEFT_LO);
	DOTOOUTREG;
	BPCCMD(LOADSAF);
    _ES

    _NS
	REGREG(RONLYOP, P0, _XLEFT_HI, _XLEFT_HI);
	DOTOOUTREG;
	BPCCMD(LOADSAI);
    _ES

    _NS
	REGREG(RONLYOP, P0, _XLEFT_HI, _XLEFT_HI);
	DOTOOUTREG;
	BPCCMD(LOADXS);
    _ES

    _NS
	/* load  YEB with Q + YVALUE, at this point Q should be loaded
	with the smaller of the two counts */
	ALUOP(ADDOP, P0);
	SETROP(_YVALUE, NONE);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADYE);
    _ES

    _NS /* load the Y start address */
	REGREG(RONLYOP, P0, _YVALUE, _YVALUE);
	DOTOOUTREG;
	BPCCMD(LOADYS);
    _ES

    _NS /* test the multimask flag, and do a noop to syncronize */
	ALUOP(ANDOP, P0);
	SETROP(0, ALL16); LOADDI(UCONST); CONST(1);
	SETSOP(NONQOP, _BIASED_DONE, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD); 
    _ES

    _NS /* now actually draw the trapezoid */
	ALUOP(FLOWOP, P0); /* set the zero condiiton bits */
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	BPCCMD(FILLTRAP); /*!!!!!!!!!!!!!!!!!!!!!!*/
	DOTOOUTREG;
	COND(IFNZ);
	SEQ(JSUB);
	NEXT(NEW_MASK);
    _ES

    _NS /* check to see if its time to exit, also test done in case of
    	a return */
	SEQ(RETN);
	COND(IFZ);
	REGREG(RONLYOP, P0, _BIASED_DONE, _BIASED_DONE);
    _ES

    _NS	/* test the biased flag, these flags are bits 1 and 2 of the word*/
	ALUOP(ANDOP, P0);
	SETROP(0, ALL16);
	LOADDI(UCONST);
	CONST(6);
	SETSOP(NONQOP, _BIASED_DONE, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* call CHECK_MASKING including a test of the BIASED flag */
	SEQ(JSUB);
	COND(IFNZ);
	NEXT(CHECK_MASKING);
	REGREG(RONLYOP, P0, _XRIGHT_HI, _XRIGHT_HI);
	DOTOOUTREG;
	BPCCMD(LOADEAI);
    _ES

    _NS /* return to the top of the loop, reload DDAEAF */
	SEQ(JUMP);
	NEXT(MASK_LOOP);
	REGREG(RONLYOP, P0, _XRIGHT_LO, _XRIGHT_LO);
	DOTOOUTREG;
	BPCCMD(LOADEAF);
    _ES

label(INTERRUPTPOLY)
    _NS /* halt */
	REGHOLD;
	MARETC(HOLD, INTERRUPT);
	SEQ(JUMP);
	NEXT(INTERRUPTPOLY);
    _ES
}
