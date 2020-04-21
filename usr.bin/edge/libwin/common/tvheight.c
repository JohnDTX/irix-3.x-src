/*
 * Find the height in pixels of a text line.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tvheight.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:48:04 $
 */

#include "tf.h"

/*
 * Figure out the maximum character height of the given line.
 */
void
tv_findheight(tv, tl)
	register textview *tv;
	register textline *tl;
{
	register struct textfont *tfont;

	if (tl->tl_looks & LOOKS_SAME) {
		tfont = &tv->tv_fontinfo[(tl->tl_looks & LOOKS_FONT)
				>> LOOKS_FONTSHIFT];
		if (!tfont->valid)
			tv_getfontdata(tfont);
		tl->tl_height = tfont->h;
	} else {
		register long *lp;
		register int i;

		if (tl->tl_len == 0) {
			/*
			 * For lines with no data on them, use font 0's height
			 */
			tfont = &tv->tv_fontinfo[0];
			if (!tfont->valid)
				tv_getfontdata(tfont);
			tl->tl_height = tfont->h;
		} else {
			lp = tl->tl_ldata;
			for (i = tl->tl_len; --i >= 0; ) {
				/* extract font info, load it if needed */
				tfont = &tv->tv_fontinfo[(*lp++ & LOOKS_FONT)
						>> LOOKS_FONTSHIFT];
				if (!tfont->valid)
					tv_getfontdata(tfont);
				if (tfont->h > tl->tl_height)
					tl->tl_height = tfont->h;
			}
		}
	}
}
