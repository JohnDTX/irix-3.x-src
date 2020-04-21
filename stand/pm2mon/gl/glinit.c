# undef  DIMTEST
# include "glx.h"
# define WP	(&GLX.win)


# define XWINMAX		80	/* max #cols inside window */
# define YWINMAX		40	/* max #lines inside window */


# define PROMSTATIC


int
glinit()
{
    register short int rx,ry,cx,cy;
    register short unsigned u;

    GLX.inited = 0;

    if( GLX.scrinit == 0 || !(*GLX.scrinit)() )
	return 0;
    glloadcolormap();

    /* round down max x to an integral number of columns */
    GLX.maxpix.x = CHARTOPIX(PIXTOCHAR(GLX.maxpix.x));

    /* round up min x to an integral number of columns */
    GLX.minpix.x += CHARWIDTH-1;
    GLX.minpix.x = CHARTOPIX(PIXTOCHAR(GLX.minpix.x));

    /* round up min y to an integral number of lines (wrt max y) */
    u = PIXTOLINE(GLX.maxpix.y-GLX.minpix.y);
    GLX.minpix.y = GLX.maxpix.y - LINETOPIX(u);

    /* set reference pixel for LINETOSCREENY() */
    GLX.yrefpix = GLX.maxpix.y-CHARHEIGHT;

    GLX.inited++;
    GLX.curr.x = GLX.curr.y = 0;
    GLX.curpix.x = CHARTOSCREENX(0);
    GLX.curpix.y = LINETOSCREENY(0);
    GLX.cursoron = 0;
    GLX.charcolor = CHARWEA;
    GLX.curscolor = CHARWE-CHARWEA;
    GLX.mapnum = 0;

    WP->tlhc.x = WP->tlhc.y = 0;

# ifdef DIMTEST
glbox(SCREENXTOCHAR(GLX.minpix.x),SCREENYTOLINE(GLX.maxpix.y-1)
,SCREENXTOCHAR(GLX.maxpix.x-1),SCREENYTOLINE(GLX.minpix.y));
# endif DIMTEST

    /*
     * determine window boundaries:
     * create window centered on visible area
     * (defined by GLX.minpix - GLX.maxpix).
     */
    rx = PIXTOCHAR(GLX.maxpix.x-GLX.minpix.x);/* compute size */
    ry = PIXTOLINE(GLX.maxpix.y-GLX.minpix.y);

    cx = SCREENXTOCHAR((unsigned short)(GLX.minpix.x+GLX.maxpix.x)/2);
    cy = SCREENYTOLINE((unsigned short)(GLX.minpix.y+GLX.maxpix.y)/2)+1;

# ifdef DIMTEST
glbox(SCREENXTOCHAR(GLX.minpix.x),SCREENYTOLINE(GLX.maxpix.y-1)
,SCREENXTOCHAR(GLX.maxpix.x-1),SCREENYTOLINE(GLX.minpix.y));
# endif DIMTEST

    if( rx > XWINMAX+2 ) rx = XWINMAX+2;	/* chop to MAX screen */
    if( ry > YWINMAX+2 ) ry = YWINMAX+2;

    cx -= (unsigned short)rx/2;
    cy -= (unsigned short)ry/2;

    glsetwin(cx,cy,cx+rx-1,cy+ry-1);

    /* TermInit(); */
    return 1;
}

/*
 * glsetwin() --
 * set (change) the window to start at (x0,y0)
 * tlhc to (x1,y1) brhc.  all WRT whole screen.
 * these dimensions include the border.
 */
glsetwin(x0,y0,x1,y1)
    register int x0,y0,x1,y1;
{
    /* shift window to screen coords */
    GLX.curr.x -= WP->tlhc.x;
    GLX.curr.y -= WP->tlhc.y;
    WP->tlhc.x = WP->tlhc.y = 0;

    WP->siz.x = x1-x0+1 - 2;
    WP->siz.y = y1-y0+1 - 2;

    glbox(x0,y0,x1,y1);
    glmovcursor(x0+1,y0+1);

    /* transform cursor to new coords */
    GLX.curr.x = GLX.curr.y = 0;
    WP->tlhc.x = x0+1;
    WP->tlhc.y = y0+1;
}

glnostate()
{
    if( GLX.nostate != 0 )
    {
	glcursor(0);
	(*GLX.nostate)();
    }
}
