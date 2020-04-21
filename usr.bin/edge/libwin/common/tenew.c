/*
 * Create a new instance of termulator
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tenew.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:48 $
 */

#include "tf.h"
#include "te.h"

termulator *
tenew(tv)
	textview *tv;
{
	register termulator *te;
	register textframe *tf;
	register int i;
	int xsize, ysize;

	te = MALLOC(termulator *, sizeof(termulator));
	if (te) {
		tf = tv->tv_tf;
		te->te_tv = tv;
		te->te_tf = tf;
		te->te_mode = 0;
		te->te_rows = 40;
		te->te_cols = 80;
		te->te_maxrows = 200;
		te->te_toprow = 0;
		te->te_row = 0;
		te->te_col = 0;
		te->te_putchars = termulate_iris;
		te->te_cursorcolor = 2;
		te->te_fgcolor = 7;
		te->te_bgcolor = 0;
		te->te_reversecolor = 3;

		/* initialize color mapping's */
		tvmapindex(tv, 0, 7);		/* slot 0 for text color */
		tvmapindex(tv, 1, 0);		/* slot 1 for page color */
		tvmapindex(tv, 2, 3);		/* slot 2 for reverse color */
		tftextcolor(tf, 0);
		tfbackcolor(tf, 1);
		tvsetbg(tv, 1);
		tvtoprow(tv, 0);

		/* XXX this should be done by the pane manager */
		getsize(&xsize, &ysize);		/* XXX */
		tvviewsize(tv, xsize, ysize);		/* XXX */
	}
	return (te);
}
