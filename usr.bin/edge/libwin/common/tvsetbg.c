/*
 * Set the textview bg handle.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tvsetbg.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:48:14 $
 */

#include "tf.h"

void
tvsetbg(tv, handle)
	textview *tv;
	int handle;
{
	tv->tv_bg = handle;
}
