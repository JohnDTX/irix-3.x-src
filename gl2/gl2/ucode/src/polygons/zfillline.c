
/*	zfill_line
 *
 *	zfill fills the current line
 *
 */
#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"
#include "polydefs.h"

/* registers used
	ZFILL_LINE
		_I
		_XRIGHT_HI,
		_XLEFT_HI,
		_Z_LEFT_HI,
		<_Z_LEFT_LO>,stored in Q, this was done to allow
				it to be shifted from a 16-bit to a 
				12-bit fraction 
		_YVALUE,
		_BIASED_DONE, 	NOTE: the done bit is the high order bit
					the biasing bits are the two
					low order bits

	FAST_ZFILL_LINE
		_XRIGHT_HI,
		_XLEFT_HI,
		_Z_LEFT_HI,
		<_Z_LEFT_LO>,stored in Q, this was done to allow
				it to be shifted from a 16-bit to a 
				12-bit fraction
		_YVALUE

   constants used
		none
    
*/

z_fill_line()
{
newfile("zfillline.c");

label(ZFILL_LINE)

    _NS /* complement _I */
	REGREG(COMPROP, P1, _I, _I);
    _ES

label(ZLINE_LOOP)
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
	REGREG(RONLYOP, P0, _Z_LEFT_LO, _Z_LINE_LO);
	SEQ(JSUB);
	COND(IFNZ);
	NEXT(CHECK_MASKING_S2D);
    _ES

    _NS
	IMMREG(ADDOP, P0, 0x8000, _Z_LINE_LO);
	CONST(0x8000);
	PROPOUT16;
    _ES

    _NS 
	REGREG(RONLYOP, P0, _Z_LEFT_HI, _Z_LINE_HI);
	PROPIN;
    _ES

    _NS	/* load the XS register */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADXS);
    _ES

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

    _NS /* load the YE register */
	REGREG(RONLYOP, P0, _YVALUE, _YVALUE);
	DOTOOUTREG;
	BPCCMD(LOADYE);
    _ES

    _NS
	DOTOOUTREG;
	BPCCMD(SETADDRS);
    _ES

    _NS
	LOADMAR(_SAVE1+5);
	CONST(_SAVE1+5);
    _ES

    _NS
	SEQ(JSUB);
	NEXT(FAST_Z_LINE_LOOP_TOP);
    _ES

    _NS
	SEQ(JSUB);
	NEXT(NEW_MASK);
    _ES

    _NS /* check to see if time to exit */
        /* figure the line length and load it into the output register */
	SEQ(JUMP);
	COND(IFZ);
	NEXT(ZFILL_LINE_RETURN);
	ALUOP(SUBSROP, P1);
	SETROP(_XLEFT_HI, NONE);
	SETSOP(NONQOP, _XRIGHT_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
    _ES

    _NS /* move the X start address into Q */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _XLEFT_HI, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	LOADDI(OUTPUTCOUNT);
	SEQ(LDCT);
    _ES

    _NS
	SEQ(JUMP);
	NEXT(ZLINE_LOOP);
    _ES

/***********************************************************************/
/***********************************************************************/
/***********************************************************************/

label(FAST_ZFILL_LINE)

    _NS
	LOADMAR(_SAVE1+5);
	CONST(_SAVE1+5);
    _ES

    _NS
	IMMREG(ADDOP, P0, 0x8000, _Z_LINE_LO);
	CONST(0x8000);
	PROPOUT16;
    _ES

    _NS 
	REGREG(RONLYOP, P0, _Z_LEFT_HI, _Z_LINE_HI);
	PROPIN;
    _ES

    _NS	/* load the XS register */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADXS);
    _ES

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

    _NS /* load the YE register */
	REGREG(RONLYOP, P0, _YVALUE, _YVALUE);
	DOTOOUTREG;
	BPCCMD(LOADYE);
    _ES

    _NS
	DOTOOUTREG;
	BPCCMD(SETADDRS);
    _ES

    _NS
	SEQ(JSUB);
	NEXT(FAST_Z_LINE_LOOP_TOP);
    _ES

    _NS
	SEQ(JSUB);
	NEXT(NEW_MASK);
    _ES

    _NS /* check to see if time to exit */
        /* figure the line length and load it into the output register */
	SEQ(JUMP);
	COND(IFZ);
	NEXT(ZFILL_LINE_RETURN);
	ALUOP(SUBSROP, P1);
	SETROP(_XLEFT_HI, NONE);
	SETSOP(NONQOP, _XRIGHT_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
    _ES

    _NS /* move the X start address into Q */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _XLEFT_HI, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	LOADDI(OUTPUTCOUNT);
	SEQ(LDCT);
    _ES

    _NS
	REGREG(RONLYOP, P0, _Z_LEFT_LO, _Z_LINE_LO);
	SEQ(JUMP);
	NEXT(FAST_ZFILL_LINE)
    _ES

}
