/*
 * Map a color handle to a given color index.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tvmapindex.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:48:07 $
 */

#include "tf.h"

void
tvmapindex(tv, colorhandle, index)
	textview *tv;
	int colorhandle, index;
{
	if ((colorhandle < 0) || (colorhandle >= TV_COLOR_HANDLES))
		return;
	tv->tv_h[colorhandle].index = index;
}
