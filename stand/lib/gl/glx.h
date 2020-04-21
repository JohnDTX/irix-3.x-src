/*
 * $Source: /d2/3.7/src/stand/lib/gl/RCS/glx.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:15:22 $
 */
# ifndef _GLX_

# define _GLX_

struct GLXxy
{
    short x,y;		/* x-y pair */
};

struct GLXlo
{
    short l,o;		/* length-offset pair */
};

struct GLXwin
{
    struct GLXxy tlhc;		/* top left hand corner (inclusive) */
    struct GLXxy siz;		/* window dimension (interior) */
    char *text;			/* pointer to text area */
    struct GLXlo *lines;	/* (offset,length) pairs */
};

struct GLX
{
    char hwversion;		/* gl1 / gl2 */
    char hwflags;		/* not used */
    unsigned short dchw;	/* DC hardware register */
    unsigned short dcconfig;	/* DC configuration modes */
    char dchi;			/* flag - DC config high / low */
    char inited;		/* flag - inited */

    int (*scrinit)();		/* scrinit() */
    int (*putat)();		/* putat(x,y,c,color) */
    int (*putline)();		/* putline(x,y,p,n,color) */
    int (*fill)();		/* fill(x0,y0,x1,y1,color,mask) */
    int (*mapcolor)();		/* mapcolor(m,c,vp) */
    int (*setmap)();		/* setmap(m) */
    int (*nostate)();		/* nostate() */

    struct GLXxy maxpix;	/* max pixel address (not inclusive) */
    struct GLXxy minpix;	/* min pixel addresss (inclusive) */
    struct GLXxy curpix;	/* cursor position (xpix,ypix) */
    struct GLXxy curr;		/* cursor position (line,col) */
    short yrefpix;		/* pixel address of top line */
    short cursoron;		/* cursor onoff flag */
    short charcolor;		/* current char color */
    short curscolor;		/* current cursor color */
    short mapnum;		/* current color map number */

    short nblines;		/* #blank lines following cursor */
    struct GLXwin win;		/* window: scrolling info */

}    GLX;

extern struct GLX GLX;

/*
 * conversion from screen col (chars) to x (pixels)
 * and from line (lines) to y (pixels):
 * columns increase with increasing x, and are
 * aligned wrt pixel 0.
 * lines decrease with increasing y, and are not
 * necessarily aligned wrt pixel 0.
 */
# define CHARTOSCREENX(C)	CHARTOPIX(C)
# define LINETOSCREENY(L)	(GLX.yrefpix-LINETOPIX(L))

# define PIXTOCHAR(x)		((x)>>Log2CharWidth)
# define PIXTOLINE(y)		((y)>>Log2CharHeight)
# define CHARTOPIX(C)		((C)<<Log2CharWidth)
# define LINETOPIX(L)		((L)<<Log2CharHeight)

# define SCREENXTOCHAR(x)	PIXTOCHAR(x)
# define SCREENYTOLINE(y)	PIXTOLINE(GLX.yrefpix+CHARHEIGHT-1-(y))

# define CHARHEIGHT		16
# define Log2CharHeight		4
# define CHARWIDTH		8
# define Log2CharWidth		3

#define	OFFCODE			X_BLACK

# define X_BLACK		0x0
# define X_WHITE_GREEN		0x1	/* BIT PLANE */
# define X_GREEN_WHITE		0x2	/* BIT PLANE */
# define X_WHITE		(X_WHITE_GREEN|X_GREEN_WHITE)
# define X_GREEN		0x4
# define X_RED			0x5
# define X_BLUE			0x6
# define X_YELLOW		0x7
# define N_GLX_COLORS		0x8

# define CHARWEA		X_WHITE_GREEN
# define CHARWEB		X_GREEN_WHITE
# define CHARWE			X_WHITE
# define LINECODE		X_WHITE
# define SCREENWE		0xFFFFFFFF

#ifdef notdef
# define NUMCHARS		0x90
#endif
#define NUMCHARS		0x100

typedef struct { unsigned short r,g,b; } ColorVec;
typedef struct { unsigned char row[16]; } GlyphVec;

# endif _GLX_
