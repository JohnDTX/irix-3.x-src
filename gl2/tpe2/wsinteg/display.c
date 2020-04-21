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

/************************ D I S P L A Y . C ******************************
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

#define D_ATTRB		(u_char)0xc0		/* default attribute - dim */
#define STAT_L  	0x30	/* length of status line string on screen */

#define isattrb(a)	((((a) & (u_char)0xc0))==(u_char)0xc0)
#define sameattrb(a,b)	(!((a) & (u_char)0xec ^ (b) & (u_char)0xec))
#define	rowinc(a,b)	((a)+1 == (b) ? 1 : (a) + 1)

/*
**	Externals
*/
extern u_short		Cursor_addr;
extern u_char		Display;
extern u_char		Display_xlat[];
extern u_char		Ebx_x8[];
extern u_char		Msg_proc;
extern u_char		Rows;

/*
**	Global functions and variables
*/
u_char			getspot();
u_short			inkindex();
u_char			Force_show_screen = 0;

/*
**	Local variables
*/
u_char			buf[COLS+2];
u_char			Ebx_status[37] = { 0 };
u_char			Screen[PXBUFSIZ + 1] = { 0 };
u_char			Changed_rows[MAX_ROWS] = { 1 };


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
	for (index = 0; index < (sizeof(Ebx_status)-1) ; )
		*bp++ = Display_xlat[Screen[index++]];
	return ((caddr_t)Ebx_status);
}


/*
**	Display a row of dots
*/
dispdots(cursor)
u_short cursor;
{
	register u_char col;

	(void)printf("\n%04x  \t", cursor);
	for (col = 0; col < 16; col++) {
		if (col == 8)
			(void)printf(" ");
		(void)printf(".. ");
	}
}


/*
**	Display a row
*/
u_char
display_row(row, attrb)
u_char attrb, row;
{
	u_char *bp;
	u_char *sp, *sp_end;

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
	if (*sp_end == 0 || *sp_end == 0x10)
		return(attrb);
	sp_end++;
	if (Display) {
		if (sp_end == Screen + S_CHARS)
			--sp_end;	/* don't display last char on row 24 */
	}
	for (sp = Screen + COLS * row; sp < sp_end;)
		if (isattrb(*sp)) {	/* display attributes */
			disp_attrb(*sp);
			attrb = *sp++;
		} else {		/* buffer chars and display string */
			if (!Display || ((attrb & 0x0c) != 0x0c)) {
				for (bp = buf; sp < sp_end && !isattrb(*sp);)
					*bp++ = (char)Display_xlat[*sp++];
				strnout(buf, bp - buf);
			} else {
				for (bp = buf; sp < sp_end && !isattrb(*sp);) {
					*bp = (char)Display_xlat[*sp++];
					*bp++ = ' ';
				}
				strnout(buf, bp - buf);
			}
#ifdef DEBUG
			if (trace) {
				*bp++ = 0x0a;
				*bp++ = 0;/* create null terminated log line */
				trace_msg(buf);
			}
#endif /* DEBUG */
		}
	return (attrb);
}


/*
**	Display in hex a row of 16 numbers
*/
disprow(cursor)
u_short cursor;
{
	register char col;
	register u_char c;

	(void)printf("\r\n%04x  \t", cursor);
	for (col = 1; col < 17; col++) {
		c = getspot(cursor++);
		buf[col] = Display_xlat[c];
		printf("%02x-",c);
		if (col == 8)
			(void)printf(" ");
	}
	printf("  ");
	for (col = 1; col < 17; col++) {
		if (buf[col] >= ' ')
			printf("%c",buf[col]);
		else
			printf(".");
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
	for (index = 0; index < (sizeof(Ebx_status)-1) ; )
		*bp++ = Ebx_x8[Screen[index++]];
	return ((caddr_t)Ebx_status);
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
char row1, row2;
{
	register char r;

	if (row1 >= Rows)
		row1 = Rows - 1;
	if (row2 >= Rows)
		row2 = Rows - 1;
	for (r = row1; r != row2; r = r+1>=Rows ? 0 : r+1)
		Changed_rows[r] = CNTU;
	Changed_rows[r] = CNTU;
}


/*
**	Return TRUE if next 16 characters at given positon are zero
*/
rowisnull(cursor)
u_short cursor;
{
	register u_char col;

	for (col = 0; col < 16; ++col)
		if (getspot(cursor++) != 0)
			return 0;
	return CNTU;
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
	register u_char zdisp;		/* 0 if row of zeros need displaying,
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
	static u_char row_attrbs[MAX_ROWS] = {
		D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB,
		D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB,
		D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB,
		D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB,
		D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB,
		D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB,
		D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB,
		D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB, D_ATTRB
	};
	static u_short old_cursor = COLS;

	u_char attrb, display_row();
	u_char *sp;
	register u_char r;

	if (context == GRAPHICS)
		return;
	if (Msg_proc > MXFER)
		if (!Force_show_screen)
			return;
	if (old_cursor >= COLS)
		erase_cursor((int)old_cursor / COLS, ((int)old_cursor % COLS) + 1);
	Changed_rows[old_cursor / COLS] = 2;
	attrb = D_ATTRB;
	if (isattrb(*(sp = Screen + COLS)) && Display == 0) {
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
	if (Display == 0 || getspot(8) == 0) {
		if (old_cursor >= COLS ) {
			xy((char)(Cursor_addr / COLS), (char)(Cursor_addr % COLS) + 1);
			show_cursor(Display_xlat[Screen[Cursor_addr]]);
		}
	}

}


/*
**	Display the status line
*/
show_status()
{
	register u_char c;
	register u_short cursor;	/* index into display buffer */
	u_char *bp;

	if (context == GRAPHICS)
		return;
	if (Display == 0) {
		bp = buf;
		start_status();
		for (cursor = 0; cursor < STAT_L; ) {
			c = getspot(cursor++);	/* get char from buffer */
			c = Display_xlat[c];	/* translate it */
			*bp++ = (char)c;	/* store it in buffer */
		}
		*bp = '\0';
		strnout (buf, STAT_L);
		end_status();
	}
}
