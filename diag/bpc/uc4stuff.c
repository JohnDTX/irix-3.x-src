/*
 *	Kurt Akeley
 *	15 August 1984
 *	Routines for the uc4 only.
 */

#ifdef UC4

#include <ucdev.h>
#include "uctest.h"
#include "console.h"

filltrap (x, y, width, height, topx, topwidth, fontaddr)
long x, y, width, height, topx, topwidth, fontaddr;
{
   /*
    *	Uses the DDA hardware to fill a trapezoid.  First version does not
    *	  bloat to simulate a bresenham fill.
    */
    long x1, x2, delta, dx;
    Save s;

    save (&s);
    LDXS (x);
    LDXE (x+width-1);
    LDYS (y);
    LDYE (y+height-1);
    LDFMADDR (fontaddr);
    if (height == 0) {
	REQUEST (UC_FILLRECT, 0);
	return;
	}
    LDDDASAF (0);
    LDDDASAI (x);
    LDDDAEAF (0);
    LDDDAEAI (x+width-1);
	/* compute start delta */
    x1 = x << 12;
    x2 = topx << 12;
    delta = (height==1) ? 0 : (x2-x1) / (height-1);
    LDDDASDF (delta);
    LDDDASDI (delta>>12);
	/* compute end delta */
    x1 = (x+width-1) << 12;
    x2 = (topx+topwidth-1) << 12;
    delta = (height==1) ? 0 : (x2-x1) / (height-1);
    LDDDAEDF (delta);
    LDDDAEDI (delta>>12);
	/* draw the trapezoid */
    REQUEST (UC_FILLTRAP, 0);
    restore (&s);
    }

depthcue (x0, y0, x1, y1, stipple, color0, color1)
long x0, y0, x1, y1, stipple, color0, color1;
{
    /*
     *	Draw a depth cued line from x0,y0 to x1,y1, iterating color from
     *	  color0 to color1.  Uses the DDA hardware of the uc4.
     */

    short dx, dy, absdx, absdy;
    long length, delta;
    Save s;

    save (&s);

    LDCONFIG (s.cfb | UC_LDLINESTIP)
    REQUEST (UC_NOOP, stipple)
    LDCONFIG (s.cfb & ~UC_LDLINESTIP)

    LDMODE (s.mdb | UC_DEPTHCUE)
    LDXS (x0)
    LDYS (y0)

    dx    = x1 - x0;
    absdx = (dx >= 0) ? dx : -dx;
    dy    = y1 - y0;
    absdy = (dy >= 0) ? dy : -dy;

    length = (absdy >= absdx) ? absdy : absdx;
    LDDDASAF (0)
    LDDDASAI (color0)
    if (length > 0) {
	delta = ((color1-color0) << 12) / length;
	LDDDASDF (delta)
	LDDDASDI (delta>>12)
	}

    if (dy >= 0) {
	if (absdy >= absdx) {
	    LDYE (y1)
	    LDED (-absdx)
	    LDEC ( absdy)
	    if (dx >= 0)
		REQUEST (DRAWLINE1, color0)
	    else
		REQUEST (DRAWLINE11, color0)
	    }
	else {
	    LDXE (x1)
	    LDED (-absdy)
	    LDEC ( absdx)
	    if (dx >= 0)
		REQUEST (DRAWLINE2, color0)
	    else
		REQUEST (DRAWLINE10, color0)
	    }
	}
    else {
	if (absdy >= absdx) {
	    LDYE (y1)
	    LDED (-absdx)
	    LDEC ( absdy)
	    if (dx >= 0)
		REQUEST (DRAWLINE5, color0)
	    else
		REQUEST (DRAWLINE7, color0)
	    }
	else {
	    LDXE (x1)
	    LDED (-absdy)
	    LDEC ( absdx)
	    if (dx >= 0)
		REQUEST (DRAWLINE4, color0)
	    else
		REQUEST (DRAWLINE8, color0)
	    }
	}
    restore (&s);
    }

#endif UC4
