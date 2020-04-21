/*
 * Scroll bar code
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/bar.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:45:44 $
 */

#include "gl.h"
#include "gsh.h"
#include "window.h"

struct	scrollbar {
	short	barcolor;		/* color of bar */
	short	butcolor;		/* color of buttons */
	rect_t	r;			/* rectangle defining bar */
};

/*
 * Font definition for down and up buttons.  These are upside down, because
 * of the fact that the gl takes the bits from left to right, bottom to top.
 * The font is 11 wide by 12 hi.
 */
#define	FW	11
#define	FH	12
Fontchar arrow_font_chars[] = {
	{ 0,	0,	0,	0,	0,	0, },		/* 0 */
	{ 0,	FW,	FH,	0,	0,	0, },		/* 1 */
	{ 12,	FW,	FH,	0,	0,	0, },		/* 2 */
};
#define	NCHARS	(sizeof(arrow_font_chars) / sizeof(Fontchar))
	
short	arrow_font_rasters[] = {

/* 0000 0.00 000 */	0x0400,		/* down arrow */
/* 0000 ...0 000 */	0x0E00,
/* 000. .... 000 */	0x1F00,
/* 00.. .... .00 */	0x3F80,
/* 0... .... ..0 */	0x7FC0, 
/* .... .... ... */	0xFFE0,
/* 000. .... 000 */	0x1F00,
/* 000. .... 000 */	0x1F00,
/* 000. .... 000 */	0x1F00,
/* 000. .... 000 */	0x1F00,
/* 000. .... 000 */	0x1F00,
/* 000. .... 000 */	0x1F00,

/* 000. .... 000 */	0x1F00,		/* up arrow */
/* 000. .... 000 */	0x1F00,
/* 000. .... 000 */	0x1F00,
/* 000. .... 000 */	0x1F00,
/* 000. .... 000 */	0x1F00,
/* 000. .... 000 */	0x1F00,
/* .... .... ... */	0xFFE0,
/* 0... .... ..0 */	0x7FC0, 
/* 00.. .... .00 */	0x3F80,
/* 000. .... 000 */	0x1F00,
/* 0000 ...0 000 */	0x0E00,
/* 0000 0.00 000 */	0x0400,
};
#define	NRASTERS	(sizeof(arrow_font_rasters) / sizeof(short))

static	struct	scrollbar sb;

/*
 * Initialize the scroll bar
 */
bar_init()
{
	defrasterfont(2, FH, NCHARS, arrow_font_chars,
			 NRASTERS, arrow_font_rasters);
	sb.barcolor = WHITE;
	sb.butcolor = BLACK;
}

bar_color(barcolor, butcolor)
	short barcolor, butcolor;
{
	sb.barcolor = barcolor;
	sb.butcolor = butcolor;
}

bar_redraw(r)
	rect_t *r;
{
	bar_reshape(r);
	bar_paint();
}

bar_reshape(r)
	rect_t *r;
{
	sb.r = *r;
}

/*
 * Paint the scroll bar
 */
bar_paint()
{
	setoutput(&sb.r);

	/* fill in everything but the border */
	color(sb.barcolor);
	rectfi(0, 0, SCROLLBAR_WIDTH, sb.r.ylen - 1);

	/* draw arrows */
	font(2);
	color(sb.butcolor);
	cmov2i(1, 1);
	charstr("\001");			/* up arrow */
	cmov2i(1, sb.r.ylen - 1 - FH);
	charstr("\002");			/* down arrow */

	/* draw lines above arrows */
	move2s(0, 1 + FH + 1);
	draw2s(SCROLLBAR_WIDTH-1, 1 + FH + 1);
	move2s(0, sb.r.ylen - 1 - FH - 1 - 1);
	draw2s(SCROLLBAR_WIDTH-1, sb.r.ylen - 1 - FH - 1 - 1);

	/* revert to old font */
	font(1);
}
