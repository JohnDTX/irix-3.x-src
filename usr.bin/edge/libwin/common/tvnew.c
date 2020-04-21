/*
 * Create a new text view.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tvnew.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:48:10 $
 */

#include "tf.h"

textview *
tvnew(tf)
	textframe *tf;
{
	register textview *tv;
	register int i;

	tv = CALLOC(textview *, 1, sizeof(textview));
	if (tv) {
		register struct textfont *tfont;

		tv->tv_tf = tf;

		/* setup default color handles */
		for (i = 0; i < TV_COLOR_HANDLES; i++)
			tv->tv_h[i].index = i;

		/* setup default font handle */
		tfont = &tv->tv_fontinfo[0];
		tfont->font = 0;
		tfont->filename = "default";
		tv_getfontdata(tfont);

		/* set default tab stop at 8 m's */
		tv->tv_tabspace = tfont->widthtab['m'] * 8;
		tv->tv_bg = 7;		/* user color handle 7 */

		/* set default behavior to full-width selection hilites */
		tv->tv_flags |= TV_SELECTTOEDGE;
	}
	return (tv);
}
