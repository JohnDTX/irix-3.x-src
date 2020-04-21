/*
 * ucscreen.c
 *
 *	supports GL2 hardware and DC3/DC4; multibus mode
 *
 * exports:
 *	initscreen()
 *	clearEOL(x, y)
 *	putcursor(x, y, set)
 *	putat(x, y, c)
 * imports:
 *	drawlogo()	from logoscreen.c
 *
 * This module is designed to replace the sunscreen.c module in the SUN
 * monitor.  These routines will perform low level output functions to
 * the IRIS graphics pipeline, to utilize the IRIS as a dumb terminal.
 */

 /* the following defines used to be invoked from the command line */

#define	IRIS 1
#define SUNV1PCREVD 1
/* define PM1 or PM2 elsehwere */
#define PROMALONE 1
#define GF2 1
#define UC4 1
#define DC4 1
#define SCREENSTUFF 1
/**/

#include	"pm2.1.h"
#include	"dcdev.h"
#define GESYSTEM 1
#define IRISOPCODES 1
#define INTER3
#include	"uc4test.h"
#include	"ucscreen.h"

#define CHARTOSCREENX(x)	(x<<3)
#define CHARTOSCREENY(y)	( ymax - ( (y+4) <<4 ) )

#include "gf2init.c"
#include "common.h"
extern unsigned short dctemp;

initscreen()
{
    inithardware();
    loadcolormap();
    loadfont();
    dctemp = _commdat->dcconfig << 8;
    if (DEFAULT_DC_HIGH)
	/* use the high bits in the dcconfig byte */
	dctemp = (_commdat->dcconfig & 0xf0) << 8;
    else
	dctemp = (_commdat->dcconfig & 0xf) << 12;
    dctemp |= DCMULTIMAP;
    DCflags = dctemp;
    fill(0, 0, xmax, ymax,0);	/* Clear the screen */
#ifdef IRISLOGO
    drawlogo();
#endif
}

initerrorscreen()
{
    inithardware();
    loadcolormap();
    loadfont();
    dctemp = _commdat->dcconfig << 8;
    if (DEFAULT_DC_HIGH)
	/* use the high bits in the dcconfig byte */
	dctemp = (_commdat->dcconfig & 0xf0) << 8;
    else
	dctemp = (_commdat->dcconfig & 0xf) << 12;
    dctemp |= DCMULTIMAP;
    DCflags = dctemp;
    fill(0, ymax - 5*CHARHEIGHT, xmax, ymax,0);
}


inithardware()
{
    UC4setup;
    register curint;

    curint = spl7();
    FBC_Reset();
    splx(curint);

    *UCRAddr = UCR_BOARDENAB + UCR_MBENAB;
    LDMODE(UCMODE);
    LDCONFIG(CONFIG);
    LDXS(0);
    LDYS(0);
    LDXE(xmax);
    LDYE(ymax);
    REQUEST(UC_SETSCRMASKX,0)
    REQUEST(UC_SETSCRMASKY,0)
    REQUEST(UC_SETWEAB,IRISWE)
}


loadcolormap()
{
/* The simplest possible color map is loaded, with only the colors 
 * black and white. Noting that unused bit plane outputs float low,
 * and assuming that bit planes are installed LS first, only codes
 * 0 and 1 are used. Depending on a config switch, the codes are
 * assigned BLACK/WHITE or WHITE/BLACK.
 */
/* the colors are loaded into map 0.	*/

    register int last, i, j, offmap, onmap;
    register short *pred, *pgrn, *pblu;

    pred = &DCramRed(0);	/* use map 0 to avoid version hacking */
    pgrn = &DCramGrn(0);
    pblu = &DCramBlu(0);

#define ColorMap(red,grn,blu)	*pred++ = (red|(red<<8)); \
				*pgrn++ = (grn|(grn<<8)); \
				*pblu++ = (blu|(blu<<8))

    offmap = BLACK;
    onmap = WHITE;

    DCflags = DCBUSOP;

    ColorMap(offmap,offmap,offmap);
    ColorMap(onmap,onmap,onmap);
    ColorMap(IRISR,IRISG,IRISB);
    ColorMap(onmap,onmap,onmap);

    DCflags = DCMULTIMAP;
}


loadfont()
{
    UC4setup;
    register int wordsleft;
    register short i;
    register short *charmasks;

/* Loads a fixed width font into the IRIS's font ram
 * for character display.  The 0th character in the font
 * is used for a polygon stipple pattern, and one character is used
 * for a cursor character.
 */

    charmasks = (short *)&MapChar[0];
    wordsleft = NUMCHARS << LOGBYTES;
    LDFMADDR(FONTBASEADDR);
    REQUEST(UC_SETADDRS,0)

    for (; wordsleft > 0; --wordsleft) {
	i = *charmasks++;
	REQUEST(WRITEFONT,i)
    }
}


clearEOL(x, y)
register int	x, y;
{
    extern int maxchar;
    y = CHARTOSCREENY(y);
    /* erase char plane */
    fill(CHARTOSCREENX(x), y, CHARTOSCREENX(maxchar), y+CHARHEIGHT,1); 
}


putcursor(x, y, curset)
	register int x, y, curset;
{
	UC4setup;

	x = CHARTOSCREENX(x);
	y = CHARTOSCREENY(y);
	LDXS(x);
	LDYS(y);
	LDXE(x+CHARWIDTH-1);
	LDYE(y+CHARHEIGHT-1);
	LDFMADDR(CURSORCHARADDR);

if (curset) REQUEST(UC_SETCOLORAB,ONCODE)
else REQUEST(UC_SETCOLORAB,OFFCODE)

	REQUEST(UC_FILLRECT,0)
}


putat(x, y, c)	/* erases char at (x,y) then draws c there */
	register int	x, y;	
	char	c;
{
	UC4setup;
	register short temp;	/* d7 */
	register short i;

    putcursor(x,y,0);	/* clear the area */

    x = CHARTOSCREENX(x);
    y = CHARTOSCREENY(y);

    REQUEST(UC_SETCOLORAB,ONCODE)	/* reset color to draw */
    LDFMADDR(CharToAddr(c));
    REQUEST(UC_DRAWCHAR,0)		/* draw the character */
}


fill(llx, lly, urx, ury, charcolor)
	int llx, lly, urx, ury, charcolor;
{
/*
**  clear box to color charcolor:
**	1=erase only char plane
**	0=erase to background color
*/
	UC4setup;

	REQUEST(UC_SETCOLORAB,OFFCODE)
	if (charcolor) 
		REQUEST(UC_SETWEAB,CHARWE)
	LDXS(llx);
	LDYS(lly);
	LDXE(urx);
	LDYE(ury);
	LDFMADDR(ERASECHARADDR);
	REQUEST(UC_FILLRECT,0)
	REQUEST(UC_SETCOLORAB,ONCODE)
	if (charcolor) 
		REQUEST(UC_SETWEAB,IRISWE)
}

drawborder(llx,lly,urx,ury) 
{
	/* draw a border on the IRIS screen, given the coordinates
	   (line,col) of the lower left and upper right vertices.
	*/
	UC4setup;

	LDEC(0);
	LDED(0);
	LDREPEAT(0);

	/* draw a two-pixel deep line for the upper border */
	LDXS(CHARTOSCREENX(llx)+3);
	LDYS(CHARTOSCREENY(ury)+3);
	LDXE(CHARTOSCREENX(urx)+4);
	REQUEST(DRAWLINE2,(-1));
	LDXS(CHARTOSCREENX(llx)+3);
	LDYS(CHARTOSCREENY(ury)+4);
	LDXE(CHARTOSCREENX(urx)+4);
	REQUEST(DRAWLINE2,(-1));

	/* two-pixel wide line for the left border */
	LDXS(CHARTOSCREENX(llx)+3);
	LDYS(CHARTOSCREENY(lly)+3);
	LDYE(CHARTOSCREENY(ury)+4);
	REQUEST(DRAWLINE1,(-1));
	LDXS(CHARTOSCREENX(llx)+4);
	LDYS(CHARTOSCREENY(lly)+3);
	LDYE(CHARTOSCREENY(ury)+4);
	REQUEST(DRAWLINE1,(-1));

	/* two-pixel wide line for the right border */
	LDXS(CHARTOSCREENX(urx)+3);
	LDYS(CHARTOSCREENY(lly)+3);
	LDYE(CHARTOSCREENY(ury)+4);
	REQUEST(DRAWLINE1,(-1));
	LDXS(CHARTOSCREENX(urx)+4);
	LDYS(CHARTOSCREENY(lly)+3);
	LDYE(CHARTOSCREENY(ury)+4);
	REQUEST(DRAWLINE1,(-1));

	/* draw a two-pixel deep line for the bottom border */
	LDXS(CHARTOSCREENX(llx)+3);
	LDYS(CHARTOSCREENY(lly)+3);
	LDXE(CHARTOSCREENX(urx)+4);
	REQUEST(DRAWLINE2,(-1));
	LDXS(CHARTOSCREENX(llx)+3);
	LDYS(CHARTOSCREENY(lly)+4);
	LDXE(CHARTOSCREENX(urx)+4);
	REQUEST(DRAWLINE2,(-1));

}
