/*
 * Primitive window handling
 */
#include "window.h"
#include "tf.h"
#include "device.h"

void
windowFunc(w, event, value)
	window_t *w;
	long event, value;
{
	switch (event) {
	  case REDRAW:
		/* inherited method */
		paneRedraw(w->w_contents);
		break;
	}
}

window_t *
newWindow()
{
	window_t *w;

	w = CALLOC(window_t *, 1, sizeof(window_t));
	if (w) {
		w->w_contents = newPane();
		catchEvent(REDRAW, windowFunc, w);
	}
	return (w);
}

void
setWindowSize(w, xlen, ylen)
	window_t *w;
	long xlen, ylen;
{
	/* inherited method */
	setPaneSize(w->w_contents, xlen, ylen);
}

void
setWindowOrigin(w, xorg, yorg)
	window_t *w;
	long xorg, yorg;
{
	/* inherited method */
	setPaneOrigin(w->w_contents, xorg, yorg);
}

void
setWindowRect(w, r)
	window_t *w;
	rect_t *r;
{
	/* inherited method */
	setPaneRect(w->w_contents, r);
}

void
windowManager()
{
	eventManager();
}
