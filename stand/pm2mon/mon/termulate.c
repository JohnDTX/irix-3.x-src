# include "Qdevices.h"
# include "Qglobals.h"
# include "remprom.h"
# include "common.h"
# include "duart.h"

# include "ctype.h"

# define KEYESC	0x1E	/*^^*/
/*
**	termulate - act as a terminal emulator
**
*/
termulate()
{
    register int r;
    register int onechar;
    register int keycond;

    modem(HOST, 1);
    wait(TOHOST);
    PutToHost('\r');
    SETTERMULATING;

    if( ISMICROSW )
	keycond = TOSCREEN;
    else
	keycond = KEYPANEL;

    while(1) {
        r = wait(KEYIN | FROMHOST);

        if( r&KEYIN && ready(TOHOST) ) {
		if( (onechar = getchar()) == KEYESC ) {
		    if( (onechar = getchar()) == 'q' ) {
			newline();
			break;
		    }
		    PutToHost(KEYESC);
		    wait(TOHOST);
		}
		PutToHost(onechar);
        }

	if( r&FROMHOST && ready(keycond) )
	{
	    if( (onechar = toascii(GetFromHost())) == PESC )
		term_interp();
	    else
		putchar(onechar);
	}
    }

    RESETTERMULATING;
    FlushKeyIn();
    modem(HOST, 0);
}

status()
{
    sendshort(0);
}

term_interp()
{
    register int cmd;

    cmd = recgcmd();  
    if( cmd == PSTATUS )
    {
	status();
    }
    else
    if( cmd == PDOWNLOAD )
    {
	RESETTERMULATING;
	shellcom("b s:");
	warmboot();
    }
    else
    if( cmd != PFASTCOM && cmd != PSLOWCOM )
    {
	printf("\niris: Graphics not loaded!!\n"); 
    }
}

term_help(n)
    int n;
{
    colprint("t(ermulate)","run terminal emulator");
    if( !n )
	return;

    noteprint("ctl-^ q to quit");
}
