/*
 * Set the top row that should be displayed on the top of the textview.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tvtoprow.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:48:16 $
 */

#include "tf.h"

void
tvtoprow(tv, rowno)
	textview *tv;
	long rowno;
{
	if (tv->tv_toprow != rowno) {
		tv->tv_toprow = rowno;
		tv->tv_flags |= TV_REDRAW;
	}
}
