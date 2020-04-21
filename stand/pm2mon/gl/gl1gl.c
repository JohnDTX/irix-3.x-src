# undef  DIMTEST
/*
 * gl1gl.c --
 *
 *	supports beta microcode and DC2/DC3; multibus mode
 *
 * exports:
 *	gl1_probe()
 *	gl1_scrinit()
 *	gl1_putat(x,y,c,color)
 *	gl1_putline(x,y,line,len,width)
 *	gl1_fill(x0,y0,x1,y1,color)
 *	gl1_mapcolor(m,color,vp)
 *	gl1_setmap(m)
 *	gl1_nostate()
 *
 * This module is designed to replace the sunscreen.c module in the SUN
 * monitor.  These routines will perform low level output functions to
 * the IRIS graphics pipeline, to utilize the IRIS as a dumb terminal.
 */

/* the following defines used to be invoked from the command line */

# define IRIS		1
# define SUNV1PCREVD	1
# define GFBETA		1
# define DC3		1
# define SCREENSTUFF	1


# ifndef MBioVA
/* extern char *MBioVA; */
# define   MBioVA	((char *)0xF70000)	/* fake out the includes */
# endif MBioVA

# include "pm2.1.h"

# include "gl1dcdev.h"
# define   GESYSTEM	1
# define   IRISOPCODES	1
# include "gl1screen.h"
# define   UCTEST_DECLARE_VARIABLES
# define   INTER2
# include "gl1uctest.h"
# include "gl1gfdev.h"
# include "gl1cmds.h"

# include "glx.h"

# define PROMSTATIC

# define CURSMWE	X_GREEN


# ifdef DIMTEST
short GL1_XDIM	= 1024;
short GL1_YDIM	= 768;
short GL1_MARGIN = 0;

# else  DIMTEST

# define GL1_XDIM	1024
# define GL1_YDIM	768
# define GL1_MARGIN	0
# endif DIMTEST


/*----- gl data / macros */
#undef	set

# define ISendSet	register short \
				*a4 = (short *)&FBCdata, \
				*a3 = (short *)&FBCflags

# define ISendWait	while( !(*a3 & (unsigned short)FBCREQ_BIT_) )

# define ISendWaitCmd	while( *a4 != 0x40 )

# define ISendCycle(x)	(*a4 = (x) , *a3 = FORCEREQ , *a3 = FORCEWAIT)

# define ISendImm(dat)	{ ISendCycle(dat); ISendWait; }
# define ISendWd(dat)	{ ISendCycle(dat); ISendWait; }
# define ISendCmd(cmd)	{ ISendWaitCmd; ISendCycle(cmd); }

# define ColorMask(c,m)	{ \
			if(gl1_color!=(c)) { \
				ISendWaitCmd; \
				ISendWd(COLORCMD); \
				ISendWd(c); \
				ISendWd(m); \
				gl1_color=(c); \
			} \
			if(gl1_wemask!=(m)) { \
				ISendWaitCmd; \
				ISendWd(WRTENCMD); \
				ISendWait; \
				ISendWd(m); \
				ISendWd(m); \
				gl1_wemask=(m); \
			} }

# define Coords(x,y)	{ \
			ISendWd(x); ISendWd(x); \
			ISendWd(x); ISendWd(y); \
			}

# define setspacing(n)	{ \
			if(gl1_spacing!=(n)) { \
				ISendWaitCmd; \
				ISendWd(FIXCHARLOADCMD); \
				ISendWd(CHARHEIGHT-1); \
				ISendWd(CHARWIDTH-1); \
				ISendWd(n); \
				ISendWd(CONFIGCMD); \
				ISendWd(CONFIGPARAM); \
				gl1_spacing = (n); \
			} }

PROMSTATIC	char gl1_spacing;
PROMSTATIC	short gl1_color;
PROMSTATIC	short gl1_wemask;
PROMSTATIC	short gl1_xmax;
PROMSTATIC	short gl1_ymax;
PROMSTATIC	short gl1_xmin;
PROMSTATIC	short gl1_ymin;
PROMSTATIC	short gl1_xdim;
PROMSTATIC	short gl1_ydim;

typedef short	IRISdata;

static
SendIRIS(datablock)
    register IRISdata *datablock;
{
    ISendSet;
    register short	wordsleft;

    wordsleft = *datablock++;
    while( --wordsleft >= 0 )
    {
	ISendWd(*datablock++);
    }
}


/*
 * Select current font, polygon stipple character, and cursor
 * character:
 */
static
IRISdata	FontSelections[] = {
	15,

	POLYSTIPPLECMD, /* Select polygon stipple character: */
	CharToAddr(POLYSTIPCHARNUM),

	FIXCHARLOADCMD,	/* fixchar load	*/
	CHARHEIGHT-1,	/* height-1 */
	CHARWIDTH-1,	/* width-1 */
	0,		/* spacing */

	CONFIGCMD,
	CONFIGPARAM,

	SELECTCURSORCMD, /* Select the cursor character: */
	CharToAddr(CURSORCHARNUM),
	CURSMWE,		/* Cursor has its own color */
	CURSMWE,		/* this is for UC2	*/
	CURSMWE,
	CURSMWE,
	CONFIGPARAM	/* and configuration. */
};

/*
 * Load a window and viewport for the FBC's character drawing routines.
 */

static
IRISdata	WindowViewportInfo[] = {
	9,

	VIEWPORTCMD,
	0,		/* lower left x */
	0,
	0,		/* lower left y */
	0,
	0x3ff,		/* upper right x */
	0x3ff,
	0x3ff,
	0x3ff
};

/*----- */



/*
 * gl1_probe() --
 * return true iff it looks like a gl1 system.
 * ALSO set up configuration variables.
 */
int
gl1_probe(dchi)
    int dchi;
{
    if( !probe(&FBCflags,2) )
	return 0;

    configure_screen(dchi);

    { extern gl1_scrinit();	GLX.scrinit = gl1_scrinit; }
    GLX.dcconfig = 0;
    GLX.dchw = 0;

    GLX.hwversion = 1;
    return 1;
}

/*
 * gl1_scrinit() --
 * init hw and glx stuff to gl1.
 */
int
gl1_scrinit()
{
    initscreen();

    /* initialize glx package globals */
    { extern gl1_putat();	GLX.putat = gl1_putat; }
    { extern gl1_putline();	GLX.putline = gl1_putline; }
    { extern gl1_fill();	GLX.fill = gl1_fill; }
    { extern gl1_mapcolor();	GLX.mapcolor = gl1_mapcolor; }
    { extern gl1_setmap();	GLX.setmap = gl1_setmap; }
    { extern gl1_nostate();	GLX.nostate = gl1_nostate; }

    GLX.maxpix.x = gl1_xmax;
    GLX.maxpix.y = gl1_ymax;
    GLX.minpix.x = gl1_xmin;
    GLX.minpix.y = gl1_ymin;

    return 1;
}

static
configure_screen(dchi)
    int dchi;
{
    gl1_xdim = GL1_XDIM; gl1_ydim = GL1_YDIM;
    gl1_xmin = GL1_MARGIN; gl1_ymin = GL1_MARGIN;
    gl1_xmax = GL1_XDIM-GL1_MARGIN; gl1_ymax = GL1_YDIM-GL1_MARGIN;

    gl1_color = -1; gl1_wemask = 0;
}

static
initscreen()
{
    extern IRISdata WindowViewportInfo[];

    inithardware();
    loadfont();
    SendIRIS(WindowViewportInfo);
    gl1_fill(0,0,gl1_xdim-1,gl1_ydim-1,OFFCODE,SCREENWE);
    /* drawlogo(); */
}

static
inithardware()
{
    register curint;

    curint = spl7();
    UCTEST_INIT;
    FBCdisabvert(RUNDEBUG);
    splx(curint);
}

/*
 * Loads a fixed width font into the IRIS's font ram, and selects
 * this font for character display.  The 0th character in the font	XXX
 * is used for a polygon stipple pattern, and one character is used
 * for a cursor character.
 */
static
loadfont()
{
    extern IRISdata FontSelections[];

    extern GlyphVec glx_MapChar[];

    ISendSet;
    register int wordsleft, fontaddr, wordsthistime;
    register unsigned char *charmasks;

# define PASSIZE	7	/* no. masks each passthru	*/

/* Load the font: */
    charmasks = (unsigned char *)&glx_MapChar[0];
    fontaddr = FONTBASEADDR;

    for( wordsleft = NUMCHARS*sizeof(GlyphVec)
	    ; wordsleft > 0; wordsleft -= PASSIZE)
    {
	ISendImm(0x808);	/* only passthru 8 allowed */
	ISendImm(LOADMASKSCMD);
	ISendWd(fontaddr);
	wordsthistime = PASSIZE /* > wordsleft ? wordsleft : PASSIZE */;
	while( --wordsthistime >= 0 )
	{
	    ISendWd((short)*charmasks++);
	}
	fontaddr += PASSIZE ;
    }

    /*
     * Select current font, polygon stipple character, and
     * cursorcharacter:
     */
    SendIRIS(FontSelections);

    gl1_spacing = 0;
}

/*
 * gl1_putat() --
 * draw the given char at the given position
 * (blhc, in pixels).
 */
gl1_putat(x,y,c,color)	/* erases char at (x,y) then draws c there */
    int x, y;	
    int c;
    int color;
{
    ISendSet;

    setspacing(0);

    ISendCmd(CHARPOSITIONCMD);		/* reset character position */
    ISendImm(POINTCMD);
    Coords(x,y);

    ColorMask(OFFCODE,color);
    ISendCmd(FIXCHARDRAWCMD);
    ISendWd(CharToAddr(ERASECHARNUM));	/* erase what was there */

    ColorMask(color,color);
    ISendCmd(FIXCHARDRAWCMD);		/* draw the character (fixchar_draw) */
    ISendWd(CharToAddr(c));
}

/*
 * gl1_putline() --
 * redraw the given line of len chars,
 * at the given position (blhc, in pixels),
 * on a screen line of width chars.
 */
gl1_putline(x,y,line,len,color)
    register int x,y;
    register unsigned char *line;
    short len;
    int color;
{
    ISendSet;
    register int c;

    setspacing(CHARWIDTH);

    ISendCmd(CHARPOSITIONCMD);
    ISendImm(POINTCMD);
    Coords(x,y);

    ColorMask(color,color);

    while( --len >= 0 )
    {
	ISendCmd(FIXCHARDRAWCMD);
	c = *line++;
	ISendWd(CharToAddr(c));
    }
}

/*
 * gl1_fill() --
 * fill the given box (blhc, trhc, in pixels)
 * with the given color and mask.
 */
gl1_fill(llx,lly,urx,ury,color,charwe)
    int llx,lly,urx,ury;
    int color,charwe;
{
    ISendSet;

    ColorMask(color,charwe);

    ISendWaitCmd;
    ISendWd(BLOCKFILLCMD);
    Coords(llx,lly);
    Coords(urx,ury);
}

static char SwizColors[] = { 0x00,0x01,0x04,0x05,0x10,0x11,0x14,0x15 };
gl1_mapcolor(m,color,vp)
    int m;
    int color;
    register ColorVec *vp;
{
    m = DCNumToReg(m);
    DCflags = DCBUSOP | m;
    DCMapColor(SwizColors[color],vp->r,vp->g,vp->b);
    DCflags = DCMULTIMAP | m;
}

gl1_setmap(m)
    register int m;
{
    DCflags = DCMULTIMAP | DCNumToReg(m);
}

gl1_nostate()
{
    gl1_color = -1; gl1_wemask = 0;
    gl1_spacing = -1;
}
