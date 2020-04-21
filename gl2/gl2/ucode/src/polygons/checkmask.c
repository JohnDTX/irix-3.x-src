/* checkmask.c
/*	check_masking
/*
/*
*/
#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "fbc.mic.h"
#include "consts.h"
#include "polydefs.h"
/* registers used
	CHECK_MASKING
		_LEFTCOUNT,
		_RIGHTCOUNT,
		_MIN_MASK,
		_MAX_MASK

   constants used
	CHECK_MASKING
		_CHARVIEW
*/

check_masking()
{
newfile("checkmask.c");

label(CHECK_MASKING)
    _NS /* load the MAR with the address of the current screen mask */
	LOADMAR(_CHARVIEW);
	CONST(_CHARVIEW);
    _ES
     
    _NS /* test this against the current MIN_MASK */
	REGRAMCOMP(LE, _MIN_MASK, INC);
    _ES

    _NS /* if the current left mask is less than the MIN_MASK, set XS
        to the new mask value */
	SEQ(JUMP);
	NEXT(NO_NEW_MIN_MASK);
	COND(IFLE);
	DOTOMAR(INC);
    _ES

    _NS /* set XS */
	REGREG(RONLYOP, P0, _MIN_MASK, _MIN_MASK);
	DOTOOUTREG;
	BPCCMD(LOADXS);
    _ES

    _NS /* test this against the current MAX_MASK */
	REGRAMCOMP(GE, _MAX_MASK, HOLD);
    _ES

    _NS /* if the current right mask is greater than the MAX_MASK, set XE
        to the new mask value */
	SEQ(JUMP);
	NEXT(NO_NEW_MAX_MASK);
	COND(IFGE);
    _ES

    _NS /* set XE */
	REGREG(RONLYOP, P0, _MAX_MASK, _MAX_MASK);
	DOTOOUTREG;
	BPCCMD(LOADXE);
	SEQ(JUMP);
	NEXT(NEW_MASKS);
    _ES

label(NO_NEW_MAX_MASK)
    _NS /* read the mask and load it */
	ALUOP(SONLYOP, P0);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADXE);
	SEQ(JUMP);
	NEXT(NEW_MASKS);
    _ES

label(NO_NEW_MIN_MASK)
    _NS /* test this against the current MAX_MASK */
	REGRAMCOMP(GE, _MAX_MASK, DEC);
    _ES

    _NS /* if the current right mask is greater than the MAX_MASK, set XE
        to the new mask value */
	DOTOMAR(DEC);
	SEQ(RETN);
	COND(IFGE);
	/* do this test for use on return */
	REGCOMP(EQ, _LEFTCOUNT, _RIGHTCOUNT);
    _ES

    _NS /* set XE */
	REGREG(RONLYOP, P0, _MAX_MASK, _MAX_MASK);
	DOTOOUTREG;
	BPCCMD(LOADXE);
    _ES

    _NS /* read the mask and load it */
	ALUOP(SONLYOP, P0);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADXS);
    _ES

label(NEW_MASKS)
    _NS /* issue the SETSRCMASKX command */
	SEQ(RETN);
	BPCCMD(SETSCRMASKX);
	DOTOOUTREG;
	/* do this test for use on return */
	REGCOMP(EQ, _LEFTCOUNT, _RIGHTCOUNT);
    _ES

/******************
******************/

label(CHECK_MASKING_S2D)
    _NS /* restore MIN_MASK and MAX_MASK */
	RAM(RAMRD, _MIN_MASK, INC);
    _ES

    _NS /* restore MIN_MASK and MAX_MASK */
	RAM(RAMRD, _MAX_MASK, INC);
    _ES

    _NS /* load the MAR with the address of the current screen mask */
	LOADMAR(_CHARVIEW);
	CONST(_CHARVIEW);
    _ES
     
    _NS /* test this against the current MIN_MASK */
	REGRAMCOMP(LE, _MIN_MASK, INC);
    _ES

    _NS /* if the current left mask is less than the MIN_MASK, set XS
        to the new mask value */
	SEQ(JUMP);
	NEXT(NO_NEW_MIN_MASK_S2D);
	COND(IFLE);
	DOTOMAR(INC);
    _ES

    _NS /* set XS */
	REGREG(RONLYOP, P0, _MIN_MASK, _MIN_MASK);
	DOTOOUTREG;
	BPCCMD(LOADXS);
    _ES

label(NO_NEW_MIN_MASK_S2D)
    _NS /* NOTE: this code assumes that nothing interesting has
        happened between the NEWMASK call and here, meaning the value of XS
        doesn't need to be reloaded unless it changes */
        /* test this against the current MAX_MASK */
	REGRAMCOMP(GE, _MAX_MASK, HOLD);
    _ES

    _NS /* if the current right mask is greater than the MAX_MASK, set XE
        to the new mask value */
	SEQ(JUMP);
	NEXT(NO_NEW_MAX_MASK_S2D);
	COND(IFGE);
    _ES

    _NS /* set XE */
	REGREG(RONLYOP, P0, _MAX_MASK, _MAX_MASK);
	DOTOOUTREG;
	BPCCMD(LOADXE);
    _ES

label(NO_NEW_MAX_MASK_S2D)
    _NS /* issue the SETSRCMASKX command */
	SEQ(RETN);
	BPCCMD(SETSCRMASKX);
	DOTOOUTREG;
    _ES

/********************************************************/
/********************************************************/
/********************************************************/

label(CHECK_MASKING_S_Z)
    _NS /* restore MIN_MASK and MAX_MASK */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	DOTOMAR(INC);
    _ES

    _NS /* restore MIN_MASK and MAX_MASK */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, LDQ, REGWRD);
	DOTOMAR(INC);
    _ES

    _NS /* load the MAR with the address of the current screen mask */
	LOADMAR(_CHARVIEW);
	CONST(_CHARVIEW);
    _ES
     
    _NS /* test this against the current MIN_MASK */
	ALUOP(SUBSROP, P1);
	SETROP(0, ALL16);
	LOADDI(OUTPUTREG); COND(IFFALSE); SEQ(CJPP);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(INC);
    _ES

    _NS /* if the current left mask is less than the MIN_MASK, set XS
        to the new mask value */
	SEQ(JUMP);
	NEXT(NO_NEW_MIN_MASK_S_Z);
	COND(IFNNEG);
	DOTOMAR(INC);
    _ES

    _NS /* set XS */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16);
	LOADDI(OUTPUTREG); COND(IFFALSE); SEQ(CJPP);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADXS);
    _ES

label(NO_NEW_MIN_MASK_S_Z)
    _NS /* transfer MAX_MASK into the OUTPUTREG from the Q reg */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
    _ES

    _NS /* NOTE: this code assumes that nothing interesting has
        happened between the NEWMASK call and here, meaning the value of XS
        doesn't need to be reloaded unless it changes */
        /* test this against the current MAX_MASK */
	ALUOP(SUBRSOP, P1);
	SETROP(0, ALL16);
	LOADDI(OUTPUTREG); COND(IFFALSE); SEQ(CJPP);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* if the current right mask is greater than the MAX_MASK, set XE
        to the new mask value */
	SEQ(JUMP);
	NEXT(NO_NEW_MAX_MASK_S_Z);
	COND(IFNNEG);
    _ES

    _NS /* set XS */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16);
	LOADDI(OUTPUTREG); COND(IFFALSE); SEQ(CJPP);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADXE);
    _ES

label(NO_NEW_MAX_MASK_S_Z)
    _NS /* issue the SETSRCMASKX command */
	SEQ(RETN);
	BPCCMD(SETSCRMASKX);
	DOTOOUTREG;
    _ES
}
