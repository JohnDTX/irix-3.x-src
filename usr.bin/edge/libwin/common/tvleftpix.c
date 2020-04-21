/*
 * Set leftmost pixel to display.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tvleftpix.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:48:06 $
 */

#include "tf.h"

void
tvleftpix(tv, leftpix)
	textview *tv;
	long leftpix;
{
	tv->tv_leftpix = leftpix;
}
