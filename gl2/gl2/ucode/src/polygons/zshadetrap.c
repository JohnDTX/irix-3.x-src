/*	zshade_trap
 *
 *	z shades the current trapezoid for each different screen mask
 *
 */
#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"
#include "polydefs.h"

/* registers used
	ZSHADE_TRAP
		_XRIGHT_HI,
		_XLEFT_HI,
		_Z_RIGHT_HI_ZS,
		_Z_RIGHT_LO_ZS,
		_Z_LEFT_HI_ZS,
		_Z_LEFT_LO_ZS,
		_COLOR_RIGHT_HI,
		_COLOR_RIGHT_LO,
		_COLOR_LEFT_HI,
		_COLOR_LEFT_LO,
		_DEL_HI,
		_DEL_LO,
		_YVALUE,
		Q,		holds the count of lines to shade
		_I,
		_BIASED_DONE	shared with _I

   constants used
		none
    
    scratch locations used
	<dummy>
	<dummy>
	_SCR_DEL_XLEFT_HI_ZS
	_SCR_DEL_XLEFT_LO_ZS
	_SCR_DEL_COLOR_LEFT_HI_ZS
	_SCR_DEL_COLOR_LEFT_LO_ZS	NOTE: these should be in this order in
	_SCR_DEL_Z_LEFT_HI_ZS 		consecutive scratch locations
	_SCR_DEL_Z_LEFT_LO_ZS		there must also be two dummy
	_SCR_DEL_XRIGHT_HI_ZS 		locations before SCR_DEL_XLEFT_HI
	_SCR_DEL_XRIGHT_LO_ZS
	_SCR_DEL_COLOR_RIGHT_HI_ZS
	_SCR_DEL_COLOR_RIGHT_LO_ZS
	_SCR_DEL_Z_RIGHT_HI_ZS
	_SCR_DEL_Z_RIGHT_LO_ZS
    
    routines called
	ADVANCE_S_Z
	ZSHADE_LINE
	FAST_ZSHADE_LINE
*/

zshade_trap()
{
newfile("zshadetrap.c");

label(ZSHADE_TRAP)
    _NS /* this tests the MULTIVIEW flag setup by the caller */
	SEQ(JUMP);
	NEXT(FAST_ZSHADE_TRAP);
	COND(IFZ);
    _ES

    _NS /* complement it, the end address as a flag  */
	REGREG(COMPROP, P1, _I, _I);
    _ES

label(ZSHADE_TRAP_LOOP_TOP)
    _NS /* save Z_LEFT_LO and Z_RIGHT_HI */
	LOADMAR(_SCR_Z_LEFT_LO_ZS_SAVE);
	CONST(_SCR_Z_LEFT_LO_ZS_SAVE);
    _ES

    _NS
	RAM(RAMWR, _Z_LEFT_LO_ZS, INC);
    _ES

    _NS
	RAM(RAMWR, _Z_RIGHT_HI_ZS, INC);
    _ES

    _NS /* load the MAR with the address of the SAVE1 area */
	LOADMAR(_SAVE1+2);
	CONST(_SAVE1+2);
	DOTOOUTREG;
    _ES

    _NS /* move _XRIGHT_HI into another reg */
	REGREG(RONLYOP, P0, _XRIGHT_HI, _DEL_HI);
	DOTOMAR(INC);
    _ES

    _NS /* calculate the length */
	ALUOP(SUBSROP, P1);
	SETROP(_XLEFT_HI, NONE);
	SETSOP(NONQOP, _DEL_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(INC);
    _ES

    _NS /* move Z_LEFT_HI_ZS into DEL_HI, and write the count */
	ALUOP(RONLYOP, P0);
	SETROP(_Z_LEFT_HI_ZS, NONE);
	SETSOP(NONQOP, _DEL_HI, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(DEC);
	/* abort this polygon if we have a negative length */
	/* **NOTE** a some point in the future this could be nicer and
	swap the two sides, recovering from a bow tie like situation
	*/
	SEQ(JUMP);
	COND(IFNEG);
	NEXT(ABORT_POLYGON);
    _ES

    _NS /* move Z_LEFT_LO_ZS into DEL_LO */
	REGREG(RONLYOP, P0, _Z_LEFT_LO_ZS, _DEL_LO);
    _ES

    _NS
	ALUOP(SUBRSOP, P1);
	SETROP(_Z_RIGHT_LO_ZS, NONE);
	SETSOP(NONQOP, _DEL_LO, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPOUT16;
    _ES

    _NS 
	ALUOP(SUBRSOP, P1);
	SETROP(_Z_RIGHT_HI_ZS, NONE);
	SETSOP(NONQOP, _DEL_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPIN;
    _ES

    _NS /* test the count */
	REGCOMP(EQ, _XLEFT_HI, _XRIGHT_HI);
    _ES

    _NS /* if length is not zero go ahead and to the divide */
	SEQ(JUMP);
	COND(IFEQ);
	NEXT(ZS_NO_LINE_DIVIDE);
	/* move the low delta into the parameter area */
	ALUOP(RONLYOP, P0);
	SETROP(_DEL_LO, NONE);
	SETSOP(NONQOP, _DEL_LO, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(DEC);
    _ES

    _NS /* load the integer part of the  delta z into scratch */
	ALUOP(RONLYOP, P0);
	SETROP(_DEL_HI, NONE);
	SETSOP(NONQOP, _DEL_HI, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	/* call the divide routine, this routine hopefully takes
	double word numerators with 16 bit fraction and produces a
	double word result with 16 bit fraction */
	SEQ(JSUB);
	NEXT(LONGDIVIDE16);
    _ES

    _NS /* load the MAR with the address of the SAVE1 area */
	LOADMAR(_SAVE1);
	CONST(_SAVE1);
	DOTOOUTREG;
    _ES

    _NS /* move _XRIGHT_HI into another reg */
	REGREG(RONLYOP, P0, _XRIGHT_HI, _DEL_HI);
	DOTOMAR(INC);
    _ES

    _NS /* calculate the length */
	ALUOP(SUBSROP, P1);
	SETROP(_XLEFT_HI, NONE);
	SETSOP(NONQOP, _DEL_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(INC);
    _ES

    _NS /* move COLOR_LEFT_HI into DEL_HI, and write the count */
	ALUOP(RONLYOP, P0);
	SETROP(_COLOR_LEFT_HI, NONE);
	SETSOP(NONQOP, _DEL_HI, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(DEC);
    _ES

    _NS /* move COLOR_LEFT_LO into DEL_LO */
	REGREG(RONLYOP, P0, _COLOR_LEFT_LO, _DEL_LO);
    _ES

    _NS
	ALUOP(SUBRSOP, P1);
	SETROP(_COLOR_RIGHT_LO, NONE);
	SETSOP(NONQOP, _DEL_LO, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPOUT16;
    _ES

    _NS 
	ALUOP(SUBRSOP, P1);
	SETROP(_COLOR_RIGHT_HI, NONE);
	SETSOP(NONQOP, _DEL_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPIN;
    _ES

    _NS	/* move the low delta into the parameter area */
	ALUOP(RONLYOP, P0);
	SETROP(_DEL_LO, NONE);
	SETSOP(NONQOP, _DEL_LO, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(DEC);
    _ES

    _NS /* load the integer part of the  delta color into scratch */
	ALUOP(RONLYOP, P0);
	SETROP(_DEL_HI, NONE);
	SETSOP(NONQOP, _DEL_HI, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	/* call the divide routine, this routine hopefully takes
	double word numerators with 16 bit fraction and produces a
	double word result with 16 bit fraction */
	SEQ(JSUB);
	NEXT(LONGDIVIDE16);
    _ES

    _NS /* load Q with the X start address */
	ALUOP(RONLYOP, P0);
	SETROP(_XLEFT_HI, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	SEQ(JSUB);
	NEXT(RESET_MASKS);
    _ES

    _NS	/* merge with the no divide case, figure the line length and
        load it into the output register */
	ALUOP(SUBSROP, P1);
	SETROP(_XLEFT_HI, NONE);
	SETSOP(NONQOP, _XRIGHT_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	SEQ(JUMP);
	NEXT(ZS_SKIP_SETUP);
    _ES

label(ZS_NO_LINE_DIVIDE)
    /* load those values which don't have to be reloaded in the mask loop */

    _NS /* load Q with the X start address */
	ALUOP(RONLYOP, P0);
	SETROP(_XLEFT_HI, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	SEQ(JSUB);
	NEXT(RESET_MASKS);
    _ES

    _NS /* load the MAR with the address of the fractional delta */
	LOADMAR(_SAVE1+2);
	CONST(_SAVE1+2);
    _ES

    _NS /* figure the length and load it into the output register */
	ALUOP(SUBSROP, P1);
	SETROP(_XLEFT_HI, NONE);
	SETSOP(NONQOP, _XRIGHT_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
    _ES

label(ZS_SKIP_SETUP)
    _NS /* test I for the first or last line of a trapezoid,
	load the counter itself as well */
	ALUOP(COMPROP, P1);
	SETROP(_I, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	LOADDI(OUTPUTCOUNT);
	SEQ(LDCT);
    _ES

    _NS /* test for last line of trap */
	ALUOP(SUBSROP, P1);
	SETROP(_I, NONE);
	SETSOP(NONQOP, _YVALUE, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	SEQ(JUMP);
	COND(IFNNEG);
	NEXT(ZSHADE_LINE);
    _ES

    _NS
	REGREG(RONLYOP, P0, _Z_LEFT_LO_ZS, _Z_LINE_LO);
	SEQ(JUMP);
	COND(IFNEG);
	NEXT(FAST_ZSHADE_LINE);
    _ES

    _NS
	SEQ(JUMP);
	NEXT(ZSHADE_LINE);
    _ES

label(ZSHADE_LINE_RETURN)
    _NS /* test _I */
	ALUOP(COMPROP, P1);
	SETROP(_I, NONE);
	SETSOP(NONQOP, _I, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* conditionally return from here */
	SEQ(RETN);
	COND(IFNNEG);
    _ES

    _NS /* load the MAR with the address of the various delta values */
	LOADMAR(_SCR_DEL_Z_RIGHT_LO_ZS);
	CONST(_SCR_DEL_Z_RIGHT_LO_ZS);
    _ES

    _NS /* now update all the deltas */
	REGRAM(ADDOP, P0, _Z_RIGHT_LO_ZS, _Z_RIGHT_LO_ZS, DEC);
	PROPOUT16;
	SEQ(JSUB);
	NEXT(ADVANCE_S_Z);
    _ES

    _NS
	SEQ(JUMP);
	NEXT(ZSHADE_TRAP_LOOP_TOP);
	REGREG(RONLYOP, P1, _YVALUE, _YVALUE);
    _ES

/********************************************************************/
/********************************************************************/
/********************************************************************/

label(FAST_ZSHADE_TRAP)
    _NS /* set up the correct screen mask */
	SEQ(JSUB);
	NEXT(RESET_MASKS);
	DOTOOUTREG;
	BPCCMD(NOOP);
    _ES

    _NS /* load the MAR with the address of BIASED_DONE, 
	NOTE!!: the INCMAR in the test sets up the MAR for use in
	check_masking_s2d */
	LOADMAR(_SCR_BIASED_DONE);
	CONST(_SCR_BIASED_DONE);
    _ES

    _NS /* check to see if bias masking is necessary */
	ALUOP(ANDOP, P0);
	SETROP(0, ALL16);
	LOADDI(UCONST);
	CONST(6);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(INC);
    _ES

    _NS /* conditionally call check masking */
	SEQ(JSUB);
	NEXT(CHECK_MASKING_S_Z);
	COND(IFNZ);
    _ES

label(FAST_ZSHADE_TRAP_LOOP_TOP)
    _NS /* save Z_LEFT_LO and Z_RIGHT_HI */
	LOADMAR(_SCR_Z_LEFT_LO_ZS_SAVE);
	CONST(_SCR_Z_LEFT_LO_ZS_SAVE);
    _ES

    _NS
	RAM(RAMWR, _Z_LEFT_LO_ZS, INC);
    _ES

    _NS
	RAM(RAMWR, _Z_RIGHT_HI_ZS, INC);
    _ES

    _NS /* load the MAR with the address of the SAVE1 area */
	LOADMAR(_SAVE1+2);
	CONST(_SAVE1+2);
	DOTOOUTREG;
    _ES

    _NS /* move _XRIGHT_HI into another reg */
	REGREG(RONLYOP, P0, _XRIGHT_HI, _DEL_HI);
	DOTOMAR(INC);
    _ES

    _NS /* calculate the length */
	ALUOP(SUBSROP, P1);
	SETROP(_XLEFT_HI, NONE);
	SETSOP(NONQOP, _DEL_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(INC);
    _ES

    _NS /* move Z_LEFT_HI_ZS into DEL_HI, and write the count */
	ALUOP(RONLYOP, P0);
	SETROP(_Z_LEFT_HI_ZS, NONE);
	SETSOP(NONQOP, _DEL_HI, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(DEC);
	/* abort this polygon if we have a negative length */
	/* **NOTE** a some point in the future this could be nicer and
	swap the two sides, recovering from a bow tie like situation
	*/
	SEQ(JUMP);
	COND(IFNEG);
	NEXT(ABORT_POLYGON);
    _ES

    _NS /* move Z_LEFT_LO_ZS into DEL_LO */
	REGREG(RONLYOP, P0, _Z_LEFT_LO_ZS, _DEL_LO);
    _ES

    _NS
	ALUOP(SUBRSOP, P1);
	SETROP(_Z_RIGHT_LO_ZS, NONE);
	SETSOP(NONQOP, _DEL_LO, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPOUT16;
    _ES

    _NS 
	ALUOP(SUBRSOP, P1);
	SETROP(_Z_RIGHT_HI_ZS, NONE);
	SETSOP(NONQOP, _DEL_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPIN;
    _ES

    _NS /* test the count */
	REGCOMP(EQ, _XLEFT_HI, _XRIGHT_HI);
    _ES

    _NS /* if length is not zero go ahead and to the divide */
	SEQ(JUMP);
	COND(IFEQ);
	NEXT(ZS_FAST_NO_LINE_DIVIDE);
	/* move the low delta into the parameter area */
	ALUOP(RONLYOP, P0);
	SETROP(_DEL_LO, NONE);
	SETSOP(NONQOP, _DEL_LO, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(DEC);
    _ES

    _NS /* load the integer part of the delta z into scratch */
	ALUOP(RONLYOP, P0);
	SETROP(_DEL_HI, NONE);
	SETSOP(NONQOP, _DEL_HI, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	/* call the divide routine, this routine hopefully takes
	double word numerators with 16 bit fraction and produces a
	double word result with 16 bit fraction */
	SEQ(JSUB);
	NEXT(LONGDIVIDE16);
    _ES

    _NS /* load the MAR with the address of the SAVE1 area */
	LOADMAR(_SAVE1);
	CONST(_SAVE1);
	DOTOOUTREG;
    _ES

    _NS /* move _XRIGHT_HI into another reg */
	REGREG(RONLYOP, P0, _XRIGHT_HI, _DEL_HI);
	DOTOMAR(INC);
    _ES

    _NS /* calculate the length */
	ALUOP(SUBSROP, P1);
	SETROP(_XLEFT_HI, NONE);
	SETSOP(NONQOP, _DEL_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(INC);
    _ES

    _NS /* move COLOR_LEFT_HI into DEL_HI, and write the count */
	ALUOP(RONLYOP, P0);
	SETROP(_COLOR_LEFT_HI, NONE);
	SETSOP(NONQOP, _DEL_HI, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(DEC);
    _ES

    _NS /* move COLOR_LEFT_LO into DEL_LO */
	REGREG(RONLYOP, P0, _COLOR_LEFT_LO, _DEL_LO);
    _ES

    _NS
	ALUOP(SUBRSOP, P1);
	SETROP(_COLOR_RIGHT_LO, NONE);
	SETSOP(NONQOP, _DEL_LO, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPOUT16;
    _ES

    _NS 
	ALUOP(SUBRSOP, P1);
	SETROP(_COLOR_RIGHT_HI, NONE);
	SETSOP(NONQOP, _DEL_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPIN;
    _ES

    _NS	/* move the low delta into the parameter area */
	ALUOP(RONLYOP, P0);
	SETROP(_DEL_LO, NONE);
	SETSOP(NONQOP, _DEL_LO, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(DEC);
    _ES

    _NS /* load the integer part of the  delta color into scratch */
	ALUOP(RONLYOP, P0);
	SETROP(_DEL_HI, NONE);
	SETSOP(NONQOP, _DEL_HI, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	/* call the divide routine, this routine hopefully takes
	double word numerators with 16 bit fraction and produces a
	double word result with 16 bit fraction */
	SEQ(JSUB);
	NEXT(LONGDIVIDE16);
    _ES

label(ZS_FAST_NO_LINE_DIVIDE)
    _NS /* figure the count loading it into the OUTPUT register */
	ALUOP(SUBSROP, P1);
	SETROP(_XLEFT_HI, NONE);
	SETSOP(NONQOP, _XRIGHT_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	DOTOMAR(INC);
    _ES

    _NS /* load the start z fraction */
	REGREG(RONLYOP, P0, _Z_LEFT_LO_ZS, _Z_LINE_LO);
	LOADDI(OUTPUTCOUNT);
	SEQ(LDCT);
    _ES

    _NS /* add in a bias value */
	IMMREG(ADDOP, P0, 0x8000, _Z_LINE_LO);
	CONST(0x8000);
	PROPOUT16;
	DOTOMAR(INC);
    _ES

    _NS /* load the start z value */
	REGREG(RONLYOP, P0, _Z_LEFT_HI_ZS, _Z_LINE_HI);
	PROPIN;
	DOTOMAR(INC);
    _ES

    _NS /* load the start z fraction */
	REGREG(RONLYOP, P0, _COLOR_LEFT_LO, _COLOR_LINE_LO);
    _ES

    _NS /* add in a bias value */
	IMMREG(ADDOP, P0, 0x8000, _COLOR_LINE_LO);
	CONST(0x8000);
	PROPOUT16;
    _ES

    _NS /* load the start z value */
	REGREG(RONLYOP, P0, _COLOR_LEFT_HI, _COLOR_LINE_HI);
	PROPIN;
    _ES

    _NS /* load Q with the X start address */
	ALUOP(RONLYOP, P0);
	SETROP(_XLEFT_HI, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(NOOP);
    _ES

    _NS	/* write the scanline Y address */
	REGREG(RONLYOP, P0, _YVALUE, _YVALUE);
	DOTOOUTREG;
	BPCCMD(LOADYS);
    _ES

    _NS	/* write the scanline Y address */
	REGREG(RONLYOP, P0, _YVALUE, _YVALUE);
	DOTOOUTREG;
	BPCCMD(LOADYE);
    _ES

    _NS /* load the X start address */
	REGREG(RONLYOP, P0, _XLEFT_HI, _XLEFT_HI);
	DOTOOUTREG;
	BPCCMD(LOADXS);
    _ES

    _NS /* load the X end address */
	REGREG(RONLYOP, P0, _XRIGHT_HI, _XRIGHT_HI);
	DOTOOUTREG;
	BPCCMD(LOADXE);
    _ES

    _NS 
	SEQ(JSUB);
	NEXT(FAST_ZS_LINE_LOOP_TOP);
	DOTOOUTREG;
	BPCCMD(SETADDRS);
    _ES

    _NS /* conditionally return from here */
	SEQ(RETN);
	COND(IFEQ);
    _ES

    _NS /* load the MAR with the address of the various delta values */
	LOADMAR(_SCR_DEL_Z_RIGHT_LO_ZS);
	CONST(_SCR_DEL_Z_RIGHT_LO_ZS);
    _ES

    _NS /* now update all the deltas */
	REGRAM(ADDOP, P0, _Z_RIGHT_LO_ZS, _Z_RIGHT_LO_ZS, DEC);
	PROPOUT16;
	SEQ(JSUB);
	NEXT(ADVANCE_S_Z);
    _ES

    _NS
	SEQ(JUMP);
	NEXT(FAST_ZSHADE_TRAP_LOOP_TOP);
	REGREG(RONLYOP, P1, _YVALUE, _YVALUE);
    _ES

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

label(FAST_ZS_LINE_LOOP_TOP)
    /* at this point XR and YR have been loaded and XS, YS, XE, and YE
    are loaded with the extremes of the line, scratch holds the
    delta Z values, _Z_LINE_HI and _Z_LINE_LO hold the start z values for 
    the scan line, Q holds the X start address and the hardware 
    counter has been loaded with a count equal to the number of pixels in the
    line. scratch also holds the delta color values and _COLOR_LINE_LO
    and COLOR_LINE_HI hold the start color values for the scan line 
    
	
	for X_ADDR = XLEFT_HI to XRIGHT_HI 
	    {
	    if pixel z vs. Z_LINE_HI => visible
		{
		load XS with X_ADDR
		SETADDRS
		DrawPixelCD (Z_LINE_HI)
		DrawPixelAB (COLOR in _SAVE1+5)
		}
	    if X_ADDR = XRIGHT_HI exit the loop
	    XADDR ++;
	    Z_LINE_LO += DEL_LO (propout16)
	    Z_LINE_HI += DEL_HI + carry
	    }
	*/
    _NS
	LOADMAR(_CONFIG);
	CONST(_CONFIG);
    _ES

    _NS /* read the mode to see if we're in double buffer mode */
	ALUOP(ANDOP, P0);
	SETROP(0, ALL16); LOADDI(UCONST); CONST(2);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(INC); /* point to config */
    _ES

    _NS /* jump to a double buffer handler */
	SEQ(JUMP);
	NEXT(DB_SHADED_ZBUFF);
	COND(IFNZ);
    _ES

    _NS
	LOADMAR(_SAVE1+7);
	CONST(_SAVE1+7);
    _ES

label(SB_SHADED_ZBUFF_INNER_LOOP)

    _NS /* request the Z value */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _Z_LINE_HI, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	BPCCMD(READPIXELCD);
	DOTOOUTREG;
    _ES

    _NS /* read and compare pixel z with Z_LINE_HI */
	ALUOP(SUBSROP, P1);
	SETROP(0, ALL16);
	READBPCBUS;
	LOADDI(BPCDATA);
	SETSOP(NONQOP, _Z_LINE_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOOUTREG; COND(IFFALSE); SEQ(CJPP);
    _ES

    _NS /* check for overflow */
	ALUOP(COMPSOP, P1);
	SETROP(0, NONE);
	SETSOP(NONQOP, _Z_LINE_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	SEQ(JUMP);
	COND(IFOVF);
	NEXT(OVERFLOW_Z_S);
    _ES

    _NS
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _Z_LINE_HI, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
	SEQ(JUMP);
	COND(IFNEG); /* not visible */
	NEXT(FAST_SKIP_ZS_DRAW);
	DOTOMAR(DEC);		/* decrement the MAR back to point at the
				fractional Z */
    _ES

label(DRAW_Z_S)
    _NS /* write the pixel value */
	REGREG(RONLYOP, P0, _COLOR_LINE_HI, _COLOR_LINE_HI);
	BPCCMD(DRAWPIXELAB);
	DOTOOUTREG;
    _ES

    _NS /* write the Z value */
	REGREG(RONLYOP, P0, _Z_LINE_HI, _Z_LINE_HI);
	BPCCMD(DRAWPIXELCD);
	DOTOOUTREG;
    _ES

    _NS /* increment Q along to the next X address */
	ALUOP(SONLYOP, P1);
	SETROP(0, NONE);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	SEQ(JUMP);
	NEXT(FAST_SKIP_XS_INCREMENT);
    _ES

label(OVERFLOW_Z_S)
    _NS
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _Z_LINE_HI, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(DEC);
	SEQ(JUMP);
	NEXT(DRAW_Z_S);
	COND(IFNEG);
    _ES

label(FAST_SKIP_ZS_DRAW)
    _NS /* increment Q along to the next X address */
	ALUOP(SONLYOP, P1);
	SETROP(0, NONE);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADXS);
    _ES

    _NS /* set the address */
	REGREG(RONLYOP, P0, 0, 0);
	BPCCMD(SETADDRS);
	DOTOOUTREG;
    _ES

label(FAST_SKIP_XS_INCREMENT)
    _NS /* add DEL_LO to Z_LINE_LO */
	ALUOP(ADDOP, P0);
	SETROP(_Z_LINE_LO, NONE);
	SETSOP(NONQOP, _Z_LINE_LO, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(DEC);
	PROPOUT16;
    _ES

    _NS /* add DEL_HI to Z_LINE_HI */
	ALUOP(ADDOP, P0);
	SETROP(_Z_LINE_HI, NONE);
	SETSOP(NONQOP, _Z_LINE_HI, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(DEC);
	PROPIN;
    _ES

    _NS /* add DEL_LO to COLOR_LINE_LO */
	ALUOP(ADDOP, P0);
	SETROP(_COLOR_LINE_LO, NONE);
	SETSOP(NONQOP, _COLOR_LINE_LO, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(DEC);
	PROPOUT16;
    _ES

    _NS /* add DEL_HI to COLOR_LINE_HI */
	ALUOP(ADDOP, P0);
	SETROP(_COLOR_LINE_HI, NONE);
	SETSOP(NONQOP, _COLOR_LINE_HI, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPIN;
    _ES

    _NS /* load the MAR with the address of the scratch location */
	LOADMAR(_SAVE1+7);
	CONST(_SAVE1+7);
    _ES

    _NS
	SEQ(RPCT);
	NEXT(SB_SHADED_ZBUFF_INNER_LOOP);
    _ES

    _NS /* load the MAR with the address of the trashed z_left_lo reg */
	LOADMAR(_SCR_Z_LEFT_LO_ZS_SAVE);
	CONST(_SCR_Z_LEFT_LO_ZS_SAVE);
    _ES
	
    _NS /* restore it */
	RAM(RAMRD, _Z_LEFT_LO_ZS, INC);
    _ES

    _NS /* restore it */
	RAM(RAMRD, _Z_RIGHT_HI_ZS, HOLD);
    _ES

    _NS /* return */
	REGCOMP(EQ, _I, _YVALUE);
	SEQ(RETN);
    _ES
}
