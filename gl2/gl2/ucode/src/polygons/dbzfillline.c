#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"
#include "polydefs.h"

zbuff()
{
newfile("zbuff.c");

label(DB_FLAT_SHADED_ZBUFF)
    _NS /* now check the config to see what I'm displaying */
	ALUOP(ANDOP, P0);
	SETROP(0, ALL16); LOADDI(UCONST); CONST(1);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* now branch to the buffer of choice */
	SEQ(JUMP);
	NEXT(DB_FLAT_SHADED_ZBUFF_A);
	COND(IFNZ);
    _ES

    _NS /* point at the free ram location */
	LOADMAR(_SAVE1+5);
	CONST(_SAVE1+5);
    _ES

    _NS /* load the "right" config */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16); LOADDI(UCONST); CONST(0x2016);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADCONFIG);
    _ES

label(DB_FLAT_SHADED_ZBUFF_B)
    _NS /* request the Z value while writing the high value out to scratch */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _Z_LINE_HI, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	BPCCMD(READPIXELAB);
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
	NEXT(OVERFLOW_Z_F_B);
    _ES

    _NS /* read _Z_LINE_HI back and increment the MAR to get the color */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _Z_LINE_HI, RAMRD);
	DOTOMAR(INC);
	FTOYANDQ(FF, OLDQ, REGWRE);
	SEQ(JUMP);
	COND(IFNEG);
	NEXT(FAST_SKIP_Z_DRAW_B);
    _ES

label(DRAW_Z_F_B)
    _NS /* load the "right" config */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16); LOADDI(UCONST); CONST(0x211a);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADCONFIG);
    _ES

    _NS /* write the pixel value */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(DRAWPIXELAB);
    _ES

    _NS /* load the "right" config */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16); LOADDI(UCONST); CONST(0x2016);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADCONFIG);
    _ES

    _NS /* write the Z value */
	REGREG(RONLYOP, P0, _Z_LINE_HI, _Z_LINE_HI);
	BPCCMD(DRAWPIXELAB);
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
	NEXT(FAST_SKIP_X_INCREMENT_B);
    _ES

label(OVERFLOW_Z_F_B)
    _NS /* reverse the sense of the test */
	/* read back Z_LINE_HI, increment MAR to the color */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _Z_LINE_HI, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(INC);
	SEQ(JUMP);
	NEXT(DRAW_Z_F_B);
	COND(IFNEG);
    _ES

label(FAST_SKIP_Z_DRAW_B)
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

label(FAST_SKIP_X_INCREMENT_B)
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
	NEXT(DB_FLAT_SHADED_ZBUFF_B)
    _ES

    _NS /* return */
	REGCOMP(EQ, _I, _YVALUE);
	SEQ(RETN);
    _ES

label(DB_FLAT_SHADED_ZBUFF_A)
    _NS /* point at the free ram location */
	LOADMAR(_SAVE1+5);
	CONST(_SAVE1+5);
    _ES

    _NS /* load the "right" config */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16); LOADDI(UCONST); CONST(0x2019);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADCONFIG);
    _ES

label(DB_FLAT_SHADED_ZBUFF_A_LOOP)
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
	NEXT(OVERFLOW_Z_F_A);
    _ES

    _NS /* read _Z_LINE_HI back and increment the MAR to get the color */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _Z_LINE_HI, RAMRD);
	DOTOMAR(INC);
	FTOYANDQ(FF, OLDQ, REGWRE);
	SEQ(JUMP);
	COND(IFNEG);
	NEXT(FAST_SKIP_Z_DRAW_A);
    _ES

label(DRAW_Z_F_A)
    _NS /* load the "right" config */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16); LOADDI(UCONST); CONST(0x2115);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADCONFIG);
    _ES

    _NS /* write the pixel value */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(DRAWPIXELAB);
    _ES

    _NS /* load the "right" config */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16); LOADDI(UCONST); CONST(0x2019);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADCONFIG);
    _ES

    _NS /* write the Z value */
	REGREG(RONLYOP, P0, _Z_LINE_HI, _Z_LINE_HI);
	BPCCMD(DRAWPIXELAB);
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
	NEXT(FAST_SKIP_X_INCREMENT_A);
    _ES

label(OVERFLOW_Z_F_A)
    _NS /* reverse the sense of the test */
	/* read back Z_LINE_HI, increment MAR to the color */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _Z_LINE_HI, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(INC);
	SEQ(JUMP);
	NEXT(DRAW_Z_F_A);
	COND(IFNEG);
    _ES

label(FAST_SKIP_Z_DRAW_A)
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

label(FAST_SKIP_X_INCREMENT_A)
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
	NEXT(DB_FLAT_SHADED_ZBUFF_A_LOOP);
    _ES

    _NS /* return */
	REGCOMP(EQ, _I, _YVALUE);
	SEQ(RETN);
    _ES

}
