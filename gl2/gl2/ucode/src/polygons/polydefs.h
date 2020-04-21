/*	polydefs.h
 *
 *	global definitions for polygon routines
 */

#define _BIG 0x3fff	/* opp of bigneg?? */

/*================================================================*/
/*    2903 registers
/*================================================================*/


/* these registers are only used by polydraw, polyshade and polyclose */
#define _LASTPT		4
#define _BOTTOMY	5
#define _BOTTOMPNT	6
#define _CURSHADE	3
#define _PREVSHADE	8
#define _XTHIS		0
#define _YTHIS		1
#define _ZTHIS		2

#define _LEFTMOSTX	9
#define _RIGHTMOSTX	10
#define _TOPY		11
#define _LEFTPNT	12
#define _RIGHTPNT	13
#define _TOPPNT		15

#define _CORNER1	1
#define _CORNER2	7

/*	_TEMP		7	defined in consts.h	*/

#define _BIASED_DONE	8

#define _DEL_XLEFT_HI	10
#define _DEL_XLEFT_LO	14
#define _DEL_XRIGHT_HI	11
#define _DEL_XRIGHT_LO	15

#define _DELXLEFTLO	3
#define _DELXRIGHTLO	2

#define _XRIGHT_LO	3
#define _XRIGHT_HI	5
#define _XLEFT_LO	2
#define _XLEFT_HI	4

#define _YVALUE		9
#define _CURRENTMODE	9

#define _LEFTCOUNT	12
#define _RIGHTCOUNT	13

#define _LEFT		6	/* NOTE: this must be the same as BOTTOMPNT */
#define _RIGHT		0

#define _MIN_MASK	7
#define _MAX_MASK	1
#define _I		8
#define _LOC_TEMP	8

#define _COLOR_LEFT_HI	14
#define _COLOR_LEFT_LO	10

#define _COLOR_RIGHT_HI	15
#define _COLOR_RIGHT_LO	11

#define _Z_LEFT_HI	14
#define _Z_LEFT_LO	10

#define _Z_RIGHT_HI	15
#define _Z_RIGHT_LO	11

#define _DEL_HI 	7
#define _DEL_LO 	1

#define _Z_LINE_HI 	7
#define _Z_LINE_LO 	1

#define _COLOR_LINE_HI 	12
#define _COLOR_LINE_LO 	13

#define _Z_LEFT_LO_ZS	13
#define _Z_LEFT_HI_ZS	6
#define _Z_RIGHT_LO_ZS	0
#define _Z_RIGHT_HI_ZS  12
