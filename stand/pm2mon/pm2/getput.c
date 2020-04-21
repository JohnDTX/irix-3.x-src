/*
**			 Quirk serial i/o primitives
**
**			  Paul Haeberli - Sept 1983
*/
#include "Qdevices.h"
#include "Qglobals.h"
# include "common.h"
# include "ctype.h"


/*
**	putchar - put to "the" output device.
**
*/
putchar( onechar )
    char onechar;
{
    if( !TERMULATING )
	while( !ready(NOTSTOPPED) )
	    ;

    if( ISMICROSW )
    {
        wait(TOSCREEN);
        ScreenChar(onechar);
    }
    else
    {
        wait(KEYPANEL);
        PutKeyPanel(onechar);
    }
}

/*
**	getchar - get from "the" input device.
**
*/
getchar()
{
    wait(KEYIN);
    return toascii(GetKeyIn());	/* no echo - let caller do it if desired */
}

ungetchar(c)
    unsigned char c;
{
    return UnGetKeyIn(c);
}
