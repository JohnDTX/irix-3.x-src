/* consts.h		<<  GF2/UC4  >>
 *
 *	global constants for FBC
 *	microcode, e.g. ram table addesses and shared registers
 */

#include "interrupts.h"

#ifdef GF2
#define SCRATCHSIZE 0xfff
#else
#define SCRATCHSIZE 0x7ff
#endif


/*	SCRATCH  RAM  ADDRESSES		*/

#define _DUPHITMODE	1	/* Copy of HITMODE - used by CHECK_HIT subr */
#define _HITBITS	2	/* Accum'ed hit bits in left half: FNTBRL   */
#define _HITCHARCT	3	/* old: count within string		*/
#define _HITSTACKPTR	4	/* Points to top of hit stack		*/
#define _HITSTACKLIM	5	/* Points to top of scratch (stack bottom   */

#define _CURSOR		6	/* cursor glyph base address in font ram*/
				/* mode bits				*/
				/* config bits for cursor draw/undraw	*/
				/* cursor color C/D			*/
				/* cursor color A/B			*/
				/* cursor wrten C/D			*/
				/* cursor wrten A/B			*/

#define _FIXPARAMS	13	/* for FIXCHAR_DRAW:	height		*/
				/*			width		*/
				/*			spacing		*/
#define _CHARVIEW	16	/* Character viewport, X (VL)	*/
				/*		       Y (VB)	*/
#define _CHARV2		18	/* 		       X (VR)	*/
				/*		       Y (VT)	*/
#define _CHARVSAVE	20	/* Save area used by chdraw.c:	x	*/
				/*				y	*/
				/* 				i	*/
#define _CHARPOSN	23	/* Character position, X	*/
				/*		     Y		*/
#define _HITMODE	25	/* flag -- 			*/
#define _PASSCHARS	26	/* flag --
				/*	 0= POINT not seen after CHAR POSN  */
				/*	-1= valid, new (load new pixel X,Y) */
				/*	-2= valid, old (don't load)	    */
#define _POLYSTIPADR	27	/* holds font ram base addr of poly stipple */
#define _MULTIVIEW	28	/* flag -- 0 if single FB viewport	*/
#define _LINESTYLE	29	/* flag -- 0 = normal vectors; n = bold */
#define _LINESTIP	30	/* holds line stipple pattern		*/
#define _LINEREPEAT	31	/* repeat count for above	*/
#define _CONFIG		32	/* bit plane mode bits		*/
				/* bit plane config bits	*/
#define _COLOR		34	/* normal mode color C/D	*/
				/* normal mode color A/B	*/
#define _WRTEN		36	/* normal mode wrten C/D	*/
				/* normal mode wrten A/B	*/
	/* next 4 wds in fixed sequence */
#define _ALTVECMODE	38	/* flag -- 0 = normal vectors
				 *   bits: ALTHITBIT
				 *	   ALTDEPBIT
				 *	   ALTMVPBIT	
				 *	   ALTZBUBIT		*/
#define _POLYGONSTYLE   39	/* 0 for flat shaded, 1 for shaded	*/
#define _ZBUFFER	40	/* 0 for normal, 1 for z-buffered	*/
#define _DEPTHCUEMODE	41
#define _BACKFACING	42
#define _LEFT_INC	43	/* left and right side polygon indices */
#define _RIGHT_INC	44
#define _ALTPOLYMODE	45	/* "or" of DEPTHCUEMODE, ZBUFFER	*/

	/* *****  free space 46 *****	*/

#define _CHARBASE	47	/* for chdraw.c;  must precede SAVE1	    */
/* initialization clears ram up to this point			*/
#define _ENDINIT	47

#define _SAVE1		48	/* 16 words to hold all of the 2903's regs. */
#define _SAVE2		64	/* 16 words to hold all of the 2903's regs. */
#define _CURRVIEWPORT   80	/* 4 words lbrt viewport values */
#define _SECONDCURSOR	84	/* 5 wds: 2nd cursor glyph, colors, we's */

	/* *****  free space 89-91 *****	*/

#define _CURSORDRAWN	92	/* flag whether cursor currently drawn	*/
#define _OLDCURSOR	93	/* X coordinate of previously drawn cursor  */
				/* Y coord...				*/
	/* the following 2 consts MUST NOT MOVE!!  -- msg		*/
#define _DIVTAB_VALID	95	/* 1 = valid; cleared if top 2K messed up   */
#define _REGSAVE	96	/* 16 words for SAVE REGS command	*/
		/*	111	 last word of save area			*/
#define _PIXELPARAMS	112	/* planes flag -- |1=AB  |2=CD	*/
				/* Xright			*/
#define _DEPTH_PARAMS	114	/* a  in  aZ+b			*/
				/* b  in  aZ+b			*/
				/* Imin				*/
				/* Imax				*/
				/* Zmin				*/
				/* Zmax				*/

#define _SCR_BIASED_DONE	120
#define _SCR_MIN_MASK		121
#define _SCR_MAX_MASK		122

/* the following 4 words are in a set order */
#define _MASKEOL	123	/* points 1 past last wd in list	*/
#define _MASKPTR	124	/* points to next viewport (or to EOL)	*/
#define _MASKLIM	125	/* (_HITSTACKLIM) or _STARTLIST		*/
#define _SAVECORNERS    126	/* the upper right corner of a poly */
				/* the lower left corner */
#define _CURSAVE	128	/* register save area (8 words) */

/* locs 136&7 are used as part of the block of deltas */
#define _SCR_DEL_XLEFT_HI	138	/* polygons delta values */
#define _SCR_DEL_XLEFT_LO	139	
#define _SCR_DEL_COLOR_LEFT_HI	140	
#define _SCR_DEL_Z_LEFT_HI	140	
#define _SCR_DEL_COLOR_LEFT_LO	141	
#define _SCR_DEL_Z_LEFT_LO	141	

#define _SCR_DEL_XRIGHT_HI	142	/* polygons delta values */
#define _SCR_DEL_XRIGHT_LO	143	
#define _SCR_DEL_COLOR_RIGHT_HI	144	
#define _SCR_DEL_Z_RIGHT_HI	144	
#define _SCR_DEL_COLOR_RIGHT_LO	145	
#define _SCR_DEL_Z_RIGHT_LO	145	

#define _SCR_DEL_XLEFT_HI_ZS	138	/* polygons delta values */
#define _SCR_DEL_XLEFT_LO_ZS	139	
#define _SCR_DEL_COLOR_LEFT_HI_ZS	140	
#define _SCR_DEL_COLOR_LEFT_LO_ZS	141	
#define _SCR_DEL_Z_LEFT_HI_ZS	142
#define _SCR_DEL_Z_LEFT_LO_ZS	143
#define _SCR_DEL_XRIGHT_HI_ZS	144	/* polygons delta values */
#define _SCR_DEL_XRIGHT_LO_ZS	145	
#define _SCR_DEL_COLOR_RIGHT_HI_ZS	146	
#define _SCR_DEL_COLOR_RIGHT_LO_ZS	147	
#define _SCR_DEL_Z_RIGHT_HI_ZS	148
#define _SCR_DEL_Z_RIGHT_LO_ZS	149

#define _SCR_Z_LEFT_LO_ZS	150
#define _SCR_Z_LEFT_HI_ZS	151
#define _SCR_Z_RIGHT_LO_ZS	152
#define _SCR_Z_RIGHT_HI_ZS	153

#define _SCR_RIGHTCOUNT		150
#define _SCR_LEFT		151
#define _SCR_RIGHT		152
#define _SCR_LEFTCOUNT		153

#define _SCR_Z_LEFT_LO_ZS_SAVE	154
#define _SCR_Z_RIGHT_HI_ZS_SAVE	155

#define _STARTLIST	158	/* Start of list of polygon vertices.  
				Needs 3 +	*/
			/* 4*(number of vertices) words of space.	*/
			/* Either this list or the variable font param	*/
			/* stuff should grow the other direction, so the */
			/* two areas can overlap if more space is needed */
			/* temporarily.					*/

	/**********  free space up to _ENDOFSCRATCH for runtime stuff *******/

#define _ENDOFSCRATCH	(0xfff - 2048 - 256 - 32)	/* before tables */

#define _LEFTMASK	(_ENDOFSCRATCH+1)
#define _RIGHTMASK	(_LEFTMASK+16)
#define _SWIZZLETAB	(_RIGHTMASK+16)
#define _DIVTAB		(_SWIZZLETAB+256)

/*================================================================*/

/*	HANDY CONSTANTS				*/

#define _CMDMASK	0x3f	/* just 6 lsb's	*/
#define _HITBITMASK	0x3f00	/* just 6 lsb's of high byte	*/
#define _CONFIGMASK	0xff7f	/* mask out loadlinestip bit	*/
#define _FINISHBIT	0x40	/* FinishLine bit in config	*/
#define _PFIMASK	0xfcff	/* mask out PFIRead and PFICD in config */
#define _PFICD		0x100
#define _PFIREAD	0x200
#define _UPDATEBITS	0xc	/* both update bits in config reg */
#define _DEPTHMASK	0xfffb 	/* not DepthCue bit in Mode reg	*/
#define _SWIZZLEBIT	1	/* Swizzle bit in Mode reg	*/
#define _DOUBLEBIT	2	/* Double  bit in Mode reg	*/

#define _ALTMVPBIT	1
#define _ALTHITBIT	2	/* bits in _ALTVECMODE flag word */
#define _ALTDEPBIT	4
#define _ALTZBUBIT	8

#define _PASSCODE	8
#define _POINTCODE	18
#define _YOFFSCREEN	1008	/* for cursor offscreen save area	*/
#define _BIGNEG		0xC000
#define _MAXPOS		32767	/* maximum positive 16-bit no.		*/

#define _EOFLENGTH	2
#define _FBCEOF1	0x108

/*		2903 REGISTER CONSTANTS		*/

/* these are used by lowmem.mic and passthru-type commands: */
#define _ZERO   15	/*scratch register for fixed zero	*/
#define _PASSN  14	/* "		" for passthru N count	*/
#define _TEMP	7
#define _ZRO    6	/* for DRAW_CHARS			*/

/*  these are used by vectors.mic, points.mic,		:*/
#define _X1		0
#define _Y1		1
#define _I1		2	/* do not disturb 0..2 between MOVE, DRAW ! */
#define _Z1		_I1
#define _X2		3
#define _Y2		4
#define _I2		5
#define _Z2		_I2
#define _DELTAX		6
#define _STIP		11
#define _BOLD	  	13
#define _NEWFRAC_COLOR  9
#define _FRAC_COLOR	10	/* holds the fractional part of the
				color from az+b */

#define _HITREG   14	/* accumulated hit bits  */
#define _LIMIT	  12	/* used in XFORM_POINT, FEEDBACK_NEXT */
#define _QUITFLAG 13	/* ditto */
#define _MODEMASK 4	/* modes.c, attributes.c, viewport.c */
