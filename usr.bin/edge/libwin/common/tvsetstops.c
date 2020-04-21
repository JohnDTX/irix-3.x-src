/*
 * Set tab spacing.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tvsetstops.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:48:15 $
 */

#include "tf.h"

/*
 * Set tab spacing.  If "spacing" is null, then used a fixed spacing
 * constant, passed in "stops".  Otherwise, use a list of preset tab
 * stops, with "stops" number of tab stops, stored in "spacing".
 */
int
tvsetstops(tv, stops, spacing)
	textview *tv;
	long stops;
	long *spacing;
{
	if (spacing) {
		tv->tv_flags |= TV_VARIABLETABS;
		tv->tv_stops = stops;
		tv->tv_spacing = MALLOC(long *, stops * sizeof(long));
		if (!tv->tv_spacing) {
			return (-1);
		}
		BCOPY(spacing, tv->tv_spacing, stops * sizeof(long));
	} else {
		tv->tv_flags &= ~TV_VARIABLETABS;
		tv->tv_tabspace = stops;
	}
	return (0);
}
