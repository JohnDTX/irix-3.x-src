/*
 * Terminal emulator for the visual 50 style terminal.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/gsh/RCS/term_v50.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:46:46 $
 */
#include "gsh.h"
#include "window.h"

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
	register unsigned char c;
	register short newstate;
	register short maxcols, maxrows;
	extern char need_redisplay;
	extern int force_colors;

	maxcols = tx->tx_cols;
	maxrows = tx->tx_rows;
	tx->tx_state |= TX_REDISPLAY;
	rp = &tx->tx_display[tx->tx_row];
	while (n--) {
		c = *(unsigned char *)cp++;
		switch (tx->tx_state & TX_STATEBITS) {
		  case TX_NORMAL:
			c &= 0x7f;
			if (c >= ' ') {
				rp->r_changed = 1;
				if (tx->tx_curcolor) {
				    rp->r_video = 1;
				    rp->r_data[tx->tx_col] = c | 0x80;
				} else
				    rp->r_data[tx->tx_col] = c;

			    /* move to new col and adjust maxcol  
		   	       			in case of overstrike */
				if (++tx->tx_col > rp->r_maxcol)
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
			c &= 0x7f;
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
					tx->tx_col++;
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
					extern int force_title;

					tx->tx_readbuf[tx->tx_count] = 0;
					if (!force_title) {
					    strcpy(title, &tx->tx_readbuf[1]);
					    if (opened)
					    	wintitle(title);
					}
				} else
				if ((tx->tx_cmd == TX_READBINDING) &&
				    (tx->tx_count > 1) &&
				    (c == tx->tx_readbuf[1])) {
					tx->tx_readbuf[tx->tx_count] = 0;
					kb_bindkey(tx->tx_readbuf);
				} else
				if (tx->tx_count < 80) {
					tx->tx_readbuf[tx->tx_count++] = c;
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
					if (!force_colors)
						tx->tx_textcolor = c;
					break;
				  case 'B':
				  case 'b':
					if (!force_colors) {
						tx->tx_pagecolor = c;
						need_redisplay = 1;
						tx_setblink(tx);
					}
					break;
				  case 'R':
				  case 'r':
					if (!force_colors)
						tx->tx_reversecolor = c;
					break;
				  case 'C':
				  case 'c':
					if (!force_colors)
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
	if (tx->tx_col != 0)
		tx_adjust(tx);
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
	if (tx->tx_col != 0)
		tx_adjust(tx);
}
