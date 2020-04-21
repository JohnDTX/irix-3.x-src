/*
 * Textview draw code.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tvdraw.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:59 $
 */

#include "tf.h"

#define	UNIX
#ifdef mips
#include "gl/globals.h"
#include "gl/TheMacro.h"
#include "gl/imdraw.h"
#include "gl/imattrib.h"
#include "gl/glerror.h"
#else
#include "gl2/globals.h"
#include "gl2/TheMacro.h"
#include "gl2/imdraw.h"
#include "gl2/imattrib.h"
#include "gl2/glerror.h"
#endif

#ifdef	mips
#define	USE_C_CODE
#endif
#ifdef	DEBUG
#define	USE_C_CODE
#endif

/*
 * Global variables to speed execution.  Too bad this code isn't
 * reentrant, eh?
 */
static	int	lastfont;
static	long	x;
static	long	y;

/*
 * Figure out where to move the character position for the given
 * tab column
 */
long
tv_tabcol(tv, current)
	register textview *tv;
	long current;
{
	register int i;

	if (tv->tv_flags & TV_VARIABLETABS) {
		register long *spacing;

		/*
		 * Find tab stop, by scanning list of stops and looking
		 * for one that is greater than the current column.
		 * If we hit the end of the list, then do nothing - just
		 * like a typewriter.
		 */
		spacing = tv->tv_spacing;
		for (i = tv->tv_stops; --i >= 0; spacing++) {
			if (*spacing > current)
				return (*spacing);
		}
	} else
		current += (tv->tv_tabspace - (current % tv->tv_tabspace));
	return (current);
}

/*
 * Get data for a particular font.
 */
tv_getfontdata(tfont)
	register struct textfont *tfont;
{
	register long *lptr;
	register int i;
	char b[2];
	short w, h, d;
	static int fontnum = 90000;

	if (fntload(tfont->filename, fontnum)) {
		/*
		 * Couldn't load font.  Use default one.
		 */
		tfont->font = 0;
	} else
		tfont->font = fontnum;
	font(tfont->font);

	/* get descender and height info */
	gl_getcharinfo(&w, &h, &d);
	tfont->h = h;
	tfont->d = d;

	/*
	 * Get width information.  This code was gleefully stolen from
	 * Paul Haeberli. (7/7/86).
	 */
	lptr = MALLOC(long *, 256 * sizeof(long));
	if (!lptr) {
		/*
		 * XXX This is wrong. XXX
		 */
		printf("can't load font %s -- out of memory\n",
			      tfont->filename);
		exit(-1);
	}
	tfont->widthtab = lptr;

	/*
	 * Use strwidth to get the width of each character in the font.
	 */
	b[1] = 0;
	for (i = 0; i < 256; i++) {
		if ((i == 0) || (i >= 128)) {
			*lptr++ = 0;
		} else {
			b[0] = i;
			*lptr++ = strwidth(b); 
		}
	}
	tfont->valid = 1;
	fontnum++;
}

#ifdef	USE_C_CODE
long
tv_drawfast(tv, tl)
	textview *tv;
	textline *tl;
{
	register unsigned char *cp;
	register long col, newcol;
	register long *widthtab;
	register struct textfont *tfont;
	register int i;
	unsigned char charcode[2];
	int fg;
	int bg;
	int fnt;
	short chary;

	charcode[1] = 0;

	/* crack looks */
	fnt = (tl->tl_looks & LOOKS_FONT) >> LOOKS_FONTSHIFT;
	if (tl->tl_looks & LOOKS_SELECT) {
		fg = tv->tv_selfg;
		bg = tv->tv_selbg;
	} else {
		fg = (tl->tl_looks & LOOKS_FG) >> LOOKS_FGSHIFT;
		bg = (tl->tl_looks & LOOKS_BG) >> LOOKS_BGSHIFT;
	}

	/* extract font info from looks, and get font going */
	tfont = &tv->tv_fontinfo[fnt];
	if (!tfont->valid)
		tv_getfontdata(tfont);
	if (tfont->font != lastfont) {
		lastfont = tfont->font;
		font(tfont->font);
	}
	chary = y + tfont->d;
	widthtab = tfont->widthtab;

	{
		int x1;
		int y1 = y + tl->tl_height - 1;
		int x2 = x + tv->tv_xsize - 1;

		if ((tl->tl_looks & LOOKS_SELECT) &&
		    (tv->tv_flags & TV_SELECTTOEDGE)) {
			/* erase old line contents */
			color(tv->tv_h[bg].index);
			x2 = x + tv->tv_xsize - 1;
			rectfi(0, y, x2, y1);
		}
		else {
			/* erase left margin */
			color(tv->tv_h[tv->tv_bg].index);
			x1 = x - 1;
			rectfi(0, y, x1, y1);

			/* erase old line contents */
			color(tv->tv_h[bg].index);
			x1 = x + tl->tl_width - 1;
			rectfi(x, y, x1, y1);

			/* erase from end of line to end of view */
			color(tv->tv_h[tv->tv_bg].index);
			x1++;
			x2 = x + tv->tv_xsize - 1;
			rectfi(x1, y, x2, y1);
		}
	}

	/* lets paint up the new data */
	/* XXX fix this to do rle */
	cmov2i(x, chary);
	color(tv->tv_h[fg].index);
	col = 0;
	cp = tl->tl_cdata;
	for (i = tl->tl_len; --i >= 0; ) {
		charcode[0] = *cp++;
		if (charcode[0] == '\t') {
			if (tv->tv_flags & TV_VARIABLETABS)
				newcol = tv_tabcol(tv, col);
			else
				newcol = col + (tv->tv_tabspace -
						(col % tv->tv_tabspace));
			cmov2i(x + newcol, chary);
			col = newcol;
		} else {
			charstr(charcode);
			col += widthtab[charcode[0]];
		}
	}
}

#else

long
tv_drawfast(tv, tl)
	textview *tv;
	textline *tl;
{
	register unsigned short *GE = &GEPORT;		/* a5 */
	register windowstate *WS = gl_wstatep;		/* a4 */
	register unsigned char *cp;			/* a3 */
	register long passcode = 0x0408001c;		/* d7 */
	register long fc;				/* d6 */
	register int charcode;				/* d5 */
	register long col, newcol;			/* d4, d3 */
	register long *widthtab;			/* a2 */
	register struct textfont *tfont;		/* ... */
	register int i;					/* d2 */
	int fg;
	int bg;
	int fnt;
	short chary;

	/* crack looks */
	fnt = (tl->tl_looks & LOOKS_FONT) >> LOOKS_FONTSHIFT;
	if (tl->tl_looks & LOOKS_SELECT) {
		fg = tv->tv_selfg;
		bg = tv->tv_selbg;
	} else {
		fg = (tl->tl_looks & LOOKS_FG) >> LOOKS_FGSHIFT;
		bg = (tl->tl_looks & LOOKS_BG) >> LOOKS_BGSHIFT;
	}

	/* extract font info from looks, and get font going */
	tfont = &tv->tv_fontinfo[fnt];
	if (!tfont->valid)
		tv_getfontdata(tfont);
	if (tfont->font != lastfont) {
		lastfont = tfont->font;
		font(tfont->font);
	}
	chary = y + tfont->d;
	widthtab = tfont->widthtab;

	{
		int x1;
		int y1 = y + tl->tl_height - 1;
		int x2 = x + tv->tv_xsize - 1;

		if ((tl->tl_looks & LOOKS_SELECT) &&
		    (tv->tv_flags & TV_SELECTTOEDGE)) {
			/* erase old line contents */
			im_color(tv->tv_h[bg].index);
			x2 = x + tv->tv_xsize - 1;
			im_rectfi(0, y, x2, y1);
		}
		else {
			/* erase left margin */
			im_color(tv->tv_h[tv->tv_bg].index);
			x1 = x - 1;
			im_rectfi(0, y, x1, y1);

			/* erase old line contents */
			im_color(tv->tv_h[bg].index);
			x1 = x + tl->tl_width - 1;
			im_rectfi(x, y, x1, y1);

			/* erase from end of line to end of view */
			im_color(tv->tv_h[tv->tv_bg].index);
			x1++;
			x2 = x + tv->tv_xsize - 1;
			im_rectfi(x1, y, x2, y1);
		}
	}

	/* lets paint up the new data */
	im_cmov2i(x, chary);
	im_color(tv->tv_h[fg].index);
	col = 0;
	fc = (long)(WS->curatrdata.currentfont->chars);
	im_outfontbase(WS->fontbase);
	cp = tl->tl_cdata;
	for (i = tl->tl_len; --i >= 0; ) {
		charcode = *cp++;
		if (charcode == '\t') {
			if (tv->tv_flags & TV_VARIABLETABS)
				newcol = tv_tabcol(tv, col);
			else
				newcol = col + (tv->tv_tabspace -
						(col % tv->tv_tabspace));
/*
 * For some reason, the code below doesn't work...  Turn on RELCHARPOS
 * once it does...
 */
#undef	RELCHARPOS
#ifdef	RELCHARPOS
			im_outshort(0x308);
			im_outshort(FBCcharposnrel);
			im_outshort(GEpoint);
			im_outshort(newcol-col);
			im_outshort(0);
#else
			/* cmov2i(x + newcol, chary); */
			im_cmov2i(x + newcol, chary);
#endif
			col = newcol;
		} else {
#ifdef	USE_C_CODE
			im_outlong(passcode);
			WS = (windowstate *)(fc + (charcode<<3));
			im_outlong(*(long *)WS);
			im_outlong(*(((long *)WS)+1));
			col += widthtab[charcode];
#else	/* USE_C_CODE */
#ifdef	IP2
			/*
			 * This is carefully coded to interleave 68020
			 * instructions with writes to the pipe, so as
			 * to maximize parallel execution.
			 */
			/* im_outlong(passcode); */
			    asm("	movl	d7, a5@");
			/* a0 = (long *)(fc + (charcode << 3)); */
			    asm("	movl	d6, a0");
			    asm("	lea	a0@(0, [d5:w*8]), a0");
			/* im_outlong(*a0++); */
			    asm("	movl	a0@+, a5@");
			/* col += widthtab[charcode]; */
			    asm("	addl	a2@(0, [d5:w*4]), d4");
			/* im_outlong(*a0); */
			    asm("	movl	a0@, a5@");
#else	/* IP2 */
			/* im_outlong(passcode); */
			    asm("	movl	d7, a5@");
			/* a0 = (long *)(fc + (charcode << 3)); */
			    asm("	movl	d5, d0");
			    asm("	asll	#3, d0");
			    asm("	addl	d6, d0");
			    asm("	movl	d0, a0");
			/* im_outlong(*a0++); */
			    asm("	movl	a0@+, a5@");
			/* im_outlong(*a0); */
			    asm("	movl	a0@, a5@");
			/* col += widthtab[charcode]; */
			    asm("	movw	d5, d0");
			    asm("	aslw	#2, d0");
			    asm("	movl	a2, a0");
			    asm("	addw	d0, a0");
			    asm("	addl	a0@, d4");
#endif	/* IP2 */
#endif	/* USE_C_CODE */
		}
	}

	WS = gl_wstatep;
	im_outfontbase(WS->fontrambase);
	im_freepipe;
	im_cleanup;
}
#endif

/*
 * Slower version of line draw code.  Has to potentially change font/color
 * on every character.
 */
long
tv_drawslow(tv, tl)
	register textview *tv;
	textline *tl;
{
	register struct textfont *tfont;
	register long *lp;
	register long l;
	register int fnt, fg, bg;
	register int charcode;
	register int i;
	register long w;
	register int col;
	char b[2];

	/*
	 * Compute maximum height of a line, if it hasn't already been
	 * computed.
	 */
	b[1] = 0;
	lp = tl->tl_ldata;
	col = 0;

	/* paint background piece to the left of the line (left margin) */
	if (x > 0) {
		if (tl->tl_len && (*lp & LOOKS_SELECT) &&
		    (tv->tv_flags & TV_SELECTTOEDGE))
			color(tv->tv_selbg);
		else color(tv->tv_h[tv->tv_bg].index);
		rectfi(0, y, x - 1, y + tl->tl_height - 1);
	}

	for (i = tl->tl_len; --i >= 0; ) {
		l = *lp++;
		charcode = l & LOOKS_INDEX;
		fnt = (l & LOOKS_FONT) >> LOOKS_FONTSHIFT;
		if (l & LOOKS_SELECT) {
			fg = tv->tv_selfg;
			bg = tv->tv_selbg;
		} else {
			fg = (l & LOOKS_FG) >> LOOKS_FGSHIFT;
			bg = (l & LOOKS_BG) >> LOOKS_BGSHIFT;
		}

		/* extract font info from looks, and get font going */
		tfont = &tv->tv_fontinfo[fnt];
		if (!tfont->valid)
			tv_getfontdata(tfont);
		if (tfont->font != lastfont) {
			lastfont = tfont->font;
			font(tfont->font);
		}

		/* draw character */
		if (charcode == '\t') {
			register int newcol;

			if (tv->tv_flags & TV_VARIABLETABS)
				newcol = tv_tabcol(tv, col);
			else
				newcol = col + (tv->tv_tabspace -
						(col % tv->tv_tabspace));
			/* erase tab region */
			color(tv->tv_h[bg].index);
			rectfi(x + col, y, x + newcol - 1, y + tl->tl_height - 1);
			col = newcol;
		} else {
			/* erase field behind character */
			color(tv->tv_h[bg].index);
			w = tfont->widthtab[charcode];
			rectfi(x + col, y, x + col + w - 1, y + tl->tl_height - 1);

			/* plop character up */
			cmov2i(x + col, y + tfont->d);
			b[0] = charcode;
			color(tv->tv_h[fg].index);
			col += w;
			charstr(b);
		}
	}
	/* paint background piece past end of line to end of view */
	if (x + tl->tl_width < x + tv->tv_xsize) {
		if (tl->tl_len && (*--lp & LOOKS_SELECT) &&
		    (tv->tv_flags & TV_SELECTTOEDGE))
			color(tv->tv_selbg);
		else color(tv->tv_h[tv->tv_bg].index);
		rectfi(x + tl->tl_width, y,
			x + tv->tv_xsize - 1, y + tl->tl_height - 1);
	}
}

void
tvdraw(tv, how)
	register textview *tv;
	int how;
{
	register textline *tl;
	register textframe *tf;
	register int i;
	register long row;
	long firstrow;
	char didsomething;

	didsomething = 0;
	tf = tv->tv_tf;
	if (tv->tv_flags & TV_REDRAW)
		how = 1;

	pushmatrix();
	pushviewport();
	viewport(0, tv->tv_xsize - 1, 0, tv->tv_ysize - 1);
	ortho2(-0.5, tv->tv_xsize - 0.5, -0.5, tv->tv_ysize - 0.5);
	lastfont = -1;
	x = tv->tv_leftpix;
	y = tv->tv_ysize;
	tl = tf_findline(tf, tv->tv_toprow, 0);
	if (!tl) {
		goto out;
	}
	row = firstrow;
	while (tl) {
		if (how || tl->tl_dirty || !tl->tl_height) {
			if (!tl->tl_height)
				tv_findheight(tv, tl);
			if (!tl->tl_width)
				tv_findwidth(tv, tl);
			y -= tl->tl_height;
			if (tl->tl_looks & LOOKS_SAME)
				tv_drawfast(tv, tl);
			else
				tv_drawslow(tv, tl);

			didsomething = 1;
			tl->tl_dirty = 0;
		} else {
			y -= tl->tl_height;
		}

		/* if run off the bottom of the window, stop */
		if (y <= 0)
			goto out;
		tl = tl->tl_next;
	}

	if ((how || didsomething) && (y >= 0)) {
		color(tv->tv_h[tv->tv_bg].index);
		rectfi(0, 0, x + tv->tv_xsize - 1, y - 1);
	}

out:
	popviewport();
	tv->tv_flags &= ~TV_REDRAW;
	popmatrix();
}
