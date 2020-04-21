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

/**************************** D I S P L A Y . C ***************************
*
*  MODULE DESCRIPTION:
*
*	This module contains the display routines for the 3279 emulator
*
*
*  ENTRY POINTS:
*
*	asc_status()	  - Return pointer to current 3278 ASCII status
*
*	ebx_status()	  - Return pointer to current 3278 EBCDIC status
*
*	getspot()         - Return a char from the screen buf at a given
*			    cursor address
*
*	inkindex()        - Increment with wrap the given cursor address
*
*	inkspot()         - Store a char into the screen buf at a given
*			    cursor address and return the incremented
*			    cursor address
*
*	repaint()         - Mark a range of screen rows for redisplay
*
*	setspot()         - Store a char into the screen buf at a given
*			    cursor address
*
*	show_hex_screen() - Hex dump of screen buffer
*
*	show_screen()     - Screen display routine
*
*	show_status()     - Status display routine
*
*
************************************************************************/

#include <sys/types.h>
#include "term.h"
#include "pxw.h"

#define D_ATTRB	0xc0		/* default attribute - dim */
#define STAT_L  0x30		/* length of status line string on screen */

#define isattrb(a)	((((a) & 0xc0))==0xc0)
#define sameattrb(a,b)	(!((a) & 0xec ^ (b) & 0xec))
#define	rowinc(a,b)	((a)+1 == (b) ? 1 : (a) + 1)

/*
**	Externals
*/
extern px_status outb;
extern u_char	pxd_debug;
extern u_short	Cursor_addr;
extern u_char	Display_xlat[];
extern u_char	Ebx_xlat[];
extern u_char	Ebx_x8[];
extern char	Rows;


/*
**	Local variables
*/
u_char buf[COLS+2];
u_char row_attrbs[MAX_ROWS] = {
		D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB,
		D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB,
		D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB,
		D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB,
		D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB,
		D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB,
		D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB,
		D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB
	};
u_short old_cursor = COLS;
u_char	Ebx_status[37] = { 0 };
u_char	Screen[4097] = { 0 };
u_char	Changed_rows[MAX_ROWS] = { 1 };


/*
**	Turn on/off the tracing of this module
**      Dummy to keep lint happy, no tracing (DT) present
*/
tr_disp(flag)
{
	trace = flag;
}


/*
** asc_status returns pointer to ASCII coded 3278 status
*/
caddr_t
asc_status()
{
	register u_char *bp;
	register u_short index;
	
	bp = Ebx_status;
	for (index = 0; index < 36 ; )
		*bp++ = Display_xlat[Screen[index++]];
	return ((caddr_t)Ebx_status);
}


/*
**	Display a row of dots
*/
dispdots(cursor)
u_short cursor;
{
	register char col;

	(void)messagef("\r\n%04x  \t", cursor);
	for (col = 0; col < 16; col++) {
		if (col == 8)
			(void)messagef(" ");
		(void)messagef(".. ");
	}
}


/*
**	Display a row
*/
u_char
display_row(row, attrb)
char row;
u_char attrb;
{
	register u_char *bp;
	register u_char *sp, *sp_end;

	xy(row, 1);			/* cursor to beginning of row */
	erase_eol();			/* clear row */
	if (row == 1)			/* set beginning of row attribute */
		backgrnd(attrb);
	else
		set_color(attrb);
	sp = Screen + COLS * row;	/* pointer to beg of disp data */
	for (sp_end = sp + COLS; --sp_end > sp && (*sp_end == 0 || *sp_end
	    == 0x10);)
		;			/* don't display trailing blanks */
	sp_end++;
	if (sp_end == Screen + S_CHARS)
		--sp_end;		/* don't display last char on row 24 */
	for (sp = Screen + COLS * row; sp < sp_end;)
		if (isattrb(*sp)) {	/* display attributes */
			disp_attrb(*sp);
			attrb = *sp++;
		} else {		/* buffer chars and display string */
			for (bp = buf; sp < sp_end && !isattrb(*sp);)
				*bp++ = Display_xlat[*sp++];
			strnout(buf, bp - buf);
		}
	return (attrb);
}


/*
** Getspot returns the character at the terminal buffer's indexed spot.
*/
u_char
getspot(index)
u_short index;
{
	return (Screen[index]);
}


/*
**	Display in hex a row of 16 numbers
*/
disprow(cursor)
u_short cursor;
{
	register char col;

	(void)messagef("\r\n%04x  \t", cursor);
	for (col = 0; col < 16; col++) {
		if (col == 8)
			(void)messagef(" ");
		(void)messagef("%02x ", getspot(cursor++));
	}
}


/*
** ebx_status returns pointer to EBCDIC coded 3278 status
*/
caddr_t
ebx_status()
{
	register u_char *bp;
	register u_short index;
	
	bp = Ebx_status;
	for (index = 0; index < 36 ; )
		*bp++ = Ebx_x8[Screen[index++]];
	return ((caddr_t)Ebx_status);
}


/*
** Inkindex increments and returns the index.
*/
u_short
inkindex(index)
u_short index;
{
	if (++index >= S_CHARS)
		index = COLS;
	return(index);
}


/*
** Inkspot stores the character and attribute at the indexed spot,
** then increments and returns the index.
*/
u_short
inkspot(index,spot)
u_short index;
u_char spot;
{
	Screen[index] = spot;
	if (++index >= S_CHARS)
		index = COLS;
	return(index);
}


/*
** Inkwbspot stores the character and attribute at the indexed spot,
** then increments and returns the index.
*/
u_short
inkwbspot(index,spot)
u_short index;
u_char spot;
{
	Screen[index] = spot;
	if (++index >= PXBUFSIZ)
		index = COLS;
	return(index);
}


/*
**	Mark a range of screen rows for redisplay
*/
repaint(row1, row2)
{
	register char r;

	if (row1 >= Rows)
		row1 = Rows;
	if (row2 >= Rows)
		row2 = Rows;
	for (r = row1; r != row2; r = r+1==Rows ? 0 : r+1)
		Changed_rows[r] = 1;
	Changed_rows[r] = 1;
}


/*
**	Return TRUE if next 16 characters at given positon are zero
*/
rowisnull(cursor)
u_short cursor;
{
	register char col;

	for (col = 0; col < 16; ++col)
		if (getspot(cursor++) != 0)
			return 0;
	return 1;
}


/*
** Setspot stores the character at the indexed spot.
*/
setspot(index,spot)
u_short index;
u_char spot;
{
	 Screen[index] = spot;
}


/*
**	Display the screen buffer on the terminal in hex
*/
show_hex_screen()
{
	register u_short cursor;	/* index into display buffer */
	register char zdisp;		/* 0 if row of zeros need displaying,
					   1 if dots should be displayed,
					   2 if zeroes should be skipped */

	for (cursor = zdisp = 0; cursor < S_CHARS; cursor += 16) {
		if (rowisnull(cursor)) {
			switch (zdisp)
			{
			case 0:
				disprow(cursor);
				zdisp = 1;
				break;
			case 1:
				dispdots(cursor);
				zdisp = 2;
				break;
			default:
				break;
			}
		} else {
			disprow(cursor);
			zdisp = 0;
		}
	}
}


/*
**	Display the screen buffer on the terminal
*/
show_screen()
{

	register u_char attrb;
	register u_char *sp;
	register char r;

	if (context == GRAPHICS)
		return;
	if (old_cursor >= COLS)
		erase_cursor((int)old_cursor / COLS, ((int)old_cursor % COLS) + 1);
	Changed_rows[old_cursor / COLS] = 2;
	attrb = D_ATTRB;
	if (isattrb(*(sp = Screen + COLS))) {
		attrb = *sp;
	} else
		for (sp = Screen + S_CHARS; --sp >= Screen + COLS; )
			if (isattrb(*sp)) {
				attrb = *sp;
				break;
				}
	for (r = 1; r < Rows; ++r) {
		if (!sameattrb(attrb, row_attrbs[r])) {
			row_attrbs[r] = attrb;
			Changed_rows[r] = 2;
		}
		if (Changed_rows[r]) {
			attrb = display_row(r, row_attrbs[r]);
			Changed_rows[r] = 0;
		} else {
			attrb = row_attrbs[r + 1];
		}
	}
	show_status();
	old_cursor = Cursor_addr;
	if (old_cursor >= COLS ) {
		xy((int)Cursor_addr / COLS, (int)(Cursor_addr % COLS) + 1);
		show_cursor(Display_xlat[Screen[Cursor_addr]]);
	}
}


/*
**	Display the status line
*/
show_status()
{
	register u_char c;
	register u_short cursor, curmax;	/* index into display buffer */
	register u_char *bp;

	if (context == GRAPHICS)
		return;
	bp = buf;
	if (start_status() == 0)	/* load user line */
		return;
	if (outb.ft)
		curmax = 0x50;
	else
		curmax = STAT_L;
	for (cursor = 0; cursor < curmax; ) {
		c = getspot(cursor++);	/* get char from buffer */
		c = Display_xlat[c];	/* translate it */
		*bp++ = c;		/* store it in buffer */
	}
	*bp = '\0';
	strnout (buf, curmax);
	end_status();
}


show_str(str)
u_char *str;
{

	register short index, length=COLS;

	for (index = length = COLS; length--; ) {
		Screen[index++] = Ebx_xlat[*str++ - 0x40];
		if (!*str)
			break;
	}
	repaint(1,1);
}

