/*
 *	Kurt Akeley					7 September 1983
 *
 *	Illustrates the ability of the UC3-DC3 combination to scroll image
 *	  data by using the font memory as an intermediate buffer.
 *	A linear font memory space is used as a circular buffer.  This
 *	  space is broken into 2 blocks, one that is always drawn first
 *	  (start block), and one that is usually drawn second (it might
 *	  have zero lines, in which case it isn't drawn).  The second
 *	  block always occupies the bottom lines of the font memory space,
 *	  and is therefore called the base block.  A structure is defined
 *	  that includes pointers and sizes of these two blocks, as well as
 *	  other information needed to draw the scrolled image.
 *	This file is part of the console program, and cannot be used with
 *	  the graphics library.  Sorry...
 */

#include "console.h"
#include "uctest.h"
#include "ucdev.h"

#define RANDOMIZE(rand)		rand = rand * 5 + 17623

typedef struct {
    long fontbase;		/* base address of fm block used	    */
    long  fontstart;		/* base address of first block to be drawn  */
    long  fonttop;		/* address of font byte just beyond block   */
    short width, height;	/* pixel width and height of scrolled area  */
    short fontwidth;		/* number of font bytes per scan line	    */
    short startlines;		/* number of lines in start block, always   */
				/*   1 or more				    */
    short baselines;		/* number of lines in base (second) block   */
				/*   may be zero			    */
    long colorcode, wecode;	/* color and write-enable		    */
    } imagestruct;



drawimage (is, x, y)
imagestruct *is;
short x, y;
{
    /*
     *	Draws the image on the screen with lower-left corner at x,y.
     *	  The start block is drawn at the base of the screen region.
     *	  If the base block has more than 0 lines, it is drawn immediately
     *	  above the start block.
     */

    setcodes (is->colorcode, is->wecode);
    LDXS (x);
    LDYS (y);
    LDXE (x + is->width - 1);
    LDYE (y + is->startlines - 1);
    LDFMADDR (is->fontstart);
    REQUEST (DRAWCHAR, 0);

    if (is->baselines > 0) {
	LDYS (y + is->startlines);
	LDYE (y + is->height - 1);
	LDFMADDR (is->fontbase);
	REQUEST (DRAWCHAR, 0);
	}
    }



clearimage (is, x, y, pattern)
imagestruct *is;
short x, y;
long pattern;
{
    /*
     *	The region to be scrolled is cleared to the opposite color
     *	  that it is drawn with (i.e. planes that are drawn 0 are
     *	  cleared to 1, planes that are drawn 1 are cleared to 0).
     *	  The write-enable is set just like the draw case.
     *  Pattern is the font address of an 8x16 pattern to be used for
     *	  the clear.  This will normally be all ones, but can be
     *	  changed if desired.
     */
    setcodes (~(is->colorcode), is->wecode);
    LDXS (x);
    LDYS (y);
    LDXE (x + is->width - 1);
    LDYE (y + is->height - 1);
    LDFMADDR (pattern);
    REQUEST (CLEAR, 0);
    }



initimage (is, fontbase, width, height)
imagestruct *is;
long fontbase;
short width, height;
{
    /*
     *	Each image structure must be initialized before it is used.
     */
    long i;
    is->fontbase = fontbase;
    is->fontstart = fontbase;
    is->fontwidth = (width+7) >> 3;
    is->fonttop = fontbase + (height * is->fontwidth);
    is->width = width;
    is->height = height;
    is->startlines = height;
    is->baselines = 0;
    for (i=fontbase; i < is->fonttop; i++) {
	LDFMADDR (i);
	REQUEST (WRITEFONT, 0);
	}
    }



colorimage (is, colorcode, wecode)
imagestruct *is;
long colorcode, wecode;
{
    is->colorcode = colorcode;
    is->wecode = wecode;
    }



bottomline (is, line)
imagestruct *is;
char *line;
{
    /*
     *	Add a line to the bottom of the image.  This has the obvious side
     *	  effect of scrolling the image up 1 pixel.  The top line is lost
     *	  (it is overwritten with the new line, then the pointers and
     *	  block sizes are adjusted appropriately).
     */
    long fp, endfp;

    if (is->baselines == 0) {
	is->startlines = 1;
	is->baselines = is->height - 1;
	is->fontstart = is->fonttop - is->fontwidth;
	}
    else {
	is->startlines += 1;
	is->baselines -= 1;
	is->fontstart -= is->fontwidth;
	}
    endfp = is->fontstart + is->fontwidth;
    for (fp=is->fontstart; fp < endfp; fp++) {
	LDFMADDR (fp);
	REQUEST (WRITEFONT, *line++);
	}
    }



topline (is, line)
imagestruct *is;
char *line;
{
    /*
     *	Add a line to the top of the image.  This has the obvious side
     *	  effect of scrolling the image down 1 pixel.  The bottom line is lost
     *	  (it is overwritten with the new line, then the pointers and
     *	  block sizes are adjusted appropriately).
     */
    long fp, endfp;

    if (is->startlines == 1) {
	is->startlines = is->height;
	is->baselines = 0;
	is->fontstart = is->fontbase;
	}
    else {
	is->startlines -= 1;
	is->baselines += 1;
	is->fontstart += is->fontwidth;
	}
    fp = is->fontstart;
    for (endfp=fp+is->fontwidth; fp < endfp; fp++) {
	LDFMADDR (fp);
	REQUEST (WRITEFONT, *line++);
	}
    }


#define MAXTEXTURE		4
#define MAXLINE			8

scrolldemo (x, y, width, height, rate)
short x, y, width, height, rate;
{
    /*
     *	Intended to be called from console.c.  Allows an arbitrary screen
     *	  rectangle to be scrolled at any rate (positive is up, negative down)
     *	  with random image data.  Real image data could be added easily
     *	  enough.
     */
    imagestruct is;
    short config;
    char line[128];
    char *lp;
    register i, j;
    static long rand;
    static char texture[MAXTEXTURE] = {0, 0xff, 0xf, 0xf0};
    static short linecount = MAXLINE;

    initimage (&is, 2000, width, height);
    colorimage (&is, 0x101, 0x101);

    config = UPDATEA | DISPLAYB;
    while (1) {
	clearimage (&is, x, y, ONESTIPADDR);
	drawimage (&is, x, y);
	if (config & UPDATEA)
	    config = UPDATEB | DISPLAYA;
	else
	    config = UPDATEA | DISPLAYB;
	LDCONFIG (config);
	if (rate > 0) {
	    for (i=0; i<rate; i++) {
		linecount += 1;
		if (linecount >= MAXLINE) {
		    linecount = 0;
		    for (j=0, lp=line; j < is.fontwidth; j++) {
			RANDOMIZE (rand);
			*lp++ = rand;
/*			*lp++ = texture[rand&(MAXTEXTURE-1)]; */
			}
		    }
		bottomline (&is, line);
		}
	    }
	else {
	    for (i=0; i>rate; i--) {
		linecount += 1;
		if (linecount >= MAXLINE) {
		    linecount = 0;
		    for (j=0, lp=line; j < is.fontwidth; j++) {
			RANDOMIZE (rand);
			*lp++ = rand;
/*			*lp++ = texture[rand&(MAXTEXTURE-1)]; */
			}
		    }
		topline (&is, line);
		}
	    }
	}
    }
