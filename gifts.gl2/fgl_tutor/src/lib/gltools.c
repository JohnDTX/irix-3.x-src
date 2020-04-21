#include "mymath.h"
#include "gl.h"
#include "device.h"
#include "stdio.h"

#define DTOA(D)	    ((Angle) (D*10.0))
#define RTOA(R)	    (DTOA(RTOD(R)))

grab_mice(mx, my)
/*---------------------------------------------------------------------------
 * Read the mouse valuator and put the values in mx and my.
 *---------------------------------------------------------------------------
 */
short *mx, *my;
{
    
    *mx = getvaluator(MOUSEX);
    *my = getvaluator(MOUSEY);
}

make_ramp(bi, ei, rb, gb, bb, re, ge, be)
/*---------------------------------------------------------------------------
 * Create a color ramp from index bi with rgb of rb,gr,bb to color index be
 * with rgb of re,ge,be.
 *---------------------------------------------------------------------------
 */
Colorindex bi, ei;
RGBvalue rb, gb, bb, re, ge, be;
{
    register float dr, dg, db;
    register int i;

    dr = ((float) (re - rb)/(float) (ei - bi));
    dg = ((float) (ge - gb)/(float) (ei - bi));
    db = ((float) (be - bb)/(float) (ei - bi));

    for (i = bi ; i <= ei ; i++){
	mapcolor(i, rb, gb, bb);
	rb = (RGBvalue) ((float) rb + dr);
	gb = (RGBvalue) ((float) gb + dg);
	bb = (RGBvalue) ((float) bb + db);
    }
}

centered_charstr(st)
/*---------------------------------------------------------------------------
 * Draws a character string st contered in X and Y about the current character
 * position.
 *---------------------------------------------------------------------------
 */
char *st;
{
    register long slen, shig;
    Screencoord cx, cy;
    Screencoord vl, vr, vb, vt;

    slen = strwidth(st)/2;
    shig = getheight()/2;
    
    getcpos(&cx, &cy);
    getviewport(&vl, &vr, &vb, &vt);
    pushmatrix();
	ortho2((float) vl, (float) vr, (float) vb, (float) vt);
	cmov2s(cx - slen, cy - shig);
	charstr(st);
	cmov2s(cx, cy);
    popmatrix();
}

line2(x1, y1, x2, y2)
/*---------------------------------------------------------------------------
 * Draw a line from (x1, y1) to (x2, y2).
 *---------------------------------------------------------------------------
 */
Coord x1, y1, x2, y2;
{
    move2(x1, y1);
    draw2(x2, y2);
}

line(x1, y1, z1, x2, y2, z2)
/*---------------------------------------------------------------------------
 * Draw a line from (x1, y1) to (x2, y2).
 *---------------------------------------------------------------------------
 */
Coord x1, y1, z1, x2, y2, z2;
{
    move(x1, y1, z1);
    draw(x2, y2, z2);
}

