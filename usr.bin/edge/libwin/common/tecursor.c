/*
 * Manage the termulator cursor.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tecursor.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:40 $
 */

#include "tf.h"
#include "te.h"

void
teliftcursor(te)
	termulator *te;
{
	if (te->te_flags & TE_CURSOR) {
		te->te_flags &= ~TE_CURSOR;
		tftouchline(te->te_tf, te->te_toprow + te->te_row);
	}
}

void
tedropcursor(te)
	termulator *te;
{
	long xpix, ypix;
	long index, fg, bg, fontnum, height, width, descender;
	char buf[2];

	if (!(te->te_flags & TE_CURSOR)) {
		/* XXX is this needed with robs tvpostopix? */
		if (te->te_toprow + te->te_row >= tfnumrows(te->te_tf))
			tfmakeline(te->te_tf, te->te_toprow + te->te_row);

		tfsetpoint(te->te_tf, te->te_toprow + te->te_row, te->te_col);
		tvapostopix(te->te_tv, te->te_toprow + te->te_row,
				       te->te_col, &xpix, &ypix);
		tvagetcharinfo(te->te_tv, &index, &fg, &bg, &fontnum,
					  &height, &width, &descender);
		ASSERT(height);
		color(te->te_cursorcolor);
		rectfi(xpix, ypix, xpix + width - 1, ypix + height - 1);
		if (index) {
			color(0);
			font(fontnum);
			buf[0] = index;
			buf[1] = 0;
			cmov2i(xpix, ypix + descender);
			charstr(buf);
		}
		te->te_flags |= TE_CURSOR;
	}
}
