/*
 * Move the termulator "cursor".
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/teclearl.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:38 $
 */
#include "tf.h"
#include "te.h"

/*
 * Clear to end of line
 */
void
tecleartoeol(te)
	register termulator *te;
{
	textframe *tf;

	tf = te->te_tf;
	tfsetpoint(tf, te->te_toprow + te->te_row, te->te_col);
	tfsetmark(tf, te->te_toprow + te->te_row, 9999);
	tfdelete(tf);				/* delete to eol */
}
