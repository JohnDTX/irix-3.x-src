/* bump_index.c
/*	bump_left_index and bump_right_index
/*
/*
*/
#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "polydefs.h"

/* registers used
	BUMP_LEFT_INDEX and BUMP_RIGHT_INDEX
		_LEFT,
		_RIGHT,
		_BIASED_DONE

   constants used
		_BIGNEG

*/

bump_indexes()
{
newfile("bump_index.c");

label(BUMP_LEFT_INDEX)
    _NS /* test the address for end of list, incrementing the MAR to
    	point at the address of the other end if this is true */
	IMMRAMCOMP(NQ,_BIGNEG,INC); CONST(_BIGNEG);
    _ES

    _NS /* if the above values were equal then its end of list,
	also test left vs. right in the hope that we're not at end of
	list */
	REGCOMP(EQ, _LEFT, _RIGHT);
	COND(IFNQ);
	SEQ(JUMP);
	NEXT(NOT_END);
    _ES

    _NS	/* if were here then we have reached end of list and must load
	left with the value of the other end of the list,
	also load the MAR with this value for use on return */
	RAM(RAMRD, _LEFT, LOAD);
    _ES

    _NS /* retest for all the points being used up */
	REGCOMP(EQ, _LEFT, _RIGHT);
	SEQ(JUMP);
	NEXT(NOT_END);
	/* increment the MAR to point at the Y coord */
	DOTOMAR(INC);
    _ES

label(BUMP_RIGHT_INDEX)
    _NS /* test the address for end of list, incrementing the MAR to
    	point at the address of the other end if this is true */
	IMMRAMCOMP(NQ,_BIGNEG,INC); CONST(_BIGNEG);
    _ES

    _NS /* if the above values were equal then its end of list,
	also test left vs. right in the hope that we're not at end of
	list */
	REGCOMP(EQ, _LEFT, _RIGHT);
	COND(IFNQ);
	SEQ(JUMP);
	NEXT(NOT_END);
    _ES

    _NS	/* if were here then we have reached end of list and must load
	left with the value of the other end of the list,
	also load the MAR with this value for use on return */
	RAM(RAMRD, _RIGHT, LOAD);
    _ES

    _NS /* retest for all the points being used up */
	REGCOMP(EQ, _LEFT, _RIGHT);
	DOTOMAR(INC);
    _ES

label(NOT_END)
    _NS	/* if the values are not equal then return without setting the
    	done flag, also move the MAR along to the next scratch location */
	SEQ(RETN);
	COND(IFNQ);
    _ES

    _NS /* if we're here then we've reach end of list and the done flag
    	must be set */
	IMMREG(IOROP, P0, 0x8000, _BIASED_DONE);
	CONST(0x8000);
    _ES

    _NS
	SEQ(RETN);
    _ES
}
