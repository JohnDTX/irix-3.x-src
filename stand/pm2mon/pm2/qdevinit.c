#include "Qdevices.h"		/* Quirk Includes */
#include "Qglobals.h"

# include "m68vectors.h"


/*
**	qdevinit - Q kernel init.  initialize the clock, the serial devices, 
**	       set the interrupt level, and flush the input streams.
**
*/
qdevinit(donttouch)
int donttouch;
{
    extern int trapF();
    extern int level4int();

    /* set the trap F vector */
    *EVEC_TRAPF = (long)trapF;

    /* and the interrupt 4 handler */
    *EVEC_LEVEL4 = (long)level4int;

    MAKEREADY(TOHOST|KEYPANEL|TOSCREEN|NOTSTOPPED);

    clockinit();			/* Start Quirk Clock device 	*/
    SerialDevInit();			/* Set up Host and Keybd uarts	*/
    IkonInit();

    if (donttouch) { spl5(); return(0); }

    /* IPinit();			/* Init IP disk--clear interrupt! */

    spl1();

    /* Flush any stray host uart input */
    FlushFromHost();

    /* Flush any stray keybd uart input */
    FlushKeyIn();
}
