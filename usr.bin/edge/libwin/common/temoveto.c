/*
 * Move the termulator "cursor".
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/temoveto.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:47 $
 */
#include "tf.h"
#include "te.h"

/*
 * Move the cursor to the given line
 */
void
temoveto(te, row, col)
	register termulator *te;
	int row, col;
{
	register textframe *tf;

	tf = te->te_tf;
	if (row >= te->te_rows) {
		/*
		 * Need to scroll view.  See if we need to scroll the frame.
		 */
		if (te->te_toprow + row < te->te_maxrows) {
			/*
			 * No need to scroll yet.  Just roll the view one
			 * line.
			 */
			te->te_toprow++;
			tvtoprow(te->te_tv, te->te_toprow);
		} else {
			/*
			 * Frame is "full".  Delete top line in frame, and
			 * add a new line at the bottom.  Don't need to
			 * adjust tvtoprow because the frame never moves
			 * from the view's point of view.
			 */
			/* delete old line from top */
			tfsetpoint(tf, 0, 0);
			tfsetmark(tf, 1, 0);
			tfdelete(tf);
			/* add new line to end */
			tfsetpoint(tf, te->te_toprow + te->te_rows - 1, 0);
			tfsplit(tf);
/*			tfputascii(tf, "\n", 1); */
			te->te_row = te->te_rows - 1;
			te->te_flags |= TE_SCROLLED;
		}
	} else {
		te->te_row = row;
	}
	te->te_col = col;
}

tescroll(te, how)
	termulator *te;
	int how;
{
	if (how) {
		if (te->te_toprow < te->te_maxrows) {
			te->te_toprow++;
			tvtoprow(te->te_tv, te->te_toprow);
		}
	} else {
		if (te->te_toprow > 0) {
			te->te_toprow--;
			tvtoprow(te->te_tv, te->te_toprow);
		}
	}
printf("scrolling %s - new toprow=%d\n", how ? "up" : "down", te->te_toprow);

	/* update display at new position */
	teliftcursor(te);
	tedraw(te);
	tedropcursor(te);
}
