/* modes.c  --  general mode-setting commands
 *
 * includes:
 *	SET_HIT_MODE:		(now moves location of screenmask list)
 *	CLEAR_HIT_MODE:		(now moves location of screenmask list)
 *	DRAW_MODE (shading,Zbuffering)
 *	EOF:
 *	the endroutine LOADFLAG
 *
 *			HIT TESTING ROUTINES
 *
 *  These routines are used to implement a hierarchical picking scheme.
 *  A stack of names is maintained in the scratch RAM
 *  and each call to LOAD_NAME, PUSH_NAME, and POP_NAME will generate an 
 *  interrupt for the 68000 if something has been drawn into the viewport 
 *  since the last call to one of these  routines.
 *
 *  The individual FBC routines will alter the hit register appropriately
 *  when the HITMODE flag is set.
 *
 *  See hitrept.mic for stack manipulators PUSH_NAME, LOAD_NAME and POP_NAME.
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"
#include "version.h"

modes()
{
newfile("modes.c");

label(SET_HIT_MODE)
/*
 *	sets HITMODE and DUPHITMODE non-zero, and clears the hitbit reg.
 */
	_NS LOADIMM(_TEMP,_DUPHITMODE); LDMAR; CONST(_DUPHITMODE); _ES
		/* address DUPhit mode flag; save (non-0) adr in a reg */

	_NS SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMWR);
	 ALUOP(RONLYOP,P0); FTODO; LDMAR;
	 COND(IFFALSE); SEQ(JUMP); CONST(_HITBITS);_ES
				/* make it non-zero; address hitbits	*/

	_NS SETROP(0,ALL16); SETSOP(NONQOP,_ZERO,RAMWR);
	 ALUOP(MOVE); FTODO; LDMAR; LOADDI(UCONST); CONST(_HITMODE); _ES
			/* clear hitbits; address of hitmode flag */

	_NS LOADIMM(_MODEMASK,_ALTHITBIT); CONST(_ALTHITBIT); _ES

	_NS REGREG(ONES,_TEMP,_TEMP); DOJUMP(LOADFLAG); _ES
					/* set bit in ALTVECMODE, HITMODE */


label(CLEAR_HIT_MODE)
/*
/*		this command clears the HITMODE flags
/*		ALSO, the microcode version no. is stored in the out reg.
*/

	_NS LOADIMM(_TEMP,VERSION); LDOUT; CONST(VERSION); _ES
		/* store VVvv in out reg, where
		 *				V = major version
		 *				v = minor version
		 */

	_NS LOADMAR(_DUPHITMODE); CONST(_DUPHITMODE); _ES
					/* address DUP hit flag */

	_NS SETROP(0,ALL16); SETSOP(NONQOP,_ZERO,RAMWR);
	 ALUOP(RONLYOP,P0); FTODO; LDMAR;
	 COND(IFFALSE); SEQ(JUMP); CONST(_HITBITS);_ES
			/* 0 DUPHITMODE; point to HITBITS	*/

	_NS SETROP(0,ALL16); SETSOP(NONQOP,_ZERO,RAMWR);
	 ALUOP(MOVE); FTODO; LDMAR; LOADDI(UCONST); CONST(_HITMODE); _ES
			/* clear hitbits; address of hitmode flag */

	_NS LOADIMM(_MODEMASK,_ALTHITBIT); CONST(_ALTHITBIT); _ES

	_NS REGREG(ZERO,0,_TEMP); DOJUMP(LOADFLAG); _ES
					/* clear bit in ALTVECMODE */


label(DRAW_MODE)	/* (shading,Zbuffering) */
/*
 * sets _POLYGONSTYLE, _ZBUFFER flags in scratch ram.
 * sets _ALTVECMODE, _ALTPOLYMODE according to value of _ZBUFFER.
 * doesn't disturb any 2903 regs.
 */
#define _POLYFLAGS	5

	_NS LOADMAR(_REGSAVE); CONST(_REGSAVE); _ES	/* save state */

	_NS RAM(RAMWR,_MODEMASK,INC); _ES
	_NS RAM(RAMWR,_POLYFLAGS,INC); _ES
	_NS RAM(RAMWR,_TEMP,HOLD); _ES

	_NS LOADMAR(_POLYGONSTYLE); CONST(_POLYGONSTYLE); _ES

	_NS LOADREG(_TEMP,ALL16,NOMORE); _ES

	_NS RAM(RAMWR,_TEMP,HOLD); _ES		/* save polyshade flag */

	_NS LOADIMM(_MODEMASK,_ALTZBUBIT); GEGET; CONST(_ALTZBUBIT); _ES

	_NS LOADMAR(_ALTPOLYMODE); CONST(_ALTPOLYMODE); _ES

	_NS RAM(RAMRD,_POLYFLAGS,HOLD); _ES

	_NS LOADREG(_TEMP,ALL16,NOMORE); _ES

	_NS REGREG(ORRS,_MODEMASK,_POLYFLAGS);	/* assume z flag is non-0 */
	 COND(IFNZ); DOJUMP(ZPOLYMODE); _ES	/* if it is, skip ahead   */

	_NS REGREG(XOR,_MODEMASK,_POLYFLAGS);
	 DOJUMP(WRTPOLYMODE); _ES		/* if not, clear the bit */

label(ZPOLYMODE)
#ifdef NOZPOLY
	_NS REGHOLD; DOJUMP(UNIMPL_TRAP); _ES
#endif

label(WRTPOLYMODE)
	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_POLYFLAGS,RAMWR);
	 ALUOP(MOVE); FTODO; LDMAR; CONST(_ZBUFFER); _ES
			/* while writing new flags, point to Zbuffer flag */

	_NS REGREG(MOVE,_TEMP,_TEMP); DOJSUB(LOADFLAGSUB); _ES
			/* get & examine value of new flag - go save it */

	_NS LOADMAR(_REGSAVE); CONST(_REGSAVE); _ES	/* restore & exit */

	_NS RAM(RAMRD,_MODEMASK,INC); _ES
	_NS RAM(RAMRD,_POLYFLAGS,INC); _ES
	_NS RAM(RAMRD,_TEMP,HOLD); GEGET; DOJUMP(DISPATCH); _ES


label(EOF)
/*
 * general-purpose "end of frame" mechanism
 *	initialize interruptcode
 *	send interruptcode, hitbits
 *	NOTE: hit mode, hit bits NOT cleared (but next SET HIT will)
 */

#define _INTCODE	6

	_NS LOADIMM(_INTCODE,_INTEOF); LDOUT;INTERRUPTHOST; CONST(_INTEOF);_ES
						/* send interrupt code	*/

	_NS LOADMAR(_HITBITS); CONST(_HITBITS);_ES

	_NS RAM(RAMRD,_TEMP,HOLD); LDOUT; INTERRUPTHOST; DOJSUB(OUTSEND); _ES
							/* send hit bits */

	_NS REGHOLD; GEGET; DOJUMP(DISPATCH); _ES

/*================================================================*/

label(LOADFLAG)
/* endroutine to set/clear a particular bit in _ALTVECMODE
 *	Enter having tested the desired value of the flag
 *		(either 0 or some ones);
 *	MAR set up to point to the flag;
 *	the _MODEMASK reg set up with the bit to change in _ALTVECMODE
 */

	_NS REGREG(ZERO,0,_TEMP); COND(IFZ); DOJUMP(WRTF); _ES

	_NS REGREG(MOVE,_MODEMASK,_TEMP); _ES

label(WRTF)
	_NS REGHOLD; DOJSUB(SUBSUBFLAG); _ES

	_NS REGHOLD; GEGET; DOJUMP(DISPATCH); _ES

label(LOADFLAGSUB)	/* subroutine version */
	_NS REGREG(ZERO,0,_TEMP); COND(IFZ); DOJUMP(SUBSUBFLAG); _ES

	_NS REGREG(MOVE,_MODEMASK,_TEMP); _ES

label(SUBSUBFLAG)		/* inner subroutine -- return to caller */
	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMWR);
	 ALUOP(MOVE); FTODO; LDMAR; CONST(_ALTVECMODE); _ES
		/* while writing user's flag, address ALTVECMODE flg */

	_NS REGREG(COMPROP,P0,_MODEMASK,_MODEMASK); _ES
					/* all bits but the one selected */

	_NS REGRAM(ANDRS,_MODEMASK,_MODEMASK,HOLD); _ES

	_NS REGREG(ORRS,_TEMP,_MODEMASK); _ES
				/* or-in either 0 or 1 in selected posn */

	_NS RAM(RAMWR,_MODEMASK,HOLD); SEQ(RETN); _ES
}
