/* dispatch.c	--         DISPATCH TABLE	<< GF2/UC4  >> *
 * The lowest memory locations of microcode are reserved for a dispatch
 * table.  All other routines are accessed from this table.
 * This file also contains the initialization code and various trivialities.
 * All undefined codes are trapped.
 */

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"


#define UNDEFINED	REGREG(ANDOP,P0,0,0); SEQ(JUMP); NEXT(UNDEF_TRAP)
/*	 rejection of unimplemented command vectors	*/

#define UNIMPLEMENTED	REGREG(ANDOP,P0,0,0); SEQ(JUMP); NEXT(UNIMPL_TRAP)
		/*	inform host of unimplemented command		*/

#define REGNOP	REGREG(ANDOP,P0,0,0)
#define CLRZERO	SETROP(0,NONE); SETSOP(NONQOP,_ZERO,RAMNOP); \
		ALUOP(FLOWOP,P0); FTOYANDQ(FF,LDQ,REGWRE)
						/*	conveniences	*/

dispatch()
{
newfile("dispatch.c");

reloc(4095)
	_NS REGNOP; INTERRUPTHOST; SEQ(JZER); _ES
		/* last location in PROM:
		 * interrupt to stop clock after next-address floats here.
		 * when interrupt dismissed, code starts in earnest at loc. 0
		 */
reloc(0)
label(V00)
/* 0 */	_NS CLRZERO; SETLED; DOTOOUTREG; SEQ(JUMP); NEXT(INIT_START); _ES
/* 1 */	_NS UNDEFINED; _ES  				/*GE Load MM.	*/
/* 2 */	_NS CLRZERO; GEOMENGDATA; SEQ(JUMP); NEXT(MASK_LIST); _ES
/* 3 */	_NS UNIMPLEMENTED; _ES				/*GE store MM	*/
/* 4 */	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(RGB_COLORCODE); _ES
/* 5 */	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(RGB_WRTEN); _ES
/* 6 */	_NS CLRZERO; SEQ(JUMP); NEXT(SET_HIT_MODE); _ES
/* 7 */	_NS CLRZERO; SEQ(JUMP); NEXT(CLEAR_HIT_MODE); _ES

/*dec/hex locations  */
/* 8 */	_NS REGREG(FLOWOP,P0,0,_PASSN); SEQ(JUMP); NEXT(PASS_THRU); _ES
/* 9 */	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(CHAR_SETUP); _ES
/*10/a*/ _NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(CD_WRTEN); _ES
/*11/b*/	_NS UNIMPLEMENTED; _ES
/*12/c*/	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(DISPATCH); _ES /* GE reconfigure   */
/*13/d*/	_NS CLRZERO; GEOMENGDATA; SEQ(JUMP); NEXT(DRAW_PIXELS); _ES
/*14/e*/	_NS REGNOP; SEQ(JUMP); NEXT(READ_PIXELS); _ES
/*15/f*/	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(DISPATCH); _ES	/* GE Noop   */


/*16/10*/	_NS CLRZERO; SEQ(JUMP); NEXT(VECTOR_MOVE); _ES
/*17/11*/	_NS REGREG(RONLYOP,P0,_Y1,_Y1); DOTOOUTREG; BPCCMD(LOADYS);
				 SEQ(JUMP); NEXT(VECTOR_DRAW); _ES
			/* make sure BPC's YS reg is loaded with Y1	*/
/*18/12*/	_NS REGNOP; SEQ(JUMP); NEXT(POINT); _ES
/*19/13*/	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(LOAD_VIEWPORT); _ES
/*20/14*/	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(LOAD_COLORCODE); _ES
/*21/15*/	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(LOAD_WRTEN); _ES
/*22/16*/	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(LOAD_CONFIG); _ES
/*23/17*/	_NS CLRZERO; GEOMENGDATA; SEQ(JUMP); NEXT(LOAD_MASKS); _ES


/*24/18*/	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(SELECT_RGB_CURSOR); _ES
/*25/19*/	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(LINE_STYLE); _ES
/*26/1a*/	_NS CLRZERO; GEOMENGDATA; SEQ(JUMP);NEXT(CHARPOSN_ABS); _ES
/*27/1b*/	_NS CLRZERO; GEOMENGDATA; SEQ(JUMP);NEXT(CHARPOSN_REL); _ES
/*28/1c*/	_NS REGREG(FLOWOP,P0,0,_ZRO); SEQ(JUMP); NEXT(DRAW_CHARS); _ES
/*29/1d*/	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(SELECT_CURSOR); _ES
/*30/1e*/	_NS REGNOP; SEQ(JUMP); NEXT(DRAW_CURSOR); _ES
/*31/1f*/	_NS REGNOP; SEQ(JUMP); NEXT(UNDRAW_CURSOR); _ES


/*32/20*/	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(LINE_STIPPLE); _ES
/*33/21*/	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(POLY_STIPPLE); _ES
/*34/22*/	_NS REGNOP; SEQ(JUMP); NEXT(SAVE_REGS); _ES
/*35/23*/	_NS REGNOP; SEQ(JUMP); NEXT(UNSAVE_REGS); _ES
/*36/24*/	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(DEPTH_SETUP); _ES
/*37/25*/	_NS CLRZERO; SEQ(JUMP); NEXT(FEEDBACK_NEXT); _ES
/*38/26*/	_NS REGNOP; SEQ(JUMP); NEXT(EOF); _ES
/*39/27*/	_NS REGNOP; SEQ(JUMP); NEXT(READ_CHARPOSN); _ES


/*40/28*/	_NS CLRZERO; GEOMENGDATA; SEQ(JUMP); NEXT(COPY_FONT); _ES
							/* was BOUNDING_BOX */
/*41/29*/	_NS REGREG(FLOWOP,P0,0,_ZRO); GEOMENGDATA; 
					SEQ(JUMP); NEXT(PUSH_NAME); _ES
/*42/2a*/	_NS REGREG(FLOWOP,P0,0,_ZRO); GEOMENGDATA; 
					SEQ(JUMP); NEXT(LOAD_NAME); _ES
/*43/2b*/ 	_NS REGREG(FLOWOP,P0,0,_ZRO); GEOMENGDATA;
					SEQ(JUMP); NEXT(POP_NAME); _ES
/*44/2c*/	_NS REGNOP; SEQ(JUMP); NEXT(FIXCHAR_LOAD); _ES
/*45/2d*/	_NS REGREG(FLOWOP,P0,0,_ZRO);
				 SEQ(JUMP); NEXT(FIXCHAR_DRAW); _ES

/*46/2e*/	_NS REGREG(FLOWOP,P0,0,_ZRO); GEOMENGDATA;
					SEQ(JUMP); NEXT(INIT_NAMESTACK); _ES
/*47/2f*/	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(PIXEL_SETUP); _ES


/*48/30*/	_NS REGNOP; SEQ(JUMP); NEXT(POLY_MOVE); _ES
/*49/31*/	_NS REGNOP; SEQ(JUMP); NEXT(POLY_DRAW); _ES
/*50/32*/	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(LOADRAM); _ES
/*51/33*/	_NS REGNOP; SEQ(JUMP); NEXT(POLY_CLOSE); _ES
/*52/34*/	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(DRAW_MODE); _ES
/*53/35*/	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(SETINTENSITY); _ES
/*54/36*/	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(SETBACKFACING); _ES
/*55/37*/	_NS REGNOP; SEQ(JUMP); NEXT(BPC_READBUS); _ES


/*56/38*/	_NS REGNOP; SEQ(JUMP); NEXT(XFORM_POINT); _ES
/*57/39*/	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(BLOCK_FILL); _ES
/*58/3a*/	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(DUMPRAM); _ES
/*59/3b*/	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(ZSHADESCANLINE); _ES
		/* was BPC LOADREG */
/*60/3c*/	_NS REGNOP; GEOMENGDATA; SEQ(JUMP); NEXT(BPC_COMMAND); _ES
/*61/3d*/	_NS CLRZERO; GEOMENGDATA; SEQ(JUMP); NEXT(COPY_SCREEN); _ES
/*62/3e*/	_NS CLRZERO; GEOMENGDATA; SEQ(JUMP); NEXT(ZSCAN_INIT); _ES
		/* was DBLFEED */
/*63/3f*/	_NS UNDEFINED; _ES

/*==============================================================*/
/* 		central dispatch				*/

label(DISPATCH)
/*64/40*/	_NS SETROP(0,ALL16);ALUOP(RONLYOP,P0);
		 FTOYANDQ(FF,OLDQ,REGWRD); COND(IFNFLAG); SEQ(VECT); _ES
			/* host sets flag to signal cursor command	*/
/*65/41*/	_NS REGNOP; DOTOOUTREG; BPCCMD(NOOP);
		 SEQ(JUMP); NEXT(SIDE_CURSOR); _ES
			/* make sure BPC is available, then go handle cursor*/
/*66/42*/	_NS CLRZERO; SEQ(JUMP); NEXT(RUN_LENGTH); _ES
/*67/43*/	_NS CLRZERO; SEQ(JUMP); NEXT(READ_RUNLENGTH); _ES
/*68/44*/	_NS CLRZERO; GEOMENGDATA; SEQ(JUMP); NEXT(BUFFER_COPY); _ES
/*69/45*/	_NS UNDEFINED; _ES
/*70/46*/	_NS UNDEFINED; _ES
/*71/47*/	_NS UNDEFINED; _ES
/*72/48*/	_NS UNDEFINED; _ES

/* lowmem.c to follow */
}