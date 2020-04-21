/* attributes.c			<< GF2/UC4  >>
 *
 *  --   bit plane mode-setting commands
 *	uses stm.mic routines
 *	uses TESTCURDRAWN in cursor.c
 *
 *    includes:
 *	CHAR_SETUP	(baseaddress)
 *	LOAD_COLORCODE	(colorAB)
 *	LOAD_WRTEN	(wrtenAB)
 *	RGB_COLORCODE	(R, G, B)
 *	RGB_WRTEN	(R, G, B)
 *	CD_COLORWE	(colorCD,wrtenCD)
 *	LOAD_CONFIG	(mode,config)
 *	LINE_STIPPLE	(repeat,stipple)
 *	LINE_STYLE	(width)
 *	SELECT_CURSOR	(baseadr,mode,config,colorCD,colorAB,wrtenCD,wrtenAB)
 *	SELECT_RGB_CURSOR (baseadr,mode,config,clrR,clrG,clrB,weR,weG,weB)
 *	DEPTH_SETUP	(a,b,Imin,Imax,Zmin,Zmax)
 *	PIXEL_SETUP	(planes,Xmax)
 *
 *	end (sub)routine INTO_RAM
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"

#define _WK	12   /* working register - don't interfere with x-y coords */
#define _ADR	5	/* temp -- scratch addr	*/
#define _WK2	6
#define _POLYFLAGS 5

attributes()
{
newfile("attributes.c");

label(CHAR_SETUP)
	_NS LOADMAR(_CHARBASE); CONST(_CHARBASE); _ES

	_NS REGHOLD; DOJUMP(SAVE1WD); _ES


label(LOAD_COLORCODE)
/*
 * bit plane color code load -- inputs one word of code for A/B planes
 *		whose bit meaning depends on swizzle, buffer mode.
 *
 *	first masks line stipple load enable out of current configuration
 *	executes bit plane color load command
 */
	_NS 
	    SETROP(0,ALL16); LOADDI(UCONST); CONST(_CONFIG+1);
	    SETSOP(NONQOP, _ADR, RAMNOP);
	    ALUOP(RONLYOP, P0);
	    FTOYANDQ(FF, OLDQ, REGWRE);
	    DOTOMAR(LOAD);
	_ES
					   /* current config bits  */

	_NS MICROCONST(_CONFIGMASK); LOADDI(UCONST);
	 SETROP(0,ALL16); SETSOP(NONQOP,0,RAMRD);
	 ALUOP(ANDOP,P0); FTODO; BPC(LOADCONFIG); CONST(_CONFIGMASK);_ES
						/* mask & strobe */

	_NS LOADMAR(_COLOR+1); CONST(_COLOR+1); _ES   /* point to save area */

	_NS LOADREG(_WK,ALL16,NOMORE); BPC(SETCOLORAB); _ES  /* write color */

label(COLOR_EXIT)
	_NS SETROP(_ADR,NONE); SETSOP(NONQOP,_WK,RAMWR);
	 ALUOP(MOVE); FTODO; LDMAR; _ES
				/* save in scratch, re-address CONFIG */

	_NS RAM(RAMRD,_TEMP,HOLD); BPC(LOADCONFIG);
	 GEGET; DOJUMP(DISPATCH); _ES	/* real config bits to BPC; exit */


label(LOAD_WRTEN)
/*
 *  loads bit plane write enables  -- similar to LOAD_COLORCODE
 */
	_NS LOADIMM(_ADR,_CONFIG+1); LDMAR; CONST(_CONFIG+1); _ES
					   /* current config bits  */

	_NS MICROCONST(_CONFIGMASK); LOADDI(UCONST);
	 SETROP(0,ALL16); SETSOP(NONQOP,0,RAMRD);
	 ALUOP(ANDOP,P0); FTODO; BPC(LOADCONFIG); CONST(_CONFIGMASK);_ES
						/* mask & strobe */

	_NS 
	    DOTOMAR(DEC); 
	    LOADREG(_WK,ALL16,NOMORE);
	_ES

	_NS /* test for double buffering */
	    ALUOP(ANDOP, P0);
	    SETROP(0, ALL16); LOADDI(UCONST); CONST(2);
	    SETSOP(NONQOP, 0, RAMRD);
	    FTOYANDQ(FF, OLDQ, REGWRD);
	_ES

	_NS 
	    REGHOLD;
	    COND(IFZ); 
	    SEQ(JUMP); 
	    NEXT(REGULAR_LOAD_WRTEN); 
	_ES

	_NS LOADIMM(_WK2, _ZBUFFER); LDMAR; CONST(_ZBUFFER); _ES

	_NS RAM(RAMRD, _WK2, HOLD); _ES

	_NS 
	    REGHOLD;
	    COND(IFZ); SEQ(JUMP); NEXT(REGULAR_LOAD_WRTEN);
	_ES

	_NS 
	    REGREG(FLOWOP, P0, _WK2, _WK2);
	    BPC(LOADMODE);
	    SEQ(JSUB); NEXT(LOAD_DBZ_WENABLES);
	_ES

	_NS REGHOLD; SEQ(JUMP); GEGET; NEXT(DISPATCH); _ES
    
label(REGULAR_LOAD_WRTEN)

	_NS LOADMAR(_WRTEN+1); CONST(_WRTEN+1); _ES   /* point to save area */

	_NS LOADREG(_WK,ALL16,NOMORE); BPC(SETWEAB); _ES    /* send wrten */

	_NS REGHOLD; DOJUMP(COLOR_EXIT); _ES

/********************/
/********************/

label(LOAD_DBZ_WENABLES)
	_NS LOADMAR(_CONFIG+1); CONST(_CONFIG+1); _ES

	_NS /* test for what buffer I'm displaying */
	    ALUOP(ANDOP, P0);
	    SETROP(0, ALL16); LOADDI(UCONST); CONST(1);
	    SETSOP(NONQOP, 0, RAMRD); DOTOMAR(DEC);
	    FTOYANDQ(FF, OLDQ, REGWRD);
	_ES

	_NS /* get the new mask */
	    COND(IFZ); NEXT(DISPLAYB_WE);
	_ES

	_NS LOADMAR(_WRTEN+1); CONST(_WRTEN+1); _ES   /* point to save area */

	_NS /* get the low byte of the new mask */
	    ALUOP(ANDOP, P0);
	    SETROP(0, ALL16); LOADDI(UCONST); CONST(0xff)
	    SETSOP(NONQOP, _WK, RAMWR);
	    FTOYANDQ(FF, LDQ, REGWRD);
	_ES

	_NS 
	    ALUOP(IOROP, P0);
	    SETROP(0, ALL16); LOADDI(UCONST); CONST(0xff00);
	    SETSOP(QOPERAND, 0, RAMNOP);
	    FTOYANDQ(FF, OLDQ, REGWRD);
	    BPC(SETWEAB);
	_ES

	_NS /* get the high byte of the new mask */
	    ALUOP(ANDOP, P0);
	    SETROP(0, ALL16); LOADDI(UCONST); CONST(0xff00);
	    SETSOP(NONQOP, _WK, RAMNOP);
	    FTOYANDQ(FF, OLDQ, REGWRE);
	_ES

	_NS
	    REGREG(SONLYOP, P0, 0, _WK);
	    YTODO(SWAP);
	_ES

	_NS 
	    ALUOP(IOROP, P0);
	    SETROP(0, ALL16); LOADDI(UCONST); CONST(0xff00);
	    SETSOP(NONQOP, _WK, RAMNOP);
	    FTOYANDQ(FF, OLDQ, REGWRD);
	    BPC(SETWECD);
	_ES

	_NS
	    REGHOLD;
	    SEQ(JUMP); 
	    NEXT(DBZ_WENABLE_EXIT);
	_ES

label(DISPLAYB_WE)

	_NS LOADMAR(_WRTEN+1); CONST(_WRTEN+1); _ES   /* point to save area */

	_NS /* get the high byte of the new mask */
	    ALUOP(ANDOP, P0);
	    SETROP(0, ALL16); LOADDI(UCONST); CONST(0xff00)
	    SETSOP(NONQOP, _WK, RAMWR);
	    FTOYANDQ(FF, LDQ, REGWRD);
	_ES

	_NS 
	    ALUOP(IOROP, P0);
	    SETROP(0, ALL16); LOADDI(UCONST); CONST(0xff);
	    SETSOP(QOPERAND, 0, RAMNOP);
	    FTOYANDQ(FF, OLDQ, REGWRD);
	    BPC(SETWECD);
	_ES

	_NS /* get the low byte of the new mask */
	    ALUOP(ANDOP, P0);
	    SETROP(0, ALL16); LOADDI(UCONST); CONST(0xff)
	    SETSOP(NONQOP, _WK, RAMNOP);
	    FTOYANDQ(FF, OLDQ, REGWRE);
	_ES

	_NS
	    REGREG(SONLYOP, P0, 0, _WK);
	    YTODO(SWAP);
	_ES

	_NS 
	    ALUOP(IOROP, P0);
	    SETROP(0, ALL16); LOADDI(UCONST); CONST(0xff);
	    SETSOP(NONQOP, _WK, RAMNOP);
	    FTOYANDQ(FF, OLDQ, REGWRD);
	    BPC(SETWEAB);
	_ES

label(DBZ_WENABLE_EXIT);
	_NS 
	    LOADMAR(_CONFIG); CONST(_CONFIG);
	_ES

	_NS 
	    RAM(RAMRD, _TEMP, HOLD); BPC(LOADMODE);
	_ES

	_NS
	    REGHOLD; DOTOMAR(INC);
	_ES

	_NS 
	    RAM(RAMRD, _TEMP, HOLD); BPC(LOADCONFIG); 
	    SEQ(RETN);
	_ES

/********************/
/********************/

label(CD_WRTEN)	/* old name */
label(CD_COLORWE)
/*
 *  loads CD bit plane color, write enables only
 */

	_NS LOADIMM(_ADR,_CONFIG+1); LDMAR; CONST(_CONFIG+1); _ES
					   /* current config bits  */

	_NS MICROCONST(_CONFIGMASK); LOADDI(UCONST);
	 SETROP(0,ALL16); SETSOP(NONQOP,0,RAMRD);
	 ALUOP(ANDOP,P0); FTODO; BPC(LOADCONFIG); CONST(_CONFIGMASK);_ES
						/* mask & strobe */

	_NS LOADMAR(_COLOR); CONST(_COLOR); _ES

	_NS LOADREG(_WK,ALL16,NOMORE); BPC(SETCOLORCD); _ES

	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_WK,RAMWR);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE); LDMAR; GEGET; CONST(_WRTEN); _ES
			/* while writing color to scratch, pt to wrten */

	_NS LOADREG(_WK,ALL16,NOMORE); BPC(SETWECD); _ES    /* send wrten */

	_NS REGHOLD; DOJUMP(COLOR_EXIT); _ES


label(RGB_COLORCODE)		/* (Red, Grn, Blu) */
	/*	NOTE -- SWIZZLE and DOUBLEBUFFER BITS MUST BE OFF IN RGB MODE
	 *	pack Red, Grn
	 *	look up Blu in swizzle table (8 bits)
	 *	mask out ldlinestip bit
	 *	write Red,Grn to AB planes (save in scratch)
	 *	write swizzled Blu to CD planes (save in scratch)
	 *	restore ldlinestip bit
	 */

	_NS LOADIMM(_ADR,_CONFIG+1); LDMAR; CONST(_CONFIG+1); _ES
					   /* current config bits  */

	_NS MICROCONST(_CONFIGMASK); LOADDI(UCONST);
	 SETROP(0,ALL16); SETSOP(NONQOP,0,RAMRD);
	 ALUOP(ANDOP,P0); FTODO; BPC(LOADCONFIG); CONST(_CONFIGMASK);_ES
					/* mask ldlinestip & strobe */

	_NS LOADREG(_WK,ALL16,MORE); _ES	/* read in red */

	_NS LOADREG(_WK,HI8,MORE); BPC(SETCOLORAB); _ES
					/* read grn into high byte; strobe */

	_NS LOADREG(_WK2,ALL16,NOMORE); _ES	/* read blue */

	_NS IMMREG(ADD,_SWIZZLETAB,_WK2); LDOUT; CONST(_SWIZZLETAB); _ES
					/* index address to outreg */

	_NS LOADDI(OUTPUTREG); SETROP(0,ALL16);
	 ALUOP(MOVE); FTODO; LDMAR; COND(IFFALSE); SEQ(CJPP); _ES
							/* move to MAR */

	_NS RAM(RAMRD,_WK2,HOLD); BPC(SETCOLORCD); _ES	/* write swiz'd blu */

	_NS LOADMAR(_COLOR); CONST(_COLOR); _ES

	_NS RAM(RAMWR,_WK2,INC); DOJUMP(COLOR_EXIT); _ES
				/* save AB planes, go save rest & exit */


label(RGB_WRTEN)
	_NS LOADIMM(_ADR,_CONFIG+1); LDMAR; CONST(_CONFIG+1); _ES
					   /* current config bits  */

	_NS MICROCONST(_CONFIGMASK); LOADDI(UCONST);
	 SETROP(0,ALL16); SETSOP(NONQOP,0,RAMRD);
	 ALUOP(ANDOP,P0); FTODO; BPC(LOADCONFIG); CONST(_CONFIGMASK);_ES
					/* mask ldlinestip & strobe */

	_NS LOADREG(_WK,ALL16,MORE); _ES	/* read in red */

	_NS LOADREG(_WK,HI8,MORE); BPC(SETWEAB); _ES
					/* read grn into high byte; strobe */

	_NS LOADREG(_WK2,ALL16,NOMORE); _ES	/* read blue */

	_NS IMMREG(ADD,_SWIZZLETAB,_WK2); LDOUT; CONST(_SWIZZLETAB); _ES
					/* index address to outreg */

	_NS LOADDI(OUTPUTREG); SETROP(0,ALL16);
	 ALUOP(MOVE); FTODO; LDMAR; COND(IFFALSE); SEQ(CJPP);_ES
							/* move to MAR */

	_NS RAM(RAMRD,_WK2,HOLD); BPC(SETWECD); _ES	/* write swiz'd blu */

	_NS LOADMAR(_WRTEN); CONST(_WRTEN); _ES

	_NS RAM(RAMWR,_WK2,INC); DOJUMP(COLOR_EXIT); _ES



label(LOAD_CONFIG)	/* (mode,config) */
/*
 *	modal setting of bit plane control configuration and mode
 *	also sets DEPTHCUE flag according to mode
 */
	_NS LOADMAR(_CONFIG); CONST(_CONFIG); _ES

	_NS /* read the new mode masking into WK2 */
	    LOADREG(_TEMP,ALL16,NOMORE); 
	_ES

	_NS /* mask off everything but the double bit */
	    ALUOP(ANDOP, P0);
	    SETROP(0, ALL16); LOADDI(UCONST); CONST(2);
	    SETSOP(NONQOP, _TEMP, RAMNOP);
	    FTOYANDQ(FF, LDQ, REGWRD);
	_ES

	_NS /* read the current mode and mask it too */
	    ALUOP(ANDOP, P0);
	    SETROP(0, ALL16); LOADDI(UCONST); CONST(2);
	    SETSOP(NONQOP, _WK2, RAMRD);
	    FTOYANDQ(FF, OLDQ, REGWRE);
	    DOTOMAR(INC); /* to point to config */
	_ES

	_NS /* XOR the two double bits */
	    ALUOP(XOROP, P0);
	    SETROP(_WK2, NONE);
	    SETSOP(QOPERAND, 0, RAMNOP);
	    FTOYANDQ(FF, OLDQ, REGWRD);
	_ES

	_NS /* cond jump to the clear code with the MAR pointing at config*/
	    SEQ(JSUB);
	    NEXT(CLEAR);
	    COND(IFNZ);
	    GEGET;
	_ES

	_NS /* read in the config and dec mar */
	    LOADREG(_WK2,ALL16,NOMORE); BPC(LOADCONFIG);
	_ES

	_NS
	    RAM(RAMWR, _WK2, DEC);
	_ES

	_NS 
	    ALUOP(RONLYOP, P0);
	    SETROP(_TEMP, NONE);
	    SETSOP(NONQOP, _TEMP, RAMWR);
	    FTOYANDQ(FF, OLDQ, REGWRE);
	    BPC(LOADMODE);
	_ES

	_NS /* command to load new config and mode */
	    LOADIMM(_MODEMASK,_ALTDEPBIT); 
	    CONST(_ALTDEPBIT); 
	    BPC(SETADDRS);
	_ES

	_NS LOADMAR(_ALTPOLYMODE); CONST(_ALTPOLYMODE); _ES

	_NS RAM(RAMRD,_POLYFLAGS,HOLD); _ES

	_NS IMMREG(ANDRS,_ALTDEPBIT,_TEMP); CONST(_ALTDEPBIT); _ES
				/* mask just depthcue bit */

	_NS REGREG(ORRS,_MODEMASK,_POLYFLAGS);	/* assume bit is one    */
	 COND(IFNZ); DOJUMP(WRTNEWMODE); _ES	/* if it is, skip ahead */

	_NS REGREG(XOR,_MODEMASK,_POLYFLAGS); _ES  /* if not, clear the bit */

label(WRTNEWMODE)
	_NS LOADDI(UCONST); SETROP(0,ALL16); SETSOP(NONQOP,_POLYFLAGS,RAMWR);
	 ALUOP(MOVE); FTODO; LDMAR; CONST(_DEPTHCUEMODE); _ES
			/* while writing, point to Depthcue flag */

	_NS REGREG(MOVE,_TEMP,_TEMP); DOJUMP(LOADFLAG); _ES
					/* write masked-out depth-cue bit */


label(POLY_STIPPLE)
/*
 *	store new polygon stipple pattern base address
 *	sent to BPC with each polygon draw
 */
	_NS LOADMAR(_CHARBASE); CONST(_CHARBASE); _ES

	_NS RAM(RAMRD,_TEMP,HOLD); _ES

	_NS LOADDI(INRJUST); SETROP(0,ALL16); SETSOP(NONQOP,_TEMP,RAMNOP);
	 ALUOP(ADD); YQ(FF,OLDQ,REGWRE); _ES
			/* add incoming address to base address */

	_NS LOADMAR(_POLYSTIPADR); CONST(_POLYSTIPADR);_ES

	_NS RAM(RAMWR,_TEMP,HOLD); GEGET; DOJUMP(DISPATCH); _ES


label(LINE_STIPPLE)	/* (repeat, stipple) */
/*
 *	store new line stipple pattern and repeat count
 *	Repeat stored now (and, for now, never again) to UC register
 *	Stipple sent to BPC with each line draw command
 */
	_NS LOADMAR(_LINEREPEAT); CONST(_LINEREPEAT); _ES

	_NS LOADREG(_WK,ALL16,NOMORE); _ES

	_NS REGREG(MOVE,_WK,_WK); GEGET; BPC(LOADREPEAT); _ES

	_NS LOADDI(INRJUST); SETROP(0,ALL16); SETSOP(NONQOP,_WK,RAMWR);
	 ALUOP(MOVE); YQ(FF,OLDQ,REGWRE); DOTOMAR(DEC); _ES
		/* write repeat, then get stipple; point to LINESTIP */

	_NS RAM(RAMWR,_WK,HOLD); GEGET; DOJUMP(DISPATCH); _ES


label(LINE_STYLE)
/*
 *  Store new line width in scratch ram
 */
	_NS LOADMAR(_LINESTYLE); CONST(_LINESTYLE);_ES

label(SAVE1WD)
	_NS LOADIMM(_TEMP,0); SEQ(LDCT); CONST(0); _ES
					/* one word read and saved */

	_NS REGHOLD; DOJUMP(INTO_RAM); _ES


label(SELECT_CURSOR)
/*	reads & saves 7 parameters in scratch at _CURSOR
 */
	_NS LOADMAR(_CURSORDRAWN); CONST(_CURSORDRAWN); _ES

	_NS RAM(RAMRD,_TEMP,HOLD); DOJSUB(TESTCURDRAWN); _ES

	_NS LOADMAR(_CURSOR); CONST(_CURSOR); _ES

	_NS LOADIMM(_TEMP,6); SEQ(LDCT); CONST(6); _ES
					/* seven words read & saved */

	_NS REGHOLD; DOJUMP(INTO_RAM); _ES


label(SELECT_RGB_CURSOR)

	_NS LOADMAR(_CURSORDRAWN); CONST(_CURSORDRAWN); _ES

	_NS RAM(RAMRD,_TEMP,HOLD); DOJSUB(TESTCURDRAWN); _ES

	_NS LOADMAR(_CURSOR); CONST(_CURSOR); _ES

	_NS REGHOLD; LOADDI(UCOUNT); DOPUSH(2); _ES

	_NS LOADREG(_WK,ALL16,NOMORE); _ES

	_NS RAM(RAMWR,_WK,INC); GEGET; SEQ(LOUP); _ES	/* wrt 1st 3 wds */

	_NS LOADIMM(_ADR,_CURSOR+3); CONST(_CURSOR+3); _ES  /* save MAR */

		/* handle colors, we's */

	_NS REGHOLD; LOADDI(UCOUNT); DOPUSH(1); _ES

	_NS LOADREG(_WK2,ALL16,MORE); _ES	/* get red */

	_NS LOADREG(_WK2,HI8,MORE);  _ES	/* combine with grn */

	_NS LOADREG(_WK,ALL16,NOMORE); _ES	/* read blue */

	_NS IMMREG(ADD,_SWIZZLETAB,_WK); LDOUT; CONST(_SWIZZLETAB); _ES
					/* index address to outreg */

	_NS LOADDI(OUTPUTREG); SETROP(0,ALL16);
	 ALUOP(MOVE); FTODO; LDMAR; COND(IFFALSE); SEQ(CJPP);_ES
							/* move to MAR */

	_NS RAM(RAMRD,_WK,HOLD); _ES	/* read swizzled blue */

	_NS REGREG(MOVE,_ADR,_ADR); LDMAR; _ES	/* restore MAR */

	_NS RAM(RAMWR,_WK,INC); _ES		/* write blu as CD planes */

	_NS IMMREG(ADD,2,_ADR); CONST(2); _ES	/* poke dest adr */

	_NS RAM(RAMWR,_WK2,HOLD); GEGET; SEQ(LOUP); _ES
			  /* write blu, get next wd & repeat */

	_NS REGHOLD; DOJUMP(DISPATCH); _ES


label(DEPTH_SETUP)

/*	reads & saves 6 wds in scratch
 */

	_NS LOADMAR(_DEPTH_PARAMS); CONST(_DEPTH_PARAMS); _ES

	_NS LOADIMM(_TEMP,5); SEQ(LDCT); CONST(5); _ES

	_NS REGHOLD; DOJUMP(INTO_RAM); _ES


label(PIXEL_SETUP)
/*	reads & saves 2 words - planes code, Xright
 */
	_NS LOADMAR(_PIXELPARAMS); CONST(_PIXELPARAMS); _ES

	_NS LOADIMM(_TEMP,1); SEQ(LDCT); CONST(1); _ES
					/* continue to subroutine */


label(INTO_RAM)		/* subroutine to input and write to scratch */
	_NS LOADREG(_WK,ALL16,NOMORE); _ES

	_NS RAM(RAMWR,_WK,INC); GEGET; SEQ(RPCT); NEXT(INTO_RAM); _ES
						/* loop until count zero */

	_NS REGHOLD; DOJUMP(DISPATCH); _ES
}
