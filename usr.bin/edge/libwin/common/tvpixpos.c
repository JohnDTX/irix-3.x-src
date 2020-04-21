/*
 * Transform a position in pixels to a textcoord.
 *
 * Written by: Kipp Hickman
 *   Rob Myers 5aug86 Mapped hits in tv_leftpix area to column 0
 *   Rob Myers 5aug86 Mapped hits beyond newline to column 0 of next row
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tvpixpos.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:48:10 $
 */

#include "tf.h"

/*
 * Given a line, and an x pixel, find the column that the pixel refers to,
 * if any.  Return -1 if its out of bounds.
 */
long
tv_pixtocol(tv, xpix, tl)
	register textview *tv;
	register long xpix;
	register textline *tl;
{
	register struct textfont *tfont;
	register int x;
	register int off;
	register int newoff;
	register int charcode;
	register int i;
	register long column;

	x = tv->tv_leftpix;
	off = 0;
	column = 0;
	if (x < xpix) {
	    if (tl->tl_looks & LOOKS_SAME) {
		register unsigned char *cp;
		register long *widthtab;

		/*
		 * For lines with identical attributes, cache the width table
		 * pointer now.
		 */
		tfont = &tv->tv_fontinfo[(tl->tl_looks & LOOKS_FONT)
				>> LOOKS_FONTSHIFT];
		if (!tfont->valid)
			tv_getfontdata(tfont);
		widthtab = tfont->widthtab;
		cp = tl->tl_cdata;
		for (i = tl->tl_len; --i >= 0; ) {
			charcode = *cp++;
			if (charcode == '\t')
				newoff = tv_tabcol(tv, off);
			else
				newoff = off + widthtab[charcode];
			if ((x + off <= xpix) && (xpix < x + newoff)) {
			    if (xpix > x + newoff - (widthtab[charcode]>>1))
				column++;
			    break;
			}
			off = newoff;
			column++;
		}
		if (xpix >= x + newoff + widthtab[' '])
		    column = -1;
	    } else {
		register long *lp;
		long l;

		lp = tl->tl_ldata;
		for (i = tl->tl_len; --i >= 0; ) {
			l = *lp++;
			charcode = l & LOOKS_INDEX;
			tfont = &tv->tv_fontinfo[(l & LOOKS_FONT)
						>> LOOKS_FONTSHIFT];
			if (!tfont->valid)
				tv_getfontdata(tfont);
			if (charcode == '\t')
				newoff = tv_tabcol(tv, off);
			else
				newoff = off + tfont->widthtab[charcode];
			if ((x + off <= xpix) && (xpix < x + newoff)) {
			    if (xpix > x+newoff-(tfont->widthtab[charcode]>>1))
				column++;
			    break;
			}
			off = newoff;
			column++;
		}
		if (!tfont) {
		    tfont = &tv->tv_fontinfo[(tl->tl_looks & LOOKS_FONT)
				>> LOOKS_FONTSHIFT];
		    if (!tfont->valid)
			tv_getfontdata(tfont);
		}
		if (xpix >= x + newoff + tfont->widthtab[' '])
		    column = -1;
	    }
	}
	return (column);
}

int
tvpixtopos(tv, xpix, ypix, rowp, colp)
	textview *tv;
	long xpix, ypix;
	long *rowp, *colp;
{
	register textline *tl;
	register int i;
	register long row;
	textframe *tf;
	long firstrow;
	int y;

	/* pessimize a bit */
	*rowp = -1;
	*colp = -1;

	/* make sure pixel is inside boundaries of frame */
	if ((xpix < 0) || (xpix > tv->tv_xsize) ||
	    (ypix < 0) || (ypix > tv->tv_ysize))
		return (0);

	/* fix toprow, if its messed up */
	tf = tv->tv_tf;
	if (tv->tv_toprow >= tf->tf_rows)
		return (0);

	y = tv->tv_ysize;
	tl = tf_findline(tf, tv->tv_toprow, 0);
	if (!tl)
		return (0);
	row = tv->tv_toprow;
	while (tl) {
		if (!tl->tl_height)
			tv_findheight(tv, tl);
		if ((y - tl->tl_height <= ypix) && (ypix < y)) {
			/*
			 * Found the correct row.  Now run through the
			 * row and try to find the column.
			 */
			*rowp = row;
			*colp = tv_pixtocol(tv, xpix, tl);
			if (*colp == -1) {	/* hit beyond newline;*/
			    *rowp = row+1;	/* return beginning   */
			    *colp = 0;		/* of next text line  */
			    if (i==0 && !tl->tl_next) /* no next line */
				return (0);
			}
			return (1);
		}
		y -= tl->tl_height;
		tl = tl->tl_next;
		row++;
	}
	return (0);
}
