/*
 * Return the height (in pixels) of a given row.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tvrowh.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:48:13 $
 */

#include "tf.h"

long
tvrowheight(tv, row)
	textview *tv;
	int row;
{
	textline *tl;

	tl = tf_findline(tv->tv_tf, row, 0);
	if (tl) {
		if (!tl->tl_height)
			tv_findheight(tv, tl);
		return (tl->tl_height);
	}
	return (0);
}
