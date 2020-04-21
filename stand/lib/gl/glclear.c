/*
 * $Source: /d2/3.7/src/stand/lib/gl/RCS/glclear.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:15:16 $
 */
# include "glx.h"
# define WP	(&GLX.win)


/*
 * glclearEOL() --
 * clear to EOL from current cursor position.
 */
glclearEOL()
{
    glclearbox(GLX.curr.x,GLX.curr.y,WP->siz.x-1,GLX.curr.y);
}

/*
 * glclearbox() --
 * clear box from tlhc (x0,y0) to brhc (x1,y1)
 * inclusive.  WRT current window.
 */
glclearbox(x0,y0,x1,y1)
    int x0,y0,x1,y1;
{
    TermClearBox(x0,y0,x1,y1);

    if( x0 <= GLX.curr.x && GLX.curr.x <= x1
     && y0 <= GLX.curr.y && GLX.curr.y <= y1 )
	GLX.cursoron = 0;

    /* transform to screen coords */
    x0 += WP->tlhc.x; x1 += WP->tlhc.x;
    y0 += WP->tlhc.y; y1 += WP->tlhc.y;

    glfill(CHARTOSCREENX(x0),LINETOSCREENY(y1),
	    CHARTOSCREENX(x1+1)-1,1+LINETOSCREENY(y0-1)-1,OFFCODE,CHARWE); 

    glcursor(1);
}
