/*
 * Map a color handle to a given rgb value.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tvmaprgb.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:48:08 $
 */

#include "tf.h"

void
tvmaprgb(tv, colorhandle, r, g, b)
	register textview *tv;
	int colorhandle, r, g, b;
{
	if ((colorhandle < 0) || (colorhandle >= TV_COLOR_HANDLES))
		return;
	tv->tv_h[colorhandle].r = r;
	tv->tv_h[colorhandle].g = g;
	tv->tv_h[colorhandle].b = b;
}
