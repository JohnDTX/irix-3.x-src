/*
 * Destroy a text view.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tvfree.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:48:00 $
 */

#include "tf.h"

void
tvfree(tv)
	textview *tv;
{
	FREE(tv);
}
