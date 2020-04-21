/* lowmem.c	--         Initialization / Selftest	<< GF2/UC4  >>
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"

#define _SCRATCHSIZE	4095

#define UNDEFINED	REGREG(ANDOP,P0,0,0); SEQ(JUMP); NEXT(UNDEF_TRAP)
/*	 rejection of unimplemented command vectors	*/

#define UNIMPLEMENTED	REGREG(ANDOP,P0,0,0); SEQ(JUMP); NEXT(UNIMPL_TRAP)
		/*	inform host of unimplemented command		*/

#define REGNOP	REGREG(ANDOP,P0,0,0)
#define CLRZERO	SETROP(0,NONE); SETSOP(NONQOP,_ZERO,RAMNOP); \
		ALUOP(FLOWOP,P0); FTOYANDQ(FF,LDQ,REGWRE)
						/*	conveniences	*/

lowmem()
{
newfile("lowmem.c");

declare(dummy2)
declare(dummy1)

label(PASS_THRU)	/* follows dispatch.c */

/*	handle PassThru command words. The left byte contains N, the
 *	number of words (beyond fbc cmd) passed.
 *	Save this in a reserved register
 *	for commands that need to know it.
 */
	  _NS LOADREG(_PASSN,LO8,NOMORE); _ES
					/* exact N count into ls byte	*/

	  _NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(DISPATCH); _ES
				/* get new command	*/


label(UNDEF_TRAP)
	_NS LOADIMM(0,_INTILLEGAL); DOTOOUTREG; INTERRUPTHOST; COND(IFFALSE);
	 SEQ(JUMP); CONST(_INTILLEGAL); _ES
				/* tell host about illegal instruction	*/

label(UNDEFCONT)
	_NS LOADREG(0,ALL16,NOMORE); DOTOOUTREG; INTERRUPTHOST; _ES
			/*  put illegal code into out reg for host to read */

	_NS LOADDI(OUTPUTREG); REGNOP; COND(IFFALSE); SEQ(CJPP); _ES

label(UNDEF_LOOP)
	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(UNDEF_LOOP); _ES
						/* flush pipe forever */


label(UNIMPL_TRAP)
	_NS LOADIMM(0,_INTUNIMPL); DOTOOUTREG; INTERRUPTHOST; COND(IFFALSE);
	 SEQ(JUMP); CONST(_INTUNIMPL); _ES
				  /* report unimplemented command  */

	_NS REGNOP; SEQ(JUMP); NEXT(UNDEFCONT); _ES	/* go trap	*/


label(INIT_START)

/*	tests some basic machine functions, then initializes scratch ram
 *	if machine healthy, "1" left in outreg; otherwise sequence infloops.
 */

	_NS REGREG(RONLYOP,P0,_ZERO,0); DOTOMAR(LOAD); COND(IFNZ);
	 SEQ(JUMP); NEXT(V00); _ES	/* clear MAR; copy 0 to reg. 0	   */
		/* if reg. not cleared by instruc. 0, go trap (test IFNZ)   */

	_NS REGREG(SUBSROP,P0,0,_ZERO); COND(IFNEG); SEQ(JUMP); NEXT(V00); _ES
			/* decrement _ZERO to -1; test alu sign bit	*/

	_NS REGREG(RONLYOP,P1,_ZERO,_ZERO); SETLED; DOTOOUTREG;
	 COND(IFNNEG); SEQ(JUMP); NEXT(V00); _ES
	   /*  inc _ZERO to 0, load outreg (LED set); test for sign bit set */

	_NS REGREG(FHIGHOP,P0,0,_ZERO); COND(IFNZ); SEQ(JUMP); NEXT(V00); _ES
			/* test previous increment operation	*/

	_NS LOADDI(OUTPUTREG); SETROP(0,ALL16); SETSOP(NONQOP,_ZERO,RAMNOP);
	 ALUOP(RONLYOP,P0); FTOYANDQ(FF,OLDQ,REGWRE);COND(IFFALSE); SEQ(CJPP); _ES
		/* recycle outreg into _ZERO */

	_NS REGNOP; COND(IFNZ); SEQ(JUMP); NEXT(V00); _ES
		/* test for correct DI bus enabling, outreg loading	*/

	_NS REGREG(FHIGHOP,P0,0,_ZERO); SEQ(JSUB); NEXT(STM15UP); _ES
		/* test ram write, jsub; go write ones to ram loc. 0	*/

	_NS CLRZERO; SEQ(JSUB); NEXT(LM15UP); _ES
		/* clear reg, go read ones back in			*/

	_NS CLRZERO; COND(IFZ); SEQ(JUMP); NEXT(V00); _ES
						/* test results and IFZ	*/

/* byteswap and carryin mux test */
	_NS REGREG(FHIGHOP,P0,0,1); _ES		/* A = -1 */

	_NS REGREG(FLOWOP,P0,0,3); _ES		/* C = 0  */

	_NS REGREG(RONLYOP,P1,3,2); _ES		/* B = 1  */

	_NS LOADDI(OUTPUTREG); SETROP(1,LO8); SETSOP(NONQOP,4,RAMNOP);
	 ALUOP(RONLYOP,P0); FTOYANDQ(FF,OLDQ,REGWRE);
	 COND(IFFALSE); SEQ(CJPP); _ES
				/* outreg holds 0 - make a word 0xff00 */

	_NS SETSOP(NONQOP,4,RAMNOP); ALUOP(SONLYOP,P0); YTODO(SWAP);
	 FTOYANDQ(FF,OLDQ,REGWRD); DOTOOUTREG;    /* 0x00ff into outreg */
	 COND(IFNNEG); SEQ(JUMP); NEXT(V00); _ES   /* test 0xff00 for neg */

	_NS LOADDI(OUTPUTREG); SETROP(0,ALL16); SETSOP(NONQOP,4,RAMNOP);
	 ALUOP(ADDOP,P1); FTOYANDQ(FF,OLDQ,REGWRE);
	 COND(IFFALSE); SEQ(CJPP); _ES	/* 0xff00 + 0x00ff + 1 = 0 */

	_NS SETROP(2,NONE); SETSOP(NONQOP,1,RAMNOP); ALUOP(ADDOP,P0);
	 FTOYANDQ(FF,OLDQ,REGWRD); PROPOUT12;		/* A + B = 0 */
	 COND(IFNZ); SEQ(JUMP); NEXT(V00); _ES	/* check prev result */

	_NS SETROP(3,NONE); SETSOP(NONQOP,3,RAMNOP); ALUOP(RONLYOP,P0);
	 FTOYANDQ(FF,OLDQ,REGWRD); PROPIN;	/* try prop'g a 1 (C+ci)*/
	 COND(IFNZ); SEQ(JUMP); NEXT(V00); _ES	/* check for lsw = 0	*/

	_NS REGREG(ADDOP,P0,2,1); PROPOUT16;		/* A += B (=0 lsw) */
	 COND(IFZ); SEQ(JUMP); NEXT(V00); _ES		/* check for cin=1 */

	_NS REGREG(RONLYOP,P0,3,3); PROPIN;	/* C += Cin(1) (=1 msw) */
	 COND(IFNZ); SEQ(JUMP); NEXT(V00); _ES	/* check for 0 lsw result */

/* now A = 0    B = 1    C = 1  */
	_NS REGREG(RONLYOP,P0,1,1); PROPOUT12;	/* try prop'g a 0 */
	 COND(IFZ); SEQ(JUMP); NEXT(V00); _ES	/* check for 1 msw result */

	_NS REGREG(RONLYOP,P0,1,1); PROPIN; _ES	/* prop 0 into msw */

	_NS REGREG(SUBRSOP,P1,1,2); PROPOUT16;	/* A - B = -1 lsw */
	 COND(IFNZ); SEQ(JUMP); NEXT(V00); _ES	/* check for 0 cin to msw */

	_NS REGREG(SUBRSOP,P0,3,1); PROPIN;	 /* C - A - 1 + Cin = 0 msw */
	 COND(IFNNEG); SEQ(JUMP); NEXT(V00); _ES /* check for -1 lsw */

	_NS REGREG(RONLYOP,P1,_ZERO,1);		/* make a 1	 */
	 COND(IFNZ); SEQ(JUMP); NEXT(V00); _ES  /* check prev op */

/*   now initialize scratch parameters	*/

	_NS LOADMAR(1); COND(IFFALSE); SEQ(JUMP); CONST(1); _ES
						/* address low ram	*/

	_NS LOADDI(UCOUNT); REGNOP; SEQ(PUSH); CONST(_ENDINIT-1); _ES
					/* clear bottom scratch consts */

	_NS RAM(RAMWR,_ZERO,INC); SEQ(LOUP); _ES

	_NS LOADMAR(_DIVTAB_VALID); CONST(_DIVTAB_VALID); _ES
		/* hack for now ----- should be done whenever tbl fixed */

	_NS RAM(RAMWR,1,HOLD); _ES	/* set valid flag */

/*  GF2 scratch ram is always the same -- report it */

	_NS LOADIMM(_TEMP,_SCRATCHSIZE); DOTOOUTREG;
	 COND(IFFALSE); SEQ(JUMP); CONST(_SCRATCHSIZE); _ES

	_NS LOADIMM(_TEMP,_ENDOFSCRATCH); CONST(_ENDOFSCRATCH); _ES
					/* end of non-table scratch area */

	_NS LOADMAR(_HITSTACKPTR); COND(IFFALSE);SEQ(JUMP); CONST(_HITSTACKPTR); _ES

	_NS RAM(RAMWR,_TEMP,INC); _ES		/* save in HITSTACKPTR	*/

	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMWR);
	 ALUOP(RONLYOP,P0); FTOYANDQ(FF,OLDQ,REGWRD);
	 DOTOMAR(LOAD);  CONST(_MASKLIM); _ES	/* and in HITSTACKLIM   */

	_NS RAM(RAMWR,_TEMP,HOLD); _ES		/* and in MASKLIM */

		/* end of initialization	*/

/*********** central command fetch ***************/

label(GETCMD)
	 _NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(DISPATCH); _ES

label(OUTSEND)	/* let host spy on outreg; inc MAR */

	_NS LOADDI(OUTPUTREG); REGNOP; COND(IFFALSE); SEQ(CJPP); _ES
						/* enable outreg read  */

	_NS REGNOP; DOTOMAR(INC); SEQ(RETN); _ES  /* inc MAR and return */

#ifdef NOZSCAN
label(ZSHADESCANLINE)
label(ZSCAN_INIT)
label(Z_VEC_DRAW)
	_NS REGNOP; SEQ(JUMP); NEXT(UNIMPL_TRAP); _ES
#endif

#ifdef NORUNLEN
label(RUN_LENGTH)
label(READ_RUNLENGTH)
	_NS REGNOP; SEQ(JUMP); NEXT(UNIMPL_TRAP); _ES
#endif


label(FREE_HIGH_2K)	/* to be called by user code (rev 2.3 & later) */
#ifdef _DIVTAB_VALID
	_NS LOADMAR(_SAVE2); CONST(_SAVE2); _ES

	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,0,RAMWR);
 	 ALUOP(RONLYOP,P0); FTOYANDQ(FF,OLDQ,REGWRE);
	 DOTOMAR(LOAD); CONST(_DIVTAB_VALID); _ES
 		/* save reg 0, point to flag */

	_NS REGREG(FLOWOP,P0,0,0); _ES	/* make a 0 */

	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,0,RAMWR);
	 ALUOP(RONLYOP,P0); FTOYANDQ(FF,OLDQ,REGWRE);
	 DOTOMAR(LOAD); CONST(_SAVE2); _ES
		/* write a 0, point to saved value */

	_NS RAM(RAMRD,0,HOLD); SEQ(RETN); _ES
#else
	_NS REGNOP; SEQ(RETN); _ES
#endif
}
