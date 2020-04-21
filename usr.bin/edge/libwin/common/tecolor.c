/*
 * Set the termulator cursor color.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tecolor.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:39 $
 */

#include "tf.h"
#include "te.h"

void
tesetcursorcolor(te, c)
	termulator *te;
	long c;
{
	te->te_cursorcolor = c;
}
