/* divide.c
 *
 *	division subroutines:
 *		DIVIDE
 *		DIVIDE16
 *	and the necessary subroutines.
 *	includes table optimization for small num/den
 *
 * 2's complement, 4-quadrant division of two integers with PREC
 *	sig. digits each, producing an integer part
 *	and a PREC+1 bit 2's comp fractional part.
 *
 * (numbers of PREC-bit resolution can be divided with full fractional 
 *  accuracy)
 * Fractional result is right-justified in word. Binary point is to right
 *  of bit PREC.  Single sign bit is to left.
 * PREC is limited to 15.  In this special case, a final left-shift is done
 *   to get rid of the sign bit.  This facilitates doubleword operations.
 *
 * Consult 2903 data book for details of integer division algorithm.
 *	This algorithm was then tweaked to achieve the necessary postamble
 *	for the operation REM/DEN => QUOfract.
 *
 *	USAGE:  dividend in _REM
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
#define _num	11
#define _denom	12
#define _tmp	13

divide()
{
newfile("divide.c");

label(DIV_COMMAND)
	_NS REGHOLD; GEGET; _ES

	_NS LOADREG(_REM,ALL16,MORE); _ES

	_NS LOADREG(_DEN,ALL16,NOMORE); _ES

	_NS LOADMAR(_SAVE1); LDOUT; CONST(_SAVE1); _ES

	_NS RAM(RAMWR,_REM,INC); _ES

	_NS RAM(RAMWR,_DEN,HOLD); DOJSUB(DIVIDE16); _ES

	_NS RAM(RAMRD,_TMP,HOLD);LDOUT;INTERRUPTHOST; DOJUMP(DISPLA_REST); _ES


/*================================================================*/
/*  DIVIDE (12 bit fract )
/*================================================================*/

label(DIVIDE)
	_NS LOADMAR(_DIVTAB_VALID); CONST(_DIVTAB_VALID); _ES
			/* valid flag; precedes _REGSAVE	*/

	_NS SETROP(0,NONE); SETSOP(NONQOP,0,RAMRD);
	 ALUOP(SONLYOP,P0); FTODO; INCMAR; _ES
					/* examine flag; pt to save area */

	_NS RAM(RAMWR,_num,INC); COND(IFZ); DOJUMP(IGNORE_TABLE); _ES
		/* create 3 working registers; if table bad, don't use */

	_NS RAM(RAMWR,_denom,INC); _ES
	
	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_tmp,RAMWR);
	 ALUOP(MOVE); YQ(FF,LDQ,REGWRE); CONST(-32); _ES
			/* while saving reg, load it with a const */
			/* also loads Q with 0s in 4 lsbs */

/* perform test for table optimization: 5 sig bits or fewer in both num and
 * denom.  Note that -32 must be tested for, since it is out of range.
 */

	_NS LOADDI(OUTPUTREG); SETROP(0,ALL16); SETSOP(NONQOP,0,RAMNOP);
	 ALUOP(MOVE); FTODO; LDMAR; COND(IFFALSE); SEQ(CJPP); _ES
					/* load MAR with param ptr */

	_NS RAM(RAMRD,_num,INC); _ES		/* get num, point to denom */

	_NS REGCOMP(EQ,_num,_tmp); COND(IFNEG); DOJUMP(OPT.-NUM); _ES

	_NS RAM(RAMRD,_denom,HOLD); _ES		/* positive num- get denom */

	_NS REGCOMP(EQ,_denom,_tmp); COND(IFNNEG); DOJUMP(OPT.SAME_SIGN); _ES

label(OPT.-DEN)
	_NS REGREG(COM2,_denom,_denom); COND(IFNQ);DOJUMP(OPT.OPP_SIGN);_ES
		/* negate denom; if it's -32 exactly, can't use table */

	_NS REGHOLD; DOJUMP(NORMAL_DIVIDE); _ES

label(OPT.-NUM)
	_NS REGREG(COM2,_num,_num); COND(IFNQ);DOJUMP(OPT.-TEST_DENOM); _ES
		/* negate numerator; if it's -32 exactly, can't use table */

	_NS REGHOLD; DOJUMP(NORMAL_DIVIDE); _ES

label(OPT.-TEST_DENOM)
	_NS RAM(RAMRD,_denom,HOLD); _ES		/* negative num - get denom */

	_NS REGCOMP(EQ,_denom,_tmp); COND(IFNNEG); DOJUMP(OPT.OPP_SIGN); _ES

	_NS REGREG(COM2,_denom,_denom); COND(IFNQ); DOJUMP(OPT.SAME_SIGN); _ES

	_NS REGHOLD; DOJUMP(NORMAL_DIVIDE); _ES

label(OPT.SAME_SIGN)
	_NS REGREG(ORRS,_num,_denom); _ES

	_NS IMMREG(ANDRS,0xffe0,_denom); CONST(0xffe0); _ES

	_NS RAM(RAMRD,_denom,HOLD); COND(IFNZ); DOJUMP(NORMAL_DIVIDE); _ES
					/* reread denom for later */

	_NS REGREG(MOVE,_num,_num); SWAPBYTE;
	 COND(IFNEG); DOJSUB(NEGATE_DEN); _ES	/* if neg, make it pos */

	_NS SETROP(_num,NONE); SETSOP(NONQOP,_num,RAMNOP);
	 ALUOP(MOVE); YQ(FLR,QR,REGWRE); DOJSUB(TBL_LOOKUP); _ES
				/* start creating index; go look up */
			/* return having read pos. integer result into _num */

	_NS RAM(RAMRD,_denom,HOLD); DOJUMP(TABLE_RESULT); _ES
					/* get fract result; go store */

label(OPT.OPP_SIGN)
	_NS REGREG(ORRS,_num,_denom); _ES

	_NS IMMREG(ANDRS,0xffe0,_denom); CONST(0xffe0); _ES

	_NS RAM(RAMRD,_denom,HOLD); COND(IFNZ); DOJUMP(NORMAL_DIVIDE); _ES

	_NS REGREG(MOVE,_num,_num); SWAPBYTE;
	 COND(IFNEG); DOJSUB(NEGATE_DEN); _ES	/* if neg, make it pos */

	_NS SETROP(_num,NONE); SETSOP(NONQOP,_num,RAMNOP);
	 ALUOP(MOVE); YQ(FLR,QR,REGWRE); DOJSUB(TBL_LOOKUP); _ES
				/* start creating index; go look up */
			/* return having read pos. integer result into _num */

	_NS REGRAM(COMPSOP,P1,0,_denom,DEC); PROPOUT12; _ES  /* take -fract */

	_NS REGRAM(COMPSOP,P1,0,_num,HOLD); PROPIN; DOJUMP(TABLE_RESULT); _ES
						/* take -int part; go store */

label(NEGATE_DEN)
	_NS REGREG(COM2,_denom,_denom); SEQ(RETN); _ES
						/* make denom positive */

label(TBL_LOOKUP)	/* create byte-swapped num >>3 */
	_NS SETROP(_num,NONE); SETSOP(NONQOP,_num,RAMNOP);
	 ALUOP(MOVE); YQ(FLR,QR,REGWRE); _ES

	_NS SETROP(_num,NONE); SETSOP(NONQOP,_num,RAMNOP);
	 ALUOP(MOVE); YQ(FLR,QR,REGWRE); _ES

	_NS SETROP(_num,NONE); SETSOP(NONQOP,_denom,RAMNOP);
	 ALUOP(ORRS); YQ(FLL,QL,REGWRE); _ES
					/* combine num | denom, left 1 */

	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_denom,RAMNOP);
	 ALUOP(ADD); FTODO; LDMAR; CONST(_DIVTAB); _ES
					/* ... plus base gives address */

	_NS RAM(RAMRD,_num,INC); SEQ(RETN); _ES	/* read integer part */


label(TABLE_RESULT)
	_NS LOADDI(OUTPUTREG); SETROP(0,ALL16); SETSOP(NONQOP,_tmp,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE); COND(IFFALSE); SEQ(CJPP); _ES
					/* get pointer to parameter area */

	_NS IMMREG(ADD,3,_tmp); LDOUT; LDMAR; CONST(3); _ES
					/* point to result area (fract) */

	_NS RAM(RAMWR,_denom,DEC); _ES		/* write out fract part */

	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_num,RAMWR);
	 ALUOP(MOVE); FTODO; LDMAR; CONST(_REGSAVE); _ES
		/* while writing out int, load adr of save area */

	_NS RAM(RAMRD,11,INC); _ES

	_NS RAM(RAMRD,12,INC); _ES

	_NS RAM(RAMRD,13,HOLD); DOJUMP(DIVRETURN); _ES

label(IGNORE_TABLE)	/* continue to save regs, do normal divide */
	_NS RAM(RAMWR,12,INC); _ES

	_NS RAM(RAMWR,13,INC); DOJUMP(NORMAL_SETUP); _ES

/*================================================================*/

label(NORMAL_DIVIDE)
	_NS LOADMAR(_REGSAVE+3); CONST(_REGSAVE+3); _ES

label(NORMAL_SETUP)
	_NS RAM(RAMWR,14,INC); _ES

	_NS SETROP(0,NONE); SETSOP(NONQOP,_ZERO,RAMWR);
	 ALUOP(ZERO); YQ(FF,OLDQ,REGWRE); _ES
			/* while saving reg, load it with 0 */

	_NS LOADDI(OUTPUTREG); SETROP(0,ALL16); SETSOP(NONQOP,_TMP,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE); LDMAR;
	 COND(IFFALSE); SEQ(CJPP); _ES
						/* load MAR with param ptr */

	_NS IMMREG(ADD,3,_TMP); LDOUT; CONST(3); _ES
						/* fix up pointer for later */

	_NS RAM(RAMRD,_REM,INC); _ES		/* get num, point  to DEN */

	_NS RAM(RAMRD,_DEN,INC); COND(IFZ); DOJUMP(D_ZERORESULT); _ES
							/* trivial result */

	_NS REGREG(MOVE,_DEN,_DEN); DOJSUB(INT_DIVIDE); _ES
			/* test denom; go do int part; result to _TMP */

	_NS REGREG(MOVE,_REM,_SAVE); DOJSUB(FRACT_DIV_16); _ES
			/* save num; go do fract part; result to _SAVE */

/* NOTE : later if there's space break out FRACT_DIV_12 to save 3 long,
	3 short states (costs 23 states space)
 */

	_NS LOADDI(UCONST); RAM(RAMWR,_TMP,INC); SEQ(LDCT); CONST(2); _ES

label(RSHIFT3)
	_NS SETROP(_SAVE,NONE); SETSOP(NONQOP,_SAVE,RAMNOP);
	 ALUOP(MOVE); YQ(FLR,OLDQ,REGWRE); SEQ(RPCT); NEXT(RSHIFT3); _ES
					/* right-shift fract */

	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_SAVE,RAMWR);
	 ALUOP(MOVE); FTODO; LDMAR; CONST(_REGSAVE); _ES
			/* while writing out fract, load adr of save area */

label(DIVEXIT)
	_NS RAM(RAMRD,11,INC); DOJSUB(LM12UP); _ES

label(DIVRETURN)
	_NS LOADDI(OUTPUTREG); SETROP(0,ALL16); SETSOP(NONQOP,0,RAMNOP);
	 ALUOP(MOVE); FTODO; LDMAR;
	 COND(IFFALSE); SEQ(CJPP); _ES

	_NS REGHOLD; SEQ(RETN); _ES
				/* load MAR, pt to fract, exit */

label(D_ZERORESULT)
	_NS RAM(RAMWR,_REM,INC); _ES	/* zero to int result */

	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_REM,RAMWR);
	 ALUOP(MOVE); FTODO; LDMAR; CONST(_REGSAVE); _ES
			/* while writing out fract, load adr of save area */

	_NS REGHOLD; DOJUMP(DIVEXIT); _ES

/*================================================================*/
/*  DIVIDE16
/*================================================================*/

label(DIVIDE16)
	_NS LOADMAR(_REGSAVE); CONST(_REGSAVE); _ES

	_NS RAM(RAMWR,11,INC); DOJSUB(STM12UP); _ES

	_NS REGREG(ZERO,0,_ZERO); _ES

	_NS LOADDI(OUTPUTREG); SETROP(0,ALL16); SETSOP(NONQOP,_TMP,RAMNOP);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE); LDMAR;
	 COND(IFFALSE); SEQ(CJPP); _ES		/* load MAR with param ptr */

	_NS IMMREG(ADD,3,_TMP); LDOUT; CONST(3); _ES
						/* fix up pointer for later */

	_NS RAM(RAMRD,_REM,INC); _ES		/* point  to DEN */

	_NS RAM(RAMRD,_DEN,INC); DOJSUB(INT_DIVIDE); _ES
			/* test denom; go do int part; result to _TMP */

	_NS REGREG(MOVE,_REM,_SAVE); DOJSUB(FRACT_DIV_16); _ES
			/* save num; go do fract part; result to _SAVE */

/* NOTE : later if there's space break out FRACT_DIV_12 to save 3 long,
	3 short states (costs 23 states space)
 */

	_NS RAM(RAMWR,_TMP,INC); _ES

	_NS SETROP(_SAVE,NONE); SETSOP(NONQOP,_SAVE,RAMNOP);
	 ALUOP(MOVE); YQ(FLL,QL,REGWRE); _ES	/* leftshift fract */

	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_SAVE,RAMWR);
	 ALUOP(MOVE); FTODO; LDMAR; CONST(_REGSAVE); _ES
			/* while writing out fract, load adr of save area */

	_NS RAM(RAMRD,11,INC); DOJSUB(LM12UP); _ES

	_NS LOADDI(OUTPUTREG); SETROP(0,ALL16); SETSOP(NONQOP,0,RAMNOP);
	 ALUOP(MOVE); FTODO; LDMAR;
	 COND(IFFALSE); SEQ(CJPP); _ES

	_NS REGHOLD; SEQ(RETN); _ES
				/* load MAR, pt to fract, exit */

/*================================================================*/
/*  integer part
/*================================================================*/

label(INT_DIVIDE)
	_NS SETROP(_REM,NONE); SETSOP(NONQOP,_TMP,RAMNOP);
	 ALUOP(MOVE); YQ(FF,LDQ,REGWRE); COND(IFZ); DOJUMP(ZERDIV); _ES
			/* load Q with numerator to prep for integer divide;
			 * move to TEMP for test subtract */

label(TEST_NUM)
	_NS REGREG(MOVE,_DEN,_SAVE); COND(IFNNEG); DOJUMP(TEST_DEN); _ES
			/* if numerator >=0, no negation. Move denom. */

	_NS REGREG(SUBRSC,_ZERO,_TMP); _ES
					/* negate num */
	
	_NS REGREG(MOVE,_DEN,_SAVE); _ES

label(TEST_DEN)
	_NS REGCOMP(GE,_TMP,_SAVE); COND(IFNNEG); DOJUMP(TEST_INTEGER); _ES
			/* compare num and denom; skip if denom not neg */

	_NS REGREG(SUBRSC,_ZERO,_SAVE); _ES
					/* negate num */

	_NS REGCOMP(GE,_TMP,_SAVE); _ES

label(TEST_INTEGER)
	_NS REGREG(MOVE,_REM,_SAVE); COND(IFGE); DOJUMP(DO_INT_DIV); _ES
			/* if num > denom, must do integer phase;
			 * test num in order to sign-smear;
			 * save it for postamble */

	_NS SETROP(0,NONE); SETSOP(NONQOP,0,RAMNOP);
	 ALUOP(ZERO); YQ(FF,LDQ,REGWRD); DOJUMP(INTDONE); _ES
		/* zero the Q reg and skip (we know int part of quo is 0) */

label(DO_INT_DIV)
	_NS REGREG(ZERO,0,_REM); COND(IFNNEG); DOJUMP(START_INT); _ES
					/* sign smear MSW of dividend */

	_NS REGREG(ONES,0,_REM); _ES

label(START_INT)
		/* now doubleword dividend is in _REM | Q... divisor in _DEN
		 */
	_NS SETROP(_REM,NONE); SETSOP(NONQOP,_REM,RAMNOP);
	 ALUOP(MOVE); YQ(FLL,QL,REGWRE); _ES
					/* initial upshift of rem, Q */

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

	_NS SETROP(_SAVE,NONE); SETSOP(NONQOP,_REM,RAMNOP);
	 ALUOP(ANDRS); FTODO; COND(IFZ); DOJUMP(INTDONE); _ES
		/* if rem=0, skip postamble; test sign of saved num & rem */

	_NS REGCOMP(EQ,_DEN,_REM); COND(IFNNEG); DOJUMP(NORMCHK); _ES
			/* if both non-neg, go do normal correction check;
			 * first half of magnitude check */

	_NS SETROP(_DEN,NONE); SETSOP(NONQOP,_REM,RAMNOP);
	 ALUOP(ADD); FTODO; COND(IFEQ); DOJUMP(EQMAG); _ES
			/* other half of magnitude check (neg divisor) */

	_NS SETROP(0,NONE); SETSOP(QOPERAND,_TMP,RAMNOP); ALUOP(SONLYOP,P0);
	 YQ(FF,OLDQ,REGWRE); COND(IFEQ); DOJUMP(EQMAGNXT); _ES
				/* if equal mag, go test Q for special corr;
				 * save integer quo in TEMP */

label(INTDONE)
	_NS SETROP(0,NONE); SETSOP(QOPERAND,_TMP,RAMNOP); ALUOP(SONLYOP,P0);
	 YQ(FF,OLDQ,REGWRE); COND(IFOVF); DOJUMP(OVERFLO); _ES
			/* save final integer quotient in TEMP register */

	_NS REGHOLD; SEQ(RETN); _ES

/*================================================================*/
/* 	fractional part
/*================================================================*/
#undef PREC
#define PREC 15		/* number of significant bits in fraction result */

label(FRACT_DIV_16)
	_NS REGREG(MOVE,_REM,_SAVE); _ES	/* save "whole" remainder */

	_NS SETROP(0,NONE); SETSOP(NONQOP,_ZERO,RAMNOP);
	 ALUOP(ZERO); YQ(FF,LDQ,REGWRE);
	 COND(IFZ); DOJUMP(FRACTDONE); _ES
			/* clear fractional num; if no remainder, skipit */

	_NS DIVFIRST(_DEN,_REM); /*LOADDI(UCOUNT);*/ DOPUSH(PREC-2); _ES
			/* start dividing; set up iterations for PREC bits */

	_NS DIV(_DEN,_REM); SEQ(LOUP); _ES
			/* iterate the fractional part */

	_NS DIVCOR(_DEN,_REM); _ES	/* correction */

	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(QOPERAND,0,RAMNOP);
	 ALUOP(ORRS); YQ(FF,LDQ,REGWRD); CONST(1); _ES
				/* correction to correction! "or" one to Q */

	_NS REGREG(MOVE,_REM,_REM); _ES		/* examine remainder */

	_NS REGCOMP(EQ,_DEN,_REM); COND(IFZ); DOJUMP(FRACTDONE); _ES
			/* if rem=0, done!; first half of magnitude check */

	_NS SETROP(_DEN,NONE); SETSOP(NONQOP,_REM,RAMNOP);
	 ALUOP(ADD); FTODO; COND(IFEQ); DOJUMP(F_EQMAG); _ES
			/* other half of magnitude check (neg divisor) */

	_NS REGREG(MOVE,_DEN,_DEN); COND(IFEQ); DOJUMP(F_EQMAGNXT); _ES
				/* if rem = |den| do A/B correction;
				 * examine denom for A/B determination */

	_NS SETROP(_REM,NONE); SETSOP(NONQOP,_SAVE,RAMNOP);
	 ALUOP(XOR); FTODO; _ES		/* divisor sign opposite remainder? */

	_NS SETROP(0,NONE); SETSOP(QOPERAND,_SAVE,RAMNOP); ALUOP(SONLYOP,P0);
	 YQ(FF,OLDQ,REGWRE); COND(IFNEG); DOJUMP(FCORR); _ES
					/* if not same, go do C/D corr;
					 * Q to SAVE temporarily */

label(FRACTDONE)
	_NS SETROP(0,NONE); SETSOP(QOPERAND,_SAVE,RAMNOP); ALUOP(SONLYOP,P0);
	 YQ(FF,OLDQ,REGWRE); COND(IFOVF); DOJUMP(OVERFLO); _ES
			/* move final fractional quotient to SAVE register */

	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_SAVE,RAMNOP);
	 ALUOP(ANDRS); FTODO; CONST(1<<PREC); _ES
				/* test sign of fraction */

	_NS SETROP(0,NONE); SETSOP(NONQOP,0,RAMNOP);
	 ALUOP(ZERO); YQ(FF,LDQ,REGWRD); COND(IFZ); SEQ(RETN); _ES
					/* clear Q; If fract negative.... */

	_NS REGREG(SUBSR,_ZERO,_TMP); SEQ(RETN); _ES
						/* make integer 1's comp */
/* routine exits with fract justified S.15 */

/*================================================================*/
/* 	integer postamble branches
/*================================================================*/


label(NORMCHK)
	_NS SETROP(_REM,NONE); SETSOP(NONQOP,_SAVE,RAMNOP);
	 ALUOP(ORRS); FTODO; _ES
				/* check if remainder and dividend positive */

	_NS SETROP(0,NONE); SETSOP(QOPERAND,_TMP,RAMNOP); ALUOP(SONLYOP,P0);
	 YQ(FF,OLDQ,REGWRE); COND(IFNNEG); DOJUMP(INTDONE);_ES
					/* if so, exit; examine and save Q */

	_NS REGHOLD; COND(IFNEG); DOJUMP(ADDONE); _ES
						/* if Q neg, correct +1	*/

label(SUBONE)
	_NS REGREG(ADD,_DEN,_REM); _ES		/* correct remainder */

	_NS SETROP(_ZERO,NONE); SETSOP(QOPERAND,_TMP,RAMNOP); ALUOP(SUBSR);
	 YQ(FF,LDQ,REGWRE); DOJUMP(INTDONE); _ES
		 				/* correct Q is Q-1; exit */

label(EQMAG)
	_NS SETROP(0,NONE); SETSOP(QOPERAND,_TMP,RAMNOP); ALUOP(SONLYOP,P0);
	 YQ(FF,OLDQ,REGWRE); _ES

label(EQMAGNXT)
	_NS REGHOLD; COND(IFNEG); DOJUMP(SUBONE); _ES
				/* decrement negative quotients (corr A) */

label(ADDONE)
	_NS REGREG(SUBSRC,_DEN,_REM); _ES	/* correct remainder */

	_NS SETROP(0,NONE); SETSOP(QOPERAND,_TMP,RAMNOP); ALUOP(SONLYOP,P1);
	 YQ(FF,LDQ,REGWRE); DOJUMP(INTDONE); _ES
						/* correct Q is Q+1; exit */

/*================================================================*/
/*  fraction postamble branches
/*================================================================*/

label(FCORR)
	_NS IMMREG(ANDRS,1<<PREC,_SAVE); CONST(1<<PREC); _ES
						/* test sign of Q */

	_NS REGHOLD; COND(IFNZ); DOJUMP(F_ADDONE); _ES	/* if set, Q+1 */

label(F_SUBONE)
	_NS SETROP(_ZERO,NONE); SETSOP(QOPERAND,0,RAMNOP); ALUOP(SUBSR);
	 YQ(FF,LDQ,REGWRD); DOJUMP(FRACTDONE); _ES
		 				/* correct Q is Q-1; exit */

label(F_EQMAG)
	_NS REGREG(MOVE,_DEN,_DEN); _ES		/* test sign of divisor */

label(F_EQMAGNXT)
	_NS REGHOLD; COND(IFNNEG); DOJUMP(F_SUBONE); _ES

label(F_ADDONE)
	_NS SETROP(0,NONE); SETSOP(QOPERAND,0,RAMNOP); ALUOP(SONLYOP,P1);
	 YQ(FF,LDQ,REGWRD); DOJUMP(FRACTDONE); _ES
						/* correct Q is Q+1; exit */

/*================================================================*/

label(ZERDIV)
label(OVERFLO)
	_NS REGREG(ONES,0,_TMP); LDOUT; INTERRUPTHOST; DOJUMP(GETCMD); _ES
					/* temporary error action */
}
