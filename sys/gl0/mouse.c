#include "../h/param.h"
#include "../h/systm.h"
#include "machine/cpureg.h"

#ifdef	pmII
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
#else
mouse_intr()
{
}
#endif

lpentick()
{
}
