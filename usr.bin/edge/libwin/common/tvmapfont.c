/*
 * Define a font mapping.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tvmapfont.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:48:06 $
 */

#include "tf.h"

void
tvmapfont(tv, fonthandle, fontname)
	textview *tv;
	int fonthandle;
	char *fontname;
{
	register struct textfont *tfont;

	if ((fonthandle < 0) || (fonthandle >= TV_FONT_HANDLES))
		return;
	tfont = &tv->tv_fontinfo[fonthandle];
	if (tfont->valid) {
		/* free width table */
		FREE(tfont->widthtab);

		/* delete old font definition */
		defrasterfont(tfont->font, 0, 0, 0, 0, 0);

		tfont->valid = 0;
	}

	/* save name of font */
	tfont->filename = fontname;
}
