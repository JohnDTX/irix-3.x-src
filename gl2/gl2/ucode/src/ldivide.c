/* ldivide.c
 *
 *	divide routines:
 *		DIVIDE
 *		LONGDIVIDE
 *		DIVIDE16
 *		LONGDIVIDE16
 *
 *	plus "long" division subroutine
 *
 *	Enter with outreg containing a pointer to a parameter area:
 *
 *	--->	 dividend, integer part
 *		[dividend, fractional part]	(LONG versions only)
 *		 divisor (integer)
 *		 quotient, integer part
 *		 quotient, fractional part
 *
 *	Routines return with MAR pointing at quotient fraction, no registers
 *	disturbed.
 *
 * 2's complement, 4-quadrant division of an integer+fraction by an integer
 *	producing an integer part
 *	and a PREC bit fractional part. (no sign bit)
 */

#define PREC 12		/* number of significant bits in fraction result */

/* Fractional result is right-justified in word. Binary point is to right
 *  of bit 2**PREC.  No sign bit in fraction.
 * The first step is to divide the integer part of the dividend by the divisor
 *	to produce an integer and a 15-bit (left-flush) fraction.
 * Then the fractional part of the dividend is divided to produce another
 *	fraction.  These two results are added to produce the final answer.
 *
 * Consult 2903 data book for details of integer division algorithm.
 *	This algorithm was then tweaked to achieve the necessary postamble
 *	for the operation REM/DEN => QUOfract.
 *
 *	USAGE:  dividend in _REM,_FRACT (destroyed)
 *		divisor  in _DEN
 *		constant: _ZERO
 *		working regs: _TMP, _SAVE
 *		results: INT part in _TMP, FRACT part in _SAVE
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"

#define _REM	14
#define _DEN	13
#define _SAVE	12
#define _TMP	11
#define _QUOI	10
#define _QUOF	9
#define _NUMSAV 8
#define _FRACT	7

ldivide()
{
newfile("ldivide.c");

label(LDIVIDE)				/* test entry point */
	_NS REGHOLD; GEGET; _ES

	_NS LOADREG(_REM,ALL16,MORE); _ES

	_NS LOADREG(_FRACT,ALL16,MORE); _ES

	_NS LOADREG(_DEN,ALL16,NOMORE); _ES

	_NS LOADMAR(_SAVE1); LDOUT; CONST(_SAVE1); _ES

	_NS RAM(RAMWR,_REM,INC); _ES

	_NS RAM(RAMWR,_FRACT,INC); _ES

	_NS RAM(RAMWR,_DEN,HOLD); DOJSUB(LONGDIVIDE16); _ES

	_NS RAM(RAMRD,_TMP,HOLD); LDOUT; INTERRUPTHOST; _ES

label(DISPLA_REST)
	_NS REGHOLD; DECMAR; _ES

	_NS RAM(RAMRD,_TMP,HOLD); LDOUT; INTERRUPTHOST; DOJUMP(GETCMD); _ES


/*================================================================*/
/*  LONGDIVIDE16
/*================================================================*/

label(LONGDIVIDE16)
	_NS LOADMAR(_REGSAVE); CONST(_REGSAVE); _ES

	_NS RAM(RAMWR,7,INC); DOJSUB(STM8UP); _ES

	_NS REGREG(ZERO,0,_ZERO); _ES

	_NS LOADDI(OUTPUTREG); SETROP(0,ALL16); SETSOP(NONQOP,_TMP,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE); LDMAR;
	 COND(IFFALSE); SEQ(CJPP); _ES		/* load MAR with param ptr */

	_NS IMMREG(ADD,4,_TMP); LDOUT; CONST(4); _ES
						/* fix up pointer for later */

	_NS RAM(RAMRD,_REM,INC); _ES

	_NS REGREG(MOVE,_REM,_NUMSAV); _ES	/* save numerator */

	_NS RAM(RAMRD,_FRACT,INC); COND(IFNNEG); DOJUMP(REGULAR_DIV); _ES
		/* get fract; point to DEN; if num pos, skip next junk */

	_NS REGHOLD; COND(IFZ); DOJUMP(REGULAR_DIV); _ES
			/* if num is exact neg intgr, do normal divide */

	_NS REGREG(INCR,_REM,_REM); _ES	/* else int part needs to be 2s comp*/

label(REGULAR_DIV)
	_NS RAM(RAMRD,_DEN,INC); DOJSUB(INT_DIVIDE); _ES
				/* start test for zero-divide */
				/* go divide - result in _TMP, rem in _REM */

	_NS REGREG(MOVE,_REM,_SAVE); DOJSUB(FRACT_DIV_16); _ES
			/* save "whole" remainder - go do fract part */
			/* result in _SAVE */

	_NS REGREG(MOVE,_TMP,_QUOI); _ES

	_NS SETROP(_SAVE,NONE); SETSOP(NONQOP,_QUOF,RAMNOP);
	 ALUOP(MOVE); YQ(FLL,OLDQ,REGWRE); DOJSUB(DO_DBL_FRACT); _ES
		/* save results away (fract left just'd); go do fract */

label(FINISH_LDIV16)
	_NS RAM(RAMWR,_QUOI,INC); _ES

	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_QUOF,RAMWR);
	 ALUOP(MOVE); FTODO; LDMAR; CONST(_REGSAVE); _ES
			/* while writing out fract, load adr of save area */

	_NS RAM(RAMRD,7,INC); DOJSUB(LM8UP); _ES

	_NS LOADDI(OUTPUTREG); SETROP(0,ALL16); SETSOP(NONQOP,0,RAMNOP);
	 ALUOP(MOVE); FTODO; LDMAR;
	 COND(IFFALSE); SEQ(CJPP); _ES

	_NS REGHOLD; SEQ(RETN); _ES
				/* load MAR, pt to fract, exit */

/*================================================================*/
/*  LONGDIVIDE
/*================================================================*/
/* tiny difference - rightshift fract 4 places. */
label(LONGDIVIDE)
	_NS LOADMAR(_REGSAVE); CONST(_REGSAVE); _ES

	_NS RAM(RAMWR,7,INC); DOJSUB(STM8UP); _ES

	_NS REGREG(ZERO,0,_ZERO); _ES

	_NS LOADDI(OUTPUTREG); SETROP(0,ALL16); SETSOP(NONQOP,_TMP,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE); LDMAR;
	 COND(IFFALSE); SEQ(CJPP); _ES		/* load MAR with param ptr */

	_NS IMMREG(ADD,4,_TMP); LDOUT; CONST(4); _ES
						/* fix up pointer for later */

	_NS RAM(RAMRD,_REM,INC); _ES

	_NS REGREG(MOVE,_REM,_NUMSAV); _ES	/* save numerator */

	_NS RAM(RAMRD,_FRACT,INC); COND(IFNNEG); DOJUMP(REG_DIV); _ES	
							/* point to DEN */

	_NS REGHOLD; COND(IFZ); DOJUMP(REG_DIV); _ES
			/* int is exact neg int - do  normally */

	_NS REGREG(INCR,_REM,_REM); _ES

label(REG_DIV)
	_NS RAM(RAMRD,_DEN,INC); DOJSUB(INT_DIVIDE); _ES
				/* start test for zero-divide */
				/* go divide - result in _TMP, rem in _REM */

	_NS REGREG(MOVE,_REM,_SAVE); DOJSUB(FRACT_DIV_16); _ES
			/* save "whole" remainder - go do fract part */
			/* result in _SAVE */

	_NS REGREG(MOVE,_TMP,_QUOI); _ES

	_NS SETROP(_SAVE,NONE); SETSOP(NONQOP,_QUOF,RAMNOP);
	 ALUOP(MOVE); YQ(FLL,OLDQ,REGWRE); DOJSUB(DO_DBL_FRACT); _ES
		/* make into left-justified fract ; go div fract part */

label(FINISH_LDIV)
	_NS LOADDI(UCONST); RAM(RAMWR,_QUOI,INC); SEQ(LDCT); CONST(3); _ES

label(RSHIFT4)
	_NS SETROP(_QUOF,NONE); SETSOP(NONQOP,_QUOF,RAMNOP);
	 ALUOP(MOVE); YQ(FLR,OLDQ,REGWRE); SEQ(RPCT); NEXT(RSHIFT4); _ES
						/* right-shift fract */

	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_QUOF,RAMWR);
	 ALUOP(MOVE); FTODO; LDMAR; CONST(_REGSAVE); _ES
			/* while writing out fract, load adr of save area */

	_NS RAM(RAMRD,7,INC); DOJSUB(LM8UP); _ES

	_NS LOADDI(OUTPUTREG); SETROP(0,ALL16); SETSOP(NONQOP,0,RAMNOP);
	 ALUOP(MOVE); FTODO; LDMAR;
	 COND(IFFALSE); SEQ(CJPP); _ES

	_NS REGHOLD; SEQ(RETN); _ES
				/* load MAR, pt to fract, exit */


/*================================================================*/
/* 	fractional part subroutine
/*================================================================*/

label(DO_DBL_FRACT)
	_NS SETROP(_FRACT,NONE); SETSOP(NONQOP,_TMP,RAMNOP);
	 ALUOP(MOVE); YQ(FF,LDQ,REGWRE); _ES
			/* load Q with numerator to prep for integer divide */

	_NS REGREG(MOVE,_NUMSAV,_NUMSAV); _ES	/* test true sign of num */

label(FTEST_NUM)
	_NS REGREG(ZERO,0,_REM); COND(IFNNEG); DOJUMP(FTEST_POS); _ES
			/* if numerator >=0, no negation. Smear msw. */

	_NS REGREG(ONES,0,_REM); _ES			/* neg smear msw */

	_NS REGREG(COM2,_TMP,_TMP); DOJUMP(FTEST_FRAC); _ES
		/* negate num */

label(FTEST_POS)
	_NS REGREG(MOVE,_TMP,_TMP); _ES	/* look at "sign" of fract */

label(FTEST_FRAC)
	_NS REGREG(MOVE,_DEN,_SAVE); COND(IFNEG); DOJUMP(FDO_INT_DIV); _ES
		/* save & test denom; if fract still "neg" it's big */

label(FTEST_DEN)
	_NS REGCOMP(GE,_TMP,_SAVE); COND(IFNNEG); DOJUMP(FTEST_INT); _ES
			/* compare num and denom; skip if denom not neg */

	_NS REGREG(COM2,_SAVE,_SAVE); _ES
					/* negate num */

	_NS REGCOMP(GE,_TMP,_SAVE); _ES

label(FTEST_INT)
	_NS REGREG(MOVE,_FRACT,_SAVE); COND(IFGE); DOJUMP(FDO_INT_DIV); _ES
			/* if num > denom, must do integer phase;
			 * test num in order to sign-smear;
			 * save it for postamble */

	_NS SETROP(0,NONE); SETSOP(NONQOP,_TMP,RAMNOP);
	 ALUOP(ZERO); YQ(FF,LDQ,REGWRE); DOJUMP(DFRACADD); _ES
		/* zero Q reg & result and skip (we know quo is 0) */

label(FDO_INT_DIV)
		/* now doubleword dividend is in _REM | Q... divisor in _DEN
		 */

	_NS DIVFIRST(_DEN,_REM); /*LOADDI(UCOUNT);*/ DOPUSH(13); _ES
				/* first divide cycle (calc sign of quo) */

	_NS DIV(_DEN,_REM); SEQ(LOUP); _ES
				/* iterate (integer) divide cycles */

	_NS DIVCOR(_DEN,_REM); _ES

	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(QOPERAND,0,RAMNOP);
	 ALUOP(ORRS); YQ(FF,LDQ,REGWRD); CONST(1); _ES
				/* correction to correction! "or" one to Q */

/* integer postamble (straight from the 2903 book) */

	_NS REGREG(MOVE,_REM,_REM); _ES		/* examine remainder */

	_NS SETROP(_NUMSAV,NONE); SETSOP(NONQOP,_REM,RAMNOP);
	 ALUOP(ANDRS); FTODO; COND(IFZ); DOJUMP(DFRACDONE); _ES
		/* if rem=0, skip postamble; test sign of saved num & rem */

	_NS REGCOMP(EQ,_DEN,_REM); COND(IFNNEG); DOJUMP(FNORMCHK); _ES
			/* if both non-neg, go do normal correction check;
			 * first half of magnitude check */

	_NS SETROP(_DEN,NONE); SETSOP(NONQOP,_REM,RAMNOP);
	 ALUOP(ADD); FTODO; COND(IFEQ); DOJUMP(FEQMAG); _ES
			/* other half of magnitude check (neg divisor) */

	_NS SETROP(0,NONE); SETSOP(QOPERAND,_TMP,RAMNOP); ALUOP(SONLYOP,P0);
	 YQ(FF,OLDQ,REGWRE); COND(IFEQ); DOJUMP(FEQMAGNXT); _ES
				/* if equal mag, go test Q for special corr;
				 * save integer quo in TEMP */

label(DFRACDONE)
	_NS SETROP(0,NONE); SETSOP(QOPERAND,_TMP,RAMNOP); ALUOP(SONLYOP,P0);
	 YQ(FLL,OLDQ,REGWRE); COND(IFOVF); DOJUMP(OVERFLO); _ES
			/* save final integer quotient in TEMP register */
			/* with final left shift */

label(DFRACADD)
	_NS REGREG(ADD,_TMP,_QUOF); PROPOUT16; COND(IFNEG);DOJUMP(NEGADD);_ES
				/* add two fractional parts together */
				/* if frac neg, go sign smear msw */

	_NS REGREG(MOVE,_QUOI,_QUOI); PROPIN; SEQ(RETN); _ES
			/* propagate to integer part */

label(NEGADD)
	_NS REGREG(SUBSR,_ZERO,_QUOI); PROPIN; SEQ(RETN); _ES
			/* tricky: quoi+(-1)+cin = quoi-(0)-1+cin */

/*================================================================*/
/* 	postamble branches
/*================================================================*/


label(FNORMCHK)
	_NS SETROP(_REM,NONE); SETSOP(NONQOP,_NUMSAV,RAMNOP);
	 ALUOP(ORRS); FTODO; _ES
				/* check if remainder and dividend positive */

	_NS SETROP(0,NONE); SETSOP(QOPERAND,_TMP,RAMNOP); ALUOP(SONLYOP,P0);
	 YQ(FF,OLDQ,REGWRE); COND(IFNNEG); DOJUMP(DFRACDONE);_ES
					/* if so, exit; examine and save Q */

	_NS REGHOLD; COND(IFNEG); DOJUMP(FADDONE); _ES
						/* if Q neg, correct +1	*/

label(FSUBONE)
/*	_NS REGREG(ADD,_DEN,_REM); _ES		/* correct remainder */

	_NS SETROP(_ZERO,NONE); SETSOP(QOPERAND,_TMP,RAMNOP); ALUOP(SUBSR);
	 YQ(FF,LDQ,REGWRE); DOJUMP(DFRACDONE); _ES
		 				/* correct Q is Q-1; exit */

label(FEQMAG)
	_NS SETROP(0,NONE); SETSOP(QOPERAND,_TMP,RAMNOP); ALUOP(MOVE);
	 YQ(FF,OLDQ,REGWRE); _ES

label(FEQMAGNXT)
	_NS REGHOLD; COND(IFNEG); DOJUMP(FSUBONE); _ES
				/* decrement negative quotients (corr A) */

label(FADDONE)
/*	_NS REGREG(SUBSRC,_DEN,_REM); _ES	/* correct remainder */

	_NS SETROP(0,NONE); SETSOP(QOPERAND,_TMP,RAMNOP); ALUOP(SONLYOP,P1);
	 YQ(FF,LDQ,REGWRE); DOJUMP(DFRACDONE); _ES
						/* correct Q is Q+1; exit */
}
