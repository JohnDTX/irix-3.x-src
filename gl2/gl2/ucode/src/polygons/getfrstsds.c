/* get_first_sides.c
/*
/*
/*	get_first_sides
/*
/*
*/
#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "polydefs.h"
#include "fbc.mic.h"

/*
	registers used
		_BOTTOMPNT, _LEFT	==shared
		_RIGHT
		_XLEFT_HI
		_XRIGHT_HI
		_YVALUE
		_DEL_XLEFT_HI
		_DEL_XRIGHT_HI
		_LEFTCOUNT
		_RIGHTCOUNT
		_BIASED_DONE
		Q
		_DEL_XLEFT_LO
		_DEL_XRIGHT_LO

	constants used
		_POLYSTIPADR

	scratch locations
		_LEFT_INC	NOTE: these must be in this order (adjacent)
		_RIGHT_INC
	
	routines called
		BUMP_LEFT_INDEX
		BUMP_RIGHT_INDEX
        	SWAP_SIDES
		START_LEFT_SIDE_F2D
		START_RIGHT_SIDE_F2D
		FINISH_LEFT_SIDE
		FINISH_RIGHT_SIDE
	exits to
		START_TRAPEZOID_LOOP_F2D
	    or
		through SWAP_SIDES if throwing out backfacing polygons

*/
get_first_sides()
{
newfile("get_first_sides.c");

label(GET_FIRST_SIDES_F2D)

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
	RAM(RAMRD, _YVALUE, HOLD);
    _ES

label(LOOP_TOP_F2D)
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
	NEXT(RIGHT_LOOP_ST_F2D);
	/* test done and del_xleft_hi*/
	ALUOP(IOROP,P0);
	SETROP(_DEL_XLEFT_HI, NONE);
	SETSOP(NONQOP, _BIASED_DONE, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* if the result is non zero then this should drop through */
	SEQ(JUMP);
	COND(IFZ);
	NEXT(LOOP_TOP_F2D);
    _ES

    _NS
	REGREG(RONLYOP, P0, _DEL_XLEFT_HI, _DEL_XRIGHT_HI);
    _ES

label(RIGHT_LOOP_ST_F2D)
    _NS
	REGREG(RONLYOP, P0, _BIASED_DONE, _BIASED_DONE);
    _ES

    _NS
	SEQ(JUMP);
	NEXT(DIFFERING_DELTAS_TEST_F2D);
	COND(IFNZ);
    _ES

label(RIGHT_LOOP_F2D)
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
	NEXT(DIFFERING_DELTAS_TEST_F2D);
	/* test done and del_xright_hi*/
	ALUOP(IOROP,P0);
	SETROP(_DEL_XRIGHT_HI, NONE);
	SETSOP(NONQOP, _BIASED_DONE, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* if the result is non zero then this should drop through */
	SEQ(JUMP);
	COND(IFZ);
	NEXT(RIGHT_LOOP_F2D);
    _ES

label(DIFFERING_DELTAS_TEST_F2D)
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
	NEXT(FLAT_SIDE_LOOP_F2D);
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
	NEXT(FINISH_F2D);
	/* test done and leftcount for arrival at FINISH */
	ALUOP(IOROP,P0);
	SETROP(_LEFTCOUNT, NONE);
	SETSOP(NONQOP, _BIASED_DONE, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

label(FLAT_SIDE_LOOP_F2D)
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
	NEXT(MATCHING_DELTAS_TEST_F2D);
    _ES

    _NS /* if the result of ORing in done is still zero branch back to
        the top of the loop, this is the case when there are two flat
	sides moving in the same direction from the bottom point */
	SEQ(JUMP);
	COND(IFZ);
	NEXT(LOOP_TOP_F2D);
    _ES

label(MATCHING_DELTAS_TEST_F2D)
    _NS /* test leftcount against zero */
	REGREG(RONLYOP, P0, _LEFTCOUNT, _LEFTCOUNT);
    _ES

    _NS /* if it is non zero branch, if zero test _RIGHTCOUNT for zero */
	SEQ(JUMP);
	NEXT(OTHER_COMB_F2D);
	COND(IFNZ);
	/* test rightcount */
	REGREG(RONLYOP, P0, _RIGHTCOUNT, _RIGHTCOUNT);
    _ES

    _NS /* if rightcount is also zero branch to OTHER_COMB */
	SEQ(JUMP);
	NEXT(CHECK_DELTAS_AND_COUNTS_F2D);
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
	NEXT(FINISH_F2D);
	/* test done and leftcount for arrival at FINISH */
	ALUOP(IOROP,P0);
	SETROP(_LEFTCOUNT, NONE);
	SETSOP(NONQOP, _BIASED_DONE, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

label(OTHER_COMB_F2D)
#ifdef undef
    _NS /* test rightcount against zero */
	REGREG(RONLYOP, P0, _RIGHTCOUNT, _RIGHTCOUNT);
    _ES
#endif

    _NS /* if it is non zero branch */
	SEQ(JUMP);
	NEXT(CHECK_DELTAS_AND_COUNTS_F2D);
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
	NEXT(FINISH_F2D);
	/* test done and leftcount for arrival at FINISH */
	ALUOP(IOROP,P0);
	SETROP(_LEFTCOUNT, NONE);
	SETSOP(NONQOP, _BIASED_DONE, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

label(CHECK_DELTAS_AND_COUNTS_F2D)
    _NS /* test to see if the deltas and counts are the same, if they
        are then the sides have the same slope, new ones are needed */
	REGCOMP(EQ, _LEFTCOUNT, _RIGHTCOUNT);
    _ES

    _NS /* conditionally exit the loop and test the deltas */
	SEQ(JUMP);
	NEXT(COMPUTE_SLOPES_F2D);
	COND(IFNQ);
	/* test the deltas */
	REGCOMP(EQ, _DEL_XLEFT_HI, _DEL_XRIGHT_HI);
    _ES

    _NS /* conditionally exit the loop and test done */
	SEQ(JUMP);
	NEXT(COMPUTE_SLOPES_F2D);
	COND(IFNQ);
	/* test done */
	REGREG(RONLYOP, P0, _BIASED_DONE, _BIASED_DONE);
    _ES

    _NS /* conditionally jump to the top of the loop or fall through to
	the code that computes and compares slopes */
	SEQ(JUMP);
	NEXT(LOOP_TOP_F2D);
	COND(IFZ);
    _ES

label(COMPUTE_SLOPES_F2D)
    _NS /* check leftcount */
	REGREG(RONLYOP, P0, _LEFTCOUNT, _LEFTCOUNT);
    _ES

    _NS /* if its nonzero do the divide, otherwise skip this and check
	rightcount */
	SEQ(JUMP);
	NEXT(RIGHT_SLOPE_F2D);
	COND(IFZ);
	/* zero the low del_xleft register */
	REGREG(FLOWOP, P0, 0, _DEL_XLEFT_LO);
    _ES

    /*====================================*/
    /* compute the actual left slope here */
    /*====================================*/
    _NS
	SEQ(JSUB);
	NEXT(COMPUTE_LEFT_SLOPE_F2D);
    _ES

label(RIGHT_SLOPE_F2D)
    _NS /* check rightcount */
	REGREG(RONLYOP, P0, _RIGHTCOUNT, _RIGHTCOUNT);
    _ES

    _NS /* if its nonzero do the divide, otherwise skip this and check
	rightcount */
	SEQ(JUMP);
	NEXT(CHECK_SLOPES_F2D);
	COND(IFZ);
	/* zero the low del_xright register */
	REGREG(FLOWOP, P0, 0, _DEL_XRIGHT_LO);
    _ES

    /*====================================*/
    /* compute the actual right slope here */
    /*====================================*/
    _NS
	SEQ(JSUB);
	NEXT(COMPUTE_RIGHT_SLOPE_F2D);
    _ES

label(CHECK_SLOPES_F2D)
    _NS	/* first check to see if they are not equal, NOTE: both the  hi
    	and low halves of the words may have to be tested, the order of
	the test assumes that differences are more likely in the low
	half, the actual order of the two tests is probably not
	important */
	REGCOMP(EQ, _DEL_XLEFT_LO, _DEL_XRIGHT_LO);
    _ES

    _NS /* now branch if they are not equal,  also test the high halves
    	just in case*/
	SEQ(JUMP);
	NEXT(SLOPES_UNEQUAL_F2D);
	COND(IFNQ);
	/* test low halves */
	REGCOMP(EQ, _DEL_XLEFT_HI, _DEL_XRIGHT_HI);
    _ES

    _NS	/* check the low half now */
	REGREG(RONLYOP, P0, _BIASED_DONE, _BIASED_DONE);
	SEQ(JUMP);
	NEXT(SLOPES_EQUAL_F2D);
	COND(IFEQ);
    _ES

label(SLOPES_UNEQUAL_F2D)
    _NS	/* compare for del_xleft >  del_xright */
	REGCOMP(GT, _DEL_XRIGHT_LO, _DEL_XLEFT_LO);
	PROPOUT12;
    _ES

    _NS /* now do the other half */
	REGCOMP(GT, _DEL_XRIGHT_HI, _DEL_XLEFT_HI);
	PROPIN;
    _ES

    _NS /* branch to second finish if they don't need swapping */
	SEQ(JUMP);
	NEXT(SECOND_FINISH_F2D);
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

    _NS /* swap del_xleft_lo and del_xright_lo
	load del_xleft_lo into Q */
	ALUOP(RONLYOP, P0);
	SETROP(_DEL_XLEFT_LO, NONE);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
    _ES

    _NS /* load right into left */
	REGREG(RONLYOP, P0, _DEL_XRIGHT_LO, _DEL_XLEFT_LO);
    _ES

    _NS /* load Q into right */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, _DEL_XRIGHT_LO, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	SEQ(JUMP);
	NEXT(SECOND_FINISH_F2D);
    _ES

label(SLOPES_EQUAL_F2D)
    _NS /* the test of done has already been made */
	SEQ(JUMP);
	NEXT(SKIP_LOOP_TOP_F2D);
	COND(IFNEG);
	/* load the MAR with the address of one of the ends of the 
	polygon line if needed */
	SETROP(_CORNER1, NONE);
	SETSOP(NONQOP, _CORNER1, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(LOAD);
    _ES

    _NS /* load the MAR with the address of _SAVE1 and restore
        _DEL_XRIGHT_HI */
	LOADMAR(_SAVE1);
	CONST(_SAVE1);
    _ES

    _NS /* read the value */
	RAM(RAMRD, _DEL_XRIGHT_HI, HOLD);
	SEQ(JUMP);
	NEXT(LOOP_TOP_F2D);
    _ES

label(SKIP_LOOP_TOP_F2D)
	/* If the code ever reaches this point the polygon has had only
	sides of the same slope moving in the same direction, here the
	test is made for polygons of only horizontal sides, the code
	should now retrieve the values at corners and find del count
	and x */

    _NS /* get the x coord of the first point */
	RAM(RAMRD, _XLEFT_HI, INC);
    _ES

    _NS	/* get the y value of the coord */
	RAM(RAMRD, _YVALUE, HOLD);
    _ES

    _NS /* load the address of the x values */
	ALUOP(RONLYOP, P0);
	SETROP(_CORNER2, NONE);
	SETSOP(NONQOP, _LEFT, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(LOAD);
    _ES

    _NS /* incmar to the y value */
	REGHOLD;
	DOTOMAR(INC);
    _ES

    _NS /* figure count */
	REGRAM(SUBSROP, P1, _YVALUE, _LEFTCOUNT, DEC);
    _ES

    /* now copy the results to the  right side */
    _NS REGREG(RONLYOP, P0, _XLEFT_HI, _XRIGHT_HI); _ES
    _NS REGREG(RONLYOP, P0, _LEFT, _RIGHT); _ES
    _NS REGREG(RONLYOP, P0, _LEFTCOUNT, _RIGHTCOUNT); _ES

    _NS
	COND(IFNZ);
	SEQ(JUMP);
	NEXT(SECOND_FINISH_F2D);
    _ES

    _NS
	REGRAM(SUBSROP, P1, _XLEFT_HI, _DEL_XLEFT_HI, HOLD);
    _ES

    _NS 
	REGRAM(SONLYOP, P0, 0, _XRIGHT_HI, HOLD);
	SEQ(JUMP);
	NEXT(SECOND_FINISH_F2D);
    _ES

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

label(FINISH_F2D)
    _NS	/* do the (!done & !leftcount) test, NOTE: all code jumping
        here has set up the test */
	SEQ(JUMP);
	NEXT(NOT_FLAT_LEFT_F2D);
	COND(IFNZ);
	/* set up test to make sure the side isn't flat, the done flag
	could have been set */ 
	REGREG(RONLYOP, P0, _LEFTCOUNT, _LEFTCOUNT);
    _ES

    _NS /* jump to START_LEFT_SIDE, also load the MAR with the left, to
	set up for the testing which must take place */
	SEQ(JSUB);
	NEXT(START_LEFT_SIDE_F2D);
	REGREG(RONLYOP, P0, _LEFT, _LEFT);
	DOTOMAR(LOAD);
    _ES

    _NS /* jump around the test and divide code */
	SEQ(JUMP);
	NEXT(TEST_RIGHT_SIDE_F2D);
	/* setup the test for the right side */
	ALUOP(IOROP,P0);
	SETROP( _BIASED_DONE, NONE);
	SETSOP( NONQOP, _RIGHTCOUNT, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

label(NOT_FLAT_LEFT_F2D)
    _NS /* make sure leftcount is non-zero then do the divide */
	SEQ(JUMP);
	NEXT(TEST_RIGHT_SIDE_F2D);
	COND(IFZ);
	/* setup the test for the right side */
	ALUOP(IOROP,P0);
	SETROP( _BIASED_DONE, NONE);
	SETSOP( NONQOP, _RIGHTCOUNT, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    /*====================================*/
    /* compute the actual left slope here */
    /*====================================*/
    _NS
	SEQ(JSUB);
	NEXT(COMPUTE_LEFT_SLOPE_F2D);
    _ES

    _NS
	/* setup the test for the right side */
	ALUOP(IOROP,P0);
	SETROP( _BIASED_DONE, NONE);
	SETSOP( NONQOP, _RIGHTCOUNT, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

label(TEST_RIGHT_SIDE_F2D)
    _NS	/* do the (!done & !rightcount) test, NOTE: all code jumping
        here has set up the test */
	SEQ(JUMP);
	NEXT(NOT_FLAT_RIGHT_F2D);
	COND(IFNZ);
	/* set up test to make sure the side isn't flat, the done flag
	could have been set */ 
	REGREG(RONLYOP, P0, _RIGHTCOUNT, _RIGHTCOUNT);
    _ES

    _NS /* jump to START_RIGHT_SIDE, also load the MAR with right, to
	set up for the testing which must take place */
	SEQ(JSUB);
	NEXT(START_RIGHT_SIDE_F2D);
	REGREG(RONLYOP, P0, _RIGHT, _RIGHT);
	DOTOMAR(LOAD);
    _ES

    _NS /* jump around the test and divide code */
	SEQ(JUMP);
	NEXT(SECOND_FINISH_F2D);
    _ES

label(NOT_FLAT_RIGHT_F2D)
    _NS /* make sure rightcount is non-zero then do the divide */
	SEQ(JUMP);
	NEXT(SECOND_FINISH_F2D);
	COND(IFZ);
    _ES

    /*====================================*/
    /* compute the actual right slope here */
    /*====================================*/
    _NS
	SEQ(JSUB);
	NEXT(COMPUTE_RIGHT_SLOPE_F2D);
    _ES

label(SECOND_FINISH_F2D)
    _NS /* finish off the two sides, loading the MAR for use in the
        routine */
	SEQ(JSUB);
	NEXT(FINISH_LEFT_SIDE);
	REGREG(RONLYOP, P0, _LEFT, _LEFT);
	DOTOMAR(LOAD);
    _ES

    _NS 
	SEQ(JSUB);
	NEXT(FINISH_RIGHT_SIDE);
	REGREG(RONLYOP, P0, _RIGHT, _RIGHT);
	DOTOMAR(LOAD);
    _ES

/*label(SETUP_STIPPLE)*/
    _NS /* load the MAR with the address of the polygon stipple pattern,
	syncronize */
	LOADMAR(_POLYSTIPADR);
	CONST(_POLYSTIPADR);
    _ES

    _NS /* read the stipple address into Q */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, LDQ, REGWRD);
    _ES

    _NS
	LOADMAR(_MULTIVIEW);
	CONST(_MULTIVIEW);
    _ES

    _NS /* read the multiview flag oring it with done */
	ALUOP(IOROP, P0);
	SETROP(_BIASED_DONE, NONE);
	SETSOP(NONQOP, _BIASED_DONE, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRE);
	BPCCMD(NOOP);
	DOTOOUTREG;
    _ES

    _NS /* now send the font address to the BPC */
	ALUOP(SONLYOP, P0);
	SETSOP(QOPERAND, 0, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADFA);
	SEQ(JUMP);
	NEXT(START_TRAPEZOID_LOOP_F2D);
    _ES

/************************************************************************/
/************************************************************************/
/************************************************************************/

label(COMPUTE_RIGHT_SLOPE_F2D)
    _NS /* load the MAR with the address of the save area */
	LOADMAR(_SAVE1);
	CONST(_SAVE1);
	DOTOOUTREG;		/* dump the address into the output reg
				for the call */
    _ES

    _NS /* test the numerator */
	REGREG(RONLYOP, P0, _DEL_XRIGHT_HI, _DEL_XRIGHT_HI);
    _ES

    _NS /* load the numerator */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _DEL_XRIGHT_HI, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(INC);
	COND(IFZ); 
	SEQ(JUMP);
	NEXT(ZERO_NUMERATOR_RIGHT);
    _ES

    _NS /* load the denominator */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _RIGHTCOUNT, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(INC);
	SEQ(JSUB);
	NEXT(DIVIDE);
    _ES

    _NS /* retrieve the fractional part */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _DEL_XRIGHT_LO, RAMRD);
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

label(COMPUTE_LEFT_SLOPE_F2D)
    _NS /* load the MAR with the address of the save area */
	LOADMAR(_SAVE1);
	CONST(_SAVE1);
	DOTOOUTREG;		/* dump the address into the output reg
				for the call */
    _ES

    _NS /* test the numerator */
	REGREG(RONLYOP, P0, _DEL_XLEFT_HI, _DEL_XLEFT_HI);
    _ES

    _NS /* load the numerator */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _DEL_XLEFT_HI, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(INC);
	COND(IFZ);
	SEQ(JUMP);
	NEXT(ZERO_NUMERATOR_LEFT);
    _ES

    _NS /* load the denominator */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _LEFTCOUNT, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(INC);
	SEQ(JSUB);
	NEXT(DIVIDE);
    _ES

    _NS /* retrieve the fractional part */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _DEL_XLEFT_LO, RAMRD);
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

label(ZERO_NUMERATOR_LEFT);
    _NS
	REGREG(FLOWOP, P0, 0, _DEL_XLEFT_LO);
    _ES

    _NS
	REGREG(FLOWOP, P0, 0, _DEL_XLEFT_HI);
	SEQ(RETN);
    _ES

label(ZERO_NUMERATOR_RIGHT);
    _NS
	REGREG(FLOWOP, P0, 0, _DEL_XRIGHT_LO);
    _ES

    _NS
	REGREG(FLOWOP, P0, 0, _DEL_XRIGHT_HI);
	SEQ(RETN);
    _ES

}
