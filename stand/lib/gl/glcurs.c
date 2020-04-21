/*
 * $Source: /d2/3.7/src/stand/lib/gl/RCS/glcurs.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:15:19 $
 */
# include "glx.h"
# define WP	(&GLX.win)

/*
 * glcursor() --
 * turn the cursor on/off.
 */
glcursor(onoff)
    int onoff;
{
    (*GLX.fill)(GLX.curpix.x,GLX.curpix.y
	    ,GLX.curpix.x+CHARWIDTH-1,GLX.curpix.y+CHARHEIGHT-1
	    ,onoff?GLX.curscolor:OFFCODE,GLX.curscolor);
    GLX.cursoron = onoff;
}

/*
 * glmovcursor() --
 * move the cursor to the given new location.
 * by erasing it at the current location
 * and re-drawing it at the new location.
 * all WRT current window.
 */
glmovcursor(x,y)
    register int x,y;
{
    if( GLX.cursoron )
    {
	if( GLX.curr.x == x && GLX.curr.y == y )
	    return;
	glcursor(0);
    }

    GLX.curr.x = x;
    GLX.curr.y = y;

    /* transform to screen coords */
    x += WP->tlhc.x;
    y += WP->tlhc.y;
    GLX.curpix.x = CHARTOSCREENX(x);
    GLX.curpix.y = LINETOSCREENY(y);

    glcursor(1);
}
