/*
 * Get all sorts of info about a given character.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/tvagetchar.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:56 $
 */

#include "tf.h"

void
tvagetcharinfo(tv, index, fg, bg, fontnum, height, width, descender)
	textview *tv;
	long *index, *fg, *bg, *fontnum, *height, *width, *descender;
{
	textframe *tf;
	textline *tl;
	int col;
	long looks;
	long fnt;
	struct textfont *tfont;

	tf = tv->tv_tf;
	tl = tf_findline(tf, tf->tf_point.tc_row, 1);
	if (tl) {
		col = tf->tf_point.tc_col;
		if (!tl->tl_height)
			tv_findheight(tv, tl);
		if (!tl->tl_width)
			tv_findwidth(tv, tl);
		if (tl->tl_looks & LOOKS_SAME) {
			looks = tl->tl_looks;
			if (col < tl->tl_len)
				*index = *(tl->tl_cdata + col);
			else
				*index = 0;
		} else {
			if (col < tl->tl_len)
				looks = *(tl->tl_ldata + col);
			else
				looks = *(tl->tl_ldata + tl->tl_len - 1) &
					~LOOKS_INDEX;
			*index = looks & LOOKS_INDEX;
		}
		*fg = (looks & LOOKS_FG) >> LOOKS_FGSHIFT;
		*bg = (looks & LOOKS_BG) >> LOOKS_BGSHIFT;
		fnt = (looks & LOOKS_FONT) >> LOOKS_FONTSHIFT;
		*height = tl->tl_height;
		tfont = &tv->tv_fontinfo[fnt];
		if (!tfont->valid)
			tv_getfontdata(tfont);
		*fontnum = (unsigned)tfont->font;
		if (col >= tl->tl_len) {
			*width = tfont->widthtab['m'];
		} else
			*width = tfont->widthtab[*index];
		*descender = tfont->d;
	} else {
		*height = 0;
printf("tvagetcharinfo: no line at %d\n", tf->tf_point.tc_row);
	}
}
