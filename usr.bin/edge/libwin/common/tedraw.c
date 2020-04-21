/*
 * Update a termulator display.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tedraw.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:42 $
 */

#include "tf.h"
#include "te.h"

void
tedraw(te)
	register termulator *te;
{
	if (te->te_flags & (TE_SCROLLED|TE_REPAINT))
		tvdraw(te->te_tv, 1);
	else
		tvdraw(te->te_tv, 0);
	te->te_flags &= ~(TE_SCROLLED | TE_REPAINT);
}
