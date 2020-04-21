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
#define _XTHIS		0
#define _YTHIS		1
#define _ZTHIS		2

/*	_TEMP		7	defined in consts.h	*/

#define _BIASED_DONE	8

#define _DEL_XLEFT_HI
#define _DEL_XLEFT_LO	
#define _DEL_XRIGHT_HI
#define _DEL_XRIGHT_LO
#define _XRIGHT_LO	3	
#define _XRIGHT_HI	5
#define _XLEFT_LO	2
#define _XLEFT_HI	4

#define _YVALUE		14

#define _LEFTCOUNT	12
#define _RIGHTCOUNT	13

#define _LEFT		6	/* NOTE: this must be the same as BOTTOMPNT */
#define _RIGHT		0

#define _MIN_MASK
#define _MAX_MASK	

#define _COLOR_LEFT_HI	7
#define _COLOR_LEFT_LO  9

#define _COLOR_RIGHT_HI	10
#define _COLOR_RIGHT_LO	11

/* these 4 registers (2&2) may be shared? */
#define _DEL_COLOR_HI 	
#define _DEL_COLOR_LO 		

#define _DEL_X_HI
#define _DEL_X_LO

#define _YCOUNT		
#define _I			
