/*
 * $Source: /d2/3.7/src/stand/lib/gl/RCS/glchar.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:15:16 $
 */
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
