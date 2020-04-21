#ifndef CHARSDEF
#define CHARSDEF
/**************************************************************************
 *									  *
 * 		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/
	/* ANSI standard mnemonics for control characters */

#define EOT	4	/* control D - Unix end of file */
#define BELL 	7	/* control G - Bell */
#define BS	8	/* control H - Backspace */
#define TAB	9	/* control I - Horizontal tab */
#define LF	10	/* control J - Line feed */
#define VT	11	/* control K - Vertical Tab */
#define FF	12	/* control L - Form Feed */
#define CR	13	/* control M - Return */
#define XON 	17  	/* control Q */
#define XOFF	19  	/* control S */
#define DC4	20	/* control T */
#define NAK	21	/* control U */
#define SYN	22	/* control V */
#define ETB	23	/* control W */
#define CAN	24	/* control X */
#define ESC	27	/* escape */
#define DEL	127	/* delete or rubout */
#define TELNET_ESCAPE 036

	/* special character codes */

#define GREYCHAR   TAB		/* grey font character */
#define TOPCHAR    LF		/* top of window character */
				/* alternate is 31 */
#define BOTCHAR    LF		/* bottom of window character */
				/* alternate is 32 */
#define LEFTCHAR   VT		/* left window character */
#define RIGHTCHAR  FF		/* right window character */
#define STRIPECHAR CR		/* the stripes in the header */
#define BLACKCHAR  0177		/* the black character */

#define STANDOUT	0200	/* standout bit */

#endif CHARSDEF
