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
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/textport.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:46:08 $
 */

#include "stdio.h"
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
	tx_repaint(tn);
}

/*
 * tx_addchars:
 *	- add some characters to the display
 * TODO	- add in ic, dc, im, etc
 *	- add in settable tabs
 *	- add in reset command
 *	- add in outline color change command and appropriate reset
 *	- add in real reverse video
 *	- add in cursor blink color
 */
tx_addchars(tn, cp, n)
	int tn;
	register char *cp;
	register int n;
{
	register struct txport *tx = &txport[tn];
	register struct row *rp;
	register char c;
	register short newstate;
	register short maxcols, maxrows;
	extern char need_redisplay;

	maxcols = tx->tx_cols;
	maxrows = tx->tx_rows;
	tx->tx_state |= TX_REDISPLAY;
	rp = &tx->tx_display[tx->tx_row];
	while (n--) {
		c = *cp++ & 0x7f;
		switch (tx->tx_state & TX_STATEBITS) {
		  case TX_NORMAL:
			if (c >= ' ') {
				rp->r_changed = 1;
				if (tx->tx_curcolor) {
				    rp->r_video = 1;
				    rp->r_data[tx->tx_col] = c | 0x80;
				} else
				    rp->r_data[tx->tx_col] = c;

			    /* move to new col and adjust maxcol  
		   	       			in case of overstrike */
				if (++(tx->tx_col) > rp->r_maxcol)
					rp->r_maxcol = tx->tx_col;

			    /* handle automatic margins */
				if (tx->tx_col == maxcols) {
					tx->tx_col = 0;
					tx_downln(tx, 1);
					rp = &tx->tx_display[tx->tx_row];
				}
			} else {
			    switch (c) {
			      case '\033':			/* escape */
				tx->tx_state |= TX_ESCAPE;
				break;

			      case '\b':			/* backspace */
				if (tx->tx_col) {
					rp->r_changed = 1;
					tx->tx_col--;
					tx_adjust(tx);
				}
				break;
			      case '\007':			/* bell */
				ringbell();
				break;

			      case '\t':			/* tab */
				rp->r_changed = 1;
				tx->tx_col += (8-(tx->tx_col & 7));
				if (tx->tx_col >= maxcols) {
					tx->tx_col = 0;
					tx_downln(tx, 1);
				}
				tx_adjust(tx);
				break;

			      case '\n':			/* new line */
				rp->r_changed = 1;
				tx_downln(tx, 1);
				break;

			      case '\r':			/* return */
				rp->r_changed = 1;
				tx->tx_col = 0;
				break;
			  }
			  rp = &tx->tx_display[tx->tx_row];
			}
			break;

		  case TX_ESCAPE:
			tx->tx_state &= ~TX_ESCAPE;
			newstate = TX_NORMAL;
			switch (c) {
			  case '\033':		/* escape escape; ignore */
				break;
			  case '0':		/* normal video */
				newstate = TX_COLLECT;
				tx->tx_cmd = TX_READ0;
				tx->tx_count = 1;
				break;
			  case '7':		/* set reverse video color */
				newstate = TX_COLLECT;
				tx->tx_cmd = TX_READ7;
				tx->tx_count = 2;
				break;
			  case '9':		/* reverse video */
				newstate = TX_COLLECT;
				tx->tx_cmd = TX_READ9;
				tx->tx_count = 1;
				break;
			  case ',':		/* screen size; ignore */
				newstate = TX_COLLECT;
				tx->tx_cmd = TX_READCOMMA;
				tx->tx_count = 5;
				break;
			  case 'T':
				newstate = TX_COLLECT;
				tx->tx_cmd = TX_READTITLE;
				tx->tx_count = 0;
				break;
			  case ';':		/* set keypad mode */
			  case '=':		/* set keypad mode */
				tx->tx_keypadmode = 2;
				break;
			  case '>':		/* reset keypad mode */
				tx->tx_keypadmode = 0;
				break;
			  case 'A':		/* move cursor up */
				rp->r_changed = 1;
				tx_upln(tx, 0);
				break;
			  case 'B':		/* move cursor down */
				rp->r_changed = 1;
				tx_downln(tx, 0);
				break;
			  case 'C':		/* move cursor right */
				if (tx->tx_col < (maxcols - 1)) {
					rp->r_changed = 1;
					(tx->tx_col)++;
					tx_adjust(tx);
				}
				break;
			  case 'D':		/* move cursor left */
				if (tx->tx_col) {
					rp->r_changed = 1;
					tx->tx_col--;
					tx_adjust(tx);
				}
				break;
			  case 'H':		/* home cursor */
				rp->r_changed = 1;
				tx->tx_row = tx->tx_col = 0;
				tx_adjust(tx);
				break;
			  case 'I':		/* reverse index */
				rp->r_changed = 1;
				tx_upln(tx, 1);
				break;
			  case 'v':		/* clear entire display */
				tx->tx_row = tx->tx_col = 0;
				/* FALL THROUGH */
			  case 'J':		/* clear to end of page */
				for (rp = &tx->tx_display[tx->tx_row];
					rp < &tx->tx_display[maxrows]; rp++) {
					rp->r_maxcol = 0;
					rp->r_video = 0;
					rp->r_changed = 1;
				}
				break;
			  case 'K':		/* clear to eol */
				rp = &tx->tx_display[tx->tx_row];
				rp->r_maxcol = tx->tx_col;
				rp->r_changed = 1;
				break;
			  case 'L':		/* add line */
				tx_insertln(tx, tx->tx_row);
				break;
			  case 'M':		/* delete line */
				tx_deleteln(tx, tx->tx_row);
				break;
			  case 'Y':		/* move cursor */
				newstate = TX_COLLECT;
				tx->tx_cmd = TX_READY;
				tx->tx_count = 2;
				break;
			  case '.':		/* return scr size */
				tx_sendscreensize(tx);
				break;
			  case 'k':		/* bind key */
				newstate = TX_COLLECT;
				tx->tx_cmd = TX_READBINDING;
				tx->tx_count = 0;
				break;
			  case 'r':		/* report key binding */
				newstate = TX_COLLECT;
				tx->tx_cmd = TX_READKEY;
				tx->tx_count = 1;
				break;
			  default:		/* ignore garbage */
				break;
			}
			tx->tx_state |= newstate;
		        rp = &tx->tx_display[tx->tx_row];
			break;

		  case TX_COLLECT:
			if ((tx->tx_cmd == TX_READTITLE) ||
			    (tx->tx_cmd == TX_READBINDING)) {
				/*
				 * See if a terminator has been read.  For
				 * titles, the terminator will be in
				 * tx_readbuf[0]; for key bindings,
				 * tx_readbuf[0] contains the key to bind and
				 * tx_readbuf[1] contains the terminator.
				 */
				if ((tx->tx_cmd == TX_READTITLE) &&
				    tx->tx_count && (c == tx->tx_readbuf[0])) {
					tx->tx_readbuf[tx->tx_count] = 0;
					strcpy(title, &tx->tx_readbuf[1]);
					wintitle(title);
				} else
				if ((tx->tx_cmd == TX_READBINDING) &&
				    (tx->tx_count > 1) &&
				    (c == tx->tx_readbuf[1])) {
					tx->tx_readbuf[tx->tx_count] = 0;
					kb_bindkey(tx->tx_readbuf);
				} else
				if (tx->tx_count < 80) {
					tx->tx_readbuf[(tx->tx_count)++] = c;
					break;
				}
			} else {
				tx->tx_readbuf[--tx->tx_count] = c;
				if (tx->tx_count)
					break;
			}
			switch (tx->tx_cmd) {
			  case TX_READ0:
				tx->tx_curcolor = 0;	/* no reverse */
				break;
			  case TX_READ9:
				tx->tx_curcolor = 1;	/* reverse */
				break;
			  case TX_READ7:
				c = (tx->tx_readbuf[0] - '0') & 0xff;
				switch (tx->tx_readbuf[1]) {
				  case 'F':
				  case 'f':
					tx->tx_textcolor = c;
					bar_color(tx->tx_pagecolor,
						  tx->tx_textcolor);
					break;
				  case 'B':
				  case 'b':
					tx->tx_pagecolor = c;
					need_redisplay = 1;
					bar_color(tx->tx_pagecolor,
						  tx->tx_textcolor);
					tx_setblink(tx);
					break;
				  case 'R':
				  case 'r':
					tx->tx_reversecolor = c;
					break;
				  case 'C':
				  case 'c':
					setcursorcolor(tx, c);
					break;
				}
			        tx->tx_state |= 
				  (TX_REDISPLAY | TX_SCROLLED);
				tx->tx_curcolor = 0;
				break;
			  case TX_READY:
				c = tx->tx_readbuf[1];
				if ((c < 32) || (c >= (32 + maxrows)))
					break;
				tx->tx_row = c - 32;
				c = tx->tx_readbuf[0];
				if ((c < 32) || (c >= (32 + maxcols)))
					break;
				tx->tx_col = c - 32;
				rp->r_changed = 1;
				tx_adjust(tx);
				break;
			  case TX_READCOMMA:
				/*
				 * Swallow a screen sizing escape sequence.
				 */
				break;
			  case TX_READKEY:
				tx_sendbinding(tx, tx->tx_readbuf[0]);
				break;
			}
			tx->tx_state &= ~TX_STATEBITS;
			tx->tx_state |= TX_NORMAL;
			rp = &tx->tx_display[tx->tx_row];
			break;
		}
	}
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
 * tx_upln:
 *	- move cursor up one line, scrolling if needed (and allowed)
 */
tx_upln(tx, scroll)
	register struct txport *tx;
	short scroll;
{
	if (tx->tx_row == 0) {
		if (scroll) {
			tx_insertln(tx, 0);
			if(tx->tx_col != 0)
				tx_adjust(tx);
			tx->tx_state |= TX_SCROLLED;
		}
	} else {
		tx->tx_row--;
		tx_adjust(tx);
	}
}

/*
 * tx_downln:
 *	- move the cursor down one line, scrolling if needed (and allowed)
 */
tx_downln(tx, scroll)
	register struct txport *tx;
	short scroll;
{
	if (tx->tx_row == (tx->tx_rows - 1)) {
		if (scroll) {
			tx_deleteln(tx, 0);
			if(tx->tx_col != 0)
				tx_adjust(tx);
			tx->tx_state |= TX_SCROLLED;
		}
	} else {
		tx->tx_row++;
		tx_adjust(tx);
	}
}

/*
 * tx_adjust:
 *	- adjust display info, if we moved cursor into a line which previously
 *	  contained some data
 */
tx_adjust(tx)
	register struct txport *tx;
{
	register struct row *rp;

	rp = &tx->tx_display[tx->tx_row];
	if (tx->tx_col > rp->r_maxcol) {
		while (tx->tx_col > rp->r_maxcol)
			rp->r_data[rp->r_maxcol++] = ' ';
		rp->r_changed = 1;
	}
}

/*
 * tx_insertln:
 *	- insert "line" in the screen moving the current and all lower
 *	  lines down one
 */
tx_insertln(tx, line)
	register struct txport *tx;
	short line;
{
	register struct row *rp, *linerp;
	register char *savedata;

	rp = &tx->tx_display[(short) (tx->tx_rows-1)];
	savedata = rp->r_data;
	linerp = &tx->tx_display[line];
	for (; rp > linerp; rp--) {
		rp->r_data = (rp - 1)->r_data;
		rp->r_maxcol = (rp - 1)->r_maxcol;
		rp->r_video = (rp - 1)->r_video;
		rp->r_changed = 1;
	}
	linerp->r_data = savedata;
	linerp->r_video = 0;
	linerp->r_maxcol = 0;
	linerp->r_changed = 1;
}

/*
 * tx_deleteln:
 *	- delete "line" from the screen moving all lower lines up one
 */
tx_deleteln(tx, line)
	register struct txport *tx;
	short line;
{
	register struct row *rp, *rowsrp;
	register char *savedata;

	rp = &tx->tx_display[line];
	savedata = rp->r_data;
	rowsrp = &tx->tx_display[(short) (tx->tx_rows-1)];
	for (; rp < rowsrp; rp++) {
		rp->r_data = (rp + 1)->r_data;
		rp->r_maxcol = (rp + 1)->r_maxcol;
		rp->r_video = (rp + 1)->r_video;
		rp->r_changed = 1;
	}
	rowsrp->r_data = savedata;
	rowsrp->r_video = 0;
	rowsrp->r_maxcol = 0;
	rowsrp->r_changed = 1;
}

/*
 * tx_open:
 *	- open up a text port
 */
tx_open(txnum)
	short txnum;
{
	tx_init(txnum);
	txport[txnum].tx_state = TX_NORMAL | TX_SCROLLED | TX_OPEN | TX_ON |
				 TX_GOING;
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

	if (!(tx->tx_state & TX_ON)) {			/* ICK */
		return;
	}

/* XXX fix this to order x2,x1 and y2,y1 */
	width = x2 - x1 + 1;
	if (width<0) 
	    width = -width;

	height = y2 - y1 + 1;
	if (height<0) 
	    height = -height;

	cols = width / charwidth;
	if(cols>MAXCOLS) 
	    cols = MAXCOLS;
	else if(cols<1) {
	    cols = 1;
	    width = charwidth;
	}

	rows = height / charheight;
	if(rows>MAXROWS) 
	    rows = MAXROWS;
	else if(rows<1) {
	    rows = 1;
	    height = charheight;
	}
	llx = x1 + (width - cols*charwidth)/2;
	lly = y1 + (height - rows*charheight)/2;

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
tx_repaint(tn)
int	tn;
{
	txport[tn].tx_state |= TX_SCROLLED;
	tx_update(tn);
}

/*
 * tx_update:
 *	- update display, given new data
 */
tx_update(tn)
int	tn;
{
	register struct txport *tx = &txport[tn];
	register struct row *rp;
	struct row *lastrp;
	register short i, scrolled;
	register short xsize, ysize;

	setoutput(&tx->tx_r);
	scrolled = tx->tx_state & TX_SCROLLED;

	/* redisplay the text */
/*
	xsize = XLEN(tx->tx_cols) - 1;
*/
	xsize = tx->tx_r.xlen;
	lastrp = &tx->tx_display[(short) tx->tx_rows];
	for (rp = &tx->tx_display[0]; rp < lastrp; rp++) {
		if ((rp->r_changed == 0) && (scrolled == 0))
		    	continue;
		color(tx->tx_pagecolor);
		rectfs(0,rp->r_screeny-chardescender,xsize,
			rp->r_screeny+charheight-chardescender- 1);
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

	llx = tx->tx_col * charwidth;
	lly = (tx->tx_rows - tx->tx_row - 1) * charheight;
	if (!(tx->tx_state & TX_SELECTED)) {
		color(tx->tx_pagecolor);
		rectfs(llx, lly, llx + charwidth - 1, lly + charheight - 1);
		color(tx->tx_cursorcolor);
		rects(llx, lly, llx + charwidth - 1, lly + charheight - 1);
	} else {
		color(tx->tx_cursorcolor);
		rectfs(llx, lly, llx + charwidth - 1, lly + charheight - 1);
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
		cmov2i(llx, lly + chardescender);
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
		rp->r_screeny = charheight * (tx->tx_rows-i-1) 
							+ chardescender;
		rp++;
	}
}

tx_initcolors(tx)
	register struct txport *tx;
{
	register struct txport *consoletx = &txport[0];

	tx->tx_curcolor = 0;
	tx->tx_textcolor = C_FOREGROUND;
	tx->tx_pagecolor = C_BACKGROUND;
	tx->tx_textwritemask = 0xfff;
	tx->tx_pagewritemask = 0xfff;
	tx->tx_cursorcolor = C_CURSOR;
	tx->tx_reversecolor = C_REVERSE;
	tx->tx_state |= (TX_REDISPLAY | TX_SCROLLED);

	/* init cursor color mapping */
	blink(0, tx->tx_cursorcolor, 0, 0, 0);
	mapcolor(tx->tx_cursorcolor, 0, 255, 0);
	tx->tx_cursor_r = 0;
	tx->tx_cursor_g = 255;
	tx->tx_cursor_b = 0;
}

#ifdef	notdef
tx_newtextcolor(tx, color)
register struct txport *tx;
register long color;
{
    tx->tx_textcolor = color;
    tx->tx_state |= (TX_REDISPLAY | TX_SCROLLED);
}

tx_newtextwritemask(tx, color)
register struct txport *tx;
register long color;
{
    tx->tx_textwritemask = color;
    tx->tx_state |= (TX_REDISPLAY | TX_SCROLLED);
}

tx_newpagecolor(tx, color)
register struct txport *tx;
register long color;
{
    tx->tx_pagecolor = color;
    tx->tx_state |= (TX_REDISPLAY | TX_SCROLLED);
}

tx_newpagewritemask(tx, color)
register struct txport *tx;
register long color;
{
    tx->tx_pagewritemask = color;
    tx->tx_state |= (TX_REDISPLAY | TX_SCROLLED);
}
#endif
