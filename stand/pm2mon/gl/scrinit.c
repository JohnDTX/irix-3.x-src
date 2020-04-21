# include "glx.h"
# define WP	(&GLX.win)
# include "common.h"


/*
 * ScreenConfig() --
 * (UGH) set up gl config info.			XXX
 */
int
ScreenConfig()
{
    GLX.dchi = DEFAULT_DC_HIGH != 0;
    GLX.dcconfig = 0;
    GLX.scrinit = 0;
    GLX.nblines = 5;

    if( gl1_probe((int)GLX.dchi) )
	;
    else
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

    _commdat->screenx = WP->tlhc.x+GLX.curr.x;
    _commdat->screeny = WP->tlhc.y+GLX.curr.y;
    _commdat->dcconfig = GLX.dcconfig;
    _commdat->savenblines = _commdat->nblines = GLX.nblines;

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

    x = _commdat->screenx - WP->tlhc.x;
    y = _commdat->screeny - WP->tlhc.y;

    if( (unsigned)x < WP->siz.x && (unsigned)y < WP->siz.y )
    {
	glmovcursor(x,y);
	return;
    }

    ScreenClear();
    glmovcursor(0,0);
}
