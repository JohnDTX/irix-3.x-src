
/* computeslope.c
/*	computeslope
/*
*/
#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "polydefs.h"

/* registers used

	COMPUTE_LEFT_SLOPE_S2D
		_DEL_HI		holds DEL_COLOR_HI on entry
		_DEL_LO		holds DEL_Z_HI on entry
		Q		holds DEL_X_HI on entry
		_LEFTCOUNT

	COMPUTE_RIGHT_SLOPE_S2D
		_DEL_HI		holds DEL_COLOR_HI on entry
		_DEL_LO		holds DEL_Z_HI on entry
		Q		holds DEL_X_HI on entry
		_RIGHTCOUNT

    constants used
		none
    
    scratch locations used
		<dummy>
		<dummy>
		_SCR_DEL_XLEFT_HI
		_SCR_DEL_XLEFT_LO
		_SCR_DEL_COLOR_LEFT_HI
		_SCR_DEL_COLOR_LEFT_LO
		_SCR_DEL_XRIGHT_HI
		_SCR_DEL_XRIGHT_LO
		_SCR_DEL_COLOR_RIGHT_HI
		_SCR_DEL_COLOR_RIGHT_LO

*/

computeslope()
{
newfile("computeslope.c");

label(COMPUTE_LEFT_SLOPE_S2D)
    _NS /* test LEFTCOUNT */
       REGREG(RONLYOP, P0, _LEFTCOUNT, _LEFTCOUNT);
    _ES

    _NS /* (return) branch around divide if zero denom */
	SEQ(RETN);
	COND(IFZ);
    _ES

    _NS /* load the MAR with the address */
	LOADMAR(_SCR_DEL_XLEFT_HI);
	CONST(_SCR_DEL_XLEFT_HI);
	DOTOOUTREG;
    _ES

    _NS /* load DEL_COLOR_HI, and transfer DEL_X to a regular register 
	RAM(RAMWR,  _DEL_HI, INC); */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, _DEL_HI, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(INC);
    _ES

    _NS /* write the count */
	RAM(RAMWR, _LEFTCOUNT, HOLD);
	SEQ(JSUB);
	NEXT(DIVIDE16);
    _ES

    _NS /* load the MAR with the address */
	LOADMAR(_SCR_DEL_XLEFT_HI-2);
	CONST(_SCR_DEL_XLEFT_HI-2);
	DOTOOUTREG;
    _ES

    _NS /* load DEL_X_HI */
	RAM(RAMWR,  _DEL_HI, INC);
    _ES

    _NS /* write the count, and jump to divide so it returns where I
    	want */
	RAM(RAMWR, _LEFTCOUNT, HOLD);
	SEQ(JUMP);
	NEXT(DIVIDE16);
    _ES

/****************************************************************/
/****************************************************************/
/****************************************************************/

label(COMPUTE_RIGHT_SLOPE_S2D)
    _NS /* test RIGHTCOUNT */
       REGREG(RONLYOP, P0, _RIGHTCOUNT, _RIGHTCOUNT);
    _ES

    _NS /* (return) branch around divide if zero denom */
	SEQ(RETN);
	COND(IFZ);
    _ES

    _NS /* load the MAR with the address */
	LOADMAR(_SCR_DEL_XRIGHT_HI);
	CONST(_SCR_DEL_XRIGHT_HI);
	DOTOOUTREG;
    _ES

    _NS /* load DEL_COLOR_HI, and transfer DEL_X_HI */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, _DEL_HI, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(INC);
    _ES

    _NS /* write the count, and divide */
	RAM(RAMWR, _RIGHTCOUNT, HOLD);
	SEQ(JSUB);
	NEXT(DIVIDE16);
    _ES

    _NS /* load the MAR with the address */
	LOADMAR(_SAVE1);
	CONST(_SAVE1);
	DOTOOUTREG;
    _ES

    _NS /* load DEL_X_HI */
	RAM(RAMWR,  _DEL_HI, INC);
    _ES

    _NS /* write the count, and divide */
	RAM(RAMWR, _RIGHTCOUNT, HOLD);
	SEQ(JSUB);
	NEXT(DIVIDE16);
    _ES

    _NS /* retrieve the x fraction */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF,  LDQ, REGWRD);
	DOTOMAR(DEC);
    _ES

    _NS /* retrieve the x integer part */
	RAM(RAMRD, _DEL_HI, HOLD);
    _ES

    _NS /* load the MAR */
       LOADMAR(_SCR_DEL_XRIGHT_HI);
       CONST(_SCR_DEL_XRIGHT_HI);
    _ES

    _NS /* now write the two values into scratch where they belong */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, _DEL_HI, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(INC);
    _ES

    _NS /* now write the two fractions into scratch where they belong */
	RAM(RAMWR, _DEL_HI, HOLD);
	SEQ(RETN);
    _ES

/********************************************************************/
/********************************************************************/
/********************************************************************/

label(COMPUTE_LEFT_SLOPE_S_Z)
  /*_NS  test LEFTCOUNT **NOTE: done in call 
       REGREG(RONLYOP, P0, _LEFTCOUNT, _LEFTCOUNT);
    _ES */

    _NS /* (return) branch around divide if zero denom */
	SEQ(RETN);
	COND(IFZ);
    _ES

    _NS /* load the MAR with the address */
	LOADMAR(_SCR_DEL_Z_LEFT_HI_ZS-2);
	CONST(_SCR_DEL_Z_LEFT_HI_ZS-2);
	DOTOOUTREG;
    _ES

    _NS /* load DEL_Z_HI, and transfer DEL_X to a regular register */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, _DEL_LO, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(INC);
    _ES

    _NS /* write the count */
	RAM(RAMWR, _LEFTCOUNT, HOLD);
	SEQ(JSUB);
	NEXT(DIVIDE16);
    _ES

    _NS /* load the MAR with the address */
	LOADMAR(_SCR_DEL_COLOR_LEFT_HI_ZS-2);
	CONST(_SCR_DEL_COLOR_LEFT_HI_ZS-2);
	DOTOOUTREG;
    _ES

    _NS /* load DEL_COLOR_HI */
	RAM(RAMWR,  _DEL_HI, INC);
    _ES

    _NS /* write the count */
	RAM(RAMWR, _LEFTCOUNT, HOLD);
	SEQ(JSUB);
	NEXT(DIVIDE16);
    _ES

    _NS /* load the MAR with the address */
	LOADMAR(_SCR_DEL_XLEFT_HI_ZS-2);
	CONST(_SCR_DEL_XLEFT_HI_ZS-2);
	DOTOOUTREG;
    _ES

    _NS /* load DEL_X_HI */
	RAM(RAMWR,  _DEL_LO, INC);
    _ES

    _NS /* write the count, and jump to divide so it returns where I
    	want */
	RAM(RAMWR, _LEFTCOUNT, HOLD);
	SEQ(JUMP);
	NEXT(DIVIDE16);
    _ES

/****************************************************************/
/****************************************************************/
/****************************************************************/

label(COMPUTE_RIGHT_SLOPE_S_Z)
  /*_NS test RIGHTCOUNT **NOTE: done in call
       REGREG(RONLYOP, P0, _RIGHTCOUNT, _RIGHTCOUNT);
    _ES */

    _NS /* (return) branch around divide if zero denom */
	SEQ(RETN);
	COND(IFZ);
    _ES

    _NS /* load the MAR with the address */
	LOADMAR(_SCR_DEL_Z_RIGHT_HI_ZS-2);
	CONST(_SCR_DEL_Z_RIGHT_HI_ZS-2);
	DOTOOUTREG;
    _ES

    _NS /* load DEL_Z_HI, and transfer DEL_X_HI */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, _DEL_LO, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(INC);
    _ES

    _NS /* write the count, and divide */
	RAM(RAMWR, _RIGHTCOUNT, HOLD);
	SEQ(JSUB);
	NEXT(DIVIDE16);
    _ES

    _NS /* load the MAR with the address */
	LOADMAR(_SCR_DEL_COLOR_RIGHT_HI_ZS-2);
	CONST(_SCR_DEL_COLOR_RIGHT_HI_ZS-2);
	DOTOOUTREG;
    _ES

    _NS /* load DEL_COLOR_HI */
	RAM(RAMWR,  _DEL_HI, INC);
    _ES

    _NS /* write the count, and divide */
	RAM(RAMWR, _RIGHTCOUNT, HOLD);
	SEQ(JSUB);
	NEXT(DIVIDE16);
    _ES

    _NS /* load the MAR with the address */
	LOADMAR(_SAVE1);
	CONST(_SAVE1);
	DOTOOUTREG;
    _ES

    _NS /* load DEL_X_HI */
	RAM(RAMWR,  _DEL_LO, INC);
    _ES

    _NS /* write the count, and divide */
	RAM(RAMWR, _RIGHTCOUNT, HOLD);
	SEQ(JSUB);
	NEXT(DIVIDE16);
    _ES

    _NS /* retrieve the x fraction */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF,  LDQ, REGWRD);
	DOTOMAR(DEC);
    _ES

    _NS /* retrieve the x integer part */
	RAM(RAMRD, _DEL_HI, HOLD);
    _ES

    _NS /* load the MAR */
       LOADMAR(_SCR_DEL_XRIGHT_HI_ZS);
       CONST(_SCR_DEL_XRIGHT_HI_ZS);
    _ES

    _NS /* now write the two values into scratch where they belong */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, _DEL_HI, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(INC);
    _ES

    _NS /* now write the fraction into scratch where they belong */
	RAM(RAMWR, _DEL_HI, HOLD);
	SEQ(RETN);
    _ES
}
