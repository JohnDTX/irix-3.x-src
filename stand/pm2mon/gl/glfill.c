# include "glx.h"

/*
 * glfill() --
 * fill the area from blhc (x0,y0) to trhc (x1,y1)
 * in pixels, inclusive; with the given color and mask.
 * all WRT whole screen.
 */
glfill(x0,y0,x1,y1,color,wemask)
    int x0,y0,x1,y1;
    int color,wemask;
{
   (*GLX.fill)(x0,y0,x1,y1,color,wemask);
}
