/*	zfill_trap
 *
 *	z fills the current trapezoid for each different screen mask
 *
 */
#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"
#include "polydefs.h"

/* registers used
	ZFILL_TRAP
		_XRIGHT_HI,
		_XLEFT_HI,
		_Z_RIGHT_HI,
		_Z_RIGHT_LO,
		_Z_LEFT_HI,
		_Z_LEFT_LO,
		_DEL_HI,
		_DEL_LO,
		_YVALUE,
		Q,		holds the count of lines to fill
		_I,
		_BIASED_DONE	shared with _I

   constants used
		none
    
    scratch locations used
	<dummy>
	<dummy>
	_SCR_DEL_XLEFT_HI
	_SCR_DEL_XLEFT_LO	NOTE: these should be in this order in
	_SCR_DEL_Z_LEFT_HI 		consecutive scratch locations
	_SCR_DEL_Z_LEFT_LO		there must also be two dummy
	_SCR_DEL_XRIGHT_HI 		locations before SCR_DEL_XLEFT_HI
	_SCR_DEL_XRIGHT_LO
	_SCR_DEL_Z_RIGHT_HI
	_SCR_DEL_Z_RIGHT_LO
    
    routines called
	ADVANCE_S2D
	ZFILL_LINE
	FAST_ZFILL_LINE
*/

/* *********************NOTE *************
	error delta and error correction should have been zeroed early
	on, like in get_first_sides, with poly stippling code */

zfill_trap()
{
newfile("zfilltrap.c");

label(ZFILL_TRAP)
    _NS /* get the AB color from scratch */
	LOADMAR(_COLOR+1);
	CONST(_COLOR+1);
    _ES

    _NS /* read the color */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _I, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE); 
    _ES

    _NS
	LOADMAR(_SAVE1+6);
	CONST(_SAVE1+6);
    _ES

    _NS /* store the end y address in _I, while writing the color to
    	scratch */
	ALUOP(ADDOP, P0);
	SETROP(_YVALUE, NONE);
	SETSOP(QOPERAND, _I, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(INC);
    _ES

    _NS /* load the MAR with the address of the single screen mask flag */
	LOADMAR(_MULTIVIEW);
	CONST(_MULTIVIEW);
    _ES

    _NS /* test the flag */
	ALUOP(SONLYOP,  P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS
	SEQ(JUMP);
	NEXT(FAST_ZFILL_TRAP);
	COND(IFZ);
    _ES

    _NS /* complement it */
	REGREG(COMPROP, P1, _I, _I);
    _ES

label(ZFILL_TRAP_LOOP_TOP)
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

    _NS /* move Z_LEFT_HI into DEL_HI, and write the count */
	ALUOP(RONLYOP, P0);
	SETROP(_Z_LEFT_HI, NONE);
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

    _NS /* move Z_LEFT_LO into DEL_LO */
	REGREG(RONLYOP, P0, _Z_LEFT_LO, _DEL_LO);
    _ES

    _NS
	ALUOP(SUBRSOP, P1);
	SETROP(_Z_RIGHT_LO, NONE);
	SETSOP(NONQOP, _DEL_LO, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPOUT16;
    _ES

    _NS 
	ALUOP(SUBRSOP, P1);
	SETROP(_Z_RIGHT_HI, NONE);
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
	NEXT(Z_NO_LINE_DIVIDE);
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
	double word result with 12 bit fraction */
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
	NEXT(Z_SKIP_SETUP);
    _ES

label(Z_NO_LINE_DIVIDE)
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
	LOADMAR(_SAVE1+5);
	CONST(_SAVE1+5);
    _ES

    _NS /* figure the length and load it into the output register */
	ALUOP(SUBSROP, P1);
	SETROP(_XLEFT_HI, NONE);
	SETSOP(NONQOP, _XRIGHT_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
    _ES

label(Z_SKIP_SETUP)
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
	NEXT(ZFILL_LINE);
    _ES

    _NS
	REGREG(RONLYOP, P0, _Z_LEFT_LO, _Z_LINE_LO);
	SEQ(JUMP);
	COND(IFNEG);
	NEXT(FAST_ZFILL_LINE);
    _ES

    _NS
	SEQ(JUMP);
	NEXT(ZFILL_LINE);
    _ES

label(ZFILL_LINE_RETURN)
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
	LOADMAR(_SCR_DEL_Z_RIGHT_LO);
	CONST(_SCR_DEL_Z_RIGHT_LO);
    _ES

    _NS /* now update all the deltas */
	REGRAM(ADDOP, P0, _Z_RIGHT_LO, _Z_RIGHT_LO, DEC);
	PROPOUT16;
	SEQ(JSUB);
	NEXT(ADVANCE_S2D);
    _ES

    _NS
	SEQ(JUMP);
	NEXT(ZFILL_TRAP_LOOP_TOP);
	REGREG(RONLYOP, P1, _YVALUE, _YVALUE);
    _ES

/********************************************************************/
/********************************************************************/
/********************************************************************/

label(FAST_ZFILL_TRAP)
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
	NEXT(CHECK_MASKING_S2D);
	COND(IFNZ);
    _ES

label(FAST_ZFILL_TRAP_LOOP_TOP)
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

    _NS /* move Z_LEFT_HI into DEL_HI, and write the count */
	ALUOP(RONLYOP, P0);
	SETROP(_Z_LEFT_HI, NONE);
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

    _NS /* move Z_LEFT_LO into DEL_LO */
	REGREG(RONLYOP, P0, _Z_LEFT_LO, _DEL_LO);
    _ES

    _NS
	ALUOP(SUBRSOP, P1);
	SETROP(_Z_RIGHT_LO, NONE);
	SETSOP(NONQOP, _DEL_LO, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	PROPOUT16;
    _ES

    _NS 
	ALUOP(SUBRSOP, P1);
	SETROP(_Z_RIGHT_HI, NONE);
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
	NEXT(Z_FAST_NO_LINE_DIVIDE);
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

#ifdef NOTDEF
    _NS /* skip around the MAR setup */
	SEQ(JUMP);
	NEXT(SKIP_Z_FAST_NO_LINE_DIVIDE);
	DOTOMAR(INC);
    _ES
#endif

label(Z_FAST_NO_LINE_DIVIDE)

#ifdef NOTDEF
    _NS /* load the MAR with the address of the color */
	LOADMAR(_SAVE1+5);
	CONST(_SAVE1+5);
    _ES
#endif

label(SKIP_Z_FAST_NO_LINE_DIVIDE)
    _NS /* figure the count loading it into the OUTPUT register */
	ALUOP(SUBSROP, P1);
	SETROP(_XLEFT_HI, NONE);
	SETSOP(NONQOP, _XRIGHT_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
    _ES

    _NS /* load the start z fraction */
	REGREG(RONLYOP, P0, _Z_LEFT_LO, _Z_LINE_LO);
	LOADDI(OUTPUTCOUNT);
	SEQ(LDCT);
    _ES

    _NS /* add in a bias value */
	IMMREG(ADDOP, P0, 0x8000, _Z_LINE_LO);
	CONST(0x8000);
	PROPOUT16;
    _ES

    _NS /* load the start z value */
	REGREG(RONLYOP, P0, _Z_LEFT_HI, _Z_LINE_HI);
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
	NEXT(FAST_Z_LINE_LOOP_TOP);
	DOTOOUTREG;
	BPCCMD(SETADDRS);
    _ES

    _NS /* conditionally return from here */
	SEQ(RETN);
	COND(IFEQ);
    _ES

    _NS /* load the MAR with the address of the various delta values */
	LOADMAR(_SCR_DEL_Z_RIGHT_LO);
	CONST(_SCR_DEL_Z_RIGHT_LO);
    _ES

    _NS /* now update all the deltas */
	REGRAM(ADDOP, P0, _Z_RIGHT_LO, _Z_RIGHT_LO, DEC);
	PROPOUT16;
	SEQ(JSUB);
	NEXT(ADVANCE_S2D);
    _ES

    _NS
	SEQ(JUMP);
	NEXT(FAST_ZFILL_TRAP_LOOP_TOP);
	REGREG(RONLYOP, P1, _YVALUE, _YVALUE);
    _ES

label(FAST_Z_LINE_LOOP_TOP)
    /* at this point XR and YR have been loaded and XS, YS, XE, and YE
    are loaded with the extremes of the line, scratch holds the
    delta Z values, _Z_LINE_HI and _Z_LINE_LO hold the start z values for 
    the scan line, Q holds the X start address and the hardware 
    counter has been loaded with a count equal to the number of pixels in the
    line. The color used for writing the AB planes is stored in scratch
    at the location _SAVE1+6, the location following the divide result.
	
	for X_ADDR = XLEFT_HI to XRIGHT_HI 
	    {
	    if pixel z vs. Z_LINE_HI => visible
		{
		load XS with X_ADDR
		SETADDRS
		DrawPixelAB (COLOR in _SAVE1+6)
		DrawPixelCD (Z_LINE_HI)
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

    _NS /* read the mode to see if we're double buffered */
	ALUOP(ANDOP, P0);
	SETROP(0, ALL16); LOADDI(UCONST); CONST(2);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(INC);/* point to config */
    _ES

    _NS /* jump to a double buffered handler */
	SEQ(JUMP);
	NEXT(DB_FLAT_SHADED_ZBUFF);
	COND(IFNZ);
    _ES

    _NS
	LOADMAR(_SAVE1+5);
	CONST(_SAVE1+5);
    _ES

label(SB_FLAT_SHADED_ZBUFF_INNER_LOOP)
    _NS /* request the Z value while writing the high value out to scratch */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _Z_LINE_HI, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	BPCCMD(READPIXELCD);
	DOTOOUTREG;
    _ES

    _NS /* read and compare pixel z with Z_LINE_HI, the result must be stored 
	in _Z_LINE_HI, */
	ALUOP(SUBSROP, P1);
	SETROP(0, ALL16);
	READBPCBUS;
	LOADDI(BPCDATA);
	SETSOP(NONQOP, _Z_LINE_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOOUTREG; COND(IFFALSE); SEQ(CJPP);
    _ES

    _NS /* check for overflow and retest the compare for the visibility test */
	ALUOP(COMPSOP, P1);
	SETROP(0, NONE);
	SETSOP(NONQOP, _Z_LINE_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	SEQ(JUMP);
	COND(IFOVF); /* check for overflow */
	NEXT(OVERFLOW_Z_F);
    _ES

    _NS /* read _Z_LINE_HI back and increment the MAR to get the color */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _Z_LINE_HI, RAMRD);
	DOTOMAR(INC);
	FTOYANDQ(FF, OLDQ, REGWRE);
	SEQ(JUMP);
	COND(IFNEG);
	NEXT(FAST_SKIP_Z_DRAW);
    _ES

label(DRAW_Z_F)
    _NS /* write the pixel value */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(DRAWPIXELAB);
    _ES

    _NS /* write the Z value */
	REGREG(RONLYOP, P0, _Z_LINE_HI, _Z_LINE_HI);
	BPCCMD(DRAWPIXELCD);
	DOTOOUTREG;
    _ES

    _NS /* increment Q along to the next X address, also back up the
        MAR to point at the fractional  Z */
	ALUOP(SONLYOP, P1);
	SETROP(0, NONE);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	DOTOMAR(DEC);
	SEQ(JUMP);
	NEXT(FAST_SKIP_X_INCREMENT);
    _ES

label(OVERFLOW_Z_F)
    _NS /* reverse the sense of the test */
	/* read back Z_LINE_HI, increment MAR to the color */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _Z_LINE_HI, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(INC);
	SEQ(JUMP);
	NEXT(DRAW_Z_F);
	COND(IFNEG);
    _ES

label(FAST_SKIP_Z_DRAW)
    _NS /* increment Q along to the next X address, also back up the
        MAR to point at the fractional  Z */
	ALUOP(SONLYOP, P1);
	SETROP(0, NONE);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	DOTOMAR(DEC);
    _ES

    _NS /* load XS with the incremented Q */
	ALUOP(SONLYOP, P0);
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

label(FAST_SKIP_X_INCREMENT)
    _NS /* decrement the MAR to point at DEL_LO */
	REGHOLD;
	DOTOMAR(DEC);
    _ES

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
	DOTOMAR(INC);
	PROPIN;
    _ES

    _NS
	DOTOMAR(INC);
	SEQ(RPCT);
	NEXT(SB_FLAT_SHADED_ZBUFF_INNER_LOOP);
    _ES

    _NS /* return */
	REGCOMP(EQ, _I, _YVALUE);
	SEQ(RETN);
    _ES

}
