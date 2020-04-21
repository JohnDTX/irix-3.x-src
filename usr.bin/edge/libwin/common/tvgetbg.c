/*
 * Get the textview bg handle.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tvgetbg.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:48:01 $
 */

#include "tf.h"

int
tvgetbg(tv)
	textview *tv;
{
	return (tv->tv_bg);
}
