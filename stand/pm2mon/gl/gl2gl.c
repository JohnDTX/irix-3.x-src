# undef  DIMTEST
/*
 * gl2gl.c --
 *
 *	supports GL2 hardware and DC3/DC4; multibus mode
 *
 * exports:
 *	gl2_probe()
 *	gl2_scrinit()
 *	gl2_putat(x,y,c,color)
 *	gl2_putline(x,y,line,len,color)
 *	gl2_fill(x0,y0,x1,y1,color,wemask)
 *	gl2_mapcolor(m,color,vp)
 *	gl2_setmap(m)
 *	gl2_nostate()
 *
 * This module is designed to replace the sunscreen.c module in the SUN
 * monitor.  These routines will perform low level output functions to
 * the IRIS graphics pipeline, to utilize the IRIS as a dumb terminal.
 */

# define IRIS		1
# define SUNV1PCREVD	1
# define PROMALONE	1
# define GF2		1
# define UC4		1
# define DC4		1
# define SCREENSTUFF	1
# define GFBETA		1
# define GESYSTEM	1
# define IRISOPCODES	1
# define INTER3		1

# define PROMSTATIC

# include "pm2.1.h"

# include "gl2dcdev.h"
# include "gl2uc4.h"
# include "gl2uctest.h"
# include "gl2screen.h"

# include "gl2gfdev.h"
# include "gl2cmds.h"

# include "glx.h"


PROMSTATIC	short gl2_color;
PROMSTATIC	short gl2_wemask;
PROMSTATIC	short gl2_ymax;
PROMSTATIC	short gl2_xmax;
PROMSTATIC	short gl2_ymin;
PROMSTATIC	short gl2_xmin;
PROMSTATIC	short gl2_ydim;
PROMSTATIC	short gl2_xdim;
PROMSTATIC	unsigned short gl2_dchw;
PROMSTATIC	unsigned short gl2_dcconfig;

# ifdef DIMTEST
short RS170_XDIM = 637;
short RS170_YDIM = 487;
short RS170_MARGIN = 32;

short GL2_XDIM = 1024;
short GL2_YDIM = 768;
short GL2_MARGIN = 0;

# else  DIMTEST

# define RS170_XDIM	637
# define RS170_YDIM	487
# define RS170_MARGIN	32

# define GL2_XDIM	1024
# define GL2_YDIM	768
# define GL2_MARGIN	0
# endif DIMTEST


# define Coords(x0,y0,x1,y1)	{ \
				LDXS(x0); LDYS(y0); \
				LDXE(x1); LDYE(y1); \
				}

# define ColorMask(c,m)		{ \
				if(gl2_color!=(c)) { \
					REQUEST(UC_SETCOLORAB,c); \
					gl2_color=(c); \
				} \
				if(gl2_wemask!=(m)) { \
					REQUEST(UC_SETWEAB,m); \
					gl2_wemask=(m); \
				} }

#define FBCsend(n)		{ \
				FBCdata = (n); \
				FBCflags = CYCINDEBUG; \
				FBCflags = RUNDEBUG; \
				}



/*
 * gl2_probe() --
 * return true iff it looks like a gl2 system.
 * ALSO set configuration variables.
 */
int
gl2_probe(dchigh)
    int dchigh;
{
    if( !probe(&FBCflags,2) )
	return 0;

    configure_screen(dchigh);

    { extern gl2_scrinit();	GLX.scrinit = gl2_scrinit; }
    GLX.dchw = gl2_dchw;
    GLX.dcconfig = gl2_dcconfig;

    GLX.hwversion = 2;
    return 1;
}

/*
 * gl2_scrinit() --
 * init hw and glx stuff to gl2.
 */
int
gl2_scrinit()
{
    initscreen();

    /* initialize glx package globals */
    { extern gl2_putat();	GLX.putat = gl2_putat; }
    { extern gl2_putline();	GLX.putline = gl2_putline; }
    { extern gl2_fill();	GLX.fill = gl2_fill; }
    { extern gl2_mapcolor();	GLX.mapcolor = gl2_mapcolor; }
    { extern gl2_setmap();	GLX.setmap = gl2_setmap; }
    { extern gl2_nostate();	GLX.nostate = gl2_nostate; }

    GLX.maxpix.x = gl2_xmax;
    GLX.maxpix.y = gl2_ymax;
    GLX.minpix.x = gl2_xmin;
    GLX.minpix.y = gl2_ymin;

    return 1;
}

static
configure_screen(dchigh)
    int dchigh;
{
    /*
       read the configuration prom bytes to get low/high modes of the 
       current dc4 board.  These bytes are masked and combined in 
       _commdat->dcconfig.  
       The prom dc mode is LOW unless the environment is the MONITOR
       environment and AUTOBOOT is set, in which case the mode is HIGH.
       This mode determination is done by qrom.c when the switches
       are examined.  It sets
       the flag DC_HIGH in the common structure flags word if
       the mode is HIGH.
    */
    register unsigned char temp,modes;

    /* read the LOW configuration byte */
    if( ((char *)ROM3)[0] == (char)DC4_PROMVAL0
     && ((char *)ROM3)[2] == (char)DC4_PROMVAL1 )
    {
	/* shift to the low position */
	temp = ((char *)ROM3)[4];
	modes = (temp>>4)&0xf;
	temp = ((char *)ROM3)[6];
	modes |= (temp & 0xf0);
    }
    else
    {
	/* oops - no prom.  Assume the default configuration */
	modes = /* HIGH = NI */ ((PIPE4|PROM)>>7) |
		/* LOW = I */   0;
    }

    gl2_dcconfig = modes;

    /*
       set up the screen parameters
       (gl2_xmax, gl2_ymax)
       by the default dc mode
     */

    if( dchigh ) modes >>= 4;

    if( modes & ((OPTCLK>>11) & 0xF) )
    {
	/* default is RS170 - */
	gl2_xdim = RS170_XDIM; gl2_ydim = RS170_YDIM;
	gl2_xmin = RS170_MARGIN; gl2_ymin = RS170_MARGIN;
	gl2_xmax = RS170_XDIM-RS170_MARGIN; gl2_ymax = RS170_YDIM-RS170_MARGIN;
    } 
    else
    {
	gl2_xdim = GL2_XDIM; gl2_ydim = GL2_YDIM;
	gl2_xmin = GL2_MARGIN; gl2_ymin = GL2_MARGIN;
	gl2_xmax = GL2_XDIM-GL2_MARGIN; gl2_ymax = GL2_YDIM-GL2_MARGIN;
    }

    gl2_dchw = gl2_dcconfig << (dchigh ? 7 : 11);
    gl2_dchw = (gl2_dchw&0x7800) | DCMULTIMAP;

    gl2_color = -1; gl2_wemask = 0;
}

static
initscreen()
{
    inithardware();
    loadfont();

    DCflags = gl2_dchw;

    gl2_fill(0,0,gl2_xdim-1,gl2_ydim-1,OFFCODE,SCREENWE);
    /* drawlogo(); */
}

static
inithardware()
{
    UC4setup;

    FBC_Reset();

    *UCRAddr = /*UCR_BOARDENAB + */ UCR_MBENAB;
    LDMODE(UCMODE);
    LDCONFIG(CONFIGPARAM);
    Coords(0,0,gl2_xdim-1,gl2_ydim-1);
    REQUEST(UC_SETSCRMASKX,0)
    REQUEST(UC_SETSCRMASKY,0)
}

PROMSTATIC	short gl2_rdelay = 2;
/*
 *   FBC_Reset()
 *	foolproof reset sequence
 *	returns 0x40 if OK
 */
static
FBC_Reset()
{
	register short _ii;
	register short splsave;

	splsave = spl7();

	GEflags = GERESET1;
	FBCflags = STARTDEV;
	decimsdelay(gl2_rdelay);/*50*/
	FBCflags = STARTDEV & ~FORCEREQ_BIT_;
	FBCflags = STARTDEV & ~FORCEACK_BIT_;
	FBCflags = STARTDEV;
	FBCclrint;
	FBCflags = RUNDEBUG;
	FBCdata = 8;
	for (_ii=0; _ii<20; _ii++) {
		decimsdelay(gl2_rdelay);/*50*/
		FBCclrint;
	}
	FBCflags = READOUTRUN;
	_ii = FBCdata;
	GEflags = GEDEBUG;
	FBCflags = RUNDEBUG;

	splx(splsave);
	return(_ii);
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
    extern GlyphVec glx_MapChar[];

    UC4setup;
    register int wordsleft;
    register short i;
    register unsigned char *charmasks;

    LDFMADDR(FONTBASEADDR);
    REQUEST(UC_SETADDRS,0)

    charmasks = (unsigned char *)&glx_MapChar[0];
    wordsleft = NUMCHARS * sizeof(GlyphVec);

    while( --wordsleft >= 0 )
    {
	i = ((short)*charmasks++<<8) | 0xFF;
	REQUEST(WRITEFONT,i)
    }
}

/*
 * gl2_putat() --
 * draw the given char at the given position
 * (blhc, in pixels).
 */
gl2_putat(x,y,c,color)	/* draws c at (x,y) */
	register int	x, y;	
	int c;
	int color;
{
	UC4setup;

	Coords(x,y,x+CHARWIDTH-1,y+CHARHEIGHT-1);

	ColorMask(OFFCODE,color);
	LDFMADDR(CharToAddr(ERASECHARNUM));
	REQUEST(UC_FILLRECT,0)

	ColorMask(color,color);
	LDFMADDR(CharToAddr(c));
	REQUEST(UC_DRAWCHAR,0)		/* draw the character */
}

/*
 * gl2_putline() --
 * redraw the given line of len chars,
 * at the given position (blhc, in pixels),
 * on a screen line of width chars.
 */
gl2_putline(x,y,line,len,color)
	register int x,y;
	register unsigned char *line;
	short len;
	int color;
{
	UC4setup;
	register int c;

	ColorMask(color,color);

	LDYS(y);
	LDYE(y+CHARHEIGHT-1);

	while( --len >= 0 )
	{
	    LDXS(x);
	    LDXE(x+CHARWIDTH-1);
	    c = *line++;
	    LDFMADDR(CharToAddr(c));
	    REQUEST(UC_DRAWCHAR,0)		/* draw the character */
	    x += CHARWIDTH;
	}
}

/*
 * gl2_fill() --
 * fill the given box (blhc, trhc, in pixels)
 * with the given color and mask.
 */
gl2_fill(llx,lly,urx,ury,charcolor,wemask)
	int llx,lly,urx,ury;
	int charcolor,wemask;
{
	UC4setup;

	Coords(llx,lly,urx,ury);
	ColorMask(charcolor,wemask);
	LDFMADDR(CharToAddr(ERASECHARNUM));
	REQUEST(UC_FILLRECT,0)
}

gl2_mapcolor(m,color,vp)
    int m;
    int color;
    register ColorVec *vp;
{
    m = DCNumToReg(m);
    DCflags = DCBUSOP | m;
    DCMapColor(color,vp->r,vp->g,vp->b);
    DCflags = DCMULTIMAP | m;
}

gl2_setmap(m)
    int m;
{
    DCflags = gl2_dchw | DCNumToReg(m);
}

gl2_nostate()
{
    gl2_color = -1;
    gl2_wemask = 0;
}
