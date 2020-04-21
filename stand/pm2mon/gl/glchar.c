# include "glx.h"

glputat(x,y,ccc)
    int x,y;
    int ccc;
{
   (*GLX.putat)(x,y,ccc,GLX.charcolor);
}

/*
 * glchar() --
 * draw a char (destructively) at the
 * current cursor position.
 */
glchar(ccc)
    int ccc;
{
    TermAddChar(ccc);
    (*GLX.putat)(GLX.curpix.x,GLX.curpix.y,ccc,GLX.charcolor);
}
