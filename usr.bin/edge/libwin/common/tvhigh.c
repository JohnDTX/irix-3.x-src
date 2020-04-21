/*
 * Set highlighting colors.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tvhigh.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:48:04 $
 */

#include "tf.h"

void
tvhighlight(tv, back, fore)
	register textview *tv;
	int back, fore;
{
	tv->tv_selfg = fore;
	tv->tv_selbg = back;
}
