/*
 * Simple pane stuff
 */
#include "pane.h"
#include "tf.h"

pane_t *
newPane()
{
	pane_t *p;

	p = CALLOC(pane_t *, 1, sizeof(pane_t));
	return (p);
}

void
freePane(p)
	pane_t *p;
{
	free(p);
}

void
setPaneSize(p, xlen, ylen)
	pane_t *p;
	int xlen, ylen;
{
	p->p_r.xlen = xlen;
	p->p_r.ylen = ylen;
}

void
setPaneOrigin(p, xorg, yorg)
	pane_t *p;
	int xorg, yorg;
{
	p->p_r.xorg = xorg;
	p->p_r.yorg = yorg;
}

void
setPaneRect(p, r)
	pane_t *p;
	rect_t *r;
{
	p->p_r = *r;
}

void
setPaneView(p)
	pane_t *p;
{
	static rect_t lastr;

	if ((lastr.xlen != p->p_r.xlen) || (lastr.ylen != p->p_r.ylen) ||
	    (lastr.xorg != p->p_r.xorg) || (lastr.yorg != p->p_r.yorg)) {
		viewport(p->p_r.xorg, p->p_r.xorg + p->p_r.xlen - 1,
				  p->p_r.yorg, p->p_r.yorg + p->p_r.ylen - 1);
		ortho2(-0.5, (p->p_r.xlen - 1) + 0.5,
			     -0.5, (p->p_r.ylen - 1) + 0.5);
		lastr = p->p_r;
	}
}

void
setPaneRedraw(p, func)
	pane_t *p;
	int (*func)();
{
	p->p_redraw = func;
}

/*
 * Redraw ourselves, then redraw our contents
 */
void
paneRedraw(p)
	pane_t *p;
{
	if (p->p_redraw)
		(*p->p_redraw)(p);
	if (p = p->p_contents) {
		while (p) {
			paneRedraw(p);
			p = p->p_next;
		}
	}
}
