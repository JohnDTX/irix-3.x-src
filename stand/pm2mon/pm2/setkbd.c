# include "Qglobals.h"
# include "Qdevices.h"
# include "duart.h"
# include "iriskeybd.h"
# include "common.h"


#define	putkbdchar(x) (msdelay(32)/*15000*/ , putcraw(x,SCREEN))

setkbd()
{
    register int j;

    MAKEREADY(TOSCREEN|NOTSTOPPED);

    ScreenConfig();	/* (UGH) set up dcconfig */

    /*
     * just in case we are rebooting, wait for messages
     * to be displayed on the screen
     */
    msdelay(43);/*20000*/

    /* initialize the duarts */
    dinit(0);
    dinit(2);

    msdelay(43);/*20000*/		/* wait for duarts to settle */

    SETASCII;

    /* set the screen port to 600 baud */
    setbaud(SCREEN,600);
    msdelay(43);/*20000*/		/* wait for init */

    flush(SCREEN);
    putkbdchar(ResetKeybd);		/* Send the reset value to Port A */
    msdelay(214);/*100000*/		/* wait even longer for a response */

    if (nwgetcraw(SCREEN) == 0xaa)	/* if '0xaa' comes back it's the */
    {
	/* flash the kbd leds */
	for(j = 1; j < 0x80; j <<= 1)
		putkbdchar(j|1);
	putkbdchar(1);
	beep();

    	if (!ScreenInit())
	{
	    printf("init: ScreenInit failed\n");
	}
	else
	{
	    SETMICSW;
	}
    }

    if( !ISMICROSW )
	newline();
}
