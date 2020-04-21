
/*	shade_trapezoid
 *
 *	shade fills the current trapezoid for each different screen mask
 *
 */
#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"
#include "polydefs.h"

/* registers used
	SHADE_TRAPEZOID
		_XRIGHT_HI,
		_XLEFT_HI,
		_COLOR_RIGHT_HI,
		_COLOR_RIGHT_LO,
		_COLOR_LEFT_HI,
		_COLOR_LEFT_LO,
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
	_SCR_DEL_COLOR_LEFT_HI 		consecutive scratch locations
	_SCR_DEL_COLOR_LEFT_LO		there must also be two dummy
	_SCR_DEL_XRIGHT_HI 		locations before SCR_DEL_XLEFT_HI
	_SCR_DEL_XRIGHT_LO
	_SCR_DEL_COLOR_RIGHT_HI
	_SCR_DEL_COLOR_RIGHT_LO
    
    routines called
	ADVANCE_S2D
	SHADE_LINE
	FAST_SHADE_LINE
*/

/* *********************NOTE *************
	error delta and error correction should have been zeroed early
	on, like in get_first_sides, with poly stippling code */

shade_trapezoid()
{
newfile("shadetrap.c");

label(SHADE_TRAP)
    _NS /* store the end y address in _I */
	ALUOP(ADDOP, P0);
	SETROP(_YVALUE, NONE);
	SETSOP(QOPERAND, _I, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
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
	NEXT(FAST_SHADE_TRAP);
	COND(IFZ);
    _ES

    _NS /* complement it */
	REGREG(COMPROP, P1, _I, _I);
    _ES

label(SHADE_TRAP_LOOP_TOP)
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
	/* abort this polygon if we have a negative length */
	/* **NOTE** a some point in the future this could be nicer and
	swap the two sides, recovering from a bow tie like situation
	*/
	SEQ(JUMP);
	COND(IFNEG);
	NEXT(ABORT_POLYGON);
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

    _NS /* test the count */
	REGCOMP(EQ, _XLEFT_HI, _XRIGHT_HI);
    _ES

    _NS /* if length is not zero go ahead and to the divide */
	SEQ(JUMP);
	COND(IFEQ);
	NEXT(NO_LINE_DIVIDE);
	/* move the low delta into the parameter area */
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
	double word result with 12 bit fraction */
	SEQ(JSUB);
	NEXT(LONGDIVIDE);
    _ES

    _NS /* retrieve the delta values */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _DEL_LO, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(DEC);
    _ES

    _NS /* get the high part of the delta color and syncronize */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _DEL_HI, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
	SEQ(JSUB);
	NEXT(RESET_MASKS);
    _ES

    _NS	
	ALUOP(RONLYOP, P0);
	SETROP(_COLOR_LEFT_LO, NONE);
	SETSOP(NONQOP, _COLOR_LEFT_LO, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	/* merge with the no divide  case */
	SEQ(JUMP);
	NEXT(SKIP_SETUP);
    _ES

label(NO_LINE_DIVIDE)
    /* load those values which don't have to be reloaded in the mask loop */

    _NS /* set up the first screen mask, and syncronize, also load the
        16-bit fractional start color into Q */
	ALUOP(RONLYOP, P0);
	SETROP(_COLOR_LEFT_LO, NONE);
	SETSOP(NONQOP, _COLOR_LEFT_LO, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	SEQ(JSUB);
	NEXT(RESET_MASKS);
    _ES

label(SKIP_SETUP)
    _NS	/* write the delta DDA values to the engine */
	ALUOP(RONLYOP, P0); 
	SETROP(_DEL_HI, NONE);
	SETSOP(NONQOP, _DEL_HI, RAMNOP);
	/* right shift Q moving it toward being a 12-bit fraction */
	FTOYANDQ(FF, QR, REGWRD);
	BPCCMD(LOADSDI);
	DOTOOUTREG;
    _ES

    _NS	/* write the delta DDA values to the engine */
	ALUOP(RONLYOP, P0); 
	SETROP(_DEL_LO, NONE);
	SETSOP(NONQOP, _DEL_LO, RAMNOP);
	/* right shift Q moving it toward being a 12-bit fraction,
	it should now be 14-bits */
	FTOYANDQ(FF, QR, REGWRD);
	BPCCMD(LOADSDF);
	DOTOOUTREG;
    _ES

    _NS /* test I for the first or last line of a trapezoid */
	ALUOP(COMPROP, P1);
	SETROP(_I, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	/* right shift Q moving it toward being a 12-bit fraction,
	it should now be 13-bits */
	FTOYANDQ(FF, QR, REGWRD);
    _ES

    _NS /* test for last line of trap */
	ALUOP(SUBSROP, P1);
	SETROP(_I, NONE);
	SETSOP(NONQOP, _YVALUE, RAMNOP);
	/* right shift Q moving it toward being a 12-bit fraction,
	it should now be 12-bits */
	FTOYANDQ(FF, QR, REGWRD);
	SEQ(JUMP);
	COND(IFNNEG);
	NEXT(SHADE_LINE);
    _ES

    _NS
	SEQ(JUMP);
	COND(IFNEG);
	NEXT(FAST_SHADE_LINE);
	/* also load the start color */
	REGREG(RONLYOP, P0, _XLEFT_HI, _XLEFT_HI);
	DOTOOUTREG;
	BPCCMD(LOADXS);
    _ES

    _NS
	SEQ(JUMP);
	NEXT(SHADE_LINE);
    _ES

label(SHADE_LINE_RETURN)
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
	LOADMAR(_SCR_DEL_COLOR_RIGHT_LO);
	CONST(_SCR_DEL_COLOR_RIGHT_LO);
    _ES

    _NS /* now update all the deltas */
	REGRAM(ADDOP, P0, _COLOR_RIGHT_LO, _COLOR_RIGHT_LO, DEC);
	PROPOUT16;
	SEQ(JSUB);
	NEXT(ADVANCE_S2D);
    _ES

    _NS
	SEQ(JUMP);
	NEXT(SHADE_TRAP_LOOP_TOP);
	REGREG(RONLYOP, P1, _YVALUE, _YVALUE);
    _ES

/********************************************************************/
/********************************************************************/
/********************************************************************/

label(FAST_SHADE_TRAP)
    _NS /* set up the correct screen mask */
	SEQ(JSUB);
	NEXT(RESET_MASKS);
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

label(FAST_SHADE_TRAP_LOOP_TOP)
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
	/* abort this polygon if we have a negative length */
	/* **NOTE** a some point in the future this could be nicer and
	swap the two sides, recovering from a bow tie like situation
	*/
	SEQ(JUMP);
	COND(IFNEG);
	NEXT(ABORT_POLYGON);
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

    _NS /* test the count */
	REGCOMP(EQ, _XLEFT_HI, _XRIGHT_HI);
    _ES

    _NS /* if length is not zero go ahead and to the divide */
	SEQ(JUMP);
	COND(IFEQ);
	NEXT(FAST_NO_LINE_DIVIDE);
	/* move the low delta into the parameter area */
	ALUOP(RONLYOP, P0);
	SETROP(_DEL_LO, NONE);
	SETSOP(NONQOP, _DEL_LO, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(DEC);
    _ES

    _NS /* load the integer part of the delta color into scratch */
	ALUOP(RONLYOP, P0);
	SETROP(_DEL_HI, NONE);
	SETSOP(NONQOP, _DEL_HI, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	/* call the divide routine, this routine hopefully takes
	double word numerators with 16 bit fraction and produces a
	double word result with 12 bit fraction */
	SEQ(JSUB);
	NEXT(LONGDIVIDE);
    _ES

    _NS /* retrieve the delta values */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _DEL_LO, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(DEC);
    _ES

    _NS /* get the high part of the delta color */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _DEL_HI, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
    _ES

label(FAST_NO_LINE_DIVIDE)
    _NS	
	ALUOP(RONLYOP, P0);
	SETROP(_COLOR_LEFT_LO, NONE);
	SETSOP(NONQOP, _COLOR_LEFT_LO, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(NOOP);
    _ES

    _NS	/* write the delta DDA values to the engine */
	ALUOP(RONLYOP, P0); 
	SETROP(_DEL_HI, NONE);
	SETSOP(NONQOP, _DEL_HI, RAMNOP);
	/* right shift Q moving it toward being a 12-bit fraction 
	15 bits */
	FTOYANDQ(FF, QR, REGWRD);
	BPCCMD(LOADSDI);
	DOTOOUTREG;
    _ES

    _NS	/* write the delta DDA values to the engine */
	ALUOP(RONLYOP, P0); 
	SETROP(_DEL_LO, NONE);
	SETSOP(NONQOP, _DEL_LO, RAMNOP);
	/* right shift Q moving it toward being a 12-bit fraction,
	it should now be 14-bits */
	FTOYANDQ(FF, QR, REGWRD);
	BPCCMD(LOADSDF);
	DOTOOUTREG;
    _ES

    _NS	/* load the XS register */
	ALUOP(RONLYOP, P0); 
	SETROP(_XLEFT_HI, NONE);
	SETSOP(NONQOP, _XLEFT_HI, RAMNOP);
	/* right shift Q moving it toward being a 12-bit fraction,
	it should now be 13-bits */
	FTOYANDQ(FF, QR, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADXS);
    _ES

    _NS /* load the XE register */
	ALUOP(RONLYOP, P0); 
	SETROP(_XRIGHT_HI, NONE);
	SETSOP(NONQOP, _XRIGHT_HI, RAMNOP);
	/* right shift Q moving it toward being a 12-bit fraction,
	it should now be 12-bits */
	FTOYANDQ(FF, QR, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADXE);
    _ES

    _NS /* load the DDA start value register (frac), this comes from Q */
	ALUOP(ADDOP, P0);
	LOADDI(UCONST);
	CONST(0x800);
	PROPOUT12;
	SETROP(0, ALL16);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADSAF);
    _ES

    _NS /* load the DDA start value register (int) */
	ALUOP(RONLYOP, P0); 
	PROPIN;
	SETROP(_COLOR_LEFT_HI, NONE);
	SETSOP(NONQOP, _COLOR_LEFT_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADSAI);
    _ES

    _NS /* load the YS register */
	REGREG(RONLYOP, P0, _YVALUE, _YVALUE);
	DOTOOUTREG;
	BPCCMD(LOADYS);
    _ES

    _NS /* add one half to Q to gen the carry */
	ALUOP(ADDOP, P0);
	LOADDI(UCONST);
	CONST(0x800);
	PROPOUT12;
	SETROP(0, ALL16);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* draw the scan line, dump out the start color so this is right */
	ALUOP(RONLYOP, P0);
	SETROP(_COLOR_LEFT_HI, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	PROPIN;
	DOTOOUTREG;
	BPCCMD(OCT1VECT);
    _ES

    _NS /* test _I */
	REGCOMP(EQ, _I, _YVALUE);
    _ES

    _NS /* conditionally return from here */
	SEQ(RETN);
	COND(IFEQ);
    _ES

    _NS /* load the MAR with the address of the various delta values */
	LOADMAR(_SCR_DEL_COLOR_RIGHT_LO);
	CONST(_SCR_DEL_COLOR_RIGHT_LO);
    _ES

    _NS /* now update all the deltas */
	REGRAM(ADDOP, P0, _COLOR_RIGHT_LO, _COLOR_RIGHT_LO, DEC);
	PROPOUT16;
	SEQ(JSUB);
	NEXT(ADVANCE_S2D);
    _ES

    _NS
	SEQ(JUMP);
	NEXT(FAST_SHADE_TRAP_LOOP_TOP);
	REGREG(RONLYOP, P1, _YVALUE, _YVALUE);
    _ES

/******************************************************/
/******************************************************/
/******************************************************/

label(ABORT_POLYGON)
    _NS /* set the done flag and return */
	LOADMAR(_SCR_BIASED_DONE);
	CONST(_SCR_BIASED_DONE);
    _ES

    _NS /* set the done flag */
	ALUOP(IOROP, P0);
	SETROP(0, ALL16);
	LOADDI(UCONST);
	CONST(0x8000);
	SETSOP(NONQOP, _BIASED_DONE, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(HOLD);
    _ES

    _NS
	RAM(RAMWR, _BIASED_DONE, HOLD);
	SEQ(RETN);
    _ES

    }
