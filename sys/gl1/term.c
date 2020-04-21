/*
 * Terminal emulation portion of the window manager
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
 * $Source: /d2/3.7/src/sys/gl1/RCS/term.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:28:36 $
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../gl1/kgl.h"
#include "../gl1/textport.h"

/* default colors */
#define	C_BACKGROUND	0
#define	C_FOREGROUND	7
#define	C_CURSOR	7
#define	C_REVERSE	5

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
	register struct textport *tx = &txport[tn];
	register struct row *rp;
	register char c;
	register short newstate;

	tx->tx_state |= TX_REDISPLAY;
	while (n--) {
		c = *cp++ & 0x7f;
		rp = &tx->tx_display[tx->tx_row];
		switch (tx->tx_state & TX_STATEBITS) {
		  case TX_NORMAL:
			if (c < ' ') {
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
					kbbell();
					break;
				  case '\t':			/* tab */
					rp->r_changed = 1;
					tx->tx_col = tx->tx_col +
						(8 - (tx->tx_col & 7));
/* XXX */
					if (tx->tx_col >= tx->tx_w.w_xlen)
						tx->tx_col = tx->tx_w.w_xlen-1;
					tx_adjust(tx);
					break;

				  case '\n':			/* new line */
					rp->r_changed = 1;
					tx_downln(tx, 1);
					break;

				  case '\r':			/* return */
					rp->r_changed = 1;
					tx->tx_col = 0;
					tx_adjust(tx);
					break;
				}
			} else {
				if (c == 0x7f)			/* XXX */
					break;
				rp = &tx->tx_display[tx->tx_row];
				rp->r_data[tx->tx_col] = c & 0x7f;
				rp->r_color[tx->tx_col] = tx->tx_curcolor;
				if (tx->tx_curcolor != tx->tx_foreground)
					rp->r_video = 1;
				rp->r_changed = 1;

			    /* move to new column */
				tx->tx_col++;

			    /* adjust maxcol in case of overstrike */
				if (rp->r_maxcol < tx->tx_col) {
					rp->r_maxcol = tx->tx_col;
					rp->r_data[tx->tx_col] = 0;
				}

			    /* handle automatic margins */
				if (tx->tx_col == tx->tx_w.w_xlen) {
					tx->tx_col = 0;
					tx_downln(tx, 1);
				}
			}
			break;

		  case TX_ESCAPE:
			tx->tx_state &= ~TX_ESCAPE;
			newstate = TX_NORMAL;
			switch (c) {
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
			  case ';':		/* set keypad mode */
				keypadmode = 1;
				break;
			  case '>':		/* reset keypad mode */
				keypadmode = 0;
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
				if (tx->tx_col < (tx->tx_w.w_xlen - 1)) {
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
					rp < &tx->tx_display[tx->tx_w.w_ylen];
					rp++) {
					rp->r_maxcol = rp->r_video = 0;
					rp->r_changed = 1;
				}
				newstate |= TX_SCROLLED;
				break;
			  case 'K':		/* clear to eol */
				rp = &tx->tx_display[tx->tx_row];
				rp->r_maxcol = tx->tx_col;
				rp->r_changed = 1;
				break;
			  case 'L':		/* add line */
				tx_insertln(tx, tx->tx_row);
				newstate |= TX_SCROLLED;
				break;
			  case 'M':		/* delete line */
				tx_deleteln(tx, tx->tx_row);
				newstate |= TX_SCROLLED;
				break;
			  case 'Y':		/* move cursor */
				newstate = TX_COLLECT;
				tx->tx_cmd = TX_READY;
				tx->tx_count = 2;
				break;
			  default:		/* ignore garbage */
				break;
			}
			tx->tx_state |= newstate;
			break;

		  case TX_COLLECT:
			tx->tx_readbuf[--tx->tx_count] = c;
			if (tx->tx_count)
				break;
			switch (tx->tx_cmd) {
			  case TX_READ0:
				tx->tx_curcolor = tx->tx_foreground;
				break;
			  case TX_READ9:
				tx->tx_curcolor = tx->tx_reverse;
				break;
			  case TX_READ7:
				c = tx->tx_readbuf[0] & 7;
				switch (tx->tx_readbuf[1]) {
				  case 'F':
				  case 'f':
					tx->tx_borderon =
						tx->tx_foreground = c;
					break;
				  case 'B':
				  case 'b':
					tx->tx_borderoff =
						tx->tx_background = c;
					break;
				  case 'R':
				  case 'r':
					tx->tx_reverse = c;
					break;
				  case 'C':
				  case 'c':
					tx->tx_cursorcolor = c;
					break;
				}
				tx->tx_state |= TX_SCROLLED;
				tx->tx_curcolor = tx->tx_foreground;
				break;
			  case TX_READY:
				c = tx->tx_readbuf[1];
				if ((c < 32) || (c >= (32 + tx->tx_w.w_ylen)))
					break;
				tx->tx_row = c - 32;
				c = tx->tx_readbuf[0];
				if ((c < 32) || (c >= (32 + tx->tx_w.w_xlen)))
					break;
				tx->tx_col = c - 32;
				rp->r_changed = 1;
				tx_adjust(tx);
				break;
			}
			tx->tx_state &= ~TX_STATEBITS;
			tx->tx_state |= TX_NORMAL;
			break;
		}
	}
}

/*
 * tx_upln:
 *	- move cursor up one line, scrolling if needed (and allowed)
 */
tx_upln(tx, scroll)
	register struct textport *tx;
	short scroll;
{
	register struct row *rp;

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
	register struct textport *tx;
	short scroll;
{
	if (tx->tx_row == (tx->tx_w.w_ylen - 1)) {
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
	register struct textport *tx;
{
	register struct row *rp;
	register short delta;

	rp = &tx->tx_display[tx->tx_row];
	if (tx->tx_col > rp->r_maxcol) {
		while (tx->tx_col > rp->r_maxcol)
			rp->r_data[rp->r_maxcol++] = ' ';
		rp->r_changed = 1;
	}
}

/*
 * tx_insertln:
 *	- insert "line" from the screen moving the current and all lower
 *	  lines down one
 */
tx_insertln(tx, line)
	register struct textport *tx;
	register short line;
{
	register struct row *rp, *linerp;
	register u_char *savedata, *savecolor;

	rp = &tx->tx_display[(short) (tx->tx_w.w_ylen - 1)];
	savedata = rp->r_data;
	savecolor = rp->r_color;
	linerp = &tx->tx_display[line]; 
	for (; rp > linerp; rp--) {
		rp->r_data = (rp - 1)->r_data;
		rp->r_color = (rp - 1)->r_color;
		rp->r_maxcol = (rp - 1)->r_maxcol;
		rp->r_video = (rp - 1)->r_video;
		rp->r_changed = 1;
	}

	linerp->r_data = savedata;
	linerp->r_color = savecolor;
	linerp->r_video = 0;
	linerp->r_maxcol = 0;
	linerp->r_changed = 1;
	tx_adjust(tx);
}

/*
 * tx_deleteln:
 *	- delete "line" from the screen moving all lower lines up one
 */
tx_deleteln(tx, line)
	register struct textport *tx;
	register short line;
{
	register struct row *rp, *ylenrp;
	register u_char *savedata, *savecolor;
	register short ylen;

	rp = &tx->tx_display[line];
	savedata = rp->r_data;
	savecolor = rp->r_color;
	ylenrp = &tx->tx_display[(short) (tx->tx_w.w_ylen-1)];
	for (; rp < ylenrp; rp++) {
		rp->r_data = (rp + 1)->r_data;
		rp->r_color = (rp + 1)->r_color;
		rp->r_maxcol = (rp + 1)->r_maxcol;
		rp->r_video = (rp + 1)->r_video;
		rp->r_changed = 1;
	}

	ylenrp->r_data = savedata;
	ylenrp->r_color = savecolor;
	ylenrp->r_video = 0;
	ylenrp->r_maxcol = 0;
	ylenrp->r_changed = 1;
	tx_adjust(tx);
}
