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
		_DEL_XLEFT_LO
		_DEL_XRIGHT_LO
		_XLEFT_HI
		_XLEFT_LO
		_XRIGHT_HI
		_XRIGHT_LO

	constants used
	scratch locations used
	routines called
		RESET_MASKS
		CHECK_MASKING
		FILL_TRAPEZOID
		START_LEFT_SIDE_F2D
		FINISH_LEFT_SIDE
		START_RIGHT_SIDE_F2D
		FINISH_RIGHT_SIDE

	exits to
		POLY_EXIT
*/

trapezoid_loop()
{
newfile("traploop.c");

label(START_TRAPEZOID_LOOP_F2D)
    _NS /* if the mask reset flag is set call reset_masks and clear the
	bit*/
	ALUOP(ANDOP, P0);
	LOADDI(UCONST);
	CONST(0x9);
	SETROP(0, ALL16);
	SETSOP(NONQOP, _BIASED_DONE, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* load Q with _LEFTCOUNT for use in FILL_TRAP */
	ALUOP(RONLYOP, P0);
	SETROP(_LEFTCOUNT, NONE);
	SETSOP(NONQOP, _LEFTCOUNT, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	COND(IFNZ);
	SEQ(JSUB);
	NEXT(MY_RESET_MASKS);
    _ES

    _NS /* test the biasing flag */
	ALUOP(ANDOP, P0);
	LOADDI(UCONST);
	CONST(6);
	SETROP(0, ALL16);
	SETSOP(NONQOP, _BIASED_DONE, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRD);
    _ES

    _NS /* if either side is biased call CHECK_MASKING to make sure we 
        are masking off the overshoot which can occur 
    	test left and right counts in case this falls through */
	SEQ(JSUB);
	COND(IFNZ);
	NEXT(CHECK_MASKING);
	REGCOMP(EQ, _LEFTCOUNT, _RIGHTCOUNT);
	DOTOOUTREG;
	BPCCMD(NOOP);
    _ES

    _NS /* do the conditional branch */
	SEQ(JUMP);
	NEXT(LEFT_AND_RIGHT_NOT_EQUAL_F2D);
	COND(IFNQ);
	REGREG(RONLYOP, P0, _DEL_XLEFT_HI, _DEL_XLEFT_HI);
	DOTOOUTREG;
	BPCCMD(LOADSDI);
    _ES

    _NS /* call FILL_TRAPEZOID, also load the low left delta */
	REGREG(RONLYOP, P0, _DEL_XLEFT_LO, _DEL_XLEFT_LO);
	DOTOOUTREG;
	BPCCMD(LOADSDF); /* start delta fraction */
	SEQ(JSUB);
	NEXT(FILL_TRAPEZOID);
    _ES

    _NS /* test done, this was setup by FILL_TRAPEZOID, if true
	then the current polygon is complete */
	SEQ(JUMP);
	NEXT(POLY_EXIT);
	COND(IFNEG);
    _ES

    _NS /* call start_left_side, also load the MAR with left, to allow
	testing in this routine */
	SEQ(JSUB);
	NEXT(START_LEFT_SIDE_F2D);
	REGREG(RONLYOP, P0, _LEFT, _LEFT);
	DOTOMAR(LOAD);
    _ES

    _NS /* test done, this was setup by START_LEFT_SIDE_F2D, if true
	then the current polygon is complete, NOTE: this is optimized
	toward screen aligned rectangles, it costs others a state */
	REGREG(RONLYOP, P0, _BIASED_DONE, _BIASED_DONE);
	SEQ(JUMP);
	NEXT(POLY_EXIT);
	COND(IFNEG);
    _ES

    _NS /* call reset masks if multiview or biased */
	SEQ(JUMP);
	COND(IFZ);
	NEXT(SKIP_MASK_RESET_FLAG);
    _ES

    _NS /* set the next bit up in BIASED_DONE */
	IMMREG(IOROP, P0, 0x8, _BIASED_DONE);
	CONST(8);
    _ES

label(SKIP_MASK_RESET_FLAG)
    _NS /* now call finish_left_side, again load MAR with left */
	SEQ(JSUB);
	NEXT(FINISH_LEFT_SIDE);
	REGREG(RONLYOP, P0, _LEFT, _LEFT);
	DOTOMAR(LOAD);
    _ES

    _NS /* call start_right_side, also load the MAR with right, to allow
	testing in this routine */
	SEQ(JSUB);
	NEXT(START_RIGHT_SIDE_F2D);
	REGREG(RONLYOP, P0, _RIGHT, _RIGHT);
	DOTOMAR(LOAD);
    _ES

    _NS /* now call finish_right_side, again load MAR with right */
	SEQ(JSUB);
	NEXT(FINISH_RIGHT_SIDE);
	REGREG(RONLYOP, P0, _RIGHT, _RIGHT);
	DOTOMAR(LOAD);
    _ES

    _NS /* now call advance left and right to avoid filling the same
    	scan line twice, start the increment here */
	REGREG(ADDOP, P0, _DEL_XLEFT_LO, _XLEFT_LO);
	PROPOUT12;
    _ES

    _NS /* add the high half */
	REGREG(ADDOP, P0, _DEL_XLEFT_HI, _XLEFT_HI);
	PROPIN;
    _ES

    _NS /* decrement the count */
	IMMREG(SUBSROP, P1, 1, _LEFTCOUNT); CONST(1);
    _ES

    _NS 
	REGREG(ADDOP, P0, _DEL_XRIGHT_LO, _XRIGHT_LO);
	PROPOUT12;
    _ES

    _NS /* add the high half */
	REGREG(ADDOP, P0, _DEL_XRIGHT_HI, _XRIGHT_HI);
	PROPIN;
    _ES

    _NS /* decrement the count */
	IMMREG(SUBSROP, P1, 1, _RIGHTCOUNT); CONST(1);
    _ES

    _NS /* lastly increment yvalue and return to the top of the loop */
       REGREG(RONLYOP, P1, _YVALUE, _YVALUE);
       SEQ(JUMP);
       NEXT(START_TRAPEZOID_LOOP_F2D);
    _ES

label(LEFT_AND_RIGHT_NOT_EQUAL_F2D)
    _NS	/* test leftcount vs. rightcount */
	REGCOMP(GT, _LEFTCOUNT, _RIGHTCOUNT);
    _ES

    _NS /* now do the conditional jump to the third case */
	SEQ(JUMP);
	NEXT(LEFT_GREATER_RIGHT_F2D);
	COND(IFGT);
    _ES

    _NS /* subtract leftcount from rightcount */
	REGREG(SUBSROP, P1, _LEFTCOUNT, _RIGHTCOUNT);
    _ES

    _NS /* call FILL_TRAPEZOID, also load the low left delta */
	REGREG(RONLYOP, P0, _DEL_XLEFT_LO, _DEL_XLEFT_LO);
	DOTOOUTREG;
	BPCCMD(LOADSDF); /* start delta fraction */
	SEQ(JSUB);
	NEXT(FILL_TRAPEZOID);
    _ES

    _NS /* test done, this was setup by FILL_TRAPEZOID, if true
	then the current polygon is complete */
	REGREG(RONLYOP, P0, _BIASED_DONE, _BIASED_DONE);
	SEQ(JUMP);
	NEXT(POLY_EXIT);
	COND(IFNEG);
    _ES

    _NS /* call reset masks if multiview or biased */
	SEQ(JUMP);
	COND(IFZ);
	NEXT(SKIP_MASK_RESET_FLAG_1);
    _ES

    _NS /* set the next bit up in BIASED_DONE */
	IMMREG(IOROP, P0, 0x8, _BIASED_DONE);
	CONST(8);
    _ES

label(SKIP_MASK_RESET_FLAG_1)
    _NS /* call start_left_side, also load the MAR with left, to allow
	testing in this routine */
	SEQ(JSUB);
	NEXT(START_LEFT_SIDE_F2D);
	REGREG(RONLYOP, P0, _LEFT, _LEFT);
	DOTOMAR(LOAD);
    _ES

    _NS /* now call finish_left_side, again load MAR with left */
	SEQ(JSUB);
	NEXT(FINISH_LEFT_SIDE);
	REGREG(RONLYOP, P0, _LEFT, _LEFT);
	DOTOMAR(LOAD);
    _ES

    _NS /* now call advance left to avoid filling the same
    	scan line twice, start the increment here */
	REGREG(ADDOP, P0, _DEL_XLEFT_LO, _XLEFT_LO);
	PROPOUT12;
    _ES

    _NS /* add the high half */
	REGREG(ADDOP, P0, _DEL_XLEFT_HI, _XLEFT_HI);
	PROPIN;
    _ES

    _NS /* decrement the count */
	IMMREG(SUBSROP, P1, 1, _LEFTCOUNT);
	/* syncronize */
	DOTOOUTREG;
        BPCCMD(NOOP);
	CONST(1);
    _ES

    _NS /* lastly increment yvalue and return to the top of the loop */
        REGREG(RONLYOP, P1, _YVALUE, _YVALUE);
	DOTOOUTREG;
	BPCCMD(LOADEAI); /* request DDAEAI */
    _ES

    _NS /* read back DDAEAI and DDAEAF */
	READBPCBUS;
	DOTOOUTREG;
	LOADDI(BPCDATA); COND(IFFALSE); SEQ(CJPP);
	SETROP(0, ALL16);
	SETSOP(NONQOP, _XRIGHT_HI, RAMNOP);
    _ES

    _NS
	IMMREG(SUBSROP,P1,1,_RIGHTCOUNT);
	CONST(1);
	DOTOOUTREG;
	BPCCMD(LOADEAF); /* request DDAEAF */
    _ES

    _NS
	READBPCBUS;
	DOTOOUTREG;
	LOADDI(BPCDATA); COND(IFFALSE); SEQ(CJPP);
	SETROP(0, ALL16);
	SETSOP(NONQOP, _XRIGHT_LO, RAMNOP);
    _ES

    _NS
       SEQ(JUMP);
       NEXT(START_TRAPEZOID_LOOP_F2D);
    _ES

label(LEFT_GREATER_RIGHT_F2D)
#ifdef undef
    _NS	/* load SDI */
	REGREG(RONLYOP, P0, _DEL_XLEFT_HI, _DEL_XLEFT_HI);
	DOTOOUTREG;
	BPCCMD(LOADSDI);
    _ES
#endif

    _NS /* load Q with the value to be used in FILL_TRAP' */
	ALUOP(RONLYOP,  P0);
	SETROP(_RIGHTCOUNT, NONE);
	SETSOP(NONQOP, _RIGHTCOUNT, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
    _ES

    _NS /* call FILL_TRAPEZOID, also load the low left delta */
	REGREG(RONLYOP, P0, _DEL_XLEFT_LO, _DEL_XLEFT_LO);
	DOTOOUTREG;
	BPCCMD(LOADSDF); /* start delta fraction */
	SEQ(JSUB);
	NEXT(FILL_TRAPEZOID);
    _ES

    _NS /* test done, this was setup by FILL_TRAPEZOID, if true
	then the current polygon is complete */
	REGREG(RONLYOP, P0, _BIASED_DONE, _BIASED_DONE);
	SEQ(JUMP);
	NEXT(POLY_EXIT);
	COND(IFNEG);
    _ES

    _NS
    	/* subtract rightcount from leftcount */
	/* call reset masks if multiview or biased */
	REGREG(SUBSROP,  P1, _RIGHTCOUNT, _LEFTCOUNT);
	SEQ(JUMP);
	COND(IFZ);
	NEXT(SKIP_MASK_RESET_FLAG_2);
    _ES

    _NS /* set the next bit up in BIASED_DONE */
	IMMREG(IOROP, P0, 0x8, _BIASED_DONE);
	CONST(8);
    _ES

label(SKIP_MASK_RESET_FLAG_2)
    _NS /* call start_right_side, also load the MAR with right, to allow
	testing in this routine */
	SEQ(JSUB);
	NEXT(START_RIGHT_SIDE_F2D);
	REGREG(RONLYOP, P0, _RIGHT, _RIGHT);
	DOTOMAR(LOAD);
    _ES

    _NS /* now call finish_right_side, again load MAR with right */
	SEQ(JSUB);
	NEXT(FINISH_RIGHT_SIDE);
	REGREG(RONLYOP, P0, _RIGHT, _RIGHT);
	DOTOMAR(LOAD);
    _ES

    _NS /* now call advance right to avoid filling the same
    	scan line twice, start the increment here */
	REGREG(ADDOP, P0, _DEL_XRIGHT_LO, _XRIGHT_LO);
	PROPOUT12;
    _ES

    _NS /* add the high half */
	REGREG(ADDOP, P0, _DEL_XRIGHT_HI, _XRIGHT_HI);
	PROPIN;
    _ES

    _NS /* decrement the count */
	IMMREG(SUBSROP, P1, 1, _RIGHTCOUNT);
	/* syncronize */
	DOTOOUTREG;
        BPCCMD(NOOP);
	CONST(1);
    _ES

    _NS /* lastly increment yvalue and return to the top of the loop */
        REGREG(RONLYOP, P1, _YVALUE, _YVALUE);
	DOTOOUTREG;
	BPCCMD(LOADSAI); /* request DDASAI */
    _ES

    _NS /* read back DDASAI and DDASAF */
	READBPCBUS;
	DOTOOUTREG;
	LOADDI(BPCDATA); COND(IFFALSE); SEQ(CJPP);
	SETROP(0, ALL16);
	SETSOP(NONQOP, _XLEFT_HI, RAMNOP);
    _ES

    _NS /* decrement leftcount by one */
	IMMREG(SUBSROP,P1,1,_LEFTCOUNT);
	CONST(1);
	DOTOOUTREG;
	BPCCMD(LOADSAF); /* request DDASAF */
    _ES

    _NS
	READBPCBUS;
	DOTOOUTREG;
	LOADDI(BPCDATA); COND(IFFALSE); SEQ(CJPP);
	SETROP(0, ALL16);
	SETSOP(NONQOP, _XLEFT_LO, RAMNOP);
    _ES

    _NS
       SEQ(JUMP);
       NEXT(START_TRAPEZOID_LOOP_F2D);
    _ES

label(MY_RESET_MASKS)
    _NS /* clear the bit */
	IMMREG(ANDOP, P0, ~8, _BIASED_DONE);
	CONST(~8);
    _ES

    _NS
	SEQ(JUMP);
	NEXT(RESET_MASKS);
    _ES
}
