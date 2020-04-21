/*
 * Find the width in pixels of a text line.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tvwidth.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:48:18 $
 */

#include "tf.h"

/*
 * Figure out the maximum character width of the given line.
 */
void
tv_findwidth(tv, tl)
	register textview *tv;
	register textline *tl;
{
	register int charcode;
	register int width;
	register int i;

	width = 0;
	if (tl->tl_looks & LOOKS_SAME) {
		register long *widthtab;
		register unsigned char *cp;
		register struct textfont *tfont;

		tfont = &tv->tv_fontinfo[(tl->tl_looks & LOOKS_FONT)
				>> LOOKS_FONTSHIFT];
		if (!tfont->valid)
			tv_getfontdata(tfont);
		widthtab = tfont->widthtab;
		cp = tl->tl_cdata;
		for (i = tl->tl_len; --i >= 0; ) {
			charcode = *cp++;
			if (charcode == '\t')
				width = tv_tabcol(tv, width);
			else
				width += widthtab[charcode];
		}
	} else {
		register struct textfont *tfont;
		register long *lp;
		register long l;

		lp = tl->tl_ldata;
		for (i = tl->tl_len; --i >= 0; ) {
			l = *lp++;

			/* extract font info, load it if needed */
			tfont = &tv->tv_fontinfo[(l & LOOKS_FONT)
					>> LOOKS_FONTSHIFT];
			if (!tfont->valid)
				tv_getfontdata(tfont);
			charcode = l & LOOKS_INDEX;
			if (charcode == '\t')
				width = tv_tabcol(tv, width);
			else
				width += tfont->widthtab[charcode];
		}
	}
	tl->tl_width = width;
}
