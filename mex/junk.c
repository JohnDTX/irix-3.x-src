/*
 * Miscellaneous stuff
 *
 */
#include "win.h"
#include <stdio.h>
#include "gltypes.h"
#include "window.h"
#include "misc.h"
#include "cursors.h"

/*
 * grsafe:
 *	- tell kernel to repaint, if it needs to
 */
grsafe()
{
    grioctl(GR_SAFE, 0);
}

/*
 * movekbd:
 *	- move the keyboard from its current window to a newwindow
 *	- called with newwin==0 to select wman as current graphport 
 */
struct wm_window *movekbd(newwin)
    register struct wm_window *newwin;
{
    register struct wm_window *w;
    short tn;

    w = inputwindow;
    if(w == newwin)
	return w;
    if(w) {
	w->w_state &= ~WS_HASKEYBOARD;
	if (w->w_state & WS_OPEN)
	    wn_drawborder(w,0);
    }
    inputwindow = newwin;
    if(!newwin) {
	grioctl(GR_GFINPUTCHANNEL, 0);	/* is this always 0? */
	grioctl(GR_TXINPUTCHANNEL, -1);
	return w;
    } 
    if(newwin->w_state & WS_TEXTPORT) {
	setcursor(0, cursorcolor, cursormask);
	grioctl(GR_GFINPUTCHANNEL, -1);
	grioctl(GR_TXINPUTCHANNEL, newwin->w_txport);
    } else {
	grioctl(GR_GFINPUTCHANNEL, newwin->w_pr.gfnum);
	grioctl(GR_TXINPUTCHANNEL, -1);
    }
    newwin->w_state |= WS_HASKEYBOARD;
    wn_drawborder(newwin,0);
    return w;
}

background(redraw)
int redraw;
{
    register struct wm_window *w;

    w = backgroundw;
    if(w->w_inchan == -1) {
	if(redraw) {
	    wn_setrects(w);
	    viewport(0, XMAXSCREEN, 0, YMAXSCREEN);
	    color(0);	/* default background */
	    clear();
	    wn_setrects(0);
	    viewport(0, XMAXSCREEN, 0, YMAXSCREEN);
	}
    } else  		/* registered background process */
	wn_setpieces(w, redraw);
    w->w_state &= ~WS_REDISPLAY;
    w->w_state &= ~WS_NEWPIECES;
}

#define SOFTQSIZE 50

typedef struct {
    short 	type;
    short 	value;
} queueentry;

queueentry 		softqueue[SOFTQSIZE];
queueentry 		*softqueuein, *softqueueout;

softqreset()
{
    softqueuein = softqueueout = &softqueue[0];
}

softqenter(type, value)
short type, value;
{
    if ((softqueuein + 1 == softqueueout) ||
	((softqueuein + 1 == &softqueue[SOFTQSIZE]) &&
		(softqueueout == softqueue))) {
	    return;
    }
    softqueuein->type = type;
    softqueuein->value = value;
    softqueuein++;
    if (softqueuein == &softqueue[SOFTQSIZE])
	softqueuein = softqueue;
    return;
}

softqtest()
{
    if(softqueuein == softqueueout) 
	return 0;
    else 
	return softqueueout->type;
}

softqread(value)
register short *value;
{
    register unsigned short returnvalue;

    if(softqueuein == softqueueout) {
	*value = 0;
	return 0;
    }
    returnvalue = softqueueout->type;
    *value = softqueueout->value;
    softqueueout++;
    if (softqueueout == &softqueue[SOFTQSIZE])
	softqueueout = softqueue;
    return returnvalue;
}
