/*
 * Get tab settings.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tvgetstops.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:48:03 $
 */

#include "tf.h"

long
tvgetstops(tv, spacing)
	textview *tv;
	long *spacing;
{
	if (tv->tv_flags & TV_VARIABLETABS) {
		if (spacing)
			BCOPY(tv->tv_spacing, spacing,
					      tv->tv_stops * sizeof(long));
		return (tv->tv_stops);
	}
	return (tv->tv_tabspace);
}
