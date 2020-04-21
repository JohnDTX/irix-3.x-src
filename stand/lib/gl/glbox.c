/*
 * $Source: /d2/3.7/src/stand/lib/gl/RCS/glbox.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:15:14 $
 */
# include "glx.h"
# define WP	(&GLX.win)

/*
 * glbox() --
 * draw box with at tlhc (x0,y0) brhc (x1,y1).
 * WRT WHOLE SCREEN.
 */
glbox(x0,y0,x1,y1)
    int x0,y0,x1,y1;
{
    register int px0,px1,py0,py1;

    px0 = CHARTOSCREENX(x0)+CHARWIDTH/2;
    px1 = CHARTOSCREENX(x1)+CHARWIDTH/2;
    py0 = LINETOSCREENY(y0)+CHARHEIGHT/2;
    py1 = LINETOSCREENY(y1)+CHARHEIGHT/2;

    /* top */
    glfill(px0-1,py0-1,px1+1,py0+1,LINECODE,SCREENWE);

    /* right */
    glfill(px1-1,py1-1,px1+1,py0+1,LINECODE,SCREENWE);

    /* bottom */
    glfill(px0-1,py1-1,px1+1,py1+1,LINECODE,SCREENWE);

    /* left */
    glfill(px0-1,py1-1,px0+1,py0+1,LINECODE,SCREENWE);
}
