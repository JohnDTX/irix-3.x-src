/*
 * Set the text view viewing size.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tvview.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:48:17 $
 */

#include "tf.h"

void
tvviewsize(tv, xsize, ysize)
	textview *tv;
	long xsize;
	long ysize;
{
	tv->tv_xsize = xsize;
	tv->tv_ysize = ysize;
	tv->tv_flags |= TV_REDRAW;
}
