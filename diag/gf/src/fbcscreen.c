/*
 * fbcscreen.c
 *
 *	supports beta microcode and DC2/DC3; multibus mode
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
#define GFBETA 1
#define DC3 1
#define SCREENSTUFF 1
/**/

#include	"pm2.1.h"
#include	"dcdev.h"
#define GESYSTEM 1
#define IRISOPCODES 1
#include	"fbcdata.h"
#define UCTEST_DECLARE_VARIABLES
#define INTER2
#include	"uctest.h"

#undef	set
#define GE0flags	GEflags

#define ISendWait	while (!(*a3 & (short)FBCREQ_BIT_))
#define ISendWaitCmd	while (FBCdata != 0x40) ;

#define ISendCycle	*a3 = forcereq; \
			*a3 = forcewait;
#define TESTFLAGS	FBCflags

#define ISendSet	register short *a5 = 0; \
			register short *a4 = (short *)&FBCdata; \
			register int d7 = 0; \
			register short *a3 = (short *)&TESTFLAGS

#define ISendImm(dat)	{ \
			*a4 = dat; \
			ISendCycle; \
			ISendWait;  \
			}
#define ISendWd(src)	{ *a4 = src; \
			ISendCycle; \
			ISendWait;  \
			}
#define ISendCmd(cmd)	{ \
			ISendWaitCmd; \
			*a4 = cmd; \
			ISendCycle; \
			}

#define CHARTOSCREENX(x)	(x<<3)
#define CHARTOSCREENY(y)	( YMAX - ( (y+4) <<4 ) )

unsigned short forcereq=FORCEREQ;
unsigned short forcewait=FORCEWAIT;
unsigned short erasecharaddr = ERASECHARADDR;

initscreen()
{
    inithardware();
    loadcolormap();
    loadfont();
    SendIRIS(WindowViewportInfo);
    fill(0, 0, XMAX, YMAX,0);	/* Clear the screen */
#ifdef IRISLOGO
    drawlogo();
#endif
}

initerrorscreen()
{
    inithardware();
    loadcolormap();
    loadfont();
    SendIRIS(WindowViewportInfo);
    fill(0, YMAX - 5*CHARHEIGHT, XMAX, YMAX,0);
}


inithardware()
{
    register curint;

    curint = spl7();
    UCTEST_INIT;
    FBCdisabvert(RUNDEBUG);
    splx(curint);

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
    ISendSet;
    register int wordsleft, fontaddr, wordsthistime, i;
    register char *charmasks;

/* Loads a fixed width font into the IRIS's font ram, and selects
 * this font for character display.  The 0th character in the font
 * is used for a polygon stipple pattern, and one character is used
 * for a cursor character.
 */

#define PASSIZE	7	/* no. masks each passthru	*/

/* Load the font: */

    charmasks = (char *)&MapChar[0];
    wordsleft = NUMCHARS << LOGBYTES;
    fontaddr = FONTBASEADDR;
    for (; wordsleft > 0; wordsleft -= PASSIZE)
    {
	wordsthistime = PASSIZE;
	ISendImm(0x808);	/* only passthru 8 allowed */
	ISendImm(LOADMASKSCMD);
	ISendWd(fontaddr);
	for (i=0; i<wordsthistime; i++)
	{	/* brackets needed for macro!  */
	    ISendWd( (short)*charmasks++);
	}
	fontaddr += PASSIZE ;
    }

/* Select current font, polygon stipple character, and
 * cursorcharacter:
 */

    SendIRIS(FontSelections);
    SendIRIS(CursorSetup);
}


clearEOL(x, y)
register int	x, y;
{
    y = CHARTOSCREENY(y);
    fill(CHARTOSCREENX(x), y, XMAX, y+CHARHEIGHT,1); /* erase char plane */
}


putcursor(x, y, set)
int x, y, set;
{
    ISendSet;

while (FBCdata != 0x40) ;
    if (set)
    {
	x = CHARTOSCREENX(x);
	y = CHARTOSCREENY(y);
	ISendCmd(DRAWCURSORCMD);
	ISendWd(x);
	ISendWd(x);
	ISendWd(x);	/* yes, "x"!	*/
	ISendWd(y);
    }
    else
    {
	ISendCmd(UNDRAWCURSORCMD);
    }
   ISendWd(8);		/* needed for v.9 ucode (back-compat.) */
}

putat(x, y, c)	/* erases char at (x,y) then draws c there */
int	x, y;	
char	c;
{
    ISendSet;
    register short temp;	/* d7 */

    /* stack parts of coords for speed */
    register int xlsw, ylsw;	/* d6, d5 */
    register int xmsw, ymsw;	/* d4, d3 */
    register short i;

    xmsw = CHARTOSCREENX(x);
    ymsw = CHARTOSCREENY(y);

while (FBCdata != 0x40);
    ISendCmd(COLORCMD);		/* set color to "off	*/
    i = OFFCODE;
    ISendImm(i);
    ISendImm(OFFCODE);

    ISendCmd(CHARPOSITIONCMD);		/* reset character position */
    ISendImm(POINTCMD);
    ISendWd(xmsw);
    ISendWd(xmsw);
    ISendWd(xmsw);	/* yes, x!	*/
    ISendWd(ymsw);

    ISendCmd(0x2d);
    ISendWd(erasecharaddr);	/* erase what was there */

    ISendCmd(COLORCMD);		/* reset color to draw */
    ISendImm(ONCODE);
    ISendImm(ONCODE);

    ISendCmd(0x2d);		/* draw the character (fixchar_draw) */
    ISendWd(CharToAddr(c));
}


fill(llx, lly, urx, ury, charcolor)
int llx, lly, urx, ury, charcolor;
{
/*
**  clear box to color charcolor:
**	1=erase only char plane
**	0=erase to background color
*/
while (FBCdata != 0x40) ;
    if (charcolor) 
	SendIRIS(SetColorCharoff);
    else 
	SendIRIS(SetColorBlack);		/* meaning background */
while (FBCdata != 0x40) ;
    SendIRIS(AreaFill);
    IRISCoords(llx, lly);
    IRISCoords(urx, ury);
while (FBCdata != 0x40) ;
    SendIRIS(SetColorWhite);
}


IRISCoords(x, y)
int		x, y;
{
    IRISdata coorddata[5];
    register IRISdata *ptr;

    ptr = coorddata;
    *ptr++ = 4;		/* no. shorts to send */
    *ptr++ = x;
    *ptr++ = x;
    *ptr++ = x;		/* yes, x!	*/
    *ptr++ = y;
    SendIRIS(coorddata);
}


SendIRIS(datablock)
IRISdata *datablock;
{
    ISendSet;
    register short	wordsleft;

    wordsleft = *datablock++;
    for (; wordsleft>0; wordsleft--)
    {
	ISendWd( *datablock++);
    }
}
