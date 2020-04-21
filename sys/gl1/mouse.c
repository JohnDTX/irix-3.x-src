#include "../h/param.h"
#include "../h/systm.h"
#include "machine/cpureg.h"
#include "../gl1/kgl.h"
#include "../gl1/shmem.h"
#include "../gl1/device.h"

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
}

/*
 * mouse:
 *	- called by fbc vertical blank interrupt code to update the
 *	  mouse position
 *	- we make the mouse busy so as to insure that the level 7
 *	  mouse code won't mess up our variables whilst we are looking
 *	  at them (and vice versi)
 */
mouse()
{
	register struct shmem *sh = (struct shmem *)SHMEM_VBASE;
	register short bits, v;
	static short state;
	static short qmousex, qmousey;

    /* only do mouse processing every other time */
	state = ~state;
	if (state)
		return;

    /* set up button information (and queue events if needed) */
	bits = *(short *)MBUT;
	if (sh->Buttons[RIGHTMOUSE].queue) {
		if (sh->Buttons[RIGHTMOUSE].state != (bits & 1)) {
			qenter(RIGHTMOUSE, bits & 1);
			if (v = sh->Buttons[RIGHTMOUSE].tiedevice1)
				qenter(v, sh->valuators[v - VALOFFSET].value);
			if (v = sh->Buttons[RIGHTMOUSE].tiedevice2)
				qenter(v, sh->valuators[v - VALOFFSET].value);
		}
	}
	sh->Buttons[RIGHTMOUSE].state = bits & 1;
	bits >>= 1;
	if (sh->Buttons[MIDDLEMOUSE].queue) {
		if (sh->Buttons[MIDDLEMOUSE].state != (bits & 1)) {
			qenter(MIDDLEMOUSE, bits & 1);
			if (v = sh->Buttons[MIDDLEMOUSE].tiedevice1)
				qenter(v, sh->valuators[v - VALOFFSET].value);
			if (v = sh->Buttons[MIDDLEMOUSE].tiedevice2)
				qenter(v, sh->valuators[v - VALOFFSET].value);
		}
	}
	sh->Buttons[MIDDLEMOUSE].state = bits & 1;
	bits >>= 1;
	if (sh->Buttons[LEFTMOUSE].queue) {
		if (sh->Buttons[LEFTMOUSE].state != (bits & 1)) {
			qenter(LEFTMOUSE, bits & 1);
			if (v = sh->Buttons[LEFTMOUSE].tiedevice1)
				qenter(v, sh->valuators[v - VALOFFSET].value);
			if (v = sh->Buttons[LEFTMOUSE].tiedevice2)
				qenter(v, sh->valuators[v - VALOFFSET].value);
		}
	}
	sh->Buttons[LEFTMOUSE].state = bits & 1;

    /* update mouse quadrature */
	mousebusy = 1;
	if (_mousex < (v = sh->valuators[MOUSEX - VALOFFSET].minvalue))
		_mousex = v;
	if (_mousex > (v = sh->valuators[MOUSEX - VALOFFSET].maxvalue))
		_mousex = v;
	if (_mousey < (v = sh->valuators[MOUSEY - VALOFFSET].minvalue))
		_mousey = v;
	if (_mousey > (v = sh->valuators[MOUSEY - VALOFFSET].maxvalue))
		_mousey = v;

	sh->valuators[MOUSEX - VALOFFSET].value = _mousex;
	sh->valuators[MOUSEY - VALOFFSET].value = _mousey;
	mousebusy = 0;

    /* connect cursor to valuators if its the mouse */
	if (sh->cursorxvaluator == (MOUSEX - VALOFFSET))
		cursorx = sh->valuators[sh->cursorxvaluator].value;
	if (sh->cursoryvaluator == (MOUSEY - VALOFFSET))
		cursory = sh->valuators[sh->cursoryvaluator].value;

    /* queue mouse valuators if they changed enough */
	if (sh->valuators[MOUSEX - VALOFFSET].queue) {
		v = _mousex - qmousex;
		if (v < 0)
			v = -v;
		if (v >= sh->valuators[MOUSEX - VALOFFSET].noise) {
			qenter(MOUSEX, _mousex);
			qmousex = _mousex;
		}
	}
	if (sh->valuators[MOUSEY - VALOFFSET].queue) {
		v = _mousey - qmousey;
		if (v < 0)
			v = -v;
		if (v >= sh->valuators[MOUSEY - VALOFFSET].noise) {
			qenter(MOUSEY, _mousey);
			qmousey = _mousey;
		}
	}
}
