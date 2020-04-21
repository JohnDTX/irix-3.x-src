/*  vectors.c
 *
 *	dispatch to depth cued, Z buffered lines
 *	multi-viewport support
 *	variable-width vectors implemented
 *	feedback mode excised
 *
 *  The FBC's job for vectors entails:
 *
 *    1. Receiving the coordinate data from the Geometry Engine.
 *    2. Computing DELTAX and DELTAY.
 *    3. Using these values to determine appropriate initializations for
 *       ED and EC.
 *    4. Passing all necessary data to the BPC and issuing a vector command.
 *    5. Use BOLD flag to draw n-wide vectors.
 *
 *  The two coordinate pairs are kept in 4 of the 2903's general purpose
 *  registers, #X1, #Y1, #X2, and #Y2.  Computed values for DELTAX and DELTAY
 *  are stored in general purpose register #DELTAX and the 2903's q register
 *  respectively.  
 *
 *  Register assignments in consts.mic used.
 *  Other temporary register assignments are given below:
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "fbc.mic.h"
#include "consts.h"

#define _SAVECOORD 5
/* others from consts.h */

vectors()
{
newfile("vectors.c");

label(GET_X1Y1)		/* enter with X MSW in input register		*/
	  _NS LOADREG(_X1,ALL16,MORE);     /* Load _X1 with new coordinate */
	   DOTOOUTREG;                     /* and send it to the BPC,      */
	   BPCCMD(LOADXS);_ES              /* guessing it to be the        */
                                           /* leftmost of this vector's    */ 
                                           /* two endpoints.               */

	  _NS LOADREG(_Y1,ALL16,NOMORE);   /* Load _Y1 with new coordinate */
	   DOTOOUTREG;                     /* and send it to the BPC.      */
	   BPCCMD(LOADYS);
	   SEQ(RETN);_ES

label(GET_X2Y2)		/*  enter with X MSW in input reg	*/
	  _NS LOADREG(_X2,ALL16,MORE);     /* Load _X2 with new coordinate */
	   DOTOOUTREG;                     /* and send it to the BPC,      */
	   BPCCMD(LOADXE);_ES              /* guessing it to be the        */
                                           /* rightmost of this vector's   */
                                           /* two endpoints.               */

	  _NS LOADREG(_Y2,ALL16,NOMORE);   /* Load _Y2 with new coordinate */
	   DOTOOUTREG;                     /* and send it to the BPC.      */
	   BPCCMD(LOADYE);
	   SEQ(RETN);_ES

/*********************************************************************
 *
 *	bug note: uc bresenham doesn't like coordinates or ed,er
 *		to exceed 0x3ff. A check must be put here (or above)
 *		to limit such values.
 *
 *********************************************************************/
label(VECTOR_MOVE)			/* note -- no more feedback mode */
	_NS REGREG(ANDOP,P0,0,0);
	 GEOMENGDATA;
	 SEQ(JSUB);
	 NEXT(GET_X1Y1);_ES		/* go get coordinate pair	   */

	_NS LOADMAR(_ALTVECMODE); CONST(_ALTVECMODE); _ES

	_NS RAM(RAMRD,_TEMP,HOLD); _ES

	_NS REGHOLD; GEGET; COND(IFZ); DOJUMP(DISPATCH); _ES
					/* if in no special mode, exit */

	_NS IMMCOMP(EQ,_ALTMVPBIT,_TEMP); CONST(_ALTMVPBIT); _ES
		/* eliminate the case where MULTIVIEW is the only special */

	_NS REGHOLD; COND(IFEQ); DOJUMP(DISPATCH); _ES
		/* if just MULTIVIEW, exit - otherwise there's work to do */

	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(ANDRS); FTODO; CONST(_ALTDEPBIT); _ES
					/* bit test for DEPTH mode  */

	_NS REGHOLD; COND(IFNZ); DOJUMP(DEPTH_MOVE); _ES
					/* jump to depth cued alg. */

/* Z buffer option consists of just saving Z value */

	_NS REGHOLD; GEGET; _ES				/* ask for Z */

	_NS LOADREG(_Z1,ALL16,NOMORE); _ES	/* stick Z into reg */

#ifdef CLIPZVALUES
	_NS LOADMAR(_DEPTH_PARAMS+4); CONST(_DEPTH_PARAMS+4); _ES

	_NS REGRAMCOMP(GE,_Z1,INC); _ES		/* test lower Z limit */

	_NS REGRAMCOMP(GT,_Z1,DEC); COND(IFGE); DOJUMP(VTSTMAXZ); _ES
					/* pt to min in case have to limit */

label(LIMITZ1)
	_NS RAM(RAMRD,_Z1,HOLD); GEGET; DOJUMP(DISPATCH); _ES

label(VTSTMAXZ)
	_NS REGHOLD; INCMAR; COND(IFGT); DOJUMP(LIMITZ1); _ES
				/* point to max in case have to limit */
#endif CLIPZVALUES

	_NS REGHOLD; GEGET; DOJUMP(DISPATCH); _ES


label(VECTOR_DRAW)
	_NS LOADMAR(_ALTVECMODE); CONST(_ALTVECMODE); _ES
				/* test for any special mode set */

	_NS RAM(RAMRD,_TEMP,HOLD); _ES

	  _NS REGREG(RONLYOP,P0,_X1,_X1); /* Make sure the BPC's XS reg   */
	   DOTOOUTREG;			  /* is loaded with X1.		  */
	   BPCCMD(LOADXS);
	   COND(IFZ);                     /* If normal mode, do a draw.    */
	   SEQ(JUMP);
	   NEXT(VECTDODRAW);_ES

/* find out which of the special conditions set ALTVECMODE */

	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(ANDRS); FTODO; CONST(_ALTHITBIT); _ES
					/* bit test for HITMODE */

	_NS ALUOP(ZERO); YQ(FF,LDQ,REGWRD);
	 COND(IFNZ); DOJUMP(RECORDHIT); _ES
 
	_NS REGHOLD; GEGET; DOJSUB(GET_X2Y2); _ES
				/* we're drawing, so get the coords */

	_NS IMMCOMP(EQ,_ALTMVPBIT,_TEMP); CONST(_ALTMVPBIT); _ES
		/* eliminate the case where MULTIVIEW is the only special */

	_NS REGHOLD; COND(IFEQ); DOJUMP(MULTIVEC); _ES
		/* if just MULTIVIEW, jump - otherwise there's work to do */

	_NS LOADMAR(_LINESTIP); CONST(_LINESTIP); _ES

	_NS RAM(RAMRD,_STIP,HOLD); _ES	/* fetch stipple pattern for later */

	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(ANDRS); FTODO; CONST(_ALTDEPBIT); _ES
					/* bit test for DEPTHCUEMODE */

	_NS REGHOLD; COND(IFNZ); DOJUMP(DEPTH_DRAW); _ES
					/* branch on depthcue mode */

	_NS REGHOLD; DOJUMP(Z_VEC_DRAW); _ES

label(RECORDHIT)
	  _NS LOADREG(_HITREG,ALL16,NOMORE);_ES
			  /* Load hit bits from command word.            */

	  _NS IMMREG(ANDOP,P0,_HITBITMASK,_HITREG);
	   CONST(_HITBITMASK);             /* Mask out all but the hit     */
	   COND(IFFALSE);                  /* bits.                        */
	   SEQ(JUMP);_ES

	  _NS IMMREG(IOROP,P0,1,_HITREG);  /* Insure positive hit bit      */
	   CONST(1);			   /* value for microcode hit      */
	   COND(IFFALSE);		   /* detection.                   */
	   SEQ(JUMP);_ES

	  _NS LOADMAR(_HITBITS);           /* Or the new hit bits in with  */
	   CONST(_HITBITS);                /* the cumulative hit bits.     */
	   COND(IFFALSE);
	   SEQ(JUMP);_ES

	  _NS REGRAM(IOROP,P0,_HITREG,_HITREG,HOLD);_ES

	  _NS RAM(RAMWR,_HITREG,HOLD);     /* Return hitbits to RAM.       */
	   GEOMENGDATA;
	   SEQ(JUMP);
	   NEXT(DISPATCH);_ES              /* And return.                  */

label(VECTDODRAW)	/* simplest case - no shading, no Z, maybe bold */

	_NS REGHOLD; GEGET; DOJSUB(GET_X2Y2); _ES

	_NS LOADMAR(_LINESTYLE); CONST(_LINESTYLE); _ES

	  _NS RAM(RAMRD,_BOLD,INC);	   /* Get BOLD mode flag for later */
	   SEQ(JSUB);			   /* and point to LINESTIP	   */
	   NEXT(VECSUB); _ES		   /* perform vector draw subroutine*/

	  _NS REGREG(RONLYOP,P0,0,0);
	   GEOMENGDATA;
	   SEQ(JUMP);
	   NEXT(DISPATCH); _ES

label(MULTIVEC)	/* save coords; resetVP; loop(unsave; draw; newVP) */
	_NS LOADMAR(_SAVE1); CONST(_SAVE1); _ES

	_NS RAM(RAMWR,_X1,INC); _ES			/* save X1 */

	_NS RAM(RAMWR,_Y1,HOLD);			/* save Y1 */
	 SEQ(JSUB); NEXT(RESET_MASKS); _ES	/* this destoys UC inpregs! */

label(VLOOP_VP)
	_NS LOADMAR(_SAVE1); CONST(_SAVE1); _ES	/* restore coords */

	_NS RAM(RAMRD,_X1,INC); _ES

	_NS RAM(RAMRD,_Y1,HOLD); BPC(LOADYS); _ES	/* restore Y1 */

	_NS REGREG(RONLYOP,P0,_X2,_X2); BPC(LOADXE); _ES /* restore X2 */

	_NS REGREG(RONLYOP,P0,_Y2,_Y2); BPC(LOADYE); _ES /* restore Y2 */

	_NS LOADMAR(_LINESTYLE); CONST(_LINESTYLE); _ES  /* reread BOLD */

	_NS RAM(RAMRD,_BOLD,INC); _ES			/* point to stipple */

	_NS REGREG(RONLYOP,P0,_X1,_X1); BPC(LOADXS);	/* restore X1  */
	 SEQ(JSUB); NEXT(VECSUB); _ES			/* draw the vector */

	_NS REGREG(RONLYOP,P0,0,0);
	 SEQ(JSUB); NEXT(NEW_MASK); _ES		/* fetch next vwport*/
					/* this destroys UC input regs!	*/

	_NS REGREG(RONLYOP,P0,0,0);		/* if returned non-zero, */
	 COND(IFNZ); SEQ(JUMP); NEXT(VLOOP_VP); _ES	/* repeat the vector*/

	_NS REGREG(RONLYOP,P0,0,0);
	 GEOMENGDATA;
	 SEQ(JUMP); NEXT(DISPATCH); _ES

			   /* body of vector-drawing code */
label(VECSUB)
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
	   NEXT(REVERSED);_ES              /* reversed.  Jump to r octants.*/

label(NONREVERSED)
                                           /* At this point, _DELTAX holds */
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
	   NEXT(OCT2OR3);_ES               /* based on the sign of DELTAY. */

label(OCT0OR1)
	  _NS SETROP(_Y1,NONE);            /* Guess octant 1 and put       */
	   SETSOP(NONQOP,_Y2,RAMNOP);      /* -DELTAY in BPC's ED reg.     */
	   ALUOP(SUBRSOP,P1);
	   FTOYANDQ(FF,OLDQ,REGWRD);
	   YTODO(ALL16);
	   DOTOOUTREG;
	   BPCCMD(LOADED);
	   COND(IFNNEG);                   /* If DELTAX - DELTAY was >=    */
	   SEQ(JUMP);                      /* 0, then octant 1 was correct */
	   NEXT(OCT1);_ES                  /* and we're ready to give the  */
                                           /* BPC the appropriate command. */

/*label(MUSTBEOCT0)*/
	  _NS IMMREG(SUBRSOP,P1,0,_DELTAX);/* Just our luck.  All guesses  */
	   DOTOOUTREG;                     /* were wrong.  Set the BPC's   */
	   BPCCMD(LOADED);                 /* ED register to -DELTAX,      */
	   COND(IFFALSE);
	   SEQ(JUMP);
	   CONST(0);_ES

	  _NS SETSOP(QOPERAND,0,RAMNOP);    /* the EC register to DELTAY,   */
	   ALUOP(SONLYOP,P0);
	   FTOYANDQ(FF,OLDQ,REGWRD);
	   YTODO(ALL16);
	   DOTOOUTREG;
	   BPCCMD(LOADEC);
	   SEQ(JUMP);                      /* and give the command.        */
	   NEXT(OCT0);_ES

label(OCT2OR3)
	  _NS SETROP(_DELTAX,NONE);        /* Calculate DELTAX + DELTAY    */
	   SETSOP(QOPERAND,0,RAMNOP);      /* to decide between octants 2  */
	   ALUOP(ADDOP,P0);                /* and 3.  DELTAY is negative,  */
	   FTOYANDQ(FF,OLDQ,REGWRD);       /* so this computation may or   */
	   YTODO(ALL16);_ES                /* may not return a negative.   */

	  _NS REGREG(ANDOP,P0,0,0);        /* Too bad, nothing sensible to */
	   COND(IFNNEG);                   /* do here other than test the  */
	   SEQ(JUMP);                      /* condition code set by last   */
	   NEXT(OCT2);_ES                  /* operation.  If this is octant*/
                                           /* 2, then all is ready for the */
                                           /* BPC.                         */
/*label(MUSTBEOCT3)*/
	  _NS IMMREG(SUBRSOP,P1,0,_DELTAX); /* Wrong guess.  Must reload ED */
	   DOTOOUTREG;                     /* with -DELTAX,                */
	   BPCCMD(LOADED);
	   COND(IFFALSE);
	   SEQ(JUMP);
	   CONST(0);_ES

	  _NS SETROP(_Y1,NONE);             /* and EC with -DELTAY,         */
	   SETSOP(NONQOP,_Y2,RAMNOP);
	   ALUOP(SUBRSOP,P1);
	   FTOYANDQ(FF,OLDQ,REGWRD);
	   YTODO(ALL16);
	   DOTOOUTREG;
	   BPCCMD(LOADEC);
	   SEQ(JUMP);                      /* and give the command.        */
	   NEXT(OCT3);_ES

label(REVERSED)
                                           /* At this point, _DELTAX holds */
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
	   NEXT(OCT0ROR1R);_ES             /* based on the sign of DELTAY. */

/*OCT2ROR3R:*/
	  _NS SETROP(_Y1,NONE);            /* Guess octant 3r and put      */
	   SETSOP(NONQOP,_Y2,RAMNOP);      /* -DELTAY in BPC's EC reg.     */
	   ALUOP(SUBRSOP,P1);
	   FTOYANDQ(FF,OLDQ,REGWRD);
	   YTODO(ALL16);
	   DOTOOUTREG;
	   BPCCMD(LOADEC);
	   COND(IFNNEG);                   /* If DELTAX - DELTAY was >=    */
	   SEQ(JUMP);                      /* 0, then octant 3r was correct*/
	   NEXT(OCT3R);_ES                 /* and we're ready to give the  */
                                           /* BPC the appropriate command. */

/*MUSTBEOCT2R:*/
	  _NS IMMREG(SUBRSOP,P1,0,_DELTAX); /* Just our luck.  All guesses  */
	   DOTOOUTREG;                     /* were wrong.  Set the BPC's   */
	   BPCCMD(LOADEC);                 /* EC register to -DELTAX,      */
	   COND(IFFALSE);
	   SEQ(JUMP);
	   CONST(0);_ES

	  _NS SETSOP(QOPERAND,0,RAMNOP);   /* the ED register to DELTAY,   */
	   ALUOP(SONLYOP,P0);
	   FTOYANDQ(FF,OLDQ,REGWRD);
	   YTODO(ALL16);
	   DOTOOUTREG;
	   BPCCMD(LOADED);
	   SEQ(JUMP);                      /* and give the command.        */
	   NEXT(OCT2R);_ES

label(OCT0ROR1R)
	  _NS SETROP(_DELTAX,NONE);        /* Calculate DELTAX + DELTAY    */
	   SETSOP(QOPERAND,0,RAMNOP);      /* to decide between octants 0r */
	   ALUOP(ADDOP,P0);                /* and 1r.  DELTAX is negative, */
	   FTOYANDQ(FF,OLDQ,REGWRD);       /* so this computation may or   */
	   YTODO(ALL16);_ES                /* may not return a negative.   */

	  _NS REGREG(ANDOP,P0,0,0);        /* Too bad, nothing sensible to */
	   COND(IFNNEG);                   /* do here other than test the  */
	   SEQ(JUMP);                      /* condition code set by last   */
	   NEXT(OCT0R);_ES                 /* operation.  If this is octant*/
                                           /* 0r, then all is ready for the*/
                                           /* BPC.                         */
/*MUSTBEOCT1R:*/
	  _NS IMMREG(SUBRSOP,P1,0,_DELTAX);/* Wrong guess.  Must reload EC */
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
	   NEXT(OCT1R);_ES

/* Tue Jun 12 20:39:42 PDT 1984 - hpm 
this is "all new code" which handles line of width greater than two */

label(ENDVDRAW)
	  _NS REGREG(RONLYOP,P0,_X2,_X1);  /* X1 := X2.                    */
	   DOTOOUTREG;                     /* Set BPC's XS register to new */
	   BPCCMD(LOADXS);		   /* value for X1.                */
	   SEQ(LOUP);_ES		   /* Perform pop if we branched   */
					   /* out of POKE subr prematurely */

label(VXDRAWDONE)
	  _NS REGREG(RONLYOP,P0,_Y2,_Y1);  /* Y1 := Y2.                    */
	   DOTOOUTREG;                     /* Set BPC's YS register to new */
	   BPCCMD(LOADYS);                 /* value for Y1                 */
	   SEQ(RETN); _ES		   /* and return.                  */

label(VYDRAWDONE)
	  _NS REGREG(RONLYOP,P0,_X2,_X1);  /* X1 := X2.                    */
	   DOTOOUTREG;                     /* Set BPC's XS register to new */
	   BPCCMD(LOADXS);                 /* value for X1                 */
	   SEQ(RETN); _ES		   /* and return.                  */

/*
/*  routines for bold vectors; if BOLD isn't set, we won't return from POKE...
*/

label(OCT0)
	  _NS REGRAM(SONLYOP,P0,0,_TEMP,HOLD);
					   /* Send the line stipple pattern */
	   DOTOOUTREG;
	   BPCCMD(OCT0VECT);
	   SEQ(JSUB);			   /* Draw the first vector, then   */
	   NEXT(BOLDXSETUP);_ES		   /* decrement X1 and X2 by BOLD/2
	   				   and setup the loop counter
					   the top of the loop is setup
					   be this jump subroutine */

	  _NS REGRAM(SONLYOP,P0,0,_TEMP,HOLD);
	   DOTOOUTREG;
	   BPCCMD(OCT0VECT);
	   SEQ(JUMP);			  /* Draw the adjacent vector */
	   NEXT(POKEX1X2);_ES		  /* POKEX1X2 does a repeat loop 
					  to get back here (LOUP)*/

label(OCT1)
	  _NS REGRAM(SONLYOP,P0,0,_TEMP,HOLD);
					   /* Send the line stipple pattern */
	   DOTOOUTREG;
	   BPCCMD(OCT1VECT);
	   SEQ(JSUB);			   /* Draw the first vector, then   */
	   NEXT(BOLDYSETUP);_ES		   /* decrement Y1 and Y2 by BOLD/2
	   				   and setup the loop counter
					   the top of the loop is setup
					   be this jump subroutine */

	  _NS REGRAM(SONLYOP,P0,0,_TEMP,HOLD);
	   DOTOOUTREG;
	   BPCCMD(OCT1VECT);
	   SEQ(JUMP);			  /* Draw the adjacent vector */
	   NEXT(POKEY1Y2);_ES		  /* POKEY1Y2 does a repeat loop 
					  to get back here (LOUP)*/

label(OCT3)
	  _NS REGRAM(SONLYOP,P0,0,_TEMP,HOLD);
					   /* Send the line stipple pattern */
	   DOTOOUTREG;
	   BPCCMD(OCT3VECT);
	   SEQ(JSUB);			   /* Draw the first vector, then   */
	   NEXT(BOLDXSETUP);_ES		   /* decrement X1 and X2 by BOLD/2
	   				   and setup the loop counter
					   the top of the loop is setup
					   be this jump subroutine */

	  _NS REGRAM(SONLYOP,P0,0,_TEMP,HOLD);
	   DOTOOUTREG;
	   BPCCMD(OCT3VECT);
	   SEQ(JUMP);			  /* Draw the adjacent vector */
	   NEXT(POKEX1X2);_ES		  /* POKEX1X2 does a repeat loop 
					  to get back here (LOUP)*/

label(OCT2)
	  _NS REGRAM(SONLYOP,P0,0,_TEMP,HOLD);
					   /* Send the line stipple pattern */
	   DOTOOUTREG;
	   BPCCMD(OCT2VECT);
	   SEQ(JSUB);			   /* Draw the first vector, then   */
	   NEXT(BOLDYSETUP);_ES		   /* decrement Y1 and Y2 by BOLD/2
	   				   and setup the loop counter
					   the top of the loop is setup
					   be this jump subroutine */

	  _NS REGRAM(SONLYOP,P0,0,_TEMP,HOLD);
	   DOTOOUTREG;
	   BPCCMD(OCT2VECT);
	   SEQ(JUMP);			  /* Draw the adjacent vector */
	   NEXT(POKEY1Y2);_ES		  /* POKEY1Y2 does a repeat loop 
					  to get back here (LOUP)*/

label(OCT3R)
	  _NS REGRAM(SONLYOP,P0,0,_TEMP,HOLD);
					   /* Send the line stipple pattern */
	   DOTOOUTREG;
	   BPCCMD(OCT3RVECT);
	   SEQ(JSUB);			   /* Draw the first vector, then   */
	   NEXT(BOLDXSETUP);_ES		   /* decrement X1 and X2 by BOLD/2
	   				   and setup the loop counter
					   the top of the loop is setup
					   be this jump subroutine */

	  _NS REGRAM(SONLYOP,P0,0,_TEMP,HOLD);
	   DOTOOUTREG;
	   BPCCMD(OCT3RVECT);
	   SEQ(JUMP);			  /* Draw the adjacent vector */
	   NEXT(POKEX1X2);_ES		  /* POKEX1X2 does a repeat loop 
					  to get back here (LOUP)*/

label(OCT2R)
	  _NS REGRAM(SONLYOP,P0,0,_TEMP,HOLD);
					   /* Send the line stipple pattern */
	   DOTOOUTREG;
	   BPCCMD(OCT2RVECT);
	   SEQ(JSUB);			   /* Draw the first vector, then   */
	   NEXT(BOLDYSETUP);_ES		   /* decrement Y1 and Y2 by BOLD/2
	   				   and setup the loop counter
					   the top of the loop is setup
					   be this jump subroutine */

	  _NS REGRAM(SONLYOP,P0,0,_TEMP,HOLD);
	   DOTOOUTREG;
	   BPCCMD(OCT2RVECT);
	   SEQ(JUMP);			  /* Draw the adjacent vector */
	   NEXT(POKEY1Y2);_ES		  /* POKEY1Y2 does a repeat loop 
					  to get back here (LOUP)*/

label(OCT0R)
	  _NS REGRAM(SONLYOP,P0,0,_TEMP,HOLD);
					   /* Send the line stipple pattern */
	   DOTOOUTREG;
	   BPCCMD(OCT0RVECT);
	   SEQ(JSUB);			   /* Draw the first vector, then   */
	   NEXT(BOLDXSETUP);_ES		   /* decrement X1 and X2 by BOLD/2
	   				   and setup the loop counter
					   the top of the loop is setup
					   be this jump subroutine */

	  _NS REGRAM(SONLYOP,P0,0,_TEMP,HOLD);
	   DOTOOUTREG;
	   BPCCMD(OCT0RVECT);
	   SEQ(JUMP);			  /* Draw the adjacent vector */
	   NEXT(POKEX1X2);_ES		  /* POKEX1X2 does a repeat loop 
					  to get back here (LOUP)*/

label(OCT1R)
	  _NS REGRAM(SONLYOP,P0,0,_TEMP,HOLD);
					   /* Send the line stipple pattern */
	   DOTOOUTREG;
	   BPCCMD(OCT1RVECT);
	   SEQ(JSUB);			   /* Draw the first vector, then   */
	   NEXT(BOLDYSETUP);_ES		   /* decrement Y1 and Y2 by BOLD/2
	   				   and setup the loop counter
					   the top of the loop is setup
					   be this jump subroutine */

	  _NS REGRAM(SONLYOP,P0,0,_TEMP,HOLD);
	   DOTOOUTREG;
	   BPCCMD(OCT1RVECT);
	   SEQ(JUMP);			  /* Draw the adjacent vector */
	   NEXT(POKEY1Y2);_ES		  /* POKEY1Y2 does a repeat loop 
					  to get back here (LOUP)*/

label(BOLDXSETUP)
	_NS
	 /* test BOLD & load the counter */
	 REGREG(RONLYOP,P0,_BOLD,_BOLD);
	 /* specify the counter be loaded from the outreg */
	 YTODO(ALL16);
	 DOTOOUTREG;
	_ES
    
	_NS
	 REGREG(SONLYOP,P0,_BOLD,_BOLD);
	 /* load the counter */
	 LOADDI(OUTPUTCOUNT);
	 SEQ(LDCT);
	_ES
    
	_NS
	 /* if zero goto ENDVDRAW */
	 COND(IFZ);
	 SEQ(JUMP);
	 NEXT(ENDVDRAW);
	 /* zero _ZERO and the Q register */
	 ALUOP(FLOWOP, P0);
	 SETSOP(NONQOP, _ZERO, RAMNOP);
	 FTOYANDQ(FF, LDQ, REGWRE);
	_ES

	_NS
	 /* halve _BOLD + 1 this increment is done to ensure that the 
	 first extra line is not coincident with the one drawn on entry,
	 this means if a line of even width (2,4,6,etc) the extra line
	 is on the negative side of the actual line */
	 ALUOP(SONLYOP, P1);
	 SETSOP(NONQOP, _BOLD, RAMNOP);
	 FTOYANDQ(FAR, QR, REGWRE);
	_ES

	_NS
	 /* subtract BOLD/2 from x1 loading XS */
	 ALUOP(SUBSROP, P1);
	 SETROP(_BOLD, NONE);
	 SETSOP(NONQOP, _X1, RAMNOP);
	 FTOYANDQ(FF, OLDQ, REGWRE);
	 DOTOOUTREG;
	 BPCCMD(LOADXS);
	 SEQ(LOUP);
	_ES


label(POKEX1X2)

	_NS
	 /* decrement _BOLD to determine when we've arrived at the line
	 which has already been drawn */
	 ALUOP(SUBSROP, P0);
	 SETSOP(NONQOP, _BOLD, RAMNOP);
	 SETROP(_ZERO, NONE);
	 FTOYANDQ(FF, OLDQ, REGWRE);
	_ES

	_NS 
	 ALUOP(SONLYOP,P1);
	 SETSOP(NONQOP, _X1, RAMNOP); 		/* increment X1 */
	 FTOYANDQ(FF,OLDQ,REGWRE);
	 DOTOOUTREG;
	 /* Incremented X1 to XS register */
	 BPCCMD(LOADXS);		
	 COND(IFZ);			/* if _BOLD is now zero don't
	 				draw the line and get back to
	 				POKEX1X2 */
	 SEQ(JUMP);
	 NEXT(POKEX1X2);
	_ES

	_NS
	 /* do nothing ?! */
	 ALUOP(RONLYOP, P0);
	 SETROP(0, NONE);
	 SETSOP(NONQOP, 0, RAMNOP);
	 FTOYANDQ(FF,OLDQ,REGWRD);

	 SEQ(LOUP);			/* this should take us back to
	 				the caller of this routine unless
					we're done, in which case it
					falls through setting up _X2 */
	_ES

label(CLEANUPX)
	_NS
	 ALUOP(RONLYOP,P0);
	 SETROP(_X2, NONE);
	 SETSOP(NONQOP, _X1, RAMNOP);
	 FTOYANDQ(FF, OLDQ, REGWRE);
	 DOTOOUTREG;
	 BPCCMD(LOADXS);
	 SEQ(JUMP); 			/* this sets up _X1 and XS */
	 NEXT(VXDRAWDONE);
	_ES

label(BOLDYSETUP)

	_NS
	 /* test BOLD & load the counter */
	 REGREG(RONLYOP,P0,_BOLD,_BOLD);
	 /* specify the counter be loaded from the outreg */
	 YTODO(ALL16);
	 DOTOOUTREG;
	_ES

	_NS
	 REGREG(SONLYOP,P0,_BOLD,_BOLD);
	 /* load the counter */
	 LOADDI(OUTPUTCOUNT);
	 SEQ(LDCT);
	_ES
    
	_NS
	 /* if zero goto ENDVDRAW */
	 COND(IFZ);
	 SEQ(JUMP);
	 NEXT(ENDVDRAW);
	 /* zero _ZERO and the Q register */
	 ALUOP(FLOWOP, P0);
	 SETSOP(NONQOP, _ZERO, RAMNOP);
	 FTOYANDQ(FF, LDQ, REGWRE);
	_ES

	_NS
	 /* halve _BOLD + 1 this increment is done to ensure that the 
	 first extra line is not coincident with the one drawn on entry,
	 this means if a line of even width (2,4,6,etc) the extra line
	 is on the negative side of the actual line */
	 ALUOP(SONLYOP, P1);
	 SETSOP(NONQOP, _BOLD, RAMNOP);
	 FTOYANDQ(FAR, QR, REGWRE);
	_ES

	_NS
	 /* subtract BOLD/2 from Y1 loading YS */
	 ALUOP(SUBSROP, P1);
	 SETROP(_BOLD, NONE);
	 SETSOP(NONQOP, _Y1, RAMNOP);
	 FTOYANDQ(FF, OLDQ, REGWRE);
	 DOTOOUTREG;
	 BPCCMD(LOADYS);
	 SEQ(LOUP);
	_ES

label(POKEY1Y2)

	_NS
	 /* decrement _BOLD to determine when we've arrived at the line
	 which has already been drawn */
	 ALUOP(SUBSROP, P0);
	 SETSOP(NONQOP, _BOLD, RAMNOP);
	 SETROP(_ZERO, NONE);
	 FTOYANDQ(FF, OLDQ, REGWRE);
	_ES

	_NS 
	 ALUOP(SONLYOP,P1);
	 SETSOP(NONQOP, _Y1, RAMNOP); 		/* increment Y1 */
	 FTOYANDQ(FF,OLDQ,REGWRE);
	 DOTOOUTREG;
	 BPCCMD(LOADYS);		/* Incremented Y1 to YS register */
	 COND(IFZ);			/* if _BOLD is now zero don't
	 				draw the line and get back to
	 				POKEY1Y2 */
	 SEQ(JUMP);
	 NEXT(POKEY1Y2);
	_ES

	_NS
	 /* do nothing ?! */
	 ALUOP(RONLYOP, P0);
	 SETROP(0, NONE);
	 SETSOP(NONQOP, 0, RAMNOP);
	 FTOYANDQ(FF,OLDQ,REGWRD);
	 SEQ(LOUP);			/* this should take us back to
	 				the caller of this routine unless
					we're done, in which case it
					falls through setting up _Y2 */
	_ES

label(CLEANUPY)
	_NS
	 ALUOP(RONLYOP,P0);
	 SETSOP(NONQOP, _Y1, RAMNOP);
	 SETROP(_Y2, NONE);
	 FTOYANDQ(FF, OLDQ, REGWRE);
	 DOTOOUTREG;
	 BPCCMD(LOADYS);
	 SEQ(JUMP); 			/* this sets up _Y2 and YS */
	 NEXT(VYDRAWDONE);
	_ES

}
