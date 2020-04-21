# include "common.h"

int __ismicsw;

setkbd()
{
    getcinit();

    if( __ismicsw = ISMICROSW )
    {
	if( !ScreenInit() )
	{
	    SETASCII;
	    __ismicsw = 0;
	}
	else
	{
	    TermInit(-1);
	}
    }
}
