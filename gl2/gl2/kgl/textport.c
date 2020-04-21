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
 *		:pt:sr=\EI:up=\EA:ku=\E[A:kd=\E[B:kr=\E[C:kl=\E[D:\
 *		:cl=\Ev:ho=\EH:dl=\EM:so=\E9P:se=\E0@:
 *
 * Written by: Kipp Hickman
 */

#include "sys/types.h"
#include "shmem.h"
#include "window.h"
#include "uc4.h"
#include "gf2.h"
#include "gl2cmds.h"
#include "immed.h"
#include "kfont.h"
#include <setjmp.h>

extern int defont_nc;

/* make textport code go slightly faster, by not using second address */
#undef LASTGE
#define LASTGE	(GE)

#define im_do_loadmatrixtrans(m) {	register float *_f = (float *)m;\
				GEWAIT; im_outshort(GEloadmm); 	\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
				im_outfloat(*_f++); 		\
			    }
#undef im_rects
#define im_rects(x1,y1,x2,y2)	{register short _x1=(x1),_x2=(x2),_y1=(y1),_y2=(y2);\
				im_move2s(_x1,_y1);		\
				im_draw2s(_x1,_y2);		\
				im_draw2s(_x2,_y2);		\
				im_draw2s(_x2,_y1);		\
				im_draw2s(_x1,_y1);}
#undef im_rectfs
#define im_rectfs(x1,y1,x2,y2)	{register short _x1=(x1),_x2=(x2),_y1=(y1),_y2=(y2);\
				im_pmv2s(_x1,_y1);		\
				im_pdr2s(_x1,_y2);		\
				im_pdr2s(_x2,_y2);		\
				im_pdr2s(_x2,_y1);		\
				im_pclos();}
#define	DEBUG

/* static stash area for kernel printf's while hardware is busy */

#define STASHSIZE	400
static	char kgr_stash[STASHSIZE];
static	short kgr_stashindex = 0;
short	txneedsrepaint;

/* default colors */
#define	C_BACKGROUND	0
#define	C_FOREGROUND	7
#define	C_CURSOR	2
#define	C_REVERSE	3

/*
 * tx_stash:
 *	- stash away a character into the stash buffer (for the console port)
 *	  for later input
 *	- this is used when an interrupt routine needs to print a diagnostic
 *	  and the console textport is in use
 */
tx_stash(c)
	char c;
{
	if (kgr_stashindex >= STASHSIZE)
		return;
	kgr_stash[kgr_stashindex++] = c;
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
	short tn;
	register char *cp;
	register int n;
{
	register struct txport *tx = &txport[tn];
	register struct row *rp;
	register unsigned char c;
	register short newstate;
	register short maxcols, maxrows;

	maxcols = tx->tx_cols;
	maxrows = tx->tx_rows;
	tx->tx_state |= TX_REDISPLAY;
	rp = &tx->tx_display[tx->tx_row];
	while (n--) {
		c = *(unsigned char *)cp++;
		if (c >= defont_nc)
			c &= 0x7f;		/* assume font_nc > 127 */
		switch (tx->tx_state & TX_STATEBITS) {
		  case TX_NORMAL:
			if (c >= ' ') {
				rp->r_changed = 1;
				rp->r_data[tx->tx_col] = c;

				/* allow for reverse video only if
				 *    RV_8BIT is defined
				 *	(reverse video & 8 bit data chars),
				 * or if font 0 has <= 128 characters
				 *	(allowing 7-bit characters).
				 */
#ifdef RV_8BIT
				rp->r_flags[tx->tx_col] &= ~FLAG_REV;
				if (tx->tx_curcolor) {
					rp->r_video = 1;
					rp->r_flags[tx->tx_col] |= FLAG_REV;
				}
#else
				if (defont_nc <= 128) {
				    rp->r_data[tx->tx_col] &= 0x7f;
				    if (tx->tx_curcolor) {
					rp->r_video = 1;
					rp->r_data[tx->tx_col] |= 0x80;
				    }
#endif
				}

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
				kb_ringbell();
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
				win_softintr(tx->tx_no,'\033',0);
				win_softintr(tx->tx_no,',',0);
				tx_sendval(tx,maxcols);
				tx_sendval(tx,maxrows);
				win_softintr(tx->tx_no,'\r',0);
				break;
			  default:		/* ignore garbage */
				break;
			}
			tx->tx_state |= newstate;
		        rp = &tx->tx_display[tx->tx_row];
			break;

		  case TX_COLLECT:
			tx->tx_readbuf[--tx->tx_count] = c;
			if (tx->tx_count)
				break;
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
					break;
				  case 'B':
				  case 'b':
					tx->tx_pagecolor = c;
					break;
				  case 'R':
				  case 'r':
					tx->tx_reversecolor = c;
					break;
				  case 'C':
				  case 'c':
					tx->tx_cursorcolor = c;
					break;
				}
			        tx->tx_state |= 
				  (TX_REDISPLAY | TX_SCROLLED | TX_DRAWBORDER);
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
			}
			tx->tx_state &= ~TX_STATEBITS;
			tx->tx_state |= TX_NORMAL;
			rp = &tx->tx_display[tx->tx_row];
			break;
		}
	}
}

/*
 * tx_sendval:
 *	- return an integer value
 */
tx_sendval(tx, n)
	register struct txport *tx;
	unsigned short n;
{
	win_softintr(tx->tx_no,' '+(n&0x3f),0);
	win_softintr(tx->tx_no,' '+((n>>6)&0x3f),0);
}

/*
 * tx_upln:
 *	- move cursor up one line, scrolling if needed (and allowed)
 */
tx_upln(tx, scroll)
	register struct txport *tx;
	short scroll;
{
	register struct row *rp;

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
		while (tx->tx_col > rp->r_maxcol) {
			rp->r_data[rp->r_maxcol] = ' ';
#ifdef RV_8BIT
			rp->r_flags[rp->r_maxcol] &= ~FLAG_REV;
#endif
			rp->r_maxcol++;
		}
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
#ifdef RV_8BIT
	register char *saveflags;
#endif

	rp = &tx->tx_display[(short) (tx->tx_rows-1)];
	savedata = rp->r_data;
#ifdef RV_8BIT
	saveflags = rp->r_flags;
#endif
	linerp = &tx->tx_display[line];
	for (; rp > linerp; rp--) {
		rp->r_data = (rp - 1)->r_data;
#ifdef RV_8BIT
		rp->r_flags = (rp - 1)->r_flags;
#endif
		rp->r_maxcol = (rp - 1)->r_maxcol;
		rp->r_video = (rp - 1)->r_video;
		rp->r_changed = 1;
	}
	linerp->r_data = savedata;
#ifdef RV_8BIT
	linerp->r_flags = saveflags;
#endif
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
#ifdef RV_8BIT
	register char *saveflags;
#endif

	rp = &tx->tx_display[line];
	savedata = rp->r_data;
#ifdef RV_8BIT
	saveflags = rp->r_flags;
#endif
	rowsrp = &tx->tx_display[(short) (tx->tx_rows-1)];
	for (; rp < rowsrp; rp++) {
		rp->r_data = (rp + 1)->r_data;
#ifdef RV_8BIT
		rp->r_flags = (rp + 1)->r_flags;
#endif
		rp->r_maxcol = (rp + 1)->r_maxcol;
		rp->r_video = (rp + 1)->r_video;
		rp->r_changed = 1;
	}
	rowsrp->r_data = savedata;
#ifdef RV_8BIT
	rowsrp->r_flags = saveflags;
#endif
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
	txport[txnum].tx_state = TX_NORMAL | TX_DRAWBORDER | TX_BORDER |
		TX_SCROLLED | TX_OPEN | TX_ON;
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
	gr_qenter(gl_wmport, WMTXCLOSE, txnum);
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

    /* setup text port parameters */
	tx->tx_keypadmode = 0;
	tx->tx_state = TX_BUSY | TX_ON;
	tx_initcolors(tx);
	tx->tx_oldrow = tx->tx_oldcol = tx->tx_row = tx->tx_col = 0;

    /* setup display data structure */
	rp = &tx->tx_display[0];
	for (i = 0; i < MAXROWS; i++) {
		rp->r_changed = 0;
		rp->r_maxcol = 0;
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
	tx->tx_llx = xorg;
	tx->tx_lly = yorg;

    /* clear rows of text that are invisible */
	rp = &tx->tx_display[rows];
	for (i = rows; i < MAXROWS; i++, rp++) {
		rp->r_changed = 0;
		rp->r_maxcol = 0;
	}
	tx->tx_state |= TX_DRAWBORDER;
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

	cols = (width - 2*XBORDER) / gl_charwidth;
	if(cols>MAXCOLS) 
	    cols = MAXCOLS;
	else if(cols<1) {
	    cols = 1;
	    width = gl_charwidth + 2*XBORDER;
	}

	rows = (height - 2*YBORDER) / gl_charheight;
	if(rows>MAXROWS) 
	    rows = MAXROWS;
	else if(rows<1) {
	    rows = 1;
	    height = gl_charheight + 2*YBORDER;
	}
	llx = x1 + (width - cols*gl_charwidth)/2;
	lly = y1 + (height - rows*gl_charheight)/2;

    /* scroll text up when the port is made smaller */
	tx->tx_state |= TX_BUSY;
	nscroll = (tx->tx_rows-rows) - (tx->tx_rows-1-tx->tx_row);
	for (i = 0; i < nscroll; i++) {
	    tx_deleteln(tx,0);
	    tx->tx_row--;
	}
	tx_setsize(tx,cols,rows,llx,lly);
	tx_onepiece(tx);
	tx->tx_state &= ~TX_BUSY; 
}


/*
 * tx_onepiece:
 *	- give the textport one piece
 */
tx_onepiece(tx)
register struct txport *tx;
{
    register int pri;
    register struct piece *pp;

    if(!tx)
	return;
    if (!gl_wmport) {
	pri = spl6();			/* block repaint code */
	if (tx->tx_piecelist) {
		gr_freepieces(tx->tx_piecelist);
		tx->tx_piecelist = 0;
	}
	tx->tx_piecelist = pp = gr_allocpieces(1);
	pp->p_xmin = tx->tx_llx - XBORDER;	
	pp->p_ymin = tx->tx_lly - YBORDER;
	pp->p_xmax = pp->p_xmin + XLEN(tx->tx_cols) + XBORDER*2 - 1;
	pp->p_ymax = pp->p_ymin + YLEN(tx->tx_rows) + YBORDER*2 - 1;
	splx(pri);	
    }
}


/* 
 * tx_gettextport:
 *	- get the size and position of the textport.
 */
tx_gettextport(tx, x1, x2, y1 ,y2)
	register struct txport *tx;
	short *x1, *x2, *y1, *y2;
{
	*x1 = tx->tx_llx - XBORDER;
	*x2 = tx->tx_llx + tx->tx_cols*gl_charwidth + XBORDER - 1;
	*y1 = tx->tx_lly - YBORDER;
	*y2 = tx->tx_lly + tx->tx_rows*gl_charheight + YBORDER - 1;
}

/*
 * tx_repaint:
 *	- perform the actual repaint work for all the textports
 */
tx_repaint(restore)
    int restore;
{
    register struct txport *tx;
    register int s;
    struct inputchan *oldgfport;

    s = spl6();
    if ((s & 0x0700) >= 0x0300) {
	/*
	 * Processor priority is too high.  We have to run at lower than
	 * spl3, otherwise we may hang the system by writing to the pipe
	 * when the pipe is full.  Call gr_os_stopproc() to force a system
	 * reschedule (which will call us again, at a better priority).
	 */
	txneedsrepaint = 1;
	gr_os_stopproc();
	return;
    }

    /*
     * If somebody has the pipe busy, or if the pipe is locked because of
     * a feedback operation, don't paint now.
     */
    if (PIPEISBUSY || gl_lock) {
	txneedsrepaint = 1;
	/*
	 * If graphics isn't locked, then setup for a pipe interrupt when
	 * user completes their current graphics command.  Then the user
	 * will be stopped, and the os scheduler entered again, at which
	 * point the os will call this procedure to paint the txports.
	 */
	if (!gl_lock) 
	    fbc_setpipeint();
	splx(s);
	return;
    }
    gl_lock = 1;
    splx(s);

    txneedsrepaint = 0;			/* assume success */
    oldgfport = 0;
    for (tx = &txport[0]; tx < &txport[NTXPORTS]; tx++) {
	if (!(tx->tx_state & TX_ON))
		continue;
	s = spl6();
	if (tx->tx_state & TX_BUSY) {
	    /*
	     * Set txneedsrepaint flag to force a repaint in the
	     * future, since it can't be done now.
	     */
	    txneedsrepaint = 1;
	    splx(s);
	    continue;
	}
	if (tx->tx_state & TX_REDISPLAY) {

	    /* set busy bit to keep other asyncs away */
	    tx->tx_state |= TX_BUSY;
	    splx(s);

	    /*
	     * See if somebody else has their state in the hardware, and
	     * if so, save it. Then switch to the kernels shared memory.
	     */
	    if(gl_gfport) {
		oldgfport = gl_gfport;
		saveeverything();
		s = spl6();
		gl_gfport = 0;
		gl_shmemptr = gl_kernelshmemptr;
		(void) gr_setshmem(0);
		splx(s);
	    }
	    tx_redisplay(tx);
	} else {
	    splx(s);
	}
    }

    /*
     * If we couldn't paint everything because of a busy lockout, ask the
     * system to do a reschedule, and thus call us again.
     */
    if (txneedsrepaint) {
	gr_os_stopproc();
    }
    gl_lock = 0;
    GETOKEN = GEpassthru|((0-1)<<8);	/* in case we missed a
					 * retrace int while gl_locked
					 */
    /*
     * If we want to restore the old state back and we actually saved some
     * state, and the current running process is the one who needs the gfport,
     * then give the state back to it.
     */
    if (restore && oldgfport)
	gr_switchstate(oldgfport);
}

/*
 * Update display, given new data
 */
tx_redisplay(tx)
	register struct txport *tx;
{
	im_setup;
	register struct row *rp;
	struct row *lastrp;
	register short i, scrolled;
	register short xsize, ysize;
	register int s;
	register short autosave = 0;
	long ctx;
	if (gl_autocursor && !gl_wmport) {
		autosave = 1;
		gl_autocursor = 0;
		if(gl_cursordrawn) {
			gl_cursordrawn = 0;
			im_passcmd(1, FBCundrawcursor);
		}
	}

	/* setconfig(gl_cfr|UPDATEA|UPDATEB);*/
	im_passcmd(3, FBCconfig);
	if (gl_isblanked || gl_blankmode)
	    im_outlong((gl_cfr|UPDATEA|UPDATEB) & ~(DISPLAYA | DISPLAYB));
	else
	    im_outlong(gl_cfr|UPDATEA|UPDATEB);

	scrolled = tx->tx_state & TX_SCROLLED;

	if (!tx->tx_piecelist)
		goto alldone;
	im_passcmd(3, FBCcdcolorwe);
	im_outlong(0);			/* we off for cd planes */
	im_passcmd(2, FBCbaseaddress);
	im_outshort(0);
	im_passcmd(2, FBCsetbackfacing);
	im_outshort(0);
	/* viewportall(); */
{
	register long gezero = 0;
	register long xmax, ymax;

	xmax = (XMAXSCREEN + 1) << 7;

	/* yes XMAXSCREEN for off window support */
	ymax = (XMAXSCREEN + 1) << 7;

	im_outshort(GEloadviewport);
	im_outlong(xmax);		/* vcx (center: x, y) */
	im_outlong(ymax);		/* vcy */
	/* can you figure this out? for off window support */
	im_outlong(3*xmax);		/* vsx (half-size: x, y) */
	im_outlong(3*ymax);		/* vsy */

	im_outlong(gezero);		/* vcs (normal: near, far) */
	im_outlong(xmax);		/* vcz */
	im_outlong(gezero);		/* vss (stereo: near, far) */
	im_outlong(xmax);		/* vsz */
	im_passcmd(5, FBCloadviewport);
	im_outshort(-0x400);
	im_outshort(-0x400);
	im_outlong(0x7ff07ff);
}
	/* ortho2i(); */
{
	static float orthomattrans[4][4] = {
	2.0/3072.0,	0.0,		0.0,		-1023.0/3072.0,
	0.0,		2.0/3072.0,	0.0,		-1023.0/3072.0,
	0.0,		0.0,		-1.0,		0.0,
	0.0,		0.0,		0.0,		1.0
	};

	im_do_loadmatrixtrans(orthomattrans);
}
	setfbcpieces(tx->tx_piecelist);
	im_do_translates(tx->tx_llx, tx->tx_lly, 0);
	im_passcmd(2, FBCpolystipple);
	im_outshort(FR_DEFPATTERN);
	im_passcmd(2, FBClinewidth);
	im_outshort(0);
	im_passcmd(3, FBClinestipple);
	im_outlong(0xffff);

    /* then redisplay the text */
	xsize = XLEN(tx->tx_cols) - 1;
	ysize = YLEN(tx->tx_rows) - 1;
	lastrp = &tx->tx_display[(short) tx->tx_rows];
	for (rp = &tx->tx_display[0]; rp < lastrp; rp++) {
		if ((rp->r_changed == 0) && (scrolled == 0))
		    	continue;
		if(gl_cfr & (UC_DOUBLE << 16)) {
		   im_do_writemask(tx->tx_pagewritemask & gl_userdbwritemask);
		} else {
		    im_do_writemask(tx->tx_pagewritemask & gl_userwritemask);
		}
		im_do_color(tx->tx_pagecolor);
		im_rectfs(0,rp->r_screeny-gl_chardescender,xsize,
			rp->r_screeny+gl_charheight-gl_chardescender- 1);
		if (rp->r_maxcol) {
			if(gl_cfr & (UC_DOUBLE << 16)) {
				im_do_writemask(tx->tx_textwritemask &
							gl_userdbwritemask);
			} else {
				im_do_writemask(tx->tx_textwritemask &
							gl_userwritemask);
			}
			im_cmov2i(0, rp->r_screeny);
			if (rp->r_video) {
				register char *c = &rp->r_data[0];
#ifdef RV_8BIT
				register char *fl = &rp->r_flags[0];
#endif

				for (i=MIN(rp->r_maxcol,tx->tx_cols); i--;) {
#ifdef RV_8BIT
					if (*fl++ & FLAG_REV)
					    im_do_color(tx->tx_reversecolor);
					else
					    im_do_color(tx->tx_textcolor);
					xcharstr(c++, 1);
#else
					if (*c & 0x80) {
					    im_do_color(tx->tx_reversecolor);
					    *c &= 0x7f;    /* hack alert */
					    xcharstr(c, 1);
					    *c++ |= 0x80;
					} else {
					    im_do_color(tx->tx_textcolor);
					    xcharstr(c++, 1);
					}
#endif


				}
			} else {
				im_do_color(tx->tx_textcolor);
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

    /* lastly, draw up the border, if the border is requested */
	if ((tx->tx_state & (TX_BORDER|TX_DRAWBORDER)) 
					== (TX_BORDER|TX_DRAWBORDER)) {
		if(gl_cfr & (UC_DOUBLE << 16)) {
			im_do_writemask(tx->tx_pagewritemask &
						gl_userdbwritemask);
		} else {
			im_do_writemask(tx->tx_pagewritemask &
						gl_userwritemask);
		}
		im_do_color(tx->tx_pagecolor);
		im_rects(-1-(XBORDER-4), -1-(YBORDER-4), 
				xsize+1+(XBORDER-4), ysize+1+(YBORDER-4));
		im_rects(-2-(XBORDER-4), -2-(YBORDER-4), 
				xsize+2+(XBORDER-4), ysize+2+(YBORDER-4));
		if(gl_cfr & (UC_DOUBLE << 16)) {
			im_do_writemask(tx->tx_textwritemask &
						gl_userdbwritemask);
		} else {
			im_do_writemask(tx->tx_textwritemask &
						gl_userwritemask);
		}
		im_do_color(tx->tx_textcolor);
		im_rects(-3-(XBORDER-4), -3-(YBORDER-4), 
				xsize+3+(XBORDER-4), ysize+3+(YBORDER-4));
		im_rects(-4-(XBORDER-4), -4-(YBORDER-4), 
				xsize+4+(XBORDER-4), ysize+4+(YBORDER-4));
	}

alldone:
	tx->tx_state &= ~(TX_BUSY | TX_REDISPLAY | TX_DRAWBORDER | TX_SCROLLED);
	if(autosave)
		curson(1);
}

/*
 * tx_drawcursor:
 *	- display cursor at new location
 */
tx_drawcursor(tx)
	register struct txport *tx;
{
	im_setup;
	register struct row *rp;
	register short llx, lly;

	if(gl_cfr & (UC_DOUBLE << 16)) {
		im_do_writemask(tx->tx_textwritemask & gl_userdbwritemask);
	} else {
		im_do_writemask(tx->tx_textwritemask & gl_userwritemask);
	}
	im_do_color(tx->tx_cursorcolor);
	llx = tx->tx_col * gl_charwidth;
	lly = (tx->tx_rows - tx->tx_row - 1) * gl_charheight;
	im_rectfs(llx, lly, llx + gl_charwidth - 1, lly + gl_charheight - 1);
	rp = &tx->tx_display[tx->tx_row];
	if (tx->tx_col < rp->r_maxcol) {
		im_do_color(tx->tx_pagecolor);
		im_cmov2i(llx, lly + gl_chardescender);
		xcharstr(&rp->r_data[tx->tx_col], 1);
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
		rp->r_screeny = gl_charheight * (tx->tx_rows-i-1) 
							+ gl_chardescender;
		rp++;
	}
}

tx_initcolors(tx)
	register struct txport *tx;
{
	register struct txport *consoletx = &txport[0];

	if(tx == consoletx) {
		tx->tx_curcolor = 0;
		tx->tx_textcolor = C_FOREGROUND;
		tx->tx_pagecolor = C_BACKGROUND;
		tx->tx_textwritemask = 0xfff;
		tx->tx_pagewritemask = 0xfff;
		tx->tx_cursorcolor = C_CURSOR;
		tx->tx_reversecolor = C_REVERSE;
	} else {
		tx->tx_curcolor = consoletx->tx_curcolor;
		tx->tx_textcolor = consoletx->tx_textcolor;
		tx->tx_pagecolor = consoletx->tx_pagecolor;
		tx->tx_textwritemask = consoletx->tx_textwritemask;
		tx->tx_pagewritemask = consoletx->tx_pagewritemask;
		tx->tx_cursorcolor = consoletx->tx_cursorcolor;
		tx->tx_reversecolor = consoletx->tx_reversecolor;
	}
	tx->tx_state |= (TX_REDISPLAY | TX_SCROLLED | TX_DRAWBORDER);
}

tx_newtextcolor(tx, color)
register struct txport *tx;
register long color;
{
    tx->tx_textcolor = color;
    tx->tx_state |= (TX_REDISPLAY | TX_SCROLLED | TX_DRAWBORDER);
}

tx_newtextwritemask(tx, color)
register struct txport *tx;
register long color;
{
    tx->tx_textwritemask = color;
    tx->tx_state |= (TX_REDISPLAY | TX_SCROLLED | TX_DRAWBORDER);
}

tx_newpagecolor(tx, color)
register struct txport *tx;
register long color;
{
    tx->tx_pagecolor = color;
    tx->tx_state |= (TX_REDISPLAY | TX_SCROLLED | TX_DRAWBORDER);
}

tx_newpagewritemask(tx, color)
register struct txport *tx;
register long color;
{
    tx->tx_pagewritemask = color;
    tx->tx_state |= (TX_REDISPLAY | TX_SCROLLED | TX_DRAWBORDER);
}

tx_lock(tn)
	short tn;
{
	register struct txport *tx = &txport[tn];
	register int pri;

	pri = spl7();
	if(tx->tx_state & TX_BUSY) {
		splx(pri);
		return 0;
	}
	tx->tx_state |= TX_BUSY;
	splx(pri);
	return 1;
}

tx_unlock(tn)
	short tn;
{
	if ((tn == 0) && kgr_stashindex) {	/* this is a little scary */
		kgr_stashindex = 0;
		tx_addchars(0, kgr_stash, kgr_stashindex);
	}
	txport[tn].tx_state &= ~TX_BUSY;
}
