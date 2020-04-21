/* depthvec.c			<< GF2/UC4 >>
 *
 *	depth-cued vectors
 *
 *
 *	ASSUMPTIONS:
 *		Z values are integer (like X's and Y's)
 *		I = aZ+b for each endpoint
 *		|Z range| >= |I range|  so that  |a| <= 1
 *		slope = the integer a * 2**-14  (a = Si.ffffffffffffff)
 *		offset = the integer b
 *
 *	SYNOPSIS:
 *		MOVE
 *			[ GET_X1Y1 ]
 *			[ test DEPTHMODE (if zero, exit) ]
 *			subroutine GET_Z_INT:
 *			    input Z
 *			    calculate I1 = aZ1+b (limited)
 *			I1 -> DDASAI
 *			Q  -> DDASAF this should not be zero HPM!! 
 *
 *		DRAW
 *			[ if not ALTVECMODE go do normal draw (1 mask) ]
 *			[ if HITMODE go record ]
 *			[ GET_X2Y2 ]
 *			[ line stipple -> _STIP ]
 *			[ if not DEPTHMODE, test ZBUFFER;
 *				do Z buffered or multiview draw ]
 *			subroutine GET_Z_INT:
 *			    input Z
 *			    calculate I2 = aZ2+b (limited)
 *			I range = I2-I1
 *			test MULTIVIEW....
 *			determine octant and load ED and EC as for normal draw
 *			in the process, save "EC" (delta long axis)
 *			DDASD = deltaI = I range / "EC"
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "fbc.mic.h"
#include "consts.h"

depthvec()
{
newfile("depthvec.c");

#define _Z	8
#define _EC	9	/*  |delta long axis|	*/
#define _ADR	12


label(DEPTH_MOVE)

	_NS LOADMAR(_DEPTH_PARAMS); CONST(_DEPTH_PARAMS); _ES
				/* begin setting up to do I1 calculation */

	_NS SETSOP(NONQOP,0,RAMRD); ALUOP(SONLYOP,P0);
	 YQ(FF,LDQ,REGWRD); INCMAR; DOJSUB(GET_Z_INT); _ES
				/* "a" to Q reg; go get Z value */
				/* and calc I2 = a * Z + b */

	_NS REGREG(MOVE,_I2,_I1); BPC(LOADSAI); _ES
					/* really wanted I1 - load as is */
	_NS
	    REGREG(MOVE, _NEWFRAC_COLOR, _FRAC_COLOR);
	    LOADDI(UCONST); DOPUSH(3); 
	_ES

	_NS SETROP(_NEWFRAC_COLOR,NONE); SETSOP(NONQOP,_NEWFRAC_COLOR,RAMNOP);
	 ALUOP(MOVE); YQ(FLR,OLDQ,REGWRE); SEQ(LOUP); _ES

	_NS 
	    REGREG(MOVE, _NEWFRAC_COLOR, _NEWFRAC_COLOR);
	    BPC(LOADSAF);
	_ES				/* load the fractional part too */

	_NS REGRAMCOMP(LT,_I1,INC); _ES		/* check lower limit */

	_NS REGRAMCOMP(LE,_I1,HOLD);		/* check Imax */
	 COND(IFLT); DOJSUB(MINI); _ES		/* branch on too small */

	_NS GEGET; COND(IFLE); DOJUMP(DISPATCH); _ES	/* clear fract shade */

	_NS RAM(RAMRD,_I1,HOLD); BPC(LOADSAI); DOJUMP(DISPATCH); _ES
				/* I was too big - supply max and exit */

label(MINI)
	_NS REGHOLD; DECMAR; _ES		/* point to Imin again */

	_NS RAM(RAMRD,_I1,HOLD); BPC(LOADSAI); SEQ(RETN); _ES


label(DEPTH_DRAW)

/* already have read X2, Y2 and stuck line stipple in _STIP */

	_NS REGREG(MOVE,_STIP,_STIP); BPC(NOOP); _ES
		/* if stipple is to be loaded, do it now */

	_NS LOADMAR(_CONFIG+1); CONST(_CONFIG+1); _ES

	_NS IMMRAM(ANDRS,_CONFIGMASK,_TEMP,HOLD); BPC(LOADCONFIG);
	 CONST(_CONFIGMASK); _ES	/* force loadstip off */

	_NS LOADMAR(_DEPTH_PARAMS); CONST(_DEPTH_PARAMS); GEGET; _ES
				/* begin setting up to do I2 calculation */

	_NS SETSOP(NONQOP,0,RAMRD); ALUOP(SONLYOP,P0);
	 YQ(FF,LDQ,REGWRD); INCMAR; DOJSUB(GET_Z_INT); _ES
				/* "a" to Q reg; go get Z value */
				/* and calc I2 = aZ2+b */
				/* results to _I2; point to minI */

	_NS REGRAMCOMP(LT,_I2,INC); _ES	/* check lower limit */

	_NS REGRAMCOMP(GT,_I2,HOLD); COND(IFLT); DOJUMP(DMINI); _ES
					/* check Imax; branch on too small */

	_NS REGHOLD; COND(IFGT); DOJUMP(DMAXI); _ES
						/* if too big, supply Imax */
label(DSET)
	_NS REGREG(MOVE,_I1,_TEMP); _ES

	_NS REGREG(SUBRSC,_NEWFRAC_COLOR,_FRAC_COLOR); PROPOUT16; _ES

	_NS REGREG(SUBRSC,_I2,_TEMP); PROPIN; _ES /* I range = I2-I1  */

	_NS LOADIMM(_ADR,_SAVE1); LDMAR; CONST(_SAVE1); _ES

	_NS RAM(RAMWR,_TEMP,INC); _ES	/* set up numerator for deltaI calc */

	_NS RAM(RAMWR,_FRAC_COLOR,INC); _ES/* set up denominator for deltaI calc */

/* for now, ignore MUTLIVIEW dimension */

/* now determine octant and load Bres params, exactly as in VECSUB */

	  _NS SETROP(_X2,NONE);            /* Calculate X2 - X1 and set    */
           SETSOP(NONQOP,_X1,RAMNOP);      /* condition code.              */
	   ALUOP(SUBRSOP,P1);
	   FTOYANDQ(FF,LDQ,REGWRD);        /* Hold computed value in q reg */
	   YTODO(ALL16);                   /* temporarily.                 */
	   DOTOOUTREG;                     /* and transfer to BPC's ED     */
	   BPCCMD(LOADED);                 /* in case this is an octant 0r */
	  _ES				   /* or 3r vector.                */

	  _NS SETSOP(QOPERAND,_DELTAX,RAMNOP);
	   ALUOP(SONLYOP,P0);		   /* Transfer to _DELTAX,         */
	   FTOYANDQ(FF,OLDQ,REGWRE);
	   YTODO(ALL16);
	   DOTOOUTREG;                     /* and to BPC's EC register,    */
	   BPCCMD(LOADEC);                 /* just in case this is an      */
                                           /* octant 1 or 2 vector.        */
	   COND(IFNEG);                    /* If X2 - X1 is negative,      */
	   SEQ(JUMP);                      /* then the two coordinates are */
	   NEXT(DREV);_ES              /* reversed.  Jump to r octants.*/

/*(DNONREV)*/				/* At this point, _DELTAX holds */
					/* the correct positive value,  */
					/* the BPC's XS, YS, XE, and YE */
					/* registers are initialized,   */
					/* and the BPC's EC register    */
					/* holds DELTAX.                */

	  _NS SETROP(_Y2,NONE);            /* Calculate DELTAY,            */
	   SETSOP(NONQOP,_Y1,RAMNOP);
	   ALUOP(SUBRSOP,P1);              /* set condition code,          */
	   FTOYANDQ(FF,LDQ,REGWRD);        /* store in q register,         */
	   YTODO(ALL16);
	   DOTOOUTREG;                     /* and also store in BPC's ED   */
	   BPCCMD(LOADED);_ES              /* register for a winner if     */
                                           /* this is an octant 2 vector.  */

	  _NS SETROP(_DELTAX,NONE);        /* Calculate DELTAX - DELTAY    */
	   SETSOP(QOPERAND,0,RAMNOP);      /* to further discriminate the  */
	   ALUOP(SUBRSOP,P1);              /* octants,                     */
	   FTOYANDQ(FF,OLDQ,REGWRD);
	   YTODO(ALL16);
	   COND(IFNEG);                    /* and choose between octants   */
	   SEQ(JUMP);                      /* 0 or 1 and octants 2 or 3    */
	   NEXT(DOCT2OR3);_ES              /* based on the sign of DELTAY. */

/*(DOCT0OR1)*/
	  _NS SETROP(_Y1,NONE);            /* Guess octant 1 and put       */
	   SETSOP(NONQOP,_Y2,RAMNOP);      /* -DELTAY in BPC's ED reg.     */
	   ALUOP(SUBRSOP,P1);
	   FTOYANDQ(FF,OLDQ,REGWRD);
	   YTODO(ALL16);
	   DOTOOUTREG;
	   BPCCMD(LOADED);
	   COND(IFNNEG);		/* If DELTAX - DELTAY was >=    */
	   SEQ(JUMP);			/* 0, then octant 1 was correct */
	   NEXT(DOCT1); _ES		/* and we're ready to give the  */
					/* BPC the appropriate command. */

/*label(MUSTBEOCT0)*/
	  _NS IMMREG(SUBRSOP,P1,0,_DELTAX);/* Just our luck.  All guesses  */
	   DOTOOUTREG;                     /* were wrong.  Set the BPC's   */
	   BPCCMD(LOADED);                 /* ED register to -DELTAX,      */
	   COND(IFFALSE);
	   SEQ(JUMP);
	   CONST(0);_ES

	  _NS SETSOP(QOPERAND,_EC,RAMNOP);  /* set EC reg (and _EC) to dY */
	   ALUOP(SONLYOP,P0);
	   FTOYANDQ(FF,OLDQ,REGWRE);
	   YTODO(ALL16);
	   DOTOOUTREG;
	   BPCCMD(LOADEC);
	   SEQ(JUMP);                      /* and give the command.        */
	   NEXT(DOCT0); _ES

label(DOCT2OR3)
	  _NS SETROP(_DELTAX,NONE);        /* Calculate DELTAX + DELTAY    */
	   SETSOP(QOPERAND,0,RAMNOP);      /* to decide between octants 2  */
	   ALUOP(ADDOP,P0);                /* and 3.  DELTAY is negative,  */
	   FTOYANDQ(FF,OLDQ,REGWRD);       /* so this computation may or   */
	   YTODO(ALL16);_ES                /* may not return a negative.   */

	  _NS REGREG(MOVE,_DELTAX,_EC);	   /* deltaX is correct correction;*/
	   COND(IFNNEG);
	   SEQ(JUMP);                      /* condition code set by last   */
	   NEXT(DOCT2);_ES                 /* operation.  If this is octant*/
					   /* 2, then all is ready for the */
					   /* BPC.                         */
/*label(MUSTBEOCT3)*/
	  _NS IMMREG(SUBRSOP,P1,0,_DELTAX); /* Wrong guess.  Must reload ED */
	   DOTOOUTREG;                     /* with -DELTAX,                */
	   BPCCMD(LOADED);
	   COND(IFFALSE);
	   SEQ(JUMP);
	   CONST(0);_ES

	  _NS SETSOP(QOPERAND,_EC,RAMNOP);	/* reload EC with -DELTAY */
	   ALUOP(COMPSOP,P1);
	   YQ(FF,OLDQ,REGWRE);
	   DOTOOUTREG;
	   BPCCMD(LOADEC);
	   SEQ(JUMP);                      /* and give the command.        */
	   NEXT(DOCT3);_ES

label(DREV)			/* At this point, _DELTAX holds */
				/* the correct negative value,  */
				/* the BPC's XS, YS, XE, and YE */
				/* registers are initialized,   */
				/* and the BPC's ED register    */
				/* holds DELTAX.                */

	  _NS SETROP(_Y2,NONE);            /* Calculate DELTAY,            */
	   SETSOP(NONQOP,_Y1,RAMNOP);
	   ALUOP(SUBRSOP,P1);              /* set condition code,          */
	   FTOYANDQ(FF,LDQ,REGWRD);        /* store in q register,         */
	   YTODO(ALL16);
	   DOTOOUTREG;                     /* and also store in BPC's EC   */
	   BPCCMD(LOADEC);_ES              /* register for a winner if     */
                                           /* this is an octant 3r vector. */

	  _NS SETROP(_DELTAX,NONE);        /* Calculate DELTAX - DELTAY    */
	   SETSOP(QOPERAND,0,RAMNOP);      /* to further discriminate the  */
	   ALUOP(SUBRSOP,P1);              /* octants,                     */
	   FTOYANDQ(FF,OLDQ,REGWRD);
	   YTODO(ALL16);
	   COND(IFNNEG);                   /* and choose between octants   */
	   SEQ(JUMP);                      /* 0r or 1r and octants 2r or 3r*/
	   NEXT(DOCT0ROR1R); _ES           /* based on the sign of DELTAY. */

/*OCT2ROR3R:*/
	  _NS SETSOP(QOPERAND,_EC,RAMNOP); /* Guess octant 3R and put	*/
	   ALUOP(COMPSOP,P1);		   /* -DELTAY in BPC's EC reg,	*/
	   FTOYANDQ(FF,OLDQ,REGWRE);	   /* saving it in local reg.	*/
	   YTODO(ALL16);
	   DOTOOUTREG;
	   BPCCMD(LOADEC);
	   COND(IFNNEG);                   /* If DELTAX - DELTAY was >=    */
	   SEQ(JUMP);                      /* 0, then octant 3r was correct*/
	   NEXT(DOCT3R);_ES                /* and we're ready to give the  */
                                           /* BPC the appropriate command. */

/*MUSTBEOCT2R:*/
	  _NS REGREG(COMPROP,P1,_DELTAX,_EC); /* Just our luck- All guesses*/
	   DOTOOUTREG;                     /* were wrong.  Set the BPC's   */
	   BPCCMD(LOADEC);                 /* EC register to -DELTAX,      */
	   COND(IFFALSE);
	   SEQ(JUMP);
	   CONST(0);_ES

	  _NS SETSOP(QOPERAND,0,RAMNOP);  /* the ED register to DELTAY,   */
	   ALUOP(SONLYOP,P0);
	   FTOYANDQ(FF,OLDQ,REGWRD);
	   YTODO(ALL16);
	   DOTOOUTREG;
	   BPCCMD(LOADED);
	   SEQ(JUMP);                      /* and give the command.        */
	   NEXT(DOCT2R); _ES

label(DOCT0ROR1R)
	  _NS SETROP(_DELTAX,NONE);        /* Calculate DELTAX + DELTAY    */
	   SETSOP(QOPERAND,0,RAMNOP);      /* to decide between octants 0r */
	   ALUOP(ADDOP,P0);                /* and 1r.  DELTAX is negative, */
	   FTOYANDQ(FF,OLDQ,REGWRD);       /* so this computation may or   */
	   YTODO(ALL16);_ES                /* may not return a negative.   */

	  _NS SETSOP(QOPERAND,_EC,RAMNOP); /* save DELTAY as _EC	*/
	   ALUOP(SONLYOP,P0);
	   YQ(FF,OLDQ,REGWRE);
	   COND(IFNNEG);
	   SEQ(JUMP);                      /* condition code set by last   */
	   NEXT(DOCT0R); _ES               /* operation.  If this is octant*/
					   /* 0r, then all is ready for the*/
					   /* BPC.                         */
/*MUSTBEOCT1R:*/
	  _NS SETSOP(NONQOP,_DELTAX,RAMNOP);
	   ALUOP(COMPSOP,P1);		   /* Wrong guess.  Must reload EC */
	   YQ(FF,OLDQ,REGWRE);
	   DOTOOUTREG;                     /* with -DELTAX,                */
	   BPCCMD(LOADEC);
	   COND(IFFALSE);
	   SEQ(JUMP);
	   CONST(0);_ES

	  _NS SETROP(_Y1,NONE);            /* and ED with -DELTAY,         */
	   SETSOP(NONQOP,_Y2,RAMNOP);
	   ALUOP(SUBRSOP,P1);
	   FTOYANDQ(FF,OLDQ,REGWRD);
	   YTODO(ALL16);
	   DOTOOUTREG;
	   BPCCMD(LOADED);
	   SEQ(JUMP);                      /* and give the command.        */
	   NEXT(DOCT1R); _ES

/*================================================================*/

label(DMINI)
	_NS REGHOLD; DECMAR; _ES		/* point to Imin again */

	_NS REGRAM(SONLYOP,P1,0,_I2,HOLD); DOJUMP(DSET); _ES
							/* set to Imin+1 */

label(DMAXI)
	_NS REGRAM(SUBSR,_ZERO,_I2,HOLD); DOJUMP(DSET); _ES
							/* set to Imax-1 */

/*================================================================*/

label(DOCT0)
	_NS RAM(RAMWR,_EC,HOLD); DOJSUB(DIV_DI); _ES
			/* _EC holds |delta long axis| - go calc delta I */

	_NS REGREG(MOVE,_I1,_I1); BPC(OCT0VECT);
	 COND(IFZ); DOJUMP(DENDVEC) _ES
			/* sending the initial shade, draw the line */
			/* if DIV_DI returned with "zero" set, end of job */

	_NS REGHOLD; BPC(NOOP); DOJUMP(D_MVLOOP); _ES
			/* otherwise jump back into multiview loop */

label(DOCT1)
	_NS RAM(RAMWR,_DELTAX,HOLD); DOJSUB(DIV_DI); _ES
			/* -DELTAX is pos long axis - go calc delta I */

	_NS REGREG(MOVE,_I1,_I1); BPC(OCT1VECT);
	 COND(IFZ); DOJUMP(DENDVEC) _ES
			/* sending the initial shade, draw the line */
			/* if DIV_DI returned with "zero" set, end of job */

	_NS REGHOLD; BPC(NOOP); DOJUMP(D_MVLOOP); _ES
			/* otherwise jump back into multiview loop */

label(DOCT2)
	_NS RAM(RAMWR,_EC,HOLD); DOJSUB(DIV_DI); _ES
			/* _EC holds |delta long axis| - go calc delta I */

	_NS REGREG(MOVE,_I1,_I1); BPC(OCT2VECT);
	 COND(IFZ); DOJUMP(DENDVEC) _ES
			/* sending the initial shade, draw the line */
			/* if DIV_DI returned with "zero" set, end of job */

	_NS REGHOLD; BPC(NOOP); DOJUMP(D_MVLOOP); _ES

label(DOCT3)
	_NS RAM(RAMWR,_EC,HOLD); DOJSUB(DIV_DI); _ES
			/* _EC holds |delta long axis| - go calc delta I */

	_NS REGREG(MOVE,_I1,_I1); BPC(OCT3VECT);
	 COND(IFZ); DOJUMP(DENDVEC) _ES
			/* sending the initial shade, draw the line */
			/* if DIV_DI returned with "zero" set, end of job */

	_NS REGHOLD; BPC(NOOP); DOJUMP(D_MVLOOP); _ES

label(DOCT0R)
	_NS RAM(RAMWR,_EC,HOLD); DOJSUB(DIV_DI); _ES
			/* _EC holds |delta long axis| - go calc delta I */

	_NS REGREG(MOVE,_I1,_I1); BPC(OCT0RVECT);
	 COND(IFZ); DOJUMP(DENDVEC) _ES
			/* sending the initial shade, draw the line */
			/* if DIV_DI returned with "zero" set, end of job */

	_NS REGHOLD; BPC(NOOP); DOJUMP(D_MVLOOP); _ES

label(DOCT1R)
	_NS RAM(RAMWR,_DELTAX,HOLD); DOJSUB(DIV_DI); _ES
						/* DELTAX is long axis */

	_NS REGREG(MOVE,_I1,_I1); BPC(OCT1RVECT);
	 COND(IFZ); DOJUMP(DENDVEC) _ES
			/* sending the initial shade, draw the line */
			/* if DIV_DI returned with "zero" set, end of job */

	_NS REGHOLD; BPC(NOOP); DOJUMP(D_MVLOOP); _ES

label(DOCT2R)
	_NS RAM(RAMWR,_EC,HOLD); DOJSUB(DIV_DI); _ES
						/* DELTAX is long axis */

	_NS REGREG(MOVE,_I1,_I1); BPC(OCT2RVECT);
	 COND(IFZ); DOJUMP(DENDVEC) _ES
			/* sending the initial shade, draw the line */
			/* if DIV_DI returned with "zero" set, end of job */

	_NS REGHOLD; BPC(NOOP); DOJUMP(D_MVLOOP); _ES

label(DOCT3R)
	_NS RAM(RAMWR,_EC,HOLD); DOJSUB(DIV_DI); _ES
			/* _EC holds |delta long axis| - go calc delta I */

	_NS REGREG(MOVE,_I1,_I1); BPC(OCT3RVECT);
	 COND(IFZ); DOJUMP(DENDVEC) _ES
			/* sending the initial shade, draw the line */
			/* if DIV_DI returned with "zero" set, end of job */

	_NS REGHOLD; BPC(NOOP); DOJUMP(D_MVLOOP); _ES

/*================================================================*/

label(DENDVEC)
	_NS REGHOLD; BPC(NOOP); _ES

	_NS REGREG(MOVE,_I2,_I1); BPC(LOADSAI); _ES	/* fix up end shade */

	_NS
	    REGREG(MOVE, _NEWFRAC_COLOR, _FRAC_COLOR);
	    LOADDI(UCONST); DOPUSH(3); 
	_ES

	_NS SETROP(_NEWFRAC_COLOR,NONE); SETSOP(NONQOP,_NEWFRAC_COLOR,RAMNOP);
	 ALUOP(MOVE); YQ(FLR,OLDQ,REGWRE); SEQ(LOUP); _ES

	_NS 
	    REGREG(MOVE, _NEWFRAC_COLOR, _NEWFRAC_COLOR);
	    BPC(LOADSAF);
	_ES				/* load the fractional part too */

	_NS REGREG(MOVE,_X2,_X1); BPC(LOADXS); _ES	/* set up for draw */

	_NS REGREG(MOVE,_Y2,_Y1); BPC(LOADYS); _ES

	_NS LOADMAR(_CONFIG+1); CONST(_CONFIG+1); _ES
					/* restore color/config */

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(LOADCONFIG); _ES

	_NS LOADMAR(_COLOR+1); CONST(_COLOR+1); _ES	/* A/B planes only */

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(SETCOLORAB);
	 GEGET; DOJUMP(DISPATCH); _ES

/*================================================================*/

label(GET_Z_INT)
/*	subroutine to input Z and calculate a Z + b
 *	enter with a in Q reg, MAR pointing to B
 */
	_NS REGHOLD; GEGET; _ES		/* discard stereo X */

	_NS LOADREG(_Z,ALL16,NOMORE); _ES

/* multiply _Z times Q reg - result to _I2, with fraction in Q */
label(AZSUB)
	_NS REGREG(ZERO,0,_I2); LOADDI(UCOUNT); SEQ(LDCT); CONST(14); _ES

label(AZ1)
	_NS ALUOP(ZERO); FTODO; _ES	/* make sure DCOUT is clear next */

	_NS MUL(_Z,_I2); SEQ(RPCT); NEXT(AZ1); _ES

	_NS MULLAST(_Z,_I2); 
	    LOADDI(UCONST); DOPUSH(1); 
	_ES

/* binary point is two bits down from top of lsw, so doubleshift left 2 */
	_NS SETROP(_I2,NONE); SETSOP(NONQOP,_I2,RAMNOP);
	     ALUOP(MOVE); YQ(FLL,QL,REGWRE); SEQ(LOUP); _ES

	_NS 
	    SETROP(0, NONE); SETSOP(QOPERAND, _NEWFRAC_COLOR, RAMNOP);
	    ALUOP(SONLYOP, P0); YQ(FF,OLDQ,REGWRE);
	_ES

	_NS REGRAM(ADD,_I2,_I2,INC); SEQ(RETN); _ES
				/* add offset; return pointing to Imin */

label(DIV_DI)
/* Subroutine to finish calc deltaI / delta long axis  giving DDASD
 * and provide multiviewport looping
 * Enter having moved denom into param store
 */
	_NS REGREG(MOVE,_ADR,_ADR); LDOUT; COND(IFNZ); DOJSUB(LONGDIVIDE); _ES
		/* put param pointer in outreg; if denom non-zero, do div */

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(LOADSDF); _ES
				/* move fractional result into DDASDF */

	_NS REGHOLD; DECMAR; _ES	/* point to int part */

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(LOADSDI); _ES
					/* integer result to DDASDI	*/

/* now make multiview determination */

	_NS LOADMAR(_MULTIVIEW); CONST(_MULTIVIEW); _ES

	_NS RAM(RAMRD,_TEMP,HOLD); _ES

	_NS ALUOP(ZERO); FTODO; COND(IFZ); SEQ(RETN); _ES
			/* if no multiview, return a zero to do 1 draw */

	_NS REGHOLD; DOJSUB(RESET_MASKS); _ES

label(D_VIEWLOOP)
	_NS REGREG(MOVE,_X1,_X1); BPC(LOADXS); _ES

	_NS REGREG(MOVE,_Y1,_Y1); BPC(LOADYS); _ES

	_NS REGREG(MOVE,_X2,_X2); BPC(LOADXE); _ES

	_NS REGREG(MOVE,_Y2,_Y2); BPC(LOADYE); _ES

	_NS REGREG(MOVE,_I1,_I1); BPC(LOADSAI); _ES

	_NS REGREG(MOVE,_NEWFRAC_COLOR,_NEWFRAC_COLOR); BPC(LOADSAF); _ES

	_NS LOADMAR(_SAVE1+3); CONST(_SAVE1+3); _ES	/* pt to quo */

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(LOADSDI); DOJSUB(NEXTMA); _ES

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(LOADSDF); _ES

	_NS ALUOP(ONES); FTODO; COND(IFFALSE); SEQ(TEST); _ES
		/* jump to top-of-stack to draw next vector */

label(D_MVLOOP)
	_NS REGHOLD; DOJSUB(NEW_MASK); _ES

	_NS REGHOLD; COND(IFNZ); DOJUMP(D_VIEWLOOP); _ES
				/* if there are more boxes, loop back */

	_NS REGHOLD; SEQ(TEST); _ES	/* pop the stack */

	_NS REGHOLD; DOJUMP(DENDVEC); _ES	/* done with all views */

}
