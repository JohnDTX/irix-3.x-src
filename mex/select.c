/*
 * Window manager
 *
 * Written by: Kipp Hickman
 *
 */
#include "win.h"
#include "stdio.h"
#include "gltypes.h"
#include "misc.h"
#include "window.h"
#include "cursors.h"

#define	MAX(a, b)	((a) > (b) ? (a) : (b))
#define	MIN(a, b)	((a) < (b) ? (a) : (b))

#define XMIN	((2*BORDERWIDTH)-XMAXSCREEN)
#define XMAX	((2*XMAXSCREEN) - (2*BORDERWIDTH))

/* yea YMAX and YMIN are sorta like XMIN XMAX 
 * this is to handle windows that are partially offscreen 
 */

#define YMIN	((2*BORDERWIDTH)-XMAXSCREEN)
#define YMAX	(dragging->w_title[0] ? \
			((2*XMAXSCREEN) - (2*BORDERWIDTH) - TITLEHEIGHT) : \
			((2*XMAXSCREEN) - (2*BORDERWIDTH)))

short dragboxdrawn = 0;
short dragoffsetx, dragoffsety;

wn_drawdragbox()
{
    register struct wm_window *w = dragging;
    register int ymax;

    if(!dragboxdrawn)
	initdrag(w);
    fixrect();
    cursoff();
    writemask(cursormask);
    color(cursorcolor);
    recti(dragx1, dragy1, dragx2, dragy2);
    if(w->w_title[0]) {
	ymax = MAX(dragy1,dragy2);
	recti(dragx1, ymax, dragx2, ymax+TITLEHEIGHT);
    }
    writemask(drawmask);
    curson();
}

wn_undrawdragbox()
{
    register struct wm_window *w = dragging;
    register int ymax;

    if(w->knowposition)
	dragboxdrawn = 0;	/* next draw box needs to reset offsets */
    cursoff();
    writemask(cursormask);
    color(0);
    recti(dragx1, dragy1, dragx2, dragy2);
    if(w->w_title[0]) {
	ymax = MAX(dragy1,dragy2);
	recti(dragx1, ymax, dragx2, ymax+TITLEHEIGHT);
    }
    writemask(drawmask);
    curson();
}

initdrag(w)
register struct wm_window *w;
{
    register struct portreq *pr = &w->w_pr;

/* dragx1, dragy1 set to initial values outside of this routine */

    dragx1 = MIN(dragx1, XMAX);
    dragx1 = MAX(dragx1, XMIN);
    dragy1 = MIN(dragy1, YMAX);
    dragy1 = MAX(dragy1, YMIN);
    dragx2 = dragx1;
    dragy2 = dragy1;
    if(w->knowsize) {
	dragx1 = w->w_xmin;
	dragx2 = w->w_xmax;
	dragy1 = w->w_ymin;
	dragy2 = w->w_ymax;
	dragoffsetx = mousex - dragx1;
	dragoffsety = mousey - dragy1;
    }
    if(pr->preforigin) {
	dragx1 = pr->preforiginx;
	dragy1 = pr->preforiginy;
	if(pr->prefsize) {
	    dragx1 = MIN(pr->preforiginx,XMAX-pr->prefsizex+1);
	    dragy1 = MIN(pr->preforiginy,YMAX-pr->prefsizey+1);
	    dragx2 = dragx1 + pr->prefsizex - 1;
	    dragy2 = dragy1 + pr->prefsizey - 1;
	    w->knowsize = 1;
	    w->knowposition = 1;
	} else if(w->knowsize)
	    w->knowposition = 1;
    }
    if(pr->prefsize) {
	pr->prefsizex = MIN(pr->prefsizex,XMAXSCREEN+1);
	pr->prefsizey = MIN(pr->prefsizey,YMAXSCREEN+1);
	pr->minsizex = MIN(pr->minsizex,pr->prefsizex);
	pr->minsizey = MIN(pr->minsizey,pr->prefsizey);
	pr->maxsizex = MAX(pr->maxsizex,pr->prefsizex);
	pr->maxsizey = MAX(pr->maxsizey,pr->prefsizey);
	if (dragx1 + pr->prefsizex > XMAX + 1)
		dragx1 = XMAX - pr->prefsizex + 1;
	dragx2 = dragx1 + pr->prefsizex - 1;
	if (dragy1 + pr->prefsizey > YMAX + 1)
		dragy1 = YMAX - pr->prefsizey + 1;
	dragy2 = dragy1 + pr->prefsizey - 1;
	dragoffsetx = mousex - dragx1;
	dragoffsety = mousey - dragy1;
	w->knowsize = 1;
    }
    if(pr->xunit > 1)
	dragx2 -= (dragx2 - dragx1 - pr->xunitfudge + 1) % pr->xunit;
    if(pr->yunit > 1)
	dragy2 -= (dragy2 - dragy1 - pr->yunitfudge + 1) % pr->yunit;
    pr->minsizex = MIN(pr->minsizex,pr->maxsizex);
    pr->minsizey = MIN(pr->minsizey,pr->maxsizey);
    dragboxdrawn = 1;
}

fixrect()
{
    register struct portreq *pr = &dragging->w_pr;
    short width, height;

    if(dragging->knowposition)
	return;
    else if(dragging->knowsize) {
	width = dragx2 - dragx1;
	height = dragy2 - dragy1;
	if(width + (mousex - dragoffsetx) > XMAX)
	    dragoffsetx = width + mousex - XMAX;
	else if(mousex - dragoffsetx < XMIN)
	    dragoffsetx = mousex - XMIN;
	if(height + (mousey - dragoffsety) > YMAX)
	    dragoffsety = height + mousey - YMAX;
	else if(mousey - dragoffsety < YMIN)
	    dragoffsety = mousey - YMIN;
	dragx1 = mousex - dragoffsetx;
	dragx2 = dragx1 + width;
	dragy1 = mousey - dragoffsety;
	dragy2 = dragy1 + height;
    } else {
	if(mousex < dragx1) {
	    dragx2 = MIN(mousex,dragx1-pr->minsizex+1);
	    dragx2 = MAX(dragx2,dragx1-pr->maxsizex+1);
	    dragx2 = MAX(dragx2,XMIN);
	    if(mousey > dragy2)
		setcursor(C_ULEFT, cursorcolor, cursormask);
	    else
		setcursor(C_LLEFT, cursorcolor, cursormask);
	} else {
	    dragx2 = MAX(mousex,dragx1+pr->minsizex-1);
	    dragx2 = MIN(dragx2,dragx1+pr->maxsizex-1);
	    dragx2 = MIN(dragx2,XMAX);
	    if(mousey > dragy2)
		setcursor(C_URIGHT, cursorcolor, cursormask);
	    else
		setcursor(C_LRIGHT, cursorcolor, cursormask);
	}
	if(mousey > dragy2) {
	    dragy1 = MAX(mousey,dragy2+pr->minsizey-1);
	    dragy1 = MIN(dragy1,dragy2+pr->maxsizey-1);
	    dragy1 = MIN(dragy1,YMAX);
	} else {
	    dragy1 = MIN(mousey,dragy2-pr->minsizey+1);
	    dragy1 = MAX(dragy1,dragy2-pr->maxsizey+1);
	    dragy1 = MAX(dragy1,YMIN);
	}
	if(pr->keepaspect) {
	    if(pr->aspectx <= 0)
		pr->aspectx = 1;
	    if(pr->aspecty <= 0)
		pr->aspecty = 1;
	    width = dragx2-dragx1;
	    if(width < 0)
		width = -width;
	    height = dragy2-dragy1;
	    if(height < 0)
		height = -height;
	    if((width * pr->aspecty) > (height * pr->aspectx)) {
		height = (width * pr->aspecty) / pr->aspectx;
		if (height > pr->maxsizey - 1) {
		    height = pr->maxsizey - 1;
		    width = (height * pr->aspectx) / pr->aspecty;
		}
	    } else {
		width = (height * pr->aspectx) / pr->aspecty;
		if(width > pr->maxsizex - 1) {
		    width = pr->maxsizex - 1;
		    height = (width * pr->aspecty) / pr->aspectx;
		}
	    }
	    if(dragx2 > dragx1) {
		dragx2 = dragx1 + width;
		if(dragx2 > XMAX) {
		    dragx2 = XMAX;
		    dragx1 = dragx2 - width;
		}
	    } else {
		dragx2 = dragx1 - width;
		if(dragx2 < XMIN) {
		    dragx2 = XMIN;
		    dragx1 = dragx2 + width;
		}
	    }
	    if(dragy1 < dragy2) {
		dragy1 = dragy2 - height;
		if(dragy1 < YMIN) {
		    dragy1 = YMIN;
		    dragy2 = dragy1 + height;
		}
	    } else {
		dragy1 = dragy2 + height;
		if(dragy1 > YMAX) {
		    dragy1 = YMAX;
		    dragy2 = dragy1 - height;
		}
	    }
	}
    }
    if(pr->xunit > 1)
	if(dragx2 > dragx1)
	    dragx2 -= (dragx2 - dragx1 - pr->xunitfudge + 1) % pr->xunit;
	else
	    dragx2 += (dragx1 - dragx2 - pr->xunitfudge + 1) % pr->xunit;
    if(pr->yunit > 1)
	if(dragy2 > dragy1)
	    dragy1 += (dragy2 - dragy1 - pr->yunitfudge + 1) % pr->yunit;
	else
	    dragy1 -= (dragy1 - dragy2 - pr->yunitfudge + 1) % pr->yunit;
}

reset_dragbox()
{
    register short temp;

    if(dragx1 > dragx2) {
	temp = dragx1;
	dragx1 = dragx2;
	dragx2 = temp;
    }
    if(dragy1 > dragy2) {
	temp = dragy1;
	dragy1 = dragy2;
	dragy2 = temp;
    }
    dragoffsetx = mousex - dragx1;
    dragoffsety = mousey - dragy1;
}
