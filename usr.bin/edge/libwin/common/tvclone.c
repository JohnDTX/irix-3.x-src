/*
 * Clone a text view.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tvclone.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:58 $
 */

#include "tf.h"

textview *
tvclone(tv)
	textview *tv;
{
	textview *ntv;

	ntv = MALLOC(textview *, sizeof(textview));
	if (ntv) {
		BCOPY(tv, ntv, sizeof(*ntv));
	}
	return (ntv);
}
