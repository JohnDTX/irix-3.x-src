/*
 * polydraw.c
 *
 *	SETINTENSITY
 *	POLY_MOVE
 *	POLY_DRAW
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"
#include "polydefs.h"

/*
/*  RAM address assignments used are listed in consts.mic
*/

/*
	registers used
		_LASTPT
		_TEMP
		_XTHIS
		_YTHIS
		_ZTHIS
		_BOTTOMY
		_BOTTOMPNT
		_CURSHADE
		_PREVSHADE
		_TOPY
		_TOPPNT
		_LEFTMOSTX
		_LEFTPNT
		_RIGHTMOSTX
		_RIGHTPNT

	constants used
		_BIG
	scratch locations used
		_STARTLIST
		_ZBUFFER
		_HITMODE
	routines called
		none external to this file
	exits to
		RECORDHIT
	    or
		DISPATCH

*/
polydraw2d()
{
newfile("polydraw.c");

label(SETINTENSITY)	/* save intensity value for next poly vertex */
	_NS LOADREG(_CURSHADE,ALL16,NOMORE); _ES

	_NS REGHOLD; GEGET; DOJUMP(DISPATCH); _ES

label(SETBACKFACING)
	_NS LOADREG(_TEMP,ALL16,NOMORE); _ES

	_NS 
	    LOADMAR(_BACKFACING);
	    CONST(_BACKFACING);
	_ES

	_NS 
	    RAM(RAMWR, _TEMP, HOLD);
	    GEGET; 
	    DOJUMP(DISPATCH); 
	_ES


label(POLY_MOVE)
    /* load the scratch start address of the list of points and the
    max/mins used in recognizing flat polys and making a good guess at
    clockwise/counterclockwisness */
    _NS	
	LOADIMM(_LASTPT, _STARTLIST);
	CONST(_STARTLIST);
    _ES

    _NS	/* load the min with max */
	LOADIMM(_BOTTOMY, _BIG);
	CONST(_BIG);
    _ES

    _NS /* load the max with -BIG */
	LOADIMM(_TOPY, -_BIG);
	CONST(-_BIG);
    _ES

    _NS /* load the max with -BIG */
	LOADIMM(_RIGHTMOSTX, -_BIG);
	CONST(-_BIG);
    _ES

    _NS	/* load the min with BIG */
	LOADIMM(_LEFTMOSTX, _BIG);
	CONST(_BIG);
    _ES

label(P_MOVE_OR_DRAW)
    _NS	/* go do body of draw routine - get X,Y */
	REGHOLD;
	DOJSUB(POLYDRAW);
    _ES

    _NS	/* test the mode flag for 2 vs 3 coords, also shift q again */
	LOADMAR(_ALTPOLYMODE);
	CONST(_ALTPOLYMODE);
    _ES

    _NS /* read the flag */
	RAM(RAMRD, _TEMP, HOLD);
    _ES

    _NS /* if the flag is set, go handle the 3d case */
	REGHOLD;
	COND(IFNZ);
	SEQ(JSUB);
	NEXT(POLY_GET_Z);
    _ES

    /* in any case, do CHECK_VERTICES */

label(CHECK_VERTICES)
    _NS /* save the current shade to load the next time around */
	REGREG(RONLYOP, P0, _CURSHADE, _PREVSHADE);
    _ES

    _NS /* load the MAR with the address of the current viewport */
	LOADMAR(_CURRVIEWPORT);
	CONST(_CURRVIEWPORT);
    _ES

    _NS
	REGCOMP( GT, _LEFTMOSTX, _XTHIS);
    _ES

    _NS 
	REGCOMP( GT, _BOTTOMY, _YTHIS);
	COND(IFGT);
	SEQ(JSUB);
	NEXT(NEWLEFT);
    _ES

    _NS
	REGCOMP( LT, _RIGHTMOSTX, _XTHIS);
	DOTOMAR(INC);
	COND(IFGT);
	SEQ(JSUB);
	NEXT(NEWBOTTOM);
    _ES

    _NS
	REGCOMP( LT, _TOPY, _YTHIS);
	DOTOMAR(INC);
	COND(IFLT);
	SEQ(JSUB);
	NEXT(NEWRIGHT);
    _ES

    _NS 
	REGHOLD;
	COND(IFGE);
	SEQ(JUMP);
	NEXT(DISPATCH);
	DOTOMAR(INC);
	GEOMENGDATA;
    _ES

label(NEWTOP)
    _NS /* check to see if the points y is greater than the y of viewport */
	REGRAMCOMP(LE, _YTHIS, HOLD);
    _ES

    _NS	/* save the index of the topmost point so far */
	REGREG( RONLYOP, P0, _LASTPT, _TOPPNT);
	SEQ(JUMP);
	COND(IFLE);
	NEXT(NOTOPFIXUP);
    _ES

    _NS /* load YTHIS with the "clipped" y value */
	RAM(RAMRD, _YTHIS, HOLD);
    _ES

label(NOTOPFIXUP)
    _NS /* save its value and return to the dispatch table */
	REGREG( RONLYOP, P0, _YTHIS, _TOPY);
	SEQ(JUMP);
	NEXT(DISPATCH);
    _ES

label(NEWRIGHT)
    _NS /* check to see if the points x is greater than the x of viewport */
	REGRAMCOMP(LE, _XTHIS, HOLD);
    _ES

    _NS	/* save the index of the leftmost point so far */
	REGREG( RONLYOP, P0, _LASTPT, _RIGHTPNT);
	SEQ(JUMP);
	COND(IFLE);
	NEXT(NORIGHTFIXUP);
    _ES

    _NS /* load XTHIS with the "clipped" x value */
	RAM(RAMRD, _XTHIS, HOLD);
    _ES

label(NORIGHTFIXUP)
    _NS /* save its value and return to the dispatch table */
	REGREG( RONLYOP, P0, _XTHIS, _RIGHTMOSTX);
    _ES

    _NS /* retest for topy */
	REGCOMP( LT, _TOPY, _YTHIS);
	SEQ(RETN);
    _ES

label(NEWBOTTOM)
    _NS /* check to see if the points y is less than the y of viewport */
	REGRAMCOMP(GE, _YTHIS, HOLD);
    _ES

    _NS	/* save the index of the bottommost point so far */
	REGREG( RONLYOP, P0, _LASTPT, _BOTTOMPNT);
	SEQ(JUMP);
	COND(IFGE);
	NEXT(NOBOTTOMFIXUP);
    _ES

    _NS /* load YTHIS with the "clipped" y value */
	RAM(RAMRD, _YTHIS, HOLD);
    _ES

label(NOBOTTOMFIXUP)
    _NS /* save its value and return to the dispatch table */
	REGREG( RONLYOP, P0, _YTHIS, _BOTTOMY);
    _ES

    _NS /* retest for rightx */
	REGCOMP( LT, _RIGHTMOSTX, _XTHIS);
	SEQ(RETN);
    _ES

label(NEWLEFT)
    _NS /* check to see if the points x is less than the x of viewport */
	REGRAMCOMP(GE, _XTHIS, HOLD);
    _ES

    _NS	/* save the index of the topmost point so far */
	REGREG( RONLYOP, P0, _LASTPT, _LEFTPNT);
	SEQ(JUMP);
	COND(IFGE);
	NEXT(NOLEFTFIXUP);
    _ES

    _NS /* load XTHIS with the "clipped" x value */
	RAM(RAMRD, _XTHIS, HOLD);
    _ES

label(NOLEFTFIXUP)
    _NS /* save its value and return to the dispatch table */
	REGREG( RONLYOP, P0, _XTHIS, _LEFTMOSTX);
    _ES

    _NS /* retest for bottomy */
	REGCOMP( GE, _BOTTOMY, _YTHIS);	/* allow bottom to be equal */
	SEQ(RETN);			 /*  after a new left */
    _ES


/*****************************************************************************
/*****************************************************************************
/*****************************************************************************
*/

label(POLY_DRAW)

    _NS /* load the MAR with the address of the hitmode flag */
	LOADMAR(_HITMODE);
	CONST(_HITMODE);
    _ES

    _NS /* test the flag */
	RAM(RAMRD, _TEMP, HOLD);
    _ES

    _NS /* if clear, go handle coordinates */
	REGHOLD;
	COND(IFZ);
	SEQ(JUMP);
	NEXT(P_MOVE_OR_DRAW);
    _ES

    _NS	/* otherwise go handle the hit */
	REGHOLD;
	SEQ(JUMP);
	NEXT(RECORDHIT);
    _ES

/******************************************************************/

label(POLYDRAW)

/* subroutine: inputs X,Y; writes X, Y, shade
 */
/* not in hit mode */
    _NS	/* load the MAR with the scratch address of the points */
	REGREG(RONLYOP, P0, _LASTPT, _LASTPT);
	DOTOMAR(LOAD);
    _ES

    _NS /* increment the point index to where the points currently
    	comming in will be stored */
	IMMREG(ADDOP, P0, 4, _LASTPT); GEOMENGDATA; CONST(4);
    _ES

    _NS	/* load XTHIS with new data and write the old data out */
	LOADDI(INRJUST);
	SETROP(_XTHIS,ALL16);
	SETSOP(NONQOP,_XTHIS,RAMWR);
	ALUOP(RONLYOP,P0);
	FTOYANDQ(FF,OLDQ,REGWRE);
	YTODO(ALL16);
	DOTOMAR(INC);
    _ES

    _NS
	REGHOLD; GEOMENGDATA;
    _ES		/* separated out for side-door testing; no time wasted! */

    _NS	/* load YTHIS with new data and write the old data out */
	LOADDI(INRJUST);
	SETROP(_YTHIS,ALL16);
	SETSOP(NONQOP,_YTHIS,RAMWR);
	ALUOP(RONLYOP,P0);
	FTOYANDQ(FF,OLDQ,REGWRE);
	YTODO(ALL16);
	DOTOMAR(INC);
    _ES

    _NS
	RAM(RAMWR, _PREVSHADE, INC);
	SEQ(RETN);
    _ES

/*****************************************************************************
/*****************************************************************************
/*****************************************************************************
*/

label(POLY_GET_Z)

/* subroutine to fetch the Z coordinate */

    _NS /* get stereo x out of the pipe */
	/* point to (lastpt-1) to write shade out */
	LOADDI(UCONST);
	SETROP(0,ALL16);
	SETSOP(NONQOP,_LASTPT,RAMNOP);
	ALUOP(SUBSRC); FTODO;
	CONST(1);
	GEOMENGDATA;
	DOTOMAR(LOAD);
    _ES
	
    _NS /* get z out of the pipe, and write the shade
    	value; point to where old z will be written */
	ALUOP(RONLYOP, P0);
	SETROP(_PREVSHADE, NONE);
	SETSOP(NONQOP, _PREVSHADE, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(DEC);
	GEOMENGDATA;
    _ES

    _NS	/* load ZTHIS with new data and write the old data out */
	LOADDI(INRJUST);
	SETROP(_ZTHIS,ALL16);
	SETSOP(NONQOP,_ZTHIS,RAMWR);
	ALUOP(RONLYOP,P0);
	FTOYANDQ(FF,OLDQ,REGWRE);
	YTODO(ALL16);
    _ES

    _NS LOADMAR(_ALTPOLYMODE); CONST(_ALTPOLYMODE); _ES

    _NS IMMRAM(ANDRS,_ALTDEPBIT,_TEMP,HOLD); CONST(_ALTDEPBIT); _ES
				/* look at depthcue mode */
    
    _NS REGHOLD; COND(IFZ); SEQ(RETN); _ES
				/* return if not depthcue mode */

    _NS IMMRAM(ANDRS,_ALTZBUBIT,_TEMP,HOLD); CONST(_ALTZBUBIT); _ES
				/* look at zbuffer mode */
    
    _NS REGHOLD; COND(IFNZ); DOJUMP(POLY_GET_ZDEP); _ES
				/* go if depthcue-zbuffer mode */

label(POLY_GET_DEP)

    _NS /* point to (lastpt-2) to write shade out */
	LOADDI(UCONST);
	SETROP(0,ALL16);
	SETSOP(NONQOP,_LASTPT,RAMNOP);
	ALUOP(SUBSRC); FTODO;
	CONST(2);
	DOTOMAR(LOAD);
    _ES
	
    _NS RAM(RAMWR,_PREVSHADE,HOLD); _ES
		    /* rewrite shade value */

label(POLY_GET_ZDEP)
	_NS LOADMAR(_DEPTH_PARAMS); CONST(_DEPTH_PARAMS); _ES
				/* begin setting up to do I1 calculation */

	_NS SETSOP(NONQOP,0,RAMRD); ALUOP(SONLYOP,P0);
	 YQ(FF,LDQ,REGWRD); INCMAR; _ES
				/* "a" to Q reg; go get Z value */
				/* and calc CURSHADE = a * Z + b */

	_NS REGREG(ZERO,0,_CURSHADE); LOADDI(UCOUNT); SEQ(LDCT); CONST(14); _ES

label(POLY_GET_Z1)
	_NS ALUOP(ZERO); FTODO; _ES	/* make sure DCOUT is clear next */

	_NS MUL(_ZTHIS,_CURSHADE); SEQ(RPCT); NEXT(POLY_GET_Z1); _ES

	_NS MULLAST(_ZTHIS,_CURSHADE); LOADDI(UCONST); DOPUSH(1); _ES

/* binary point is two bits down from top of lsw, so doubleshift left 2 */
	_NS SETROP(_CURSHADE,NONE); SETSOP(NONQOP,_CURSHADE,RAMNOP);
	     ALUOP(MOVE); YQ(FLL,QL,REGWRE); SEQ(LOUP); _ES

	_NS REGRAM(ADD,_CURSHADE,_CURSHADE,HOLD); BPC(SETCOLORAB); _ES
				/* set as color of characters */

	_NS LOADMAR(_COLOR); CONST(_COLOR); _ES

	_NS RAM(RAMWR,_CURSHADE,INC);_ES	/* save color	*/

	_NS RAM(RAMWR,_CURSHADE,HOLD); SEQ(RETN); _ES	 /* in both places */

/*****************************************************************************
/*****************************************************************************
/*****************************************************************************
*/
}
