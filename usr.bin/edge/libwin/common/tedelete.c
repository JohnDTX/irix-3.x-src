/*
 * Move the termulator "cursor".
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tedelete.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:42 $
 */
#include "tf.h"
#include "te.h"

/*
 * Delete the current line
 */
void
tedeleteln(te)
	register termulator *te;
{
	register textframe *tf;

	tf = te->te_tf;
	tfsetpoint(tf, te->te_toprow + te->te_row, 0);
	tfsetmark(tf, te->te_toprow + te->te_row + 1, 0);
	tfdelete(tf);				/* delete current line */
	tfsetpoint(tf, te->te_toprow + te->te_rows - 1, 0);
	tfsplit(tf);
/*	tfputascii(tf, "\n", 1);		/* insert clean line */
	te->te_flags |= TE_SCROLLED;		/* XXX fix tf code instead */
}
