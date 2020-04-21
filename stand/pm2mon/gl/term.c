# undef  DEBUG do_debug
# include "glx.h"
# include "dprintf.h"
	extern char do_debug;

# define PROMSTATIC

# define WP	(&GLX.win)

PROMSTATIC	short DumberTerm;

# define YWINMAX	40
# define bcopy(s,t,n)	blt(t,s,n)

PROMSTATIC	struct GLXlo GLXlines[YWINMAX];
PROMSTATIC	char *GLXtarea;
PROMSTATIC	int GLXtsize;



/*
 * TermInit() --
 * this has been carefully kluged to work even
 * if called more than once PROVIDING the screen
 * hasn't scrolled yet.
 */
int
TermInit(nlines)
    int nlines;
{
    extern ColorVec glx_Black;
    register int xpixel,ypixel;

    if( (unsigned)nlines > WP->siz.y )
	nlines = WP->siz.y;

    /* both char bit planes were already cleared */
    /* KLUGE, this code contains knowledge of CHARTOSCREEN?() */
    /*
    xpixel = CHARTOSCREENX(WP->tlhc.x);
    ypixel = LINETOSCREENY(WP->tlhc.y+WP->siz.y)+CHARHEIGHT;

    glfill(xpixel,ypixel
	    ,xpixel+CHARTOPIX(WP->siz.x)-1,ypixel+LINETOPIX(WP->siz.y)-1
	    ,OFFCODE,CHARWEB);

    GLX.charcolor = CHARWEA;
    GLX.curscolor = CHARWEB;
    GLX.mapnum = 0;
     */

    return TermTextInit(nlines);
}

/*
 * TermTextInit() --
 * this has been carefully kluged to work even
 * if called more than once PROVIDING the screen
 * hasn't scrolled yet.
 * this is a likely source of BUGS!
 */
int
TermTextInit(nlines)
    int nlines;
{
    extern char *mbmalloc(),*awful_alloc();

    register int i;
    register short o;
    register struct GLXlo *lp;
    register char *cp;

    if( nlines > YWINMAX )
	return 0;

    /* get memory for text */
    i = nlines*WP->siz.x;
    cp = mbmalloc(i);
dprintf(("mbmalloc(%d) $%x\n",i,cp));
    if( cp == 0 )
	cp = awful_alloc(i);
dprintf(("awful_alloc(%d) $%x\n",i,cp));
    if( cp == 0 )
	return 0;

    /* copy over old text if any */
    if( i < GLXtsize )
	GLXtsize = i;
    if( GLXtsize > 0 )
	bcopy(GLXtarea,cp,(int)GLXtsize);
    GLXtsize = i;
    GLXtarea = cp;

    /* initialize line offsets */
    lp = GLXlines+0; o = 0;
    WP->lines = lp;

    for( i = 0; i < nlines; i++ )
    {
	/* lp->l = 0 */; lp->o = o;
	lp++; o += WP->siz.x;
    }

    WP->text = cp;
    return 1;
}


TermFree()
{
}

/*
 * TermAddChar() --
 * add a character at the current position.
 */
TermAddChar(c)
    char c;
{
    struct GLXlo *lp;
    register short i;
    register unsigned char *cp;

if(WP->text==0) return;

    /* convert to window coords */
    if( (unsigned)(i = GLX.curr.y) >= WP->siz.y )
	return;

    lp = WP->lines + i;

    /* convert to window coords */
    if( (unsigned)(i = GLX.curr.x) >= WP->siz.x )
	return;

    cp = (unsigned char *)WP->text+lp->o;

    /* extend line if necessary */
    while( i >= lp->l )
	cp[lp->l++] = ' ';

    cp[i] = c;
}

TermClearBox(x0,y0,x1,y1)
    int x0,y0,x1,y1;
{
    register struct GLXlo *lp;
    register short i,j;
    register unsigned char *cp;

if(WP->text==0) return;

    x1++;
    y1++;
    if( x1 > WP->siz.x )
	x1 = WP->siz.x;
    if( y1 > WP->siz.y )
	y1 = WP->siz.y;

    /* for each line, clear (or chop) the affected range */
    for( i = y0; i < y1; i++ )
    {
	lp = WP->lines + i;

	/* if possible, just shorten */
	if( x1 >= WP->siz.x )
	    lp->l = x0;
	if( x0 >= lp->l )
	    continue;

	for( cp = (unsigned char *)WP->text+lp->o+x0 , j = x1-x0
		; --j >= 0; )
	    *cp++ = ' ';
    }
}

TermScroll(updown)
    int updown;
{
    register short i;

if(WP->text==0) return;

    i = GLX.curr.y;
    GLX.curr.y = 0;

    if( updown > 0 )
	TermDelete();
    else
	TermInsert();

    GLX.curr.y = i;
}

TermInsert()
{
    register short i,o;
    register struct GLXlo *lp;

if(WP->text==0) return;

    if( (unsigned)GLX.curr.y >= WP->siz.y )
	return;

    /* rotate contents from last line to current line */
    i = WP->siz.y-1;
    lp = WP->lines + i;
    o = lp->o;

    while( --i >= GLX.curr.y )
    {
	lp--;
	lp[1] = lp[0];
    }

    /* the current line is empty */
    lp->o = o;
    lp->l = 0;
}

TermDelete()
{
    register short i,o;
    register struct GLXlo *lp;

if(WP->text==0) return;

    if( (unsigned)(i = GLX.curr.y) >= WP->siz.y )
	return;

    /* rotate contents from current line to last line */
    lp = WP->lines + i;
    o = lp->o;

    while( ++i < WP->siz.y )
    {
	lp[0] = lp[1];
	lp++;
    }

    /* the last line is empty */
    lp->o = o;
    lp->l = 0;
}

TermClearAhead()
{
    register short yline,lastclear;

    yline = GLX.curr.y;
    if( ++ yline >= WP->siz.y )
	yline = 0;

    /* if (TERMULATING) ... */
    lastclear = yline+GLX.nblines;
    if( lastclear >= WP->siz.y )
	lastclear = WP->siz.y-1;

    glclearbox(0,yline,WP->siz.x-1,lastclear);
    glmovcursor(GLX.curr.x,yline);
}

TermRepaint()
{
    extern ColorVec glx_Black,glx_White;

    register short i;
    register struct GLXlo *lp;
    register int xpixel,ypixel;
    short oldcolor,mapnum;

if(WP->text==0) return;

    oldcolor = GLX.charcolor;
    mapnum = GLX.mapnum;

    glcursor(0);
    glsetmap(mapnum+2);

    GLX.charcolor = GLX.curscolor;

    /* KLUGE, this code contains knowledge of CHARTOSCREEN?() */
    lp = WP->lines;
    xpixel = CHARTOSCREENX(WP->tlhc.x);
    ypixel = LINETOSCREENY(WP->tlhc.y);

    /* ASSUME the new char plane is already invisible AND blanked */
    /* re-put each line */
    for( i = WP->siz.y; --i >= 0; )
    {
	(*GLX.putline)(xpixel,ypixel
		,WP->text+lp->o,lp->l,GLX.charcolor);
	lp++;
	ypixel -= CHARHEIGHT;
    }

    ypixel += CHARHEIGHT;

    /* clear the old */
    glfill(xpixel,ypixel
	    ,xpixel+CHARTOPIX(WP->siz.x)-1,ypixel+LINETOPIX(WP->siz.y)-1
	    ,OFFCODE,oldcolor);

    /* switch char planes */
    GLX.curscolor = oldcolor;

    glsetmap(!mapnum);
    glcursor(1);
}

TermComm()
{
    if( GLX.mapnum )
	TermRepaint();
    DumberTerm = 1;
}

TermWrap(updown)
    int updown;
{
    register short yline;

if(WP->text==0||DumberTerm) { TermClearAhead(); return; }

    yline = GLX.curr.y;

    if( updown > 0
	? (++yline >= WP->siz.y)
	: (--yline < 0) )
    {
	TermScroll(updown);
	TermRepaint();
	return;
    }

    glmovcursor(GLX.curr.x,yline);
}

TermTransmit()
{
    register short i;
    register char *cp;
    register struct GLXlo *lp;

if(WP->text==0) return;

    if( (unsigned)(i = GLX.curr.y) < WP->siz.y )
    {
	lp = WP->lines + i;
	cp = WP->text + lp->o;
	for( i = lp->l; --i >= 0; )
	    ungetchar(*cp++);
    }

    ungetchar('\r');
}
