/* traploop.c
/*	start_trapezoid_loop
/*
/*
*/
#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "polydefs.h"

/*
	registers used
		_BIASED_DONE
		_LEFTCOUNT
		_RIGHTCOUNT
		_LEFT
		_RIGHT
		_YVALUE
		_DEL_HI
		_DEL_LO
		_XLEFT_HI
		_XLEFT_LO
		_XRIGHT_HI
		_XRIGHT_LO

	constants used

	scratch locations used
		_SCR_DEL  (locations)
	routines called
		SHADE_TRAP
		START_LEFT_SIDE_S_Z
		FINISH_LEFT_SIDE_S_Z
		START_RIGHT_SIDE_S_Z
		FINISH_RIGHT_SIDE_S_Z
		ADVANCE_S_Z

	exits to
		POLY_EXIT
*/

zshaded_trap_loop()
{
newfile("ztraploop.c");

label(SETUP_FOR_TRAPLOOP)
    _NS /* setup counts and save them in scratch */
	SEQ(JSUB);
	NEXT(SETUP_LEFT_RIGHT_COUNTS);
    _ES

label(START_TRAPEZOID_LOOP_S_Z)
    _NS /* load the MAR with the MULTIVIEW flag address */
	LOADMAR(_MULTIVIEW);
	CONST(_MULTIVIEW);
    _ES

    _NS 
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* do the conditional branch */
	SEQ(JUMP);
	NEXT(LEFT_AND_RIGHT_NOT_EQUAL_S_Z);
	COND(IFNZ);
    _ES

    _NS
	SEQ(JSUB);
	NEXT(ZSHADE_TRAP);
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* load the address of the flag */
	LOADMAR(_SCR_BIASED_DONE);
	CONST(_SCR_BIASED_DONE);
    _ES
    
    _NS /* test the BIASED_DONE flag for being done */
	RAM(RAMRD, _BIASED_DONE, HOLD);
    _ES

    _NS /* test done, if neg then the current polygon is complete */
	SEQ(JUMP);
	NEXT(POLY_EXIT);
	COND(IFNEG);
    _ES

    _NS /* restore _LEFT and _RIGHT, save nothing, we get new Zs */
	LOADMAR(_SCR_LEFT);
	CONST(_SCR_LEFT);
    _ES

    _NS /* read LEFT */
	RAM(RAMRD, _LEFT, INC);
    _ES

    _NS /* read RIGHT */
	RAM(RAMRD, _RIGHT, INC);
    _ES

    _NS /* call start_left_side, also load the MAR with left, to allow
	testing in this routine */
	SEQ(JSUB);
	NEXT(START_LEFT_SIDE_S_Z);
	REGREG(RONLYOP, P0, _LEFT, _LEFT);
	DOTOMAR(LOAD);
    _ES

    _NS /* test done, this was setup by START_LEFT_SIDE_S_Z, if true
	then the current polygon is complete, NOTE: this is optimized
	toward screen aligned rectangles, it costs others a state */
	SEQ(JUMP);
	NEXT(POLY_EXIT);
	COND(IFNEG);
    _ES

    _NS /* call start_right_side, also load the MAR with right, to allow
	testing in this routine */
	SEQ(JSUB);
	NEXT(START_RIGHT_SIDE_S_Z);
	REGREG(RONLYOP, P0, _RIGHT, _RIGHT);
	DOTOMAR(LOAD);
    _ES

    _NS /* load the MAR with the address of DEL_XLEFT_HI */
	LOADMAR(_SCR_DEL_XLEFT_HI_ZS);
	CONST(_SCR_DEL_XLEFT_HI_ZS);
    _ES

    _NS /* retrieve HI and then LO */
	RAM(RAMRD, _DEL_HI, INC);
    _ES

    _NS /* retrieve HI and then LO */
	RAM(RAMRD, _DEL_LO, INC);
    _ES

    _NS /* now call finish_left_side, again load MAR with left */
	SEQ(JSUB);
	NEXT(FINISH_LEFT_SIDE_S_Z);
	REGREG(RONLYOP, P0, _LEFT, _LEFT);
	DOTOMAR(LOAD);
    _ES

    _NS /* load the MAR with the address of DEL_XRIGHT_HI */
	LOADMAR(_SCR_DEL_XRIGHT_HI_ZS);
	CONST(_SCR_DEL_XRIGHT_HI_ZS);
    _ES

    _NS /* retrieve HI and then LO */
	RAM(RAMRD, _DEL_HI, INC);
    _ES

    _NS /* retrieve HI and then LO */
	RAM(RAMRD, _DEL_LO, INC);
    _ES

    _NS /* now call finish_right_side, again load MAR with right */
	SEQ(JSUB);
	NEXT(FINISH_RIGHT_SIDE_S_Z);
	REGREG(RONLYOP, P0, _RIGHT, _RIGHT);
	DOTOMAR(LOAD);
    _ES

    _NS /* lastly increment yvalue and return to the top of the loop */
       REGREG(RONLYOP, P1, _YVALUE, _YVALUE);
       SEQ(JUMP);
       NEXT(FINISH_LOOP_SETUP);
    _ES

label(LEFT_AND_RIGHT_NOT_EQUAL_S_Z)
    _NS	/* test leftcount vs. rightcount (r-l) */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* now do the conditional jump to the third case */
	SEQ(JUMP);
	NEXT(LEFT_GREATER_RIGHT_S_Z);
	COND(IFNEG);
    _ES

    _NS
	SEQ(JSUB);
	NEXT(ZSHADE_TRAP);
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* load the address of the flag */
	LOADMAR(_SCR_BIASED_DONE);
	CONST(_SCR_BIASED_DONE);
    _ES
    
    _NS /* test BIASED_DONE */
	RAM(RAMRD, _BIASED_DONE, HOLD);
    _ES

    _NS /* test done, this was setup by FILL_TRAPEZOID, if true
	then the current polygon is complete */
	SEQ(JUMP);
	NEXT(POLY_EXIT);
	COND(IFNEG);
    _ES

    _NS /* restore LEFT, RIGHT, and RIGHTCOUNT, saving RIGHT_Zs */


    /*NOTE save RIGHT_Zs */


	LOADMAR(_SCR_RIGHTCOUNT);
	CONST(_SCR_RIGHTCOUNT);
    _ES

    _NS /* read RIGHTCOUNT */
	RAM(RAMRD, _RIGHTCOUNT, INC);
    _ES

    _NS /* read LEFT */
	RAM(RAMRD, _LEFT, INC);
    _ES

    _NS /* read RIGHT into the output register */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
    _ES

    _NS /* write the register value (Z_RIGHT_LO_ZS) to scratch while loading 
	it with the new value */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16);
	LOADDI(OUTPUTREG); COND(IFFALSE); SEQ(CJPP);
	SETSOP(NONQOP, _RIGHT, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(INC);
    _ES

    _NS /* save _Z_RIGHT_HI_ZS */
	RAM(RAMWR, _Z_RIGHT_HI_ZS, HOLD);
    _ES

    _NS /* call start_left_side, also load the MAR with left, to allow
	testing in this routine */
	SEQ(JSUB);
	NEXT(START_LEFT_SIDE_S_Z);
	REGREG(RONLYOP, P0, _LEFT, _LEFT);
	DOTOMAR(LOAD);
    _ES

    _NS /* load the MAR with the address of DEL_XLEFT_HI */
	LOADMAR(_SCR_DEL_XLEFT_HI_ZS);
	CONST(_SCR_DEL_XLEFT_HI_ZS);
    _ES

    _NS /* retrieve HI and then LO */
	RAM(RAMRD, _DEL_HI, INC);
    _ES

    _NS /* retrieve HI and then LO */
	RAM(RAMRD, _DEL_LO, INC);
    _ES

    _NS /* now call finish_left_side, again load MAR with left */
	SEQ(JSUB);
	NEXT(FINISH_LEFT_SIDE_S_Z);
	REGREG(RONLYOP, P0, _LEFT, _LEFT);
	DOTOMAR(LOAD);
    _ES

    _NS
       REGREG(RONLYOP, P1, _YVALUE, _YVALUE);
       SEQ(JUMP);
       NEXT(FINISH_LOOP_SETUP);
    _ES

label(LEFT_GREATER_RIGHT_S_Z)
    _NS
	SEQ(JSUB);
	NEXT(ZSHADE_TRAP);
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* load the address of the flag */
	LOADMAR(_SCR_BIASED_DONE);
	CONST(_SCR_BIASED_DONE);
    _ES
    
    _NS /* test and restore done */
	RAM(RAMRD, _BIASED_DONE, HOLD);
    _ES

    _NS /* test done, if true then the current polygon is complete */
	SEQ(JUMP);
	NEXT(POLY_EXIT);
	COND(IFNEG);
    _ES

    _NS /* restore LEFT, RIGHT, and LEFTCOUNT, saving LEFT_Zs */
	LOADMAR(_SCR_Z_LEFT_LO_ZS); /* RIGHTCOUNT */
	CONST(_SCR_Z_LEFT_LO_ZS);
    _ES

    _NS /* write Z_LEFT_LO */
	RAM(RAMWR, _Z_LEFT_LO_ZS, INC);
    _ES

    _NS /* read LEFT into the output register */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
    _ES

    _NS /* write the register value (Z_LEFT_HI_ZS) to scratch while loading 
	it with the new value */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16);
	LOADDI(OUTPUTREG); COND(IFFALSE); SEQ(CJPP);
	SETSOP(NONQOP, _LEFT, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(INC);
    _ES

    _NS /* read RIGHT */
	RAM(RAMRD, _RIGHT, INC);
    _ES

    _NS /* read LEFTCOUNT */
	RAM(RAMRD, _LEFTCOUNT, INC);
    _ES

    _NS /* call start_right_side, also load the MAR with right, to allow
	testing in this routine */
	SEQ(JSUB);
	NEXT(START_RIGHT_SIDE_S_Z);
	REGREG(RONLYOP, P0, _RIGHT, _RIGHT);
	DOTOMAR(LOAD);
    _ES

    _NS /* load the MAR with the address of DEL_XRIGHT_HI */
	LOADMAR(_SCR_DEL_XRIGHT_HI_ZS);
	CONST(_SCR_DEL_XRIGHT_HI_ZS);
    _ES

    _NS /* retrieve HI and then LO */
	RAM(RAMRD, _DEL_HI, INC);
    _ES

    _NS /* retrieve HI and then LO */
	RAM(RAMRD, _DEL_LO, INC);
    _ES

    _NS /* now call finish_right_side, again load MAR with right */
	SEQ(JSUB);
	NEXT(FINISH_RIGHT_SIDE_S_Z);
	REGREG(RONLYOP, P0, _RIGHT, _RIGHT);
	DOTOMAR(LOAD);
    _ES

    _NS
       REGREG(RONLYOP, P1, _YVALUE, _YVALUE);
       SEQ(JUMP);
       NEXT(FINISH_LOOP_SETUP);
    _ES

/**********************************************************/
/**********************************************************/
/**********************************************************/

label(FINISH_LOOP_SETUP)
    _NS /* decrement left and right counts */
	IMMREG(SUBSROP, P1, 1, _LEFTCOUNT);
	CONST(1);
    _ES

    _NS /* decrement left and right counts */
	IMMREG(SUBSROP, P1, 1, _RIGHTCOUNT);
	CONST(1);
    _ES

    _NS /* call the routine to setup left and right counts and save
        them away */
	SEQ(JSUB);
	NEXT(SETUP_LEFT_RIGHT_COUNTS);
    _ES

    _NS /* now call advance left and right to avoid filling the same
    	scan line twice, start the increment here */
	SEQ(JSUB);
	NEXT(ADVANCE_S_Z);
	REGRAM(ADDOP, P0, _Z_RIGHT_LO_ZS, _Z_RIGHT_LO_ZS, DEC);
	PROPOUT16;
    _ES

    _NS
       SEQ(JUMP);
       NEXT(START_TRAPEZOID_LOOP_S_Z);
    _ES

/*******************************************************************/
/*******************************************************************/
/*******************************************************************/

label(SETUP_LEFT_RIGHT_COUNTS)
    _NS /* calculate the difference between left and right count */
	ALUOP(SUBSROP, P1);
	SETROP(_LEFTCOUNT, NONE);
	SETSOP(NONQOP, _RIGHTCOUNT, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
    _ES

    _NS /* conditional branch on the result loading _I with _YVALUE plus
        the lesser of leftcount and rightcount */
	SEQ(JUMP);
	COND(IFNEG);
	NEXT(RIGHT_SMALLER);
	REGREG(RONLYOP, P0, _RIGHTCOUNT, _I);
    _ES

    _NS /* load _I with _LEFTCOUNT */
	REGREG(RONLYOP, P0, _LEFTCOUNT, _I);
    _ES

    _NS /* subtract left from right */
	SEQ(JUMP);
	NEXT(SKIP_RIGHT_SMALLER);
	REGREG(SUBSROP, P1, _LEFTCOUNT, _RIGHTCOUNT);
    _ES

label(RIGHT_SMALLER)
    _NS /* subtract right from left */
	REGREG(SUBSROP, P1, _RIGHTCOUNT, _LEFTCOUNT);
    _ES

label(SKIP_RIGHT_SMALLER)
    _NS /* add _YVALUE to _I */
	REGREG(ADDOP, P0, _YVALUE, _I);
    _ES

    _NS /* now exchange left,  right, leftcount and rightcount for
	the Z left and right values */
	LOADMAR(_SCR_LEFTCOUNT);
	CONST(_SCR_LEFTCOUNT);
    _ES

    _NS /* read the z value into the output register */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
    _ES

    _NS /* write the register value (LEFTCOUNT) to scratch while loading it 
        with the new value */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16);
	LOADDI(OUTPUTREG); COND(IFFALSE); SEQ(CJPP);
	SETSOP(NONQOP, _LEFTCOUNT, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(DEC);
    _ES

    _NS /* read the z value into the output register */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
    _ES

    _NS /* write the register value (RIGHT) to scratch while loading it with
        the new value */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16);
	LOADDI(OUTPUTREG); COND(IFFALSE); SEQ(CJPP);
	SETSOP(NONQOP, _RIGHT, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(DEC);
    _ES

    _NS /* read the z value into the output register */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
    _ES

    _NS /* write the register value (LEFT) to scratch while loading 
	it with the new value */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16);
	LOADDI(OUTPUTREG); COND(IFFALSE); SEQ(CJPP);
	SETSOP(NONQOP, _LEFT, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(DEC);
    _ES

    _NS /* read the z value into the output register */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
    _ES

    _NS /* write the register value (RIGHTCOUNT) to scratch while loading 
        it with the new value */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16);
	LOADDI(OUTPUTREG); COND(IFFALSE); SEQ(CJPP);
	SETSOP(NONQOP, _RIGHTCOUNT, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(DEC); /* this decrement should back up the MAR to set
	    		up for the advance code */
    _ES

    _NS
	SEQ(RETN);
    _ES
}
