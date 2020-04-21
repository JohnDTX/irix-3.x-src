/*
 * $Source: /d2/3.7/src/stand/lib/gl/RCS/scrinit.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:15:25 $
 */
# include "glx.h"
# define WP	(&GLX.win)
# include "sys/types.h"
# include "cpureg.h"
# include "common.h"


/*
 * ScreenConfig() --
 * (UGH) set up gl config info.			XXX
 */
int
ScreenConfig()
{
    GLX.dchi = (_commdat->c_flags & DC_HIGH) != 0;
    GLX.dcconfig = _commdat->c_dcconfig;
    GLX.scrinit = 0;
    GLX.nblines = 5;

    if( gl2_probe((int)GLX.dchi) )
	;
    else
	return 0;

    ScreenComm();
    return 1;
}

/*
 * ScreenInit() --
 * determine whether the system is GL1 or GL2,
 * and set up to use appropriate routines.
 */
int
ScreenInit()
{
    if( GLX.scrinit == 0 && !ScreenConfig() )
	return 0;

    glinit();

    return 1;
}

/*
 * ScreenComm() --
 * (UGH) copy out selected gl information.	XXX
 * (UGH) discard graphics state.
 */
ScreenComm()
{
    CLEARGL;
    if( GLX.hwversion == 2 )
	SETGL2;
    if( GLX.hwversion == 1 )
	SETGL1;

    _commdat->c_screenx = WP->tlhc.x+GLX.curr.x;
    _commdat->c_screeny = WP->tlhc.y+GLX.curr.y;
    _commdat->c_savenblines = _commdat->c_nblines = GLX.nblines;

    glnostate();
}

/*
 * ScreenNoise() --
 * display selected gl info.
 */
ScreenNoise()
{
    if( ISGL2 )
	printf("\
screen modes = 0x%x, default = %s, dc HW register = 0x%x\n",
		GLX.dcconfig,
		GLX.dchi?"HIGH":"LOW",
		GLX.dchw);
    else
	;
}

InScreen()
{
    register short x,y;

    x = _commdat->c_screenx - WP->tlhc.x;
    y = _commdat->c_screeny - WP->tlhc.y;

    if( (unsigned)x < WP->siz.x && (unsigned)y < WP->siz.y )
    {
	glmovcursor(x,y);
	return;
    }

    ScreenClear();
    glmovcursor(0,0);
}
