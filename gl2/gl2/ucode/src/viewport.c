/* viewport.c
 *
 *	multi-screenmask support  - contains:
 *
 *	LOAD_VIEWPORT( XLL, YLL, XUR, YUR )
 *			command to load true viewport
 *	MASK_LIST ( CONTINUE, [XLL, YLL, XUR, YUR]+, MULTIMODE )
 *			command to load list of screenmasks, load 1st mask
 *	RESET_MASKS:	subroutine to go to top of list & load 1st mask
 *	NEW_MASK:	subroutine to load next mask from list & test for last
 *
 *		uses scratch locations in following sequence:
 *			_MASKEOL	pointer to end of all masks
 *			_MASKPTR	pointer to current mask
 *			_MASKLIM	absolute bottom of list
 *
 *		and _MULTIVIEW to flag whether >1 mask set loaded.
 *
 *	MASK_LIST has no effect in HIT MODE.
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"

viewport()
{
newfile("viewport.c");

label(LOAD_VIEWPORT)
    _NS /* load the MAR with the address of the viewport area */
	LOADMAR(_CURRVIEWPORT);
	CONST(_CURRVIEWPORT);
    _ES

    _NS /* get the left boundry */
	LOADREG(_TEMP, ALL16, MORE);
	LOADDI(INRJUST);
    _ES

    _NS /* get the bottom and write left */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16);
	SETSOP(NONQOP, _TEMP, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	GEOMENGDATA;
	DOTOMAR(INC);
	LOADDI(INRJUST);
    _ES

    _NS /* get the right and write bottom */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16);
	SETSOP(NONQOP, _TEMP, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	GEOMENGDATA;
	DOTOMAR(INC);
	LOADDI(INRJUST);
    _ES

    _NS /* get the top and write right */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16);
	SETSOP(NONQOP, _TEMP, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	DOTOMAR(INC);
	LOADDI(INRJUST);
    _ES

    _NS /* write the top and return to dispatch */
	RAM(RAMWR, _TEMP, HOLD); GEOMENGDATA;
	DOJUMP(DISPATCH);
    _ES

label(MASK_LIST)
/*	MASK_LIST ( CONTINUE, [XLL, YLL, XUR, YUR]+, MULTIMODE )
 *
 *		CONTINUE indicates   (0) reset to load brand new list
 *				     (n) add to end of current list
 *		Gets a number of 4-word masks based on PASSTHRU ct:
 *			XLL, YLL, XUR, YUR
 *		MULTIMODE indicates  (0) primitives need only use default VP
 *				     (1) primitives use all VP's in list
 *		All coords saved in scratch area.  EOL pointer updated.
 *		Always loads 1st screenmask into hardware, & to CHARVIEW area
 */

#define _WK	15	/* also _ZERO */
			/*	_PASSN	14 */
#define _PTR	13
#define _ZXRO	12

	_NS LOADMAR(_HITMODE); CONST(_HITMODE); _ES

	_NS RAM(RAMRD,_TEMP,HOLD); _ES

	_NS REGREG(SUBSR,_ZERO,_PASSN); LDOUT;	/* in case needed for loopct*/
	 COND(IFNZ); DOJUMP(ABORT_CMD); _ES
					/* avoid messing up namestack */

	_NS LOADREG(_TEMP,ALL16,NOMORE); _ES	/* get "reset" flag */

	_NS SETSOP(NONQOP,_ZXRO,RAMNOP); ALUOP(ZERO); YQ(FF,LDQ,REGWRE);
	 COND(IFZ); DOJSUB(RESTART_MASKLIST); _ES	/* take reset action*/

	_NS LOADMAR(_MASKEOL); CONST(_MASKEOL); _ES /* pt to (current) EOL */

	_NS RAM(RAMRD,_PTR,HOLD); LOADDI(UCOUNT); SEQ(PUSH); CONST(1); _ES

	_NS SETROP(_PASSN,NONE); SETSOP(NONQOP,_PASSN,RAMNOP);
	 ALUOP(MOVE); YQ(FLR,QR,REGWRE); SEQ(LOUP); _ES	   /* PASSN/4 */

label(LISTLOOP)		/* for each set of 4 words */
	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_PTR,RAMNOP);
	 ALUOP(SUBSRC); FTODO; LDMAR; CONST(3) _ES	/* PTR-3 to MAR */

	_NS REGHOLD; GEGET; LOADDI(UCOUNT); SEQ(PUSH); CONST(3); _ES
						/* load loop count */

	_NS LOADREG(_TEMP,ALL16,MORE); _ES	/* read a word */

	_NS RAM(RAMWR,_TEMP,INC); _ES		/* stash in list */

	_NS REGREG(SUBSR,_ZXRO,_PTR); SEQ(LOUP); _ES
				/* move pointer in proper direction; loop */

	_NS REGREG(SUBSR,_ZXRO,_PASSN); _ES

	_NS REGHOLD; COND(IFNZ); DOJUMP(LISTLOOP); _ES

	_NS LOADMAR(_MASKEOL); CONST(_MASKEOL); _ES	/* point to EOL */

	_NS RAM(RAMWR,_PTR,HOLD); DOJSUB(RESET_MASKS); _ES
		/* save new EOL; go copy first screenmask into work area */

	_NS LOADREG(_TEMP,ALL16,NOMORE); _ES	/* get MULTIVIEW flag */

	_NS LOADMAR(_MULTIVIEW); CONST(_MULTIVIEW); _ES	/* save in scratch */
	/* my poly code now depends on this being bit 0 only !! */

	_NS LOADIMM(_MODEMASK,_ALTMVPBIT); CONST(_ALTMVPBIT); _ES

	_NS REGREG(MOVE,_TEMP,_TEMP); DOJUMP(LOADFLAG); _ES

/*================================================================*/

label(RESTART_MASKLIST)		/* local subroutine to reset list */
	_NS LOADMAR(_MASKLIM); CONST(_MASKLIM); _ES

	_NS RAM(RAMRD,_WK,DEC); _ES		/* fetch list start adr */

	_NS RAM(RAMWR,_WK,DEC); _ES	/* reset PTR, pt to MASKEOL ptr */

	_NS RAM(RAMWR,_WK,HOLD); SEQ(RETN); _ES  /* reset it & return */

/*================================================================*/

label(RESET_MASKS)	/* global subroutine to fetch first screenmask */

	_NS LOADMAR(_REGSAVE); CONST(_REGSAVE); _ES
					/* don't mess up any registers */

	_NS RAM(RAMWR,12,INC); DOJSUB(STM13UP); _ES  /* save working regs */

	_NS LOADMAR(_MASKLIM); CONST(_MASKLIM); _ES

	_NS RAM(RAMRD,_WK,DEC); _ES		/* fetch bottom-of-list */

	_NS RAM(RAMWR,_WK,DEC); _ES	/* set pointer there; point at EOL */

	_NS REGHOLD; DOJSUB(NEW_MASK_NXT); _ES
					/* go fetch first mask coords */

	_NS REGHOLD; SEQ(RETN); _ES	/* return (having restored regs) */


label(NEW_MASK)		/* global subroutine to fetcn next screenmask
			 * exits having tested end condition (MASKPTR=MASKEOL)
			 */

	_NS LOADMAR(_REGSAVE); CONST(_REGSAVE); _ES
			/* don't mess up any registers */

	_NS RAM(RAMWR,12,INC); DOJSUB(STM13UP); _ES  /* save working regs */

	_NS LOADMAR(_MASKEOL); CONST(_MASKEOL); _ES
					/* pretest end condition */

label(NEW_MASK_NXT)
	_NS RAM(RAMRD,_WK,INC); _ES	/* point at current MASKPTR */

	_NS REGRAMCOMP(EQ,_WK,HOLD); _ES	/* compare */

	_NS REGHOLD; COND(IFEQ); DOJUMP(DONE_EXIT); _ES
				/* if PTR=EOL, take special exit */

	_NS IMMRAM(SUBSRC,3,_WK,LOAD); CONST(3); _ES	/* bottom of mask */

	_NS RAM(RAMRD,12,INC); DOJSUB(LM13UP); _ES	/* get mask */

	_NS LOADMAR(_CHARVIEW); CONST(_CHARVIEW); _ES

	_NS REGHOLD; BPC(NOOP); _ES		/* make sure BPC bus unbusy */

	_NS RAM(RAMWR,12,HOLD); BPC(LOADXS); DOJSUB(NEXTMA); _ES	
							/* save as current */

	_NS RAM(RAMWR,13,HOLD); BPC(LOADYS); DOJSUB(NEXTMA); _ES

	_NS RAM(RAMWR,14,HOLD); BPC(LOADXE); DOJSUB(NEXTMA); _ES

	_NS RAM(RAMWR,15,HOLD); BPC(LOADYE); DOJSUB(NEXTMA); _ES

	_NS LOADMAR(_MASKPTR); CONST(_MASKPTR); _ES  /* move ptr to next vp */

	_NS IMMRAM(SUBSRC,4,_WK,HOLD); CONST(4); _ES

	_NS RAM(RAMWR,_WK,HOLD); _ES

	_NS LOADMAR(_REGSAVE); CONST(_REGSAVE); _ES
				/* load new masking coords; restore regs */

	_NS RAM(RAMRD,12,INC); DOJSUB(LM13UP); _ES

	_NS REGHOLD; BPC(SETSCRMASKX); _ES

	_NS ALUOP(ONES); FTODO;			/* return saying not equal */
	 BPC(SETSCRMASKY); SEQ(RETN); _ES	/* update clipping regs */

label(DONE_EXIT)
	_NS LOADMAR(_REGSAVE); CONST(_REGSAVE); _ES

	_NS RAM(RAMWR,12,INC); DOJSUB(LM13UP); _ES

	_NS ALUOP(ZERO); FTODO; SEQ(RETN); _ES	/* return saying "equal" */

label(ABORT_CMD)
	_NS REGHOLD; LOADDI(OUTPUTCOUNT); SEQ(PUSH); _ES	/* nct-1 */

	_NS GEGET; SEQ(LOUP); _ES	/* discard data, input next cmd */

	_NS REGHOLD; DOJUMP(DISPATCH); _ES
}
