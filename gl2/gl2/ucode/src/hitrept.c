/* hitrept.c  -- hit mode reporting commands and name stack manipulation
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"

#define _WK1	10
#define _WK2	11
#define _ADR	12		/*saved _HITBITS adr*/
/*_ZRO=6		used because of LM14UP calls */

hitrept()
{
newfile("hitrept.c");

label(INIT_NAMESTACK)
/*
 *  Initializes the name stack to start at HITSTACKLIM, i.e. it loads
 *  HITSTACKPTR with HITSTACKLIM and pushes a -1 on the stack
 */
/* MAR gets current HITSTACKLIM  */
	_NS LOADMAR(_HITSTACKLIM); CONST(_HITSTACKLIM);_ES

/* get the value of the pointer; point at HITSTACKPTR */
	_NS REGRAM(SONLYOP,P0,0,_WK1,DEC);_ES

/* store new HITSTACKPTR */
	_NS 
	 ALUOP(SONLYOP, P0);
	 SETSOP(NONQOP, _WK1, RAMWR);
	 YQ(FF, OLDQ, REGWRD); 
	 DOJUMP(DISPATCH); 
	_ES

label(PUSH_NAME)
/*
 *  Inputs name, subtract one from current HITSTACKPTR, limits to within:
 *	HITSTACKLIM >= HITSTACKPTR > STPLUS2
 */
/* check for a hit */
	_NS REGHOLD; DOJSUB(CHECK_HIT);_ES	

	_NS LOADMAR(_HITSTACKPTR); CONST(_HITSTACKPTR); _ES

/* get the value of the pointer decremented */
	_NS REGRAM(SUBSROP,P0,_ZRO,_WK1,HOLD);_ES

/* check this value against _STPLUS2 */
	_NS SETROP(0,ALL16); SETSOP(NONQOP, _WK1, RAMNOP);
	    ALUOP(SUBRSOP,P1); FTODO; COND(IFFALSE); SEQ(JUMP);
	    DOTOMAR(HOLD); CONST(_STARTLIST+2); _ES

/* if the limit is ok skip the fix below */
	_NS REGHOLD; COND(IFNEG); DOJUMP(LIMS_OK); _ES

/* screwed up, set it to the lower limit */
	_NS LOADIMM(_WK1, _STARTLIST+2); CONST(_STARTLIST+2); _ES

label(LIMS_OK)
/* load the new HITSTACKPTR and load the MAR with it */
	_NS RAM(RAMWR,_WK1,LOAD); _ES

/* get the name being pushed from the GE port */
	_NS LOADREG(_WK2, ALL16, MORE); _ES

/* write the name into the stack */
/* and return to the dispatcher */
	_NS RAM(RAMWR, _WK2, HOLD); DOJUMP(DISPATCH); _ES

label(POP_NAME)
/*
 * command to pop the name stack
 * test for hits, then decrement HITSTACKPTR;
 * testing for underflow
 */
	_NS REGHOLD; DOJSUB(CHECK_HIT);_ES

/* MAR gets current HITSTACKPTR  */
	_NS LOADMAR(_HITSTACKPTR); CONST(_HITSTACKPTR);_ES

/* increment the hit stack pointer and the MAR, this points at
 * the limit being checked against */
	_NS 
	 SETSOP(NONQOP, _WK1, RAMRD); 
	 ALUOP(SONLYOP, P1); 
	 DOTOMAR(INC); 
	 YQ(FF, OLDQ, REGWRE);
	_ES
	
/* check the new pointer against the limit */
	_NS 
	 ALUOP(SUBSROP, P1); 
	 SETROP(_WK1, NONE);
	 SETSOP(NONQOP, _WK2, RAMRD); 
	 FTOYANDQ(FF, OLDQ, REGWRE);
	_ES

/* the upper limit is ok 
 * also set the MAR pointing at the HITSTACKPTR again */
	_NS 
	 ALUOP(SONLYOP, P0);
	 SETSOP(NONQOP, _WK2, RAMRD);
	 FTOYANDQ(FF, OLDQ, REGWRE);
	 DOTOMAR(DEC); 
	 COND(IFNNEG); 
	 DOJUMP(UP_LIM_OK); 
	_ES

/* the upper limit is not ok limit it to that */
	_NS 
	 ALUOP(RONLYOP, P0);
	 SETROP(_WK2, NONE);
	 SETSOP(NONQOP, _WK1, RAMNOP);
	 FTOYANDQ(FF, OLDQ, REGWRE);
	_ES

label(UP_LIM_OK)
/* save new value and exit	*/
	_NS 
	 RAM(RAMWR,_WK1,HOLD); 
	 DOJUMP(DISPATCH);
	_ES


label(LOAD_NAME)
/*
 * Test for hit;
 * Store name at ram[HITSTACKPTR]
 */
	_NS REGHOLD; DOJSUB(CHECK_HIT);_ES

/* MAR gets current HITSTACKPTR  */
	_NS LOADMAR(_HITSTACKPTR); CONST(_HITSTACKPTR);_ES

/* get the pointer loaded into (_WK1 and) MAR */
	_NS REGRAM(SONLYOP,P0,0,_WK1,LOAD);_ES

/* read new name */
	_NS LOADREG(_WK2,ALL16,MORE); _ES	

/* load the name and return to the dispatcher */
	_NS RAM(RAMWR,_WK2,HOLD); DOJUMP(DISPATCH); _ES

/*================================================================*/

label(CHECK_HIT)	/* called by name changing routines	*/
/*
 *	First tests whether HITMODE set
 *	Tests HITBITS -- if anything set, send proper interrupt sequence:
 *			1
 *			HITBITS
 *			HITCHARCT
 *			HITSTACKPTRLIM-HITSTACKPTR	( stack depth)
 *			n names (at least one, even if n=0)
 */
	_NS LOADIMM(_ADR,_HITBITS); LDMAR; CONST(_HITBITS);_ES

	_NS RAM(RAMRD,_WK1,DEC);_ES	/* look at accumulated hit bits	*/
					/* point to DUPHITMODE		*/

	_NS RAM(RAMRD,_WK1,HOLD); COND(IFZ); SEQ(RETN);_ES
				/* no hit, just return; read hitmode flag */

	_NS REGHOLD; INCMAR; COND(IFZ); SEQ(RETN); _ES
			/* point to HITBITS; return if not in hitmode */

	_NS REGREG(INCR,_ZRO,_WK1); LDOUT; INTERRUPTHOST; DOJSUB(LM15UP);_ES
			/* send interrupt type (1) to host, go pause */

	_NS RAM(RAMRD,_WK1,HOLD); LDOUT; INTERRUPTHOST; DOJSUB(OUTSEND); _ES
						/* send HITBITS	    */
			/* go enable outreg to bus; inc MAR to char ct. */

	_NS RAM(RAMRD,_WK1,HOLD); LDOUT; INTERRUPTHOST; DOJSUB(OUTSEND); _ES
						/* send char ct.  */

	_NS RAM(RAMRD,_WK1,INC);_ES
			/* get HITSTACKPTR, point to HITSTACKLIM  */

	_NS REGRAM(SUBSRC,_WK1,_WK1,HOLD); LDOUT; INTERRUPTHOST;
	 DOJSUB(OUTSEND); _ES
		/* host & WK1 get size of hit stack: 	*/
		/* = ram(_HITSTACKLIM) - ram(_HITSTACKPTR)	*/

	_NS LOADMAR(_HITSTACKLIM); CONST(_HITSTACKLIM);_ES
					  /* address name stack bot  */

	_NS RAM(RAMRD,_WK2,LOAD);_ES
			/* address bottom item in stack itself */

label(NAMELP)
	_NS RAM(RAMRD,_WK2,HOLD); LDOUT; INTERRUPTHOST;;_ES
					/* send a word from hit stack	*/

	_NS LOADDI(OUTPUTREG); REGREG(SUBSR,_ZRO,_WK1); DECMAR;
		COND(IFFALSE); SEQ(CJPP);_ES
			/* decr stack ctr; address next item; enab outreg   */

	_NS REGHOLD; COND(IFNNEG); DOJUMP(NAMELP);_ES
						/* if not done, loop back */

	_NS LOADDI(UCOUNT); REGREG(MOVE,_ADR,_ADR); LDMAR; DOPUSH(1);_ES
			/* address HITBITS again ; set up loop of 2	*/

	_NS RAM(RAMWR,_ZRO,INC); SEQ(LOUP);_ES	/* clear HITBITS, HITCHARCT */

	_NS REGHOLD; SEQ(RETN);_ES
}
