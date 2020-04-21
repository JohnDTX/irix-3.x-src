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
/*
 * Terminal emulation portion of the port manager and textport code
 *
 *
 * This code emulates a terminal which looks alot like a vt52/vt100/visual
 * 50/alot of other terminals...
 *
 * Its termcap definition is as follows:
 *	S0|iris|iris emulating a visual 50 (approximately):\
 *		:am:al=\EL:is=\E7B0\E7F7\E7C5\E7R2:\
 *		:bs:cd=\EJ:ce=\EK:cl=\EH\EJ:cm=\EY%+ %+ :co#80:li#48:nd=\EC:\
 *		:pt:sr=\EI:up=\EA:ku=\EA:kd=\EB:kr=\EC:kl=\ED:\
 *		:cl=\Ev:ho=\EH:dl=\EM:so=\E9P:se=\E0@:
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/gsh/RCS/textport.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:46:47 $
 */

#include "sys/types.h"
#include "gl.h"
#include "window.h"
#include "gsh.h"

/*
 * Redraw a textport
 */
tf_redraw(tn, r)
	int tn;
	rect_t *r;
{
	setoutput(r);
	txport[tn].tx_r = *r;

	/* now display textport */
	tx_textport(&txport[tn], 0, r->xlen, 0, r->ylen);
	tx_repaint();
}

/*
 * Setup new cursor color.  Turn off blinking, if it was on, then rebind
 * the color and turn the blinking back on.
 */
setcursorcolor(tx, c)
	struct txport *tx;
	char c;
{
	if (tx->tx_state & TX_BLINKING)
		blink(0, tx->tx_cursorcolor, 0, 0, 0);
	getmcolor(c, &tx->tx_cursor_r, &tx->tx_cursor_g, &tx->tx_cursor_b);
	mapcolor(tx->tx_cursorcolor, tx->tx_cursor_r, tx->tx_cursor_g,
				     tx->tx_cursor_b);
	tx_setblink(tx);
}

/*
 * tx_sendscreensize:
 *	- send the shell the size of the screen
 */
tx_sendscreensize(tx)
	register struct txport *tx;
{
	char buf[8];

	buf[0] = '\033';
	buf[1] = ',';
	buf[2] = ' ' + (tx->tx_cols & 0x3f);
	buf[3] = ' ' + ((tx->tx_cols >> 6) & 0x3f);
	buf[4] = ' ' + (tx->tx_rows & 0x3f);
	buf[5] = ' ' + ((tx->tx_rows >> 6) & 0x3f);
	buf[6] = '\r';
	send_shell(buf, 7);
}

/*
 * tx_sendbinding:
 *	- send the value of a function keys binding
 */
tx_sendbinding(tx, key)
	register struct txport *tx;
	char key;
{
	char buf[MAXCOLS + 10];
	register int len;
	register int i;
	char used[128];

	if (len = kb_getbinding(key, &buf[4])) {
		buf[0] = '\033';
		buf[1] = 'k';
		buf[2] = key;
		/*
		 * Make a quick tag in the used map of all the ascii characters
		 * that the key binding uses.  Then pick a code that isn't
		 * being used for the terminator character to send.
		 */
		bzero(used, sizeof(used));
		used['\n'] = 1;
		used['\r'] = 1;
		for (i = 0; i < len; i++)
			used[buf[i+4]] = 1;
		for (i = 1; i < sizeof(used); i++) {
			if (used[i] == 0) {
				buf[3] = i;
				buf[4 + len] = i;
				send_shell(buf, len + 5);
				break;
			}
		}
		/* can't fail, right? */
	}
}

/*
 * tx_open:
 *	- open up a text port
 */
tx_open(txnum, doblink)
	short txnum;
	int doblink;
{
	tx_init(txnum);
	txport[txnum].tx_state = TX_NORMAL | TX_SCROLLED | TX_OPEN | TX_ON |
				 TX_GOING;
	if (doblink)
		txport[txnum].tx_state |= TX_BLINKING;
}

/*
 * tx_close:
 *	- zap a text port so that no more updates will occur
 *	- called by OS when no more refrences to a textport exist
 */
tx_close(txnum)
	short txnum;
{
	txport[txnum].tx_state = 0;
}

/*
 * tx_init:
 *	- init a textport, assuming system defaults
 *	- the system uses borders, and thus the default piece xmin and
 *	  ymin don't directly correspond to the llx and lly!
 */
tx_init(tn)
	short tn;
{
	register struct txport *tx = &txport[tn];
	register struct row *rp;
	register short i;
	extern char *malloc();

    /* setup text port parameters */
	tx->tx_keypadmode = 0;
	tx->tx_state = TX_BUSY | TX_ON | TX_GOING;
	tx_initcolors(tx);
	tx->tx_oldrow = tx->tx_oldcol = tx->tx_row = tx->tx_col = 0;

    /* setup display data structure */
	rp = &tx->tx_display[0];
	for (i = 0; i < MAXROWS; i++) {
		rp->r_changed = 0;
		rp->r_maxcol = 0;
		rp->r_data = malloc(MAXCOLS+1);
		rp++;
	}
	tx_textport(tx, 0, XMAXSCREEN, 0, YMAXSCREEN);
	tx->tx_state = 0;
}

/*
 * tx_setsize:
 *	- set the size of the textport, and the orgin
 *	- notice, no checking of cols and rows.
 */
tx_setsize(tx, cols, rows, xorg, yorg)
	register struct txport *tx;
	short cols, rows, xorg, yorg;
{
	register struct row *rp;
	register short i;

	if (!(tx->tx_state & TX_ON))			/* ICK */
		return;

	tx->tx_cols = cols;
	tx->tx_rows = rows;
	/* XXX */
	tx->tx_lines = rows;
	tx->tx_firstline = 0;
	tx->tx_lastline = rows - 1;

    /* clear rows of text that are invisible */
	rp = &tx->tx_display[rows];
	for (i = rows; i < MAXROWS; i++, rp++) {
		rp->r_changed = 0;
		rp->r_maxcol = 0;
	}
	tx_fixcursor(tx);
}

/*
 * tx_textport:
 *	- set the size and position of the textport.
 */
tx_textport(tx, x1, x2, y1 ,y2)
	register struct txport *tx;
	short x1, x2, y1, y2;
{
	register short i;
	register short width, height;	
	register short cols, rows;	
	register short llx, lly;
	short nscroll;

	if (!(tx->tx_state & TX_ON))			/* ICK */
		return;

/* XXX fix this to order x2,x1 and y2,y1 */
	width = x2 - x1 + 1;
	if (width<0) 
	    width = -width;

	height = y2 - y1 + 1;
	if (height<0) 
	    height = -height;

	cols = width / font_width;
	if(cols>MAXCOLS) 
	    cols = MAXCOLS;
	else if(cols<1) {
	    cols = 1;
	    width = font_width;
	}

	rows = height / font_height;
	if(rows>MAXROWS) 
	    rows = MAXROWS;
	else if(rows<1) {
	    rows = 1;
	    height = font_height;
	}
	llx = x1 + (width - cols*font_width)/2;
	lly = y1 + (height - rows*font_height)/2;

    /* scroll text up when the port is made smaller */
	tx->tx_state |= TX_BUSY;
	nscroll = (tx->tx_rows-rows) - (tx->tx_rows-1-tx->tx_row);
	for (i = 0; i < nscroll; i++) {
	    tx_deleteln(tx,0);
	    tx->tx_row--;
	}
	tx_setsize(tx,cols,rows,llx,lly);
	tx->tx_state &= ~TX_BUSY; 
}

/*
*/
tx_repaint()
{
	txport[0].tx_state |= TX_SCROLLED;
	tx_update();
}

/*
 * tx_update:
 *	- update display, given new data
 */
tx_update()
{
	register struct txport *tx = &txport[0];
	register struct row *rp;
	struct row *lastrp;
	register short i, scrolled;
	register short xsize, ysize;

	setoutput(&tx->tx_r);
	scrolled = tx->tx_state & TX_SCROLLED;

	/* redisplay the text */
	xsize = XLEN(tx->tx_cols) - 1;
	lastrp = &tx->tx_display[(short) tx->tx_rows];
	for (rp = &tx->tx_display[0]; rp < lastrp; rp++) {
		if ((rp->r_changed == 0) && (scrolled == 0))
		    	continue;
		color(tx->tx_pagecolor);
		rectfs(0,rp->r_screeny-font_descender,xsize,
			rp->r_screeny+font_height-font_descender- 1);
		if (rp->r_maxcol) {
			cmov2i(0, rp->r_screeny);
			if (rp->r_video) {
				register char *c = &rp->r_data[0];

				for (i=MIN(rp->r_maxcol,tx->tx_cols); i--;) {
				    	if(*c & 0x80) {
						color(tx->tx_reversecolor);
						*c &= 0x7F;   /* hack alert! */
						xcharstr(c, 1);
						*c++ |= 0x80;
				    	} else {
						color(tx->tx_textcolor);
						xcharstr(c++, 1);
				    	}
				}
			} else {
				color(tx->tx_textcolor);
				xcharstr(rp->r_data, 
					 MIN(rp->r_maxcol,tx->tx_cols));
			}
		}
		rp->r_changed = 0;
	}
	tx->tx_oldcol = tx->tx_col;
	tx->tx_oldrow = tx->tx_row;

	/* now draw new cursor up */
	tx_drawcursor(tx);

	tx->tx_state &= ~(TX_BUSY | TX_REDISPLAY | TX_SCROLLED);
}

/*
 * tx_drawcursor:
 *	- display cursor at new location
 */
tx_drawcursor(tx)
	register struct txport *tx;
{
	register struct row *rp;
	register short llx, lly;

	llx = tx->tx_col * font_width;
	lly = (tx->tx_rows - tx->tx_row - 1) * font_height;
	if (!(tx->tx_state & TX_SELECTED)) {
		color(tx->tx_pagecolor);
		rectfs(llx, lly, llx + font_width - 1, lly + font_height - 1);
		color(tx->tx_cursorcolor);
		rects(llx, lly, llx + font_width - 1, lly + font_height - 1);
	} else {
		color(tx->tx_cursorcolor);
		rectfs(llx, lly, llx + font_width - 1, lly + font_height - 1);
	}

	rp = &tx->tx_display[tx->tx_row];
	if (tx->tx_col < rp->r_maxcol) {
		if ((tx->tx_cursor_r != 0) ||
		    (tx->tx_cursor_g != 0) ||
		    (tx->tx_cursor_b != 0)) {
			/*
			 * Cursor is not black, we can go ahead and use
			 * black for drawing the character underneath
			 * the cursor.
			 */
			color(BLACK);
		} else {
			/*
			 * Cursor is black.  Hmmm.  Use the normal character
			 * color, since we can't guess much better.
			 */
			color(tx->tx_textcolor);
		}
		cmov2i(llx, lly + font_descender);
		xcharstr(&rp->r_data[tx->tx_col], 1);
	}
}

/*
 * tx_setblink:
 *	- update blink to reflect textport state
 */
tx_setblink(tx)
	register struct txport *tx;
{
	short r, g, b;

	if (tx->tx_state & TX_BLINKING) {
		if (tx->tx_state & TX_SELECTED) {
			getmcolor(tx->tx_pagecolor, &r, &g, &b);
			blink(0, tx->tx_cursorcolor, 0, 0, 0);
			blink(VRETRACE / flag_blink, tx->tx_cursorcolor,
				       r, g, b);
		} else
			blink(0, tx->tx_cursorcolor, 0, 0, 0);
	}
}

/*
 * tx_fixcursor:
 *	- put the cursor at a good spot on the screen, after a textport
 *	  has changed size
 *	- adjust screen y positions
 */
tx_fixcursor(tx)
	register struct txport *tx;
{
	register struct row *rp;
	register short i;

	if (tx->tx_row<0)
		tx->tx_oldrow = tx->tx_row = 0;
	else if (tx->tx_row >= tx->tx_rows) 
		tx->tx_oldrow = tx->tx_row = tx->tx_rows - 1;
	if (tx->tx_col<0)
		tx->tx_oldcol = tx->tx_col = 0;
	else if (tx->tx_col >= tx->tx_cols)
		tx->tx_oldcol = tx->tx_col = tx->tx_cols - 1;
	rp = &tx->tx_display[0];
	for (i = 0; i < tx->tx_rows; i++) {
		rp->r_screeny = font_height * (tx->tx_rows-i-1) 
							+ font_descender;
		rp++;
	}
}

tx_initcolors(tx)
	register struct txport *tx;
{
	register struct txport *consoletx = &txport[0];
	extern int force_colors, c1, c2, c3, c4;

	tx->tx_curcolor = 0;
	tx->tx_textwritemask = 0xfff;
	tx->tx_pagewritemask = 0xfff;
	if (force_colors) {
		tx->tx_textcolor = c1;
		tx->tx_pagecolor = c2;
		tx->tx_reversecolor = c3;
	} else {
		tx->tx_textcolor = C_FOREGROUND;
		tx->tx_pagecolor = C_BACKGROUND;
		tx->tx_reversecolor = C_REVERSE;
	}
	tx->tx_cursorcolor = C_CURSOR;
	tx->tx_state |= (TX_REDISPLAY | TX_SCROLLED);

	/* init cursor color mapping */
	blink(0, tx->tx_cursorcolor, 0, 0, 0);
	mapcolor(tx->tx_cursorcolor, 0, 255, 0);
	tx->tx_cursor_r = 0;
	tx->tx_cursor_g = 255;
	tx->tx_cursor_b = 0;
}
