
/*	shade_line
 *
 *	shade fills the current line
 *
 */
#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"
#include "polydefs.h"

/* registers used
	SHADE_LINE
		_I
		_XRIGHT_HI,
		_XLEFT_HI,
		_COLOR_LEFT_HI,
		<_COLOR_LEFT_LO>,stored in Q, this was done to allow
				it to be shifted from a 16-bit to a 
				12-bit fraction 
		_YVALUE,
		_BIASED_DONE, 	NOTE: the done bit is the high order bit
					the biasing bits are the two
					low order bits

	FAST_SHADE_LINE
		_XRIGHT_HI,
		_XLEFT_HI,
		_COLOR_LEFT_HI,
		<_COLOR_LEFT_LO>,stored in Q, this was done to allow
				it to be shifted from a 16-bit to a 
				12-bit fraction
		_YVALUE

   constants used
		none
    
*/

shade_line()
{
newfile("shadeline.c");

label(SHADE_LINE)

    _NS /* complement _I */
	REGREG(COMPROP, P1, _I, _I);
    _ES

    _NS /* load the address of the BIASED_DONE flag */
	LOADMAR(_SCR_BIASED_DONE);
	CONST(_SCR_BIASED_DONE);
    _ES

    _NS /* test for biasing masking */
	ALUOP(ANDOP, P0);
	SETROP(0, ALL16);
	LOADDI(UCONST);
	CONST(6);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(INC);
    _ES

    _NS /* check the masks */
	SEQ(JSUB);
	COND(IFNZ);
	NEXT(CHECK_MASKING_S2D);
    _ES

    _NS /* add one half to Q to generate the carry */
	ALUOP(ADDOP, P0);
	SETROP(0, ALL16);
	CONST(0x800);
	LOADDI(UCONST);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADSAF);
	PROPOUT12;
    _ES

    _NS /* load the DDA start value register (int) */
	ALUOP(RONLYOP, P0);
	SETROP(_COLOR_LEFT_HI, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	PROPIN;
	DOTOOUTREG;
	BPCCMD(LOADSAI);
    _ES


    _NS	/* load the XS register */
	REGREG(RONLYOP, P0, _XLEFT_HI, _XLEFT_HI);
	DOTOOUTREG;
	BPCCMD(LOADXS);
    _ES

label(LINE_LOOP)
    _NS /* load the XE register */
	REGREG(RONLYOP, P0, _XRIGHT_HI, _XRIGHT_HI);
	DOTOOUTREG;
	BPCCMD(LOADXE);
    _ES

    _NS /* load the YS register */
	REGREG(RONLYOP, P0, _YVALUE, _YVALUE);
	DOTOOUTREG;
	BPCCMD(LOADYS);
    _ES

    _NS /* add one half to Q to generate the carry */
	ALUOP(ADDOP, P0);
	SETROP(0, ALL16);
	CONST(0x800);
	LOADDI(UCONST);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	PROPOUT12;
    _ES

    _NS /* draw the scan line, dump out the start color so this looks 
	right */
	ALUOP(RONLYOP, P0);
	SETROP(_COLOR_LEFT_HI, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	PROPIN;
	DOTOOUTREG;
	BPCCMD(OCT1VECT);
	SEQ(JSUB);
	NEXT(NEW_MASK);
    _ES

    _NS /* check to see if time to exit */
	SEQ(JUMP);
	COND(IFZ);
	NEXT(SHADE_LINE_RETURN);
    _ES
    
    _NS /* load the MAR with the address of the BIASED_DONE flag */
	LOADMAR(_SCR_BIASED_DONE);
	CONST(_SCR_BIASED_DONE);
    _ES

    _NS /* test for biasing masking */
	ALUOP(ANDOP, P0);
	SETROP(0, ALL16);
	LOADDI(UCONST);
	CONST(6);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* call CHECK_MASKING is biased, and load the fractional color */
	SEQ(JSUB);
	COND(IFNZ);
	NEXT(CHECK_MASKING);
    _ES

    _NS
	/* load the color */
	ALUOP(ADDOP, P0);
	SETROP(0, ALL16);
	CONST(0x800);
	LOADDI(UCONST);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADSAF);
	PROPOUT12;
    _ES

    _NS /* return to the top of the loop and load the fractional color */
	ALUOP(RONLYOP, P0);
	SETROP(_COLOR_LEFT_HI, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD); 
	PROPIN;
	DOTOOUTREG;
	BPCCMD(LOADSAI);
    _ES

    _NS
	REGREG(RONLYOP, P0, _XLEFT_HI, _XLEFT_HI);
	DOTOOUTREG;
	BPCCMD(LOADXS);
	SEQ(JUMP);
	NEXT(LINE_LOOP);
    _ES

/***********************************************************************/
/***********************************************************************/
/***********************************************************************/

label(FAST_SHADE_LINE)

    /* the X start addr register, was loaded in the calling
    stmt */

    _NS /* load the DDA start value register (frac), this comes from Q */
	ALUOP(ADDOP, P0);
	SETROP(0, ALL16);
	CONST(0x800);
	LOADDI(UCONST);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	PROPOUT12;
	BPCCMD(LOADSAF);
    _ES

    _NS /* load the DDA start value register (int) */
	ALUOP(RONLYOP, P0);
	SETROP(_COLOR_LEFT_HI, NONE);
	PROPIN;
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADSAI);
    _ES

label(FAST_LINE_LOOP)

    _NS /* load the XE register */
	REGREG(RONLYOP, P0, _XRIGHT_HI, _XRIGHT_HI);
	DOTOOUTREG;
	BPCCMD(LOADXE);
    _ES

    _NS /* load the YS register */
	REGREG(RONLYOP, P0, _YVALUE, _YVALUE);
	DOTOOUTREG;
	BPCCMD(LOADYS);
    _ES

    _NS /* add one half to Q to generate the carry */
	ALUOP(ADDOP, P0);
	SETROP(0, ALL16);
	CONST(0x800);
	LOADDI(UCONST);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	PROPOUT12;
    _ES

    _NS /* draw the scan line, dump out the start color so this is right */
	ALUOP(RONLYOP, P0);
	PROPIN;
	SETROP(_COLOR_LEFT_HI, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(OCT1VECT);
	SEQ(JSUB);
	NEXT(NEW_MASK);
    _ES

    _NS /* check to see if time to exit */
	SEQ(JUMP);
	COND(IFZ);
	NEXT(SHADE_LINE_RETURN);
    _ES

    _NS	/* load the XS register */
	REGREG(RONLYOP, P0, _XLEFT_HI, _XLEFT_HI);
	DOTOOUTREG;
	BPCCMD(LOADXS);
    _ES

    _NS /* load the start color, if execution reaches here a NOOP has
	been done */
	ALUOP(ADDOP, P0);
	SETROP(0, ALL16);
	CONST(0x800);
	LOADDI(UCONST);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADSAF);
	PROPOUT12;
    _ES
    
    _NS /* return to the top of the loop and load the fractional color */
	SEQ(JUMP);
	NEXT(FAST_LINE_LOOP);
	ALUOP(RONLYOP, P0);
	SETROP(_COLOR_LEFT_HI, NONE);
	PROPIN;
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADSAI);
    _ES
}
