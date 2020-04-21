/* shget_first_sides.c
/*
/*
/*	shget_first_sides
/*
/*
*/
#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "fbc.mic.h"
#include "consts.h"
#include "polydefs.h"

/*
	registers used
		_BOTTOMPNT, _LEFT	==shared
		_RIGHT
		_XLEFT_HI
		_XRIGHT_HI
		_COLOR_LEFT_HI
		_COLOR_RIGHT_HI
		_YVALUE
		_DEL_XLEFT_HI
		_DEL_XRIGHT_HI
		_LEFTCOUNT
		_RIGHTCOUNT
		_BIASED_DONE
		Q
		_DELXLEFTLO
		_DELXRIGHTLO
		_DEL_COLOR_HI
		_DEL_COLOR_LO

	constants used
		_POLYSTIPADR

	scratch locations
		_LEFT_INC	NOTE: these must be in this order (adjacent)
		_RIGHT_INC
	
	routines called
		BUMP_LEFT_INDEX
		BUMP_RIGHT_INDEX
        	SWAP_SIDES
		START_LEFT_SIDE_S2D
		START_RIGHT_SIDE_S2D
		FINISH_LEFT_SIDE
		FINISH_RIGHT_SIDE
	exits to
		START_TRAPEZOID_LOOP_S2D
	    or
		through SWAP_SIDES if throwing out backfacing polygons

*/
shget_first_sides()
{
newfile("shget_first_sides.c");

label(GET_FIRST_SIDES_S2D)

    _NS	/* load the left and right increments with 4 and minus 4
	left gets -4 */
	IMMREG(RONLYOP, P0, -4, _RIGHT);
	CONST(-4);
    _ES

    _NS /* load the MAR with the address of LEFT_INC */
	LOADMAR(_LEFT_INC);
	CONST(_LEFT_INC);
    _ES

    _NS
	ALUOP(COMPROP, P1);
	SETROP(_RIGHT, NONE);
	SETSOP(NONQOP, _RIGHT, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(INC);
    _ES

    _NS	/* load the address of the bottom-most point into the left and
    	right side pointers, also load the MAR with that address */
	ALUOP(RONLYOP, P0);
	SETROP(_BOTTOMPNT, NONE);
	SETSOP(NONQOP, _RIGHT, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(LOAD);
    _ES

    /* this isn't necessary if BOTTOMPNT and LEFT share a register 
    _NS
	REGREG(RONLYOP, P0, _BOTTOMPNT, _LEFT);
    _ES
    */

    _NS /* get the x coord of the first point */
	RAM(RAMRD, _XLEFT_HI, INC);
    _ES

    _NS /* copy it into the right side */
	REGREG(RONLYOP, P0, _XLEFT_HI, _XRIGHT_HI);
    _ES

    _NS	/* get the y value of the coord */
	RAM(RAMRD, _YVALUE, INC);
    _ES

    _NS /* get the color coord of the first point */
	RAM(RAMRD, _COLOR_LEFT_HI, INC);
    _ES

    _NS /* copy it into the right side */
	REGREG(RONLYOP, P0, _COLOR_LEFT_HI, _COLOR_RIGHT_HI);
    _ES

label(LOOP_TOP_S2D)
    _NS /* load the MAR with the address of LEFT_INC */
	LOADMAR(_LEFT_INC);
	CONST(_LEFT_INC);
    _ES

    _NS	/* jump to the routine which increments the left side 
	do the increment part here, "'cause I can" */
	SEQ(JSUB);
	NEXT(BUMP_LEFT_INDEX);
	REGRAM(ADDOP, P0, _LEFT, _LEFT, LOAD);
    _ES

    _NS /* now figure the ycount for this side 
	 NOTE: the MAR is set up by BUMP'INDEX */
	REGRAM(SUBSROP, P1, _YVALUE, _LEFTCOUNT, DEC);
    _ES

    _NS /* load del_xleft_hi with the difference of the first point and
	the current */
	REGRAM(SUBSROP, P1, _XLEFT_HI, _DEL_XLEFT_HI, INC);
    _ES

    _NS /* retest  _LEFTCOUNT */
	REGREG(RONLYOP, P0, _LEFTCOUNT, _RIGHTCOUNT);
    _ES

    _NS /* _LEFTCOUNT is ready for testing, test it first 
	if it is non-zero then exit the loop, also start the test of 
	del_xleft_hi and done, this can be done by ORing them together */
	SEQ(JUMP);
	COND(IFNZ);
	NEXT(RIGHT_LOOP_ST_S2D);
	/* test done and del_xleft_hi*/
	ALUOP(IOROP,P0);
	SETROP(_DEL_XLEFT_HI, NONE);
	SETSOP(NONQOP, _BIASED_DONE, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* if the result is non zero then this should drop through */
	SEQ(JUMP);
	NEXT(LOOP_TOP_S2D);
	COND(IFZ);
    _ES

    _NS
	REGREG(RONLYOP, P0, _DEL_XLEFT_HI, _DEL_XRIGHT_HI);
    _ES

label(RIGHT_LOOP_ST_S2D)
    _NS
	REGREG(RONLYOP, P0, _BIASED_DONE, _BIASED_DONE);
    _ES

    _NS /* if done skip this stuff */
	SEQ(JUMP);
	NEXT(DIFFERING_DELTAS_TEST_S2D);
	COND(IFNZ);
    _ES

label(RIGHT_LOOP_S2D)
    _NS /* load the MAR with the address of RIGHT_INC */
	LOADMAR(_RIGHT_INC);
	CONST(_RIGHT_INC);
    _ES

    _NS	/* jump to the routine which increments the right side 
	do the increment part here, "'cause I can" */
	SEQ(JSUB);
	NEXT(BUMP_RIGHT_INDEX);
	REGRAM(ADDOP, P0, _RIGHT, _RIGHT, LOAD);
    _ES

    _NS /* now figure the ycount for this side 
	NOTE: the MAR is set up by BUMP'INDEX */
	REGRAM(SUBSROP, P1, _YVALUE, _RIGHTCOUNT, DEC);
    _ES

    _NS /* load del_xright_hi with the difference of the first point and
	the current */ 
	REGRAM(SUBSROP, P1, _XRIGHT_HI, _DEL_XRIGHT_HI, INC);
    _ES

    _NS /* retest  _RIGHTCOUNT */
	REGREG(RONLYOP, P0, _RIGHTCOUNT, _RIGHTCOUNT);
    _ES

    _NS /* _RIGHTCOUNT is ready for testing, test it first 
	if it is non-zero then exit the loop, also start the test of 
	del_xright_hi and done, this can be done by ORing them together */
	SEQ(JUMP);
	COND(IFNZ);
	NEXT(DIFFERING_DELTAS_TEST_S2D);
	/* test done and del_xright_hi*/
	ALUOP(IOROP,P0);
	SETROP(_DEL_XRIGHT_HI, NONE);
	SETSOP(NONQOP, _BIASED_DONE, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* if the result is non zero then this should drop through */
	SEQ(JUMP);
	COND(IFZ);
	NEXT(RIGHT_LOOP_S2D);
    _ES

label(DIFFERING_DELTAS_TEST_S2D)
    _NS /* check for differing delta signs, if they differ then we now
        know the "handedness" of this polygon, the signs are checked with
        an XOR of the two deltas */
	ALUOP(XOROP, P0);
	SETROP(_DEL_XLEFT_HI, NONE);
	SETSOP(NONQOP, _DEL_XRIGHT_HI, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* assume that the next test is of del_xright_hi... 
	also, branch around it if the signs are the same */
	SEQ(JUMP);
	COND(IFNNEG);
	NEXT(FLAT_SIDE_LOOP_S2D);
	/* test del_xright_hi */
	REGREG(RONLYOP, P0, _DEL_XRIGHT_HI, _DEL_XRIGHT_HI);
    _ES

    _NS /* if del_xright is less than zero then things are backwards */
       	SEQ(JSUB);
       	COND(IFNEG);
       	NEXT(SWAP_SIDES);
       	/* start the swapping of _LEFT and _RIGHT, by moving _LEFT into
       	Q */
       	ALUOP(RONLYOP, P0);
       	SETROP(_LEFT, NONE);
       	SETSOP(NONQOP, 0, RAMNOP);
       	FTOYANDQ(FF, LDQ, REGWRD);
    _ES

    _NS /* jump to the finish up code, a state could be saved by
        putting the finish code here, this makes sharing harder and the
        code even less obvious */
	SEQ(JUMP);
	NEXT(FINISH_S2D);
	/* test done and leftcount for arrival at FINISH */
	ALUOP(IOROP,P0);
	SETROP(_LEFTCOUNT, NONE);
	SETSOP(NONQOP, _BIASED_DONE, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

label(FLAT_SIDE_LOOP_S2D)
    _NS /* test leftcount, rightcount, and done, if they're all zero
        then we should loop, and get more sides, OR the two counts */ 
	ALUOP(IOROP, P0);
	SETROP(_LEFTCOUNT, NONE);
	SETSOP(NONQOP, _RIGHTCOUNT, RAMNOP);
	FTOYANDQ(FF,  LDQ, REGWRD);
    _ES

    _NS /* now OR in the done flag, we can branch around and save a
    	state */
	ALUOP(IOROP, P0);
	SETROP(_BIASED_DONE, NONE);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	/* if leftcount or rightcount are nonzero then we can branch
	now*/
	SEQ(JUMP);
	COND(IFNZ);
	NEXT(MATCHING_DELTAS_TEST_S2D);
    _ES

    _NS /* if the result of ORing in done is still zero branch back to
        the top of the loop, this is the case when there are two flat
	sides moving in the same direction from the bottom point */
	SEQ(JUMP);
	COND(IFZ);
	NEXT(LOOP_TOP_S2D);
    _ES

label(MATCHING_DELTAS_TEST_S2D)
    _NS /* test leftcount against zero */
	REGREG(RONLYOP, P0, _LEFTCOUNT, _LEFTCOUNT);
    _ES

    _NS /* if it is non zero branch, if zero test _RIGHTCOUNT for zero */
	SEQ(JUMP);
	NEXT(OTHER_COMB_S2D);
	COND(IFNZ);
	/* test rightcount */
	REGREG(RONLYOP, P0, _RIGHTCOUNT, _RIGHTCOUNT);
    _ES

    _NS /* if rightcount is also zero branch to OTHER_COMB */
	SEQ(JUMP);
	NEXT(CHECK_DELTAS_AND_COUNTS_S2D);
	COND(IFZ);
	/* test del_xleft_hi for swapping */
	ALUOP(COMPROP, P1);
	SETROP(_DEL_XLEFT_HI, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* if del_xleft is greater than zero then things are backwards */
       	SEQ(JSUB);
       	COND(IFNEG);
       	NEXT(SWAP_SIDES);
       	/* start the swapping of _LEFT and _RIGHT, by moving _LEFT into
       	Q */
       	ALUOP(RONLYOP, P0);
       	SETROP(_LEFT, NONE);
       	SETSOP(NONQOP, 0, RAMNOP);
       	FTOYANDQ(FF, LDQ, REGWRD);
    _ES

    _NS /* jump to the finish up code */
	SEQ(JUMP);
	NEXT(FINISH_S2D);
	/* test done and leftcount for arrival at FINISH */
	ALUOP(IOROP,P0);
	SETROP(_LEFTCOUNT, NONE);
	SETSOP(NONQOP, _BIASED_DONE, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

label(OTHER_COMB_S2D)
    _NS /* test rightcount against zero */
	REGREG(RONLYOP, P0, _RIGHTCOUNT, _RIGHTCOUNT);
    _ES

    _NS /* if it is non zero branch */
	SEQ(JUMP);
	NEXT(CHECK_DELTAS_AND_COUNTS_S2D);
	COND(IFNZ);
	/* test del_xright_hi for swapping */
	REGREG(RONLYOP, P0, _DEL_XRIGHT_HI, _DEL_XRIGHT_HI);
    _ES

    _NS /* if del_xright is less than zero then things are backwards */
       	SEQ(JSUB);
       	COND(IFNEG);
       	NEXT(SWAP_SIDES);
       	/* start the swapping of _LEFT and _RIGHT, by moving _LEFT into
       	Q */
       	ALUOP(RONLYOP, P0);
       	SETROP(_LEFT, NONE);
       	SETSOP(NONQOP, 0, RAMNOP);
       	FTOYANDQ(FF, LDQ, REGWRD);
    _ES

    _NS /* jump to the finish up code */
	SEQ(JUMP);
	NEXT(FINISH_S2D);
	/* test done and leftcount for arrival at FINISH */
	ALUOP(IOROP,P0);
	SETROP(_LEFTCOUNT, NONE);
	SETSOP(NONQOP, _BIASED_DONE, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

label(CHECK_DELTAS_AND_COUNTS_S2D)
    _NS /* test to see if the deltas and counts are the same, if they
        are then the sides have the same slope, new ones are needed */
	REGCOMP(EQ, _LEFTCOUNT, _RIGHTCOUNT);
    _ES

    _NS /* conditionally exit the loop and test the deltas */
	SEQ(JUMP);
	NEXT(COMPUTE_SLOPES_S2D);
	COND(IFNQ);
	/* test the deltas */
	REGCOMP(EQ, _DEL_XLEFT_HI, _DEL_XRIGHT_HI);
    _ES

    _NS /* conditionally exit the loop and test done */
	SEQ(JUMP);
	NEXT(COMPUTE_SLOPES_S2D);
	COND(IFNQ);
	/* test done */
	REGREG(RONLYOP, P0, _BIASED_DONE, _BIASED_DONE);
    _ES

    _NS /* conditionally jump to the top of the loop or fall through to
	the code that computes and compares slopes */
	SEQ(JUMP);
	NEXT(LOOP_TOP_S2D);
	COND(IFZ);
    _ES

label(COMPUTE_SLOPES_S2D)
    _NS /* check leftcount */
	REGREG(RONLYOP, P0, _LEFTCOUNT, _LEFTCOUNT);
    _ES

    _NS /* if its nonzero do the divide, otherwise skip this and check
	rightcount */
	SEQ(JUMP);
	NEXT(RIGHT_SLOPE_S2D);
	COND(IFZ);
	/* zero the low del_xleft register */
	REGREG(FLOWOP, P0, 0, _DELXLEFTLO);
    _ES

    /*====================================*/
    /* compute the actual left slope here */
    /*====================================*/
    _NS /* load the MAR with the "right" address */
	LOADMAR(_SCR_DEL_XLEFT_HI-2);
	CONST(_SCR_DEL_XLEFT_HI-2);
	DOTOOUTREG;
    _ES

    _NS
	RAM(RAMWR, _DEL_XLEFT_HI, INC);
	SEQ(JSUB);
	NEXT(COMPUTE_XLEFT_SLOPE_S2D);
    _ES

label(RIGHT_SLOPE_S2D)
    _NS /* check rightcount */
	REGREG(RONLYOP, P0, _RIGHTCOUNT, _RIGHTCOUNT);
    _ES

    _NS /* if its nonzero do the divide, otherwise skip this and check
	rightcount */
	SEQ(JUMP);
	NEXT(CHECK_SLOPES_S2D);
	COND(IFZ);
	/* zero the low del_xright register */
	REGREG(FLOWOP, P0, 0, _DELXRIGHTLO);
    _ES

    /*====================================*/
    /* compute the actual right slope here */
    /*====================================*/
    _NS /* load the MAR with the "right" address */
	LOADMAR(_SCR_DEL_XRIGHT_HI-2);
	CONST(_SCR_DEL_XRIGHT_HI-2);
	DOTOOUTREG;
    _ES

    _NS
	RAM(RAMWR, _DEL_XRIGHT_HI, INC);
	SEQ(JSUB);
	NEXT(COMPUTE_XRIGHT_SLOPE_S2D);
    _ES

label(CHECK_SLOPES_S2D)
    _NS	/* first check to see if they are not equal, NOTE: both the  hi
    	and low halves of the words may have to be tested, the order of
	the test assumes that differences are more likely in the low
	half, the actual order of the two tests is probably not
	important */
	REGCOMP(EQ, _DELXLEFTLO, _DELXRIGHTLO);
    _ES

    _NS /* now branch if they are not equal,  also test the high halves
    	just in case*/
	SEQ(JUMP);
	NEXT(SLOPES_UNEQUAL_S2D);
	COND(IFNQ);
	/* test low halves */
	REGCOMP(EQ, _DEL_XLEFT_HI, _DEL_XRIGHT_HI);
    _ES

    _NS	/* check the low half now */
	REGREG(RONLYOP, P0, _BIASED_DONE, _BIASED_DONE);
	SEQ(JUMP);
	NEXT(SLOPES_EQUAL_S2D);
	COND(IFEQ);
    _ES

label(SLOPES_UNEQUAL_S2D)
    _NS	/* compare for del_xleft >  del_xright */
	REGCOMP(GT, _DELXRIGHTLO, _DELXLEFTLO);
	PROPOUT16;
    _ES

    _NS /* now do the other half */
	REGCOMP(GT, _DEL_XRIGHT_HI, _DEL_XLEFT_HI);
	PROPIN;
    _ES

    _NS /* branch to figure color slope if they don't need swapping */
	SEQ(JUMP);
	NEXT(FIGURE_COLOR_SLOPE_S2D);
	COND(IFGT);
    _ES

    _NS	/* swap the sides */
        SEQ(JSUB);
        NEXT(SWAP_SIDES);
       	/* start the swapping of _LEFT and _RIGHT, by moving _LEFT into
       	Q */
       	ALUOP(RONLYOP, P0);
       	SETROP(_LEFT, NONE);
       	SETSOP(NONQOP, 0, RAMNOP);
       	FTOYANDQ(FF, LDQ, REGWRD);
    _ES

    _NS /* load the MAR with the address of the scratch area */
	LOADMAR(_SCR_DEL_XLEFT_HI);
	CONST(_SCR_DEL_XLEFT_HI);
    _ES

    _NS /* write out all the deltas into the appropriate locations */
	RAM(RAMWR, _DEL_XLEFT_HI, INC);
    _ES

    _NS 
	RAM(RAMWR, _DELXRIGHTLO, INC);
    _ES

    _NS /* load the MAR with the address of the scratch area */
	LOADMAR(_SCR_DEL_XRIGHT_HI);
	CONST(_SCR_DEL_XRIGHT_HI);
    _ES

    _NS 
	RAM(RAMWR, _DEL_XRIGHT_HI, INC);
    _ES

    _NS 
	RAM(RAMWR, _DELXLEFTLO, INC);
	SEQ(JUMP);
	NEXT(FIGURE_COLOR_SLOPE_S2D);
    _ES

    /*
    _NS  swap del_xleft_lo and del_xright_lo
	load del_xleft_lo into Q
	ALUOP(RONLYOP, P0);
	SETROP(_DELXLEFTLO, NONE);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
    _ES

    _NS  load right into left
	REGREG(RONLYOP, P0, _DELXRIGHTLO, _DELXLEFTLO);
    _ES

    _NS  load Q into right 
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, _DELXRIGHTLO, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	SEQ(JUMP);
	NEXT(FIGURE_COLOR_SLOPE_S2D);
    _ES
    */

label(SLOPES_EQUAL_S2D)
    _NS /* the test of done has already been made */
	SEQ(JUMP);
	NEXT(SKIP_LOOP_TOP_S2D);
	COND(IFNEG);
	SETROP(_CORNER1, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(LOAD);
    _ES

    _NS /* load the MAR with the address of _SAVE1 and restore _DEL_XRIGHT_HI */
	LOADMAR(_SCR_DEL_XRIGHT_HI-2);
	CONST(_SCR_DEL_XRIGHT_HI-2);
    _ES

    _NS /* read the value */
	RAM(RAMRD, _DEL_XRIGHT_HI, HOLD);
	SEQ(JUMP);
	NEXT(LOOP_TOP_S2D);
    _ES

label(SKIP_LOOP_TOP_S2D)
    _NS /* get the x coord of the first point */
	RAM(RAMRD, _XLEFT_HI, INC);
    _ES

    _NS /* get the y coord of the first point */
	RAM(RAMRD, _YVALUE, INC);
    _ES

    _NS /* get the color of the first point */
	RAM(RAMRD, _COLOR_LEFT_HI, HOLD);
    _ES

    _NS /* load the address of the x values */
	ALUOP(RONLYOP, P0);
	SETROP(_CORNER2, NONE);
	SETSOP(NONQOP, _LEFT, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(LOAD);
    _ES

    _NS
	REGHOLD;
	DOTOMAR(INC);
    _ES

    _NS /* figure the count */
	REGRAM(SUBSROP, P1, _YVALUE, _LEFTCOUNT, DEC);
    _ES

    /* copy to the right side */
    _NS REGREG(RONLYOP, P0, _XLEFT_HI, _XRIGHT_HI); _ES
    _NS REGREG(RONLYOP, P0, _COLOR_LEFT_HI, _COLOR_RIGHT_HI); _ES
    _NS REGREG(RONLYOP, P0, _LEFT, _RIGHT); _ES
    _NS REGREG(RONLYOP, P0, _LEFTCOUNT, _RIGHTCOUNT); _ES
    
    _NS
	SEQ(JUMP);
	NEXT(FIGURE_COLOR_SLOPE_S2D);
	COND(IFNZ);
    _ES

    _NS
	RAM(RAMRD, _XRIGHT_HI, INC);
    _ES

    _NS 
	REGHOLD;
	DOTOMAR(INC);
    _ES

    _NS /* copy it to _XRIGHT_HI */
	RAM(RAMRD, _COLOR_RIGHT_HI, HOLD);
    _ES

    _NS 
	LOADMAR(_MULTIVIEW);
	CONST(_MULTIVIEW);
    _ES

    _NS /* or in the multiview bit */
	REGRAM(IOROP, P0, _BIASED_DONE, _BIASED_DONE, HOLD);
    _ES

    _NS
	LOADMAR(_SCR_BIASED_DONE);
	CONST(_SCR_BIASED_DONE);
    _ES

    _NS
	RAM(RAMWR, _BIASED_DONE, HOLD);
	SEQ(JUMP);
	NEXT(SETUP_MODE_AND_STIPPLE);
    _ES


/**************************************************************************
/**************************************************************************/

label(FINISH_S2D)
    _NS	/* do the (!done & !leftcount) test, NOTE: all code jumping
        here has set up the test */
	SEQ(JUMP);
	NEXT(NOT_FLAT_LEFT_S2D);
	COND(IFNZ);
	/* set up test to make sure the side isn't flat, the done flag
	could have been set */ 
	REGREG(RONLYOP, P0, _LEFTCOUNT, _LEFTCOUNT);
    _ES

    _NS /* jump to START_LEFT_SIDE, also load the MAR with the left, to
	set up for the testing which must take place */
	SEQ(JSUB);
	NEXT(START_LEFT_SIDE_S2D);
	REGREG(RONLYOP, P0, _LEFT, _LEFT);
	DOTOMAR(LOAD);
    _ES

    _NS /* jump around the test and divide code */
	SEQ(JUMP);
	NEXT(TEST_RIGHT_SIDE_S2D);
	/* setup the test for the right side */
	ALUOP(IOROP,P0);
	SETROP( _BIASED_DONE, NONE);
	SETSOP( NONQOP, _RIGHTCOUNT, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

label(NOT_FLAT_LEFT_S2D)
    _NS /* make sure leftcount is non-zero then do the divide */
	SEQ(JUMP);
	NEXT(TEST_RIGHT_SIDE_S2D);
	COND(IFZ);
	/* setup the test for the right side */
	ALUOP(IOROP,P0);
	SETROP( _BIASED_DONE, NONE);
	SETSOP( NONQOP, _RIGHTCOUNT, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    /*====================================*/
    /*  compute the actual left slope here, 
    /*	this must also compute the delta color			
    /*====================================*/

    _NS /* figure DEL_COLOR_HI 
	load the MAR with the address of the current left color */
	ALUOP(ADDOP, P0);
	SETROP(0, ALL16);
	LOADDI(UCONST);
	CONST(2);
	SETSOP(NONQOP, _LEFT, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(LOAD);
    _ES

    _NS /* move DEL_X_HI into Q */
	ALUOP(RONLYOP, P0);
	SETROP(_DEL_XLEFT_HI, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
    _ES

    _NS
	REGRAM(SUBSROP, P1, _COLOR_LEFT_HI, _DEL_HI, HOLD);
	SEQ(JSUB);
	NEXT(COMPUTE_LEFT_SLOPE_S2D);
    _ES

    _NS
	/* setup the test for the right side */
	ALUOP(IOROP,P0);
	SETROP( _BIASED_DONE, NONE);
	SETSOP( NONQOP, _RIGHTCOUNT, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

label(TEST_RIGHT_SIDE_S2D)
    _NS	/* do the (!done & !rightcount) test, NOTE: all code jumping
        here has set up the test */
	SEQ(JUMP);
	NEXT(NOT_FLAT_RIGHT_S2D);
	COND(IFNZ);
	/* set up test to make sure the side isn't flat, the done flag
	could have been set */ 
	REGREG(RONLYOP, P0, _RIGHTCOUNT, _RIGHTCOUNT);
    _ES

    _NS /* jump to START_RIGHT_SIDE, also load the MAR with right, to
	set up for the testing which must take place */
	SEQ(JSUB);
	NEXT(START_RIGHT_SIDE_S2D);
	REGREG(RONLYOP, P0, _RIGHT, _RIGHT);
	DOTOMAR(LOAD);
    _ES

    _NS /* jump around the test and divide code */
	SEQ(JUMP);
	NEXT(SECOND_FINISH_S2D);
    _ES

label(NOT_FLAT_RIGHT_S2D)
    _NS /* make sure rightcount is non-zero then do the divide */
	SEQ(JUMP);
	NEXT(SECOND_FINISH_S2D);
	COND(IFZ);
    _ES

    /*====================================*/
    /* compute the actual right slope here */
    /*====================================*/
    _NS /* figure DEL_COLOR_HI 
	load the MAR with the address of the current left color */
	ALUOP(ADDOP, P0);
	SETROP(0, ALL16);
	LOADDI(UCONST);
	CONST(2);
	SETSOP(NONQOP, _RIGHT, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(LOAD);
    _ES

    _NS /* move DEL_X_HI into Q */
	ALUOP(RONLYOP, P0);
	SETROP(_DEL_XRIGHT_HI, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
    _ES

    _NS
	REGRAM(SUBSROP, P1, _COLOR_RIGHT_HI, _DEL_HI, HOLD);
	SEQ(JSUB);
	NEXT(COMPUTE_RIGHT_SLOPE_S2D);
    _ES

label(SECOND_FINISH_S2D)
    _NS 
	LOADMAR(_MULTIVIEW);
	CONST(_MULTIVIEW);
    _ES

    _NS /* or in the multiview bit */
	REGRAM(IOROP, P0, _BIASED_DONE, _BIASED_DONE, HOLD);
    _ES

    _NS
	LOADMAR(_SCR_BIASED_DONE);
	CONST(_SCR_BIASED_DONE);
    _ES

    _NS	/* write the flag to scratch and load the MAR with a new address */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16); LOADDI(UCONST); CONST(_SCR_DEL_XLEFT_HI);
	SETSOP(NONQOP, _BIASED_DONE, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(LOAD);
    _ES

    _NS
	RAM(RAMRD, _DEL_HI, INC);
    _ES

    _NS
	RAM(RAMRD, _DEL_LO, INC);
    _ES

    _NS /* finish off the two sides, loading the MAR for use in the
        routine */
	SEQ(JSUB);
	NEXT(FINISH_LEFT_SIDE_S2D);
	REGREG(RONLYOP, P0, _LEFT, _LEFT);
	DOTOMAR(LOAD);
    _ES

    _NS
	LOADMAR(_SCR_DEL_XRIGHT_HI);
	CONST(_SCR_DEL_XRIGHT_HI);
    _ES

    _NS
	RAM(RAMRD, _DEL_HI, INC);
    _ES

    _NS
	RAM(RAMRD, _DEL_LO, INC);
    _ES

    _NS 
	SEQ(JSUB);
	NEXT(FINISH_RIGHT_SIDE_S2D);
	REGREG(RONLYOP, P0, _RIGHT, _RIGHT);
	DOTOMAR(LOAD);
    _ES

label(SETUP_MODE_AND_STIPPLE)
    _NS /* load the MAR with the address of the polygon stipple pattern,
	syncronize */
	LOADMAR(_POLYSTIPADR);
	CONST(_POLYSTIPADR);
    _ES

    _NS /* read polystipadr first to set flag */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(NOOP);		/* syncronize */
    _ES

    _NS /* send polystipadr to the BPC and jump on value */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADFA);
	COND(IFNZ);		/* if _POLYSTIPADR != 0, set AllPattern Bit */
	SEQ(JUMP);
	NEXT(ALL_PATTERN);
    _ES

/* _POLYSTIPADR == 0, no AllPattern Bit */
    _NS /* load the address of the CONFIG register */
	LOADMAR(_CONFIG+1);
	CONST(_CONFIG+1);
    _ES

    _NS /* force the line stipple bit on, also force the write increment on
    	CD write to the read configuration */
	ALUOP(IOROP, P0);
	SETROP(0, ALL16); LOADDI(UCONST); CONST(0x180);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, LDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADCONFIG);
    _ES

    _NS 
	SEQ(JUMP);
	NEXT(END_ALL_PATTERN);
	REGHOLD;
    _ES

label(ALL_PATTERN)
/* _POLYSTIPADR != 0, set AllPattern Bit */
    _NS /* load the address of the CONFIG register */
	LOADMAR(_CONFIG+1);
	CONST(_CONFIG+1);
    _ES

    _NS /* force the line stipple bit on, also force the write increment on
    	CD write */
	ALUOP(IOROP, P0);
	SETROP(0, ALL16); LOADDI(UCONST); CONST(0x2180);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, LDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADCONFIG);
    _ES

label(END_ALL_PATTERN)
    _NS 
	LOADMAR(_CONFIG);	/* point to mode register */
	CONST(_CONFIG);
    _ES

    _NS /* force the depth cue bit to 1 */
	ALUOP(IOROP, P0);
	SETROP(0, ALL16); LOADDI(UCONST); CONST(0x4);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADMODE);
    _ES

    _NS /* load a line stipple of all ones */
	ALUOP(FHIGHOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(NOOP);
    _ES

    _NS /* force the load stipple bit to 0 */
	ALUOP(ANDOP, P0);
	SETROP(0, ALL16);
	LOADDI(UCONST);
	CONST(~0x80);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADCONFIG);
    _ES

    _NS /* zero error delta and error correction zero right lo too */
	ALUOP(FLOWOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADED);
    _ES

    _NS /* zero error delta and error correction */
	ALUOP(FLOWOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADEC);
	SEQ(JUMP);
	NEXT(START_TRAPEZOID_LOOP_S2D);
    _ES

/************************************************************************/
/************************************************************************/
/************************************************************************/

label(COMPUTE_XRIGHT_SLOPE_S2D)
    _NS /* load the denominator */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _RIGHTCOUNT, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	SEQ(JSUB);
	NEXT(DIVIDE16);
    _ES

    _NS /* retrieve the fractional part */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _DELXRIGHTLO, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(DEC);
    _ES

    _NS /* retrieve the integer part */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _DEL_XRIGHT_HI, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
	SEQ(RETN);
    _ES

/************************************************************************/
/************************************************************************/
/************************************************************************/

label(COMPUTE_XLEFT_SLOPE_S2D)
    _NS /* load the denominator */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _LEFTCOUNT, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	SEQ(JSUB);
	NEXT(DIVIDE16);
    _ES

    _NS /* retrieve the fractional part */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _DELXLEFTLO, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(DEC);
    _ES

    _NS /* retrieve the integer part */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _DEL_XLEFT_HI, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
	SEQ(RETN);
    _ES

label(FIGURE_COLOR_SLOPE_S2D)
    _NS /* figure DEL_COLOR_HI 
	load the MAR with the address of the current left color */
	ALUOP(ADDOP, P0);
	SETROP(0, ALL16);
	LOADDI(UCONST);
	CONST(2);
	SETSOP(NONQOP, _LEFT, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(LOAD);
    _ES

    _NS /* figure the del slope */
	REGRAM(SUBSROP, P1, _COLOR_LEFT_HI, _DEL_HI, HOLD);
    _ES

    _NS /* load the MAR with the address of the SAVE1 area */
	LOADMAR(_SAVE1);
	CONST(_SAVE1);
	DOTOOUTREG;
    _ES

    _NS /* load the values into scratch */
	RAM(RAMWR, _DEL_HI, INC);
    _ES

    _NS /* load the values into scratch */
	RAM(RAMWR, _LEFTCOUNT, HOLD);
	SEQ(JSUB);
	NEXT(DIVIDE16);
    _ES

    _NS /* retrieve the fraction */
	RAM(RAMRD, _DEL_LO, DEC);
    _ES

    _NS /* retrieve the integer */
	RAM(RAMRD, _DEL_HI, HOLD);
    _ES

    _NS /* load the MAR with the destination address */
	LOADMAR(_SCR_DEL_COLOR_LEFT_HI);
	CONST(_SCR_DEL_COLOR_LEFT_HI);
    _ES

    _NS /* write the integer */
	RAM(RAMWR, _DEL_HI, INC);
    _ES

    _NS /* write the fraction */
	RAM(RAMWR, _DEL_LO, HOLD);
    _ES

    _NS /* figure DEL_COLOR_HI 
	load the MAR with the address of the current right color */
	ALUOP(ADDOP, P0);
	SETROP(0, ALL16);
	LOADDI(UCONST);
	CONST(2);
	SETSOP(NONQOP, _RIGHT, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(LOAD);
    _ES

    _NS /* figure the del slope */
	REGRAM(SUBSROP, P1, _COLOR_RIGHT_HI, _DEL_HI, HOLD);
    _ES

    _NS /* load the MAR with the address of the SAVE1 area */
	LOADMAR(_SAVE1);
	CONST(_SAVE1);
	DOTOOUTREG;
    _ES

    _NS /* load the values into scratch */
	RAM(RAMWR, _DEL_HI, INC);
    _ES

    _NS /* load the values into scratch */
	RAM(RAMWR, _RIGHTCOUNT, HOLD);
	SEQ(JSUB);
	NEXT(DIVIDE16);
    _ES

    _NS /* retrieve the fraction */
	RAM(RAMRD, _DEL_LO, DEC);
    _ES

    _NS /* retrieve the integer */
	RAM(RAMRD, _DEL_HI, HOLD);
    _ES

    _NS /* load the MAR with the destination address */
	LOADMAR(_SCR_DEL_COLOR_RIGHT_HI);
	CONST(_SCR_DEL_COLOR_RIGHT_HI);
    _ES

    _NS /* write the integer */
	RAM(RAMWR, _DEL_HI, INC);
    _ES

    _NS /* write the fraction */
	RAM(RAMWR, _DEL_LO, HOLD);
	SEQ(JUMP);
	NEXT(SECOND_FINISH_S2D);
    _ES
}
