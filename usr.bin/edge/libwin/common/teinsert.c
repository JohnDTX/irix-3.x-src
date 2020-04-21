/*
 * Move the termulator "cursor".
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/teinsert.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:45 $
 */
#include "tf.h"
#include "te.h"

/*
 * Insert a clean line at the current location.
 */
void
teinsertln(te)
	register termulator *te;
{
	register textframe *tf;

	tf = te->te_tf;
	tfsetpoint(tf, te->te_toprow + te->te_rows - 1, 0);
	tfsetmark(tf, te->te_toprow + te->te_rows, 0);
	tfdelete(tf);				/* delete last line */
	tfsetpoint(tf, te->te_toprow + te->te_row, 0);
	tfsplit(tf);
/*	tfputascii(tf, "\n", 1);		/* insert clean line */
	te->te_flags |= TE_SCROLLED;		/* XXX fix tf code instead */
}
