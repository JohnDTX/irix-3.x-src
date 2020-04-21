/*
 * Routines specific to maintaining textport's
 *
 * $Source: /d2/3.7/src/sys/gl1/RCS/textport.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:28:37 $
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/setjmp.h"
#include "machine/cpureg.h"
#include "../gl1/kgl.h"
#include "../gl1/font.h"
#include "../gl1/textport.h"
#include "../gl1/shmem.h"

/* default colors */
#define	C_BACKGROUND	0
#define	C_FOREGROUND	7
#define	C_CURSOR	2
#define	C_REVERSE	3
#define	C_BORDERON	C_FOREGROUND
#define	C_BORDEROFF	C_BACKGROUND

ushort HEIGHT		= defont_height;
ushort WIDTH		= defont_width;
ushort DESCENDER	= defont_descender;

jmp_buf	txenv;

/*
 * tx_console:
 *	- intialize the console textport
 */
tx_console()
{
	tx_init(&txport[0]);
	txport[0].tx_state |= TX_DRAWBORDER | TX_BORDER | TX_OPEN | TX_ON;
}

/*
 * tx_init:
 *	- init a textport, assuming system defaults
 *	- the system uses borders, and thus the default piece xmin and
 *	  ymin don't directly correspond to the llx and lly!
 */
tx_init(tx)
	register struct textport *tx;
{
	register struct piece *pp;
	register struct row *rp;
	register short i;

    /* setup window parameters */
	tx->tx_curcolor = tx->tx_foreground = C_FOREGROUND;
	tx->tx_background = C_BACKGROUND;
	tx->tx_cursorcolor = C_CURSOR;
	tx->tx_borderon = C_BORDERON;
	tx->tx_borderoff = C_BORDEROFF;
	tx->tx_reverse = C_REVERSE;
	tx->tx_oldrow = tx->tx_oldcol = tx->tx_row = tx->tx_col = 0;
	tx->tx_state = TX_NORMAL;
	tx->tx_w.w_xlen = MAXCOLS;
	tx->tx_w.w_ylen = MAXROWS;
	tx->tx_w.w_llx = (XMAXSCREEN - XLEN(MAXCOLS)) / 2;
	tx->tx_w.w_lly = (YMAXSCREEN - YLEN(MAXROWS)) / 2;

    /* setup piece's */
	pp = &tx->tx_w.w_piece[0];
	pp->p_state = 1;
	pp->p_xmin = tx->tx_w.w_llx - XBORDER;
	pp->p_xmax = tx->tx_w.w_llx + XLEN(tx->tx_w.w_xlen) + XBORDER;
	pp->p_ymin = tx->tx_w.w_lly - YBORDER;
	pp->p_ymax = tx->tx_w.w_lly + YLEN(tx->tx_w.w_ylen) + YBORDER;
	pp->p_ly = 0;
	pp->p_uy = pp->p_ymax - tx->tx_w.w_lly;
	pp++;
	pp->p_state = 0;

    /* setup display data structure */
	rp = &tx->tx_display[0];
	for (i = 0; i < MAXROWS; i++) {
		rp->r_changed = 1;
		rp->r_maxcol = 0;
		rp->r_data = &rp->r_buf[0];
		rp->r_color = &rp->r_colbuf[0];
		rp++;
	}
	tx_fixcursor(tx);
}

/*
 * tx_resetconsole:
 *	- used to reset the console display when the window manager isn't
 *	  running
 *	- we attempt to leave the window state alone as much as possible,
 *	  and only fix things up that are messed up
 */
tx_resetconsole()
{
	register struct textport *tx = &txport[0];
	register struct piece *pp;

    /* force window to a good state */
	tx->tx_state = TX_NORMAL | TX_ON | TX_BORDER | TX_REDISPLAY |
		TX_SCROLLED | TX_DRAWBORDER | TX_OPEN | TX_SCROLLED;

    /* make sure the colors are ok */
	if (tx->tx_background == tx->tx_foreground) {
		tx->tx_curcolor = tx->tx_foreground = C_FOREGROUND;
		tx->tx_background = C_BACKGROUND;
		tx->tx_cursorcolor = C_CURSOR;
		tx->tx_reverse = C_REVERSE;
		tx->tx_borderon = C_BORDERON;
		tx->tx_borderoff = C_BORDEROFF;
	}

    /* if window size looks okay, then leave it alone */
	if (tx->tx_w.w_xlen < MINCOLS)
		tx->tx_w.w_xlen = MINCOLS;
	else
	if (tx->tx_w.w_xlen > MAXCOLS)
		tx->tx_w.w_xlen = MAXCOLS;
	if (tx->tx_w.w_ylen < MINROWS)
		tx->tx_w.w_ylen = MINROWS;
	else
	if (tx->tx_w.w_ylen > MAXROWS)
		tx->tx_w.w_ylen = MAXROWS;

    /* adjust cursor to potentialy new window bounds */
	tx_fixcursor(tx);
	tx->tx_oldrow = tx->tx_row;
	tx->tx_oldcol = tx->tx_col;

    /* make sure window fits on the screen */
	if (!(XBORDER <= tx->tx_w.w_llx
	 && tx->tx_w.w_llx + XLEN(tx->tx_w.w_xlen) - 1 <= XMAXSCREEN - XBORDER))
		tx->tx_w.w_llx = (XMAXSCREEN - XLEN(tx->tx_w.w_xlen)) / 2;
	if (!(YBORDER <= tx->tx_w.w_lly
	 && tx->tx_w.w_lly + YLEN(tx->tx_w.w_ylen) - 1 <= YMAXSCREEN - YBORDER))
		tx->tx_w.w_lly = (YMAXSCREEN - YLEN(tx->tx_w.w_ylen)) / 2;

    /* setup piece's */
	pp = &tx->tx_w.w_piece[0];
	pp->p_state = 1;
	pp->p_xmin = tx->tx_w.w_llx - XBORDER;
	pp->p_xmax = tx->tx_w.w_llx + XLEN(tx->tx_w.w_xlen) - 1 + XBORDER;
	pp->p_ymin = tx->tx_w.w_lly - YBORDER;
	pp->p_ymax = tx->tx_w.w_lly + YLEN(tx->tx_w.w_ylen) - 1 + YBORDER;
	pp->p_ly = 0;
	pp->p_uy = pp->p_ymax - tx->tx_w.w_lly;
	pp++;
	pp->p_state = 0;
}

/*
 * tx_redisplay:
 *	- update display, given new data
 */
tx_redisplay(tx)
	register struct textport *tx;
{
	register struct row *rp, *lastrp;
	register struct piece *pp;
	struct shmem *sh = (struct shmem *)SHMEM_VBASE;
	register short scrolled;
	register short x, y;
	register int s;

	s = spltty();
	if (((tx->tx_state & (TX_REDISPLAY | TX_BUSY | TX_ON)) !=
	     (TX_ON | TX_REDISPLAY)) ||
	    (kgr & KGR_BUSY) || (shmem_pa && !sh->IdleMode)) {
		splx(s);
		return;
	}
	if (shmem_pa)				/* HACK */
		sh->IdleMode = 0;		/* block fbc cursor */
	kgr |= KGR_BUSY | KGR_REPAINTING;
	tx->tx_state |= TX_BUSY;
	splx(s);

	grlastupdate = time;
	unblankscreen();

    /* we get here from kgreset if something fouled up */
	if (setjmp(txenv)) {
		tx->tx_state |= TX_DRAWBORDER | TX_SCROLLED;
		/* maybe reset piece list too... */
	}
	scrolled = tx->tx_state & TX_SCROLLED;

	if (grproc)
		changemode(0);

	pushmatrix();
	viewport(0, XMAXSCREEN, 0, YMAXSCREEN);
	ortho2i();

    /* first draw up the border, if the border is requested */
	if ((tx->tx_state & (TX_BORDER | TX_DRAWBORDER)) ==
			(TX_BORDER | TX_DRAWBORDER)) {
		x = tx->tx_w.w_llx + XLEN(tx->tx_w.w_xlen) - 1;
		y = tx->tx_w.w_lly + YLEN(tx->tx_w.w_ylen) - 1;
		for (pp = &tx->tx_w.w_piece[0]; pp->p_state != 0; pp++) {
			scrmask(pp->p_xmin, pp->p_xmax, pp->p_ymin, pp->p_ymax);

			color(tx->tx_borderoff);
			recti(tx->tx_w.w_llx - 1, tx->tx_w.w_lly - 1,
					 x + 1, y + 1);
			recti(tx->tx_w.w_llx - 2, tx->tx_w.w_lly - 2,
					 x + 2, y + 2);
			color(tx->tx_borderon);
			recti(tx->tx_w.w_llx - 3, tx->tx_w.w_lly - 3,
					 x + 3, y + 3);
			recti(tx->tx_w.w_llx - 4, tx->tx_w.w_lly - 4,
					 x + 4, y + 4);
		}
	}

	translatei(tx->tx_w.w_llx, tx->tx_w.w_lly, 0);

    /* now undraw the cursor */
	for (pp = &tx->tx_w.w_piece[0]; pp->p_state != 0; pp++) {
		scrmask(pp->p_xmin, pp->p_xmax, pp->p_ymin, pp->p_ymax);

		tx_undrawcursor(tx);
	}

    /* then redisplay the text */
	x = XLEN(tx->tx_w.w_xlen) - 1;
	lastrp = &tx->tx_display[(short) tx->tx_w.w_ylen];
	for (rp = &tx->tx_display[0]; rp < lastrp; rp++) {
		for (pp = &tx->tx_w.w_piece[0]; pp->p_state != 0; pp++) {

			if (!((scrolled || rp->r_changed)
			 && pp->p_ly <= rp->r_screeny
			 && rp->r_screeny <= pp->p_uy))
				continue;
			scrmask(pp->p_xmin, pp->p_xmax, pp->p_ymin, pp->p_ymax);

			y = rp->r_screeny - DESCENDER;
			color(tx->tx_background);
			rectfi(0, y, x, y + HEIGHT - 1);
			if (rp->r_maxcol == 0)
				continue;
			cmov2i(0, rp->r_screeny);
			if (rp->r_video) {
				cxchar(rp);
			} else {
				color(tx->tx_foreground);
				xcharstr(rp->r_data, rp->r_maxcol);
			}

		}
		rp->r_changed = 0;
	}

	tx->tx_oldcol = tx->tx_col;
	tx->tx_oldrow = tx->tx_row;

    /* now draw new cursor up */
	for (pp = &tx->tx_w.w_piece[0]; pp->p_state != 0; pp++) {
		scrmask(pp->p_xmin, pp->p_xmax, pp->p_ymin, pp->p_ymax);

		tx_drawcursor(tx);
	}

	scrmask(0, XMAXSCREEN, 0, YMAXSCREEN);
	popmatrix();

	if (grproc)
		changemode(1);

	if (shmem_pa)
		sh->IdleMode = 1;
	tx->tx_state &= ~(TX_BUSY | TX_REDISPLAY | TX_DRAWBORDER | TX_SCROLLED);
	if (kgr & KGR_WANTED)
		wakeup((caddr_t)&kgr);
	kgr &= ~(KGR_BUSY | KGR_WANTED | KGR_REPAINTING | KGR_DIDRESET);
}

cxchar(rp)
 	register struct row *rp;
{
	register u_char *cp, *zp, oldcolor;
	register u_char *dp;
	register short span;

	cp = rp->r_color + 0;
	zp = cp + rp->r_maxcol;
	dp = rp->r_data + 0;

	while (cp < zp) {
		oldcolor = *cp;
		for (span = 0; *cp == oldcolor && cp < zp; span++)
			cp++;
		color(oldcolor);
		xcharstr(dp, span);
		dp += span;
	}
}

/*
 * tx_undrawcursor:
 *	- erase the cursor from the current location
 */
tx_undrawcursor(tx)
	register struct textport *tx;
{
	register struct row *rp;
	register short llx, lly;

	rp = &tx->tx_display[tx->tx_oldrow];
	color(tx->tx_background);
	llx = tx->tx_oldcol * WIDTH;
	lly = rp->r_screeny - DESCENDER;
	/* lly = (tx->tx_w.w_ylen - tx->tx_oldrow - 1) * HEIGHT; */
	rectfi(llx, lly, llx + WIDTH - 1, lly + HEIGHT - 1);
	if (tx->tx_oldcol < rp->r_maxcol) {
		if (rp->r_video)
			color(rp->r_color[tx->tx_oldcol]);
		else
			color(tx->tx_foreground);
		cmov2i(llx, lly + DESCENDER);
		xcharstr(&rp->r_data[tx->tx_oldcol], 1);
	}
}

/*
 * tx_drawcursor:
 *	- display cursor at new location
 */
tx_drawcursor(tx)
	register struct textport *tx;
{
	register struct row *rp;
	register short llx, lly;

	rp = &tx->tx_display[tx->tx_row];
	color(tx->tx_cursorcolor);
	llx = tx->tx_col * WIDTH;
	lly = rp->r_screeny - DESCENDER;
	/* lly = (tx->tx_w.w_ylen - tx->tx_row - 1) * HEIGHT; */
	rectfi(llx, lly, llx + WIDTH - 1, lly + HEIGHT - 1);
	if (tx->tx_col < rp->r_maxcol) {
		color(tx->tx_background);
		cmov2i(llx, lly + DESCENDER);
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
	register struct textport *tx;
{
	register struct row *rp;
	register short i, j;

	if (tx->tx_row >= tx->tx_w.w_ylen) {
		tx->tx_oldrow = tx->tx_row = tx->tx_w.w_ylen - 1;
		tx_adjust(tx);
	}
	if (tx->tx_col >= tx->tx_w.w_xlen)
		tx->tx_oldcol = tx->tx_col = tx->tx_w.w_xlen - 1;

	rp = &tx->tx_display[0];
	for (i = 0; i < MAXROWS; i++) {
		rp->r_screeny = HEIGHT * (tx->tx_w.w_ylen - i - 1) + DESCENDER;
		if (i >= tx->tx_w.w_ylen) {
			rp->r_maxcol = 0;
			rp->r_changed = 1;
		}
		rp++;
	}
}
