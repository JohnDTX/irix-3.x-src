/*
 * Update a termulator display.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/teredraw.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:50 $
 */

#include "tf.h"
#include "te.h"

void
teredraw(te)
	register termulator *te;
{
	int xsize, ysize;
	extern short font_width, font_height;
	int newrows, newcols;

	getsize(&xsize, &ysize);
	newrows = ysize / font_height;			/* XXX */
	newcols = xsize / font_width;			/* XXX */

	/*
	 * Adjust emulator's contents, if the bottom line no longer fits in
	 * the new sized window.
	 */
	if (newrows < te->te_rows) {
		if (te->te_row >= newrows)
			te->te_row = newrows - 1;
		/* XXX do something appropriate here */
	}

	te->te_rows = newrows;
	te->te_cols = newcols;
	if (te->te_col >= te->te_cols)
		te->te_col = te->te_cols - 1;

	te->te_flags &= ~(TE_SCROLLED | TE_REPAINT | TE_CURSOR);
	tvviewsize(te->te_tv, xsize, ysize);
	tvdraw(te->te_tv, 1);
	tedropcursor(te);
}
