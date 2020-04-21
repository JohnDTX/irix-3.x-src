#include "../h/param.h"
#include "../h/systm.h"
#include "../h/sgigsc.h"
#include "machine/cpureg.h"
#include "sgigsc.h"

#include <gl2/mouse.h>

short _mousex, _mousey;
#if NSGIGSC > 0
static int mx, my, mbits;
static int lastx, lasty;
#endif

/*
 * mouse_check:
 *	- called by fbc vertical blank interrupt code to update the
 *	  mouse position
 *	- squeek
 */
mouse_check(m)
	register struct mouse *m;
{
	register short bits;

#ifdef pmII
	bits = *(short *)MBUT;
#endif
#ifdef juniper
	bits = *(short *)M_BUT;
#endif
	m->right = bits & 1;
	m->middle = bits & 2;
	m->left = bits & 4;
	m->x = _mousex;
	m->y = _mousey;

#if NSGIGSC > 0
{
	/*
	 * Enter mouse position in gsc queue
	 */
	if ((m->x != mx) || (m->y != my) ||
	    (bits != mbits)) {
		struct sgigsc_qentry qe;

		lastx += (m->x - mx);
		if (lastx < 0) lastx = 0;
		if (lastx > 1023) lastx = 1023;
		lasty += (m->y - my);
		if (lasty < 0) lasty = 0;
		if (lasty > 767) lasty = 767;
		mx = m->x;
		my = m->y;
		mbits = bits;
		qe.event = SGE_MOUSE;
		qe.ev.mouse.x = lastx;
		qe.ev.mouse.y = lasty;
		qe.ev.mouse.buttons = bits & 7;
		sgqenter(&qe);
	}
}
#endif
}

updateMousePosition(x, y)
{
#if NSGIGSC > 0
	lastx = x;
	lasty = y;
#endif
}

#ifdef pmII
/*
 * mouse_intr:
 *	- this interrupt routine is used when the gf board doesn't probe
 *	- since the mouse button interrupt is generated on the cpu, we
 *	  need a default interrupt handler that will process it
 */
mouse_intr()
{
	ushort dummy;

    /* see if interrupt came from mouse/mailbox/parallel port */
	if (~(*(short *)EREG) & (ER_MOUSE|ER_MAILBOX|ER_RCV|ER_XMIT))
		dummy = *(short *)MBUT;		/* clear interrupt */
#ifdef	lint
	dummy = dummy;
#endif
}
#endif
