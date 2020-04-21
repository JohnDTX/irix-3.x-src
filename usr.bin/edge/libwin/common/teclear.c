/*
 * Clear to end-of-frame.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/teclear.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:37 $
 */
#include "tf.h"
#include "te.h"

/*
 * Clear to end of frame
 */
void
tecleartoeof(te)
	register termulator *te;
{
	register textframe *tf;
	register int i;

	tf = te->te_tf;
	for (i = te->te_row; i < te->te_rows; i++) {
		tfsetpoint(tf, te->te_toprow + i, 0);
		tfsetmark(tf, te->te_toprow + i, 9999);
		tfdelete(tf);			/* delete contents of line */
	}
}
