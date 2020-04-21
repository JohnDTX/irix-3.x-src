/*
 *	Kurt Akeley			9/18/82
 *
 *	These routines are intended to be linked with the main routine
 *	"console.b".
 *
 *	Functions:
 *		long	planecode (colorindex, sigplanes, cfr)
 *		short	getindex (planecode, sigplanes, cfr)
 *		short	mapcode (planecode, dcflags, cfr)
 *		long	newcode (index, sigbits)
 *		short	newindex (code, sigbits)
 *		boolean	readmap (index, red, green, blue)
 *		long	rgbcode (red, green, blue)
 *		long	gethexvalue (current, lowbound, highbound, string)
 *		long	bits (value, on, off)
 *
 *	Procedures:
 *		init (verbose, modify)
 *		drawline (x0, y0, x1, y1, stipple)
 *		display (leftx, bottomy, height)
 *		alphalist (list, separation)
 *		waitvert ()
 *		drawrect (x, y, width, height, fontaddr)
 *		drawchar (x, y, width, height, fontaddr)
 *		mapcolor (mapcode, sigmapbits, red, green, blue)
 *		printconfig (cfr)
 *		printmode (mdr)
 *		printdcflags (dcflags)
 *		joescale ()
 *		printplanes (p)
 *		save (s)
 *		restore (s)
 *		printrestore ();
 *
 *	Routines:
 *
 *	Updates:
 *		9/18/82	 KBA	Copied from console.c
 *		9/20/82  KBA	Drawrect added
 *		9/21/82  KBA	Static variables added to init
 *		9/27/82  KBA	Changes to color commands
 *		10/26/82 KBA	Version 2 color commands added
 *		10/28/82 KBA	DCPAL command added
 *		2/3/83   KBA	getindex added, some names changed
 *		3/18/83  KBA	added graypattern
 *		4/7/83   KBA	added rgbcode
 *		4/9/83   KBA	verbose argument added to init
 *		5/17/83  KBA	changed printdcflags to correspond to new vfsm
 *		7/6/83   KBA	changed graypattern to joescale, added colors
 *		12/6/83  KBA	added INIT_GFONLY mode to init routine
 *				  allows startup without screen clear
 */

#include "dcdev.h"
#include "ucdev.h"
#include "bpccodes.h"
#include "vfsm.h"
#include "uctest.h"
#include "console.h"
#include "getcmnd.h"
#ifdef INTER2
#include <m68000.h>	/* for intlevel only */
#endif INTER2

#define	INIT_NORMAL	0	/* Initialize to static values		*/
#define INIT_MODIFY	1	/* Change statics, then initialize	*/
#define INIT_INIT	2	/* Change default statics, then statics	*/
				/*   then initialize			*/
#define INIT_GFONLY	3	/* Initialize only the gf interface	*/
#define INIT_NOCLEAR	4	/* Normal init, but do not clear screen */

init (verbose, mode)
boolean	verbose;	/* if true, state information is printed	*/
short	mode;		/* indicate operations to be performed		*/
{
/*
 *	Clears the entire bit map to zero.  Leaves several variables
 *	  in the initial configuration that is specified by the
 *	  user.  This configuration can be changed if init is called
 *	  with modify true.
 *	Loads fonts for clear and stipple.
 *	Initializes the writeword space.
 *	Sets the window to the buffer boundries and enables it.
 */
    long		i, j, k;
    short		sigmapbits;
    short		tempmapcode;
    long		pixels[16];
    static boolean	i_init		= TRUE;
    static short	i_cfr		= UPDATEA|UPDATEB|DISPLAYA|DISPLAYB|
				  	  VIEWPORTMASK|LDLINESTIP|BACKLINE;
    static short	i_mdr		= 0;
    static short	i_dcflags	= 0;
    static short	i_colorindex	= 0;
    static short	i_wecode	= 0xffffffff;
    static long		i_sigplanes;
    static long		i_instplanes;

#ifdef INTER2
    UCTEST_INIT;	/* for GF board, really */
    intlevel (1);	/* restore to allow paul's library code to run */
#endif INTER2
#ifdef INTER3
    UCR (UCR_MBENAB);
#endif INTER3
    if (mode == INIT_GFONLY)
	return;
    if (mode == INIT_INIT)
	i_init = TRUE;
/* Determine which bitplanes are installed */
    if (i_init && mode != INIT_NOCLEAR) {
#ifdef UC4
	LDMODE (0)
#endif UC4
	LDCONFIG (UPDATEA | UPDATEB | DISPLAYA | DISPLAYB | LDLINESTIP);
	setcodes (0xffffffff, 0xffffffff);
	LDXS (0)
	LDXE (0)
	LDYS (0)
	LDYE (0)
	REQUEST (DRAWLINE1, 1)
	readpixels (pixels, 0, 0, 0xffffffff);
	i_wecode = i_instplanes = i_sigplanes = pixels[0];
	}

    if (i_init || mode) {
	i_init = FALSE;
	i_cfr = gethexvalue (i_cfr, 0, 0xffff, "configuration");
#ifdef UC4
	i_mdr = gethexvalue (i_mdr, 0, 0x7, "mode");
#endif UC4
	i_dcflags = gethexvalue (i_dcflags, 0, 0xffff, "dc register");
	i_colorindex = gethexvalue (i_colorindex, 0, 0xfff, "color index");
	i_wecode = gethexvalue (i_wecode, 0, 0xffffffff, "write enable code");
	i_sigplanes = gethexvalue (i_sigplanes, 0, 0xffffffff,
			"significant bitplanes");
	}

    LDCONFIG (i_cfr);
#ifdef UC4
    LDMODE (i_mdr);
    LDREPEAT (0);
#endif UC4
    DCR (i_dcflags);
    installedplanes = i_instplanes;
    sigplanes = i_sigplanes;
    colorindex = i_colorindex;
    wecode = i_wecode;
    colorcode = planecode (colorindex, sigplanes, uc_cfb);

    /* initialize the font memory */
    setstipple (ONESTIPADDR, onestipple);
    setstipple (CHECKSTIPADDR, checkstipple);

    /* initialize the colormap */
    mapcolor (0, 0, 0, 0, 0);	/* set all map entries to zero */

    sigmapbits = mapcode (sigplanes, dc_dcr, uc_cfb);
    tempmapcode = mapcode (planecode (3, sigplanes, uc_cfb), dc_dcr, uc_cfb);
    mapcolor (tempmapcode, sigmapbits, 255 ,255 ,255);

    tempmapcode = mapcode (planecode (2, sigplanes, uc_cfb), dc_dcr, uc_cfb);
    mapcolor (tempmapcode, sigmapbits, 0, 0, 255);

    tempmapcode = mapcode (planecode (1, sigplanes, uc_cfb), dc_dcr, uc_cfb);
    mapcolor (tempmapcode, sigmapbits, 0, 255, 0);

    tempmapcode = mapcode (planecode (0, sigplanes, uc_cfb), dc_dcr, uc_cfb);
    mapcolor (tempmapcode, sigmapbits, 255 ,0 ,0);

    /* initialize the viewportmask */
    setviewport (0, 0, 1024, 1024);

    /* clear entire screen to colorcode */
    if (mode != INIT_NOCLEAR) {
	setcodes (colorcode, 0xffffffff);
	clear (ONESTIPADDR);
	}
    setcodes (colorcode, wecode);

    if (verbose) {
	printconfig (uc_cfb);
	printmode (uc_mdb);
	printdcflags (dc_dcr);
	printf ("  significant bitplanes: %8x\n", sigplanes);
	printf ("  installed bitplanes:   %8x\n", installedplanes);
	printf ("  color index:           %8x\n", colorindex);
	printf ("  write enable code:     %8x\n", wecode);
	}
    }



drawline (x0, y0, x1, y1, stipple)
short	x0, y0, x1, y1, stipple;
{
    /*
     *	Computes the Bresenham coefficients and calls the appropriate
     *	  line drawing command.
     */

    short	dx, dy, absdx, absdy;

    LDXS (x0)
    LDXE (x1)
    LDYS (y0)
    LDYE (y1)

    dx    = x1 - x0;
    absdx = (dx >= 0) ? dx : -dx;
    dy    = y1 - y0;
    absdy = (dy >= 0) ? dy : -dy;

    if (dy >= 0) {
	if (absdy >= absdx) {
	    LDED (-absdx)
	    LDEC ( absdy)
	    if (dx >= 0)
		REQUEST (DRAWLINE1, stipple)
	    else
		REQUEST (DRAWLINE11, stipple)
	    }
	else {
	    LDED (-absdy)
	    LDEC ( absdx)
	    if (dx >= 0)
		REQUEST (DRAWLINE2, stipple)
	    else
		REQUEST (DRAWLINE10, stipple)
	    }
	}
    else {
	if (absdy >= absdx) {
	    LDED (-absdx)
	    LDEC ( absdy)
	    if (dx >= 0)
		REQUEST (DRAWLINE5, stipple)
	    else
		REQUEST (DRAWLINE7, stipple)
	    }
	else {
	    LDED (-absdy)
	    LDEC ( absdx)
	    if (dx >= 0)
		REQUEST (DRAWLINE4, stipple)
	    else
		REQUEST (DRAWLINE8, stipple)
	    }
	}
    }



display (leftx, bottomy, height)
short	leftx;		/* x pixel address of lower-left corner to display */
short	bottomy;	/* y pixel address of ............................ */
short	height;		/* number of scan lines to display		   */
{
    short	x, y, i;
    short	index;
    long	pixels[16];		/* stores 16 pixels 		*/
    Save	s;

    save (&s);
    leftx >>= 4;	/* convert to word address */
    for (y=bottomy+height-1; y>=bottomy; y--) {
        printf ("%3x ", y);
        for (x = leftx; x < (leftx + 4); x++) {
	    readpixels (pixels, x, y, sigplanes);
	    for (i=0; i<16; i++) {
		index = getindex (pixels[i], sigplanes, s.cfb);
		if (index == 0)
		    printf (".");
		else
		    printf ("%1x", index&0xf);
		}
	    printf (" ");
	    }
        printf ("\n");
        }

    leftx <<= 4;	/* back to pixel addressing */
    printf ("\n");
    for (i=8; i>=0; i-=4) {
        printf ("    ");
        for (x=leftx; x < leftx+64; x++) {
	    printf ("%1x", ((x>>i) & 15));
	    if ((x & 15) == 15)
	        printf (" ");
	    }
        printf ("\n");
        }
    restore (&s);
    }



alphalist (list, separation)
Cmnddef		*list;
short		separation;		/* separation between columns	*/
{
    /*
     *	4 August 1984 KBA - No sort!  New data format.
     *
     *  Sorts the list of strings into alpha order, then prints them in as
     *    many columns as is practical (as dictated by the length of the
     *    longest string and the separation specified).
     *  The last string in the list must be a null string.  This is not
     *	  printed, but simply allows the number of strings in the list to
     *	  be computed from the list itself.
     */
    short	nel;			/* number of elements (strings) */
					/*   in the list		*/
    short	maxlength;		/* length of the longest string	*/
    short	colnumber;		/* number of columns		*/
    short	colwidth;		/* width of each column		*/
    short	colheight;		/* height of all columns except	*/
					/*   last, which may be shorter	*/
    short	row;			/* row count during output	*/
    short	column;			/* column count during output	*/
    short	i, j;
    int		strcmp ();		/* must be explicit about this */

    
    for (nel=0, maxlength=0; ; nel++) {
	if (list[nel].name[0] == '\0')
	    break;
	if (strlen (list[nel].name) > maxlength)
	    maxlength = strlen (list[nel].name);
	}

    /***********************************
    qsort (list, nel, MAXALPHA, strcmp);
    ***********************************/

    colwidth =  maxlength + separation;
    colnumber = 79 / colwidth;
    colheight = ((nel - 1) / colnumber) + 1;

    for (row=0; row<colheight; row++) {
	for (column=0; column<colnumber; column++) {
	    i = row + (column * colheight);
	    if (i < nel) {
		printf ("%s", list[i].name);
		for (j=colwidth-strlen (list[i].name); j>0; j--)
		    putchar (' ');
		}
	    }
	printf ("\n");
	}
    }


#ifdef PM3
#   define MAXWAITVERT		20000
#   define VERTDELAY		  100
#else
#   define MAXWAITVERT		 2000
#   define VERTDELAY		   25
#endif PM3

waitvert (verbose)
int verbose;
{
    register old, new;
    register count;
    for (count=MAXWAITVERT,old=TRUE; count--;) {
	new = VERTINT;
	if (new && !old) {
	    for (count=VERTDELAY; count--;)
		;
	    return TRUE;
	    }
	old = new;
	}
    if (verbose)
	printf ("waitvert: timed out\n\007");
    return FALSE;
    }



drawrect (x, y, width, height, fontaddr)
short	x, y, width, height, fontaddr;
{
    /*
     *	The specified rectangular region is filled with the stipple
     *    pattern at fontaddr.  No change to code or we.
     */

    LDXS (x)
    LDXE (x + width - 1)
    LDYS (y)
    LDYE (y + height - 1)
    LDFMADDR (fontaddr)
    REQUEST (FILLRECT, 0)
    }



drawchar (x, y, width, height, fontaddr)
short	x, y, width, height, fontaddr;
{
    /*
     *	The specified rectangular region is filled with the stipple
     *    pattern at fontaddr.  No change to code or we.
     */

    LDXS (x)
    LDXE (x + width - 1)
    LDYS (y)
    LDYE (y + height - 1)
    LDFMADDR (fontaddr)
    REQUEST (DRAWCHAR, 0)
    }



long		planecode (colorindex, sigplanes, cfr)
short		colorindex;		/* integer from 0..4095 (DC3)	*/
long		sigplanes;		/* significant bp planes	*/
short		cfr;			/* used to determine disp. mode	*/
{
    /*
     *	Computes a code to be written into the bp planes.
     *  Returns -1 if colorindex is out of range (as defined by the number of
     *    significant planes).  Note that this value is never legal, even in
     *	  a 32-bit system.
     */

    if (SINGLEBUFMODE (cfr)) {
	return (newcode (colorindex, sigplanes & 0x3f3f));
	}
    else if (DISPLAYAMODE (cfr)) {
	return (newcode (colorindex, sigplanes & 0xf00ff));
	}
    else if (DISPLAYBMODE (cfr)) {
	return (newcode (colorindex, sigplanes & 0xf00ff00));
	}
    else
	return (-1);
    }



short		getindex (planecode, sigplanes, cfr)
long		planecode;
long		sigplanes;		/* significant bp planes	*/
short		cfr;			/* used to determine disp. mode	*/
{
    /*
     *	Inverts the index-to-planecode algorithm.
     */

    if (SINGLEBUFMODE (cfr)) {
	return (newindex (planecode, sigplanes & 0x3f3f));
	}
    else if (DISPLAYAMODE (cfr)) {
	return (newindex (planecode, sigplanes & 0xf00ff));
	}
    else if (DISPLAYBMODE (cfr)) {
	return (newindex (planecode, sigplanes & 0xf00ff00));
	}
    else
	return (-1);
    }
long		CodeBit[32] = {	A0, B0, A1, B1, A2, B2, A3, B3,
				A4, B4, A5, B5, A6, B6, A7, B7,
				C0, D0, C1, D1, C2, D2, C3, D3,
				C4, D4, C5, D5, C6, D6, C7, D7
				};
		/*
		 *	Used by newcode and newindex to establish the
		 *	  order of significance of bits in codes.
		 */



long		newcode (index, sigbits)
short		index;
long		sigbits;
{
    /*
     *  Computes a new code with the same value as "code", but using only
     *    the significant bits specified by "sigbits".  For example,
     *    newcode (5, 0x1c) = 0x14.
     *  Indexes beyond the representable range cause return of -1.
     */

    long	newcode;
    long	indexbit;	/* could be short, but being careful */
    long	max;		/*	"	"	"	"    */
    short	codex;		/* index into the code bit array	*/

    for (max=0, newcode=0, indexbit=1, codex=0; codex<32; codex++) {
	if (CodeBit[codex] & sigbits) {
	    if (indexbit & index)
		newcode |= CodeBit[codex];
	    max |= indexbit;
	    indexbit <<= 1;
	    }
	}
    return (index > max) ? -1 : newcode;
    }
	    


short		newindex (code, sigbits)
long		code;
long		sigbits;
{
    /*
     *	Inverts the index-to-code algorithm.
     */
    short	newindex;
    short	codex;
    short	indexbit;
    for (newindex=0, codex=0, indexbit=1; codex<32; codex+=1) {
	if (CodeBit[codex] & sigbits) {
	    if (CodeBit[codex] & code)
		newindex |= indexbit;
	    indexbit <<= 1;
	    }
	}
    return (newindex);
    }



#ifndef BIT
#define BIT(i)		(1<<(i))
#endif

short		mapcode (planecode, dcflags, cfr)
long		planecode;
short		dcflags;
short		cfr;
{
    /*
     *  Computes a new color map code based on the planecode that is
     *    passed.  The three modes of display operation each have
     *    distinct plane-to-color-map mappings, which are:
     *
     *					  Colormap address
     *
     *	                        11 10  9  8  7  6  5  4  3  2  1  0
     *	    Normal mode - DC2
     *		single buffer               b3 b2 b1 b0 a3 a2 a1 a0
     *		display a                   a7 a6 a5 a4 a3 a2 a1 a0
     *		display b                   b7 b6 b5 b4 b3 b2 b1 b0
     *
     *	    Normal mode - DC3
     *		single buffer   b5 a5 b4 a4 b3 a3 b2 a2 b1 a1 b0 a0
     *		display a       c3 c2 c1 c0 a7 a6 a5 a4 a3 a2 a1 a0
     *		display b       d3 d2 d1 d0 b7 b6 b5 b4 b3 b2 b1 b0
     *
     *	    Register mode - DC3
     *		single buffer   r3 r2 r1 r0 b3 a3 b2 a2 b1 a1 b0 a0
     *		display a       r3 r2 r1 r0 a7 a6 a5 a4 a3 a2 a1 a0
     *		display b       r3 r2 r1 r0 b7 b6 b5 b4 b3 b2 b1 b0
     */
    register long lmapcode, pcode;

    if (dcflags & DCREGADRMAP) {
	if (SINGLEBUFMODE (cfr)) {
	    pcode = planecode;
	    lmapcode = 0;
	    if (pcode & BIT(0))  lmapcode |= BIT(0);
	    if (pcode & BIT(8))  lmapcode |= BIT(1);
	    if (pcode & BIT(1))  lmapcode |= BIT(2);
	    if (pcode & BIT(9))  lmapcode |= BIT(3);
	    if (pcode & BIT(2))  lmapcode |= BIT(4);
	    if (pcode & BIT(10)) lmapcode |= BIT(5);
	    if (pcode & BIT(3))  lmapcode |= BIT(6);
	    if (pcode & BIT(11)) lmapcode |= BIT(7);
	    return (DCNumToIndex(dcflags) | lmapcode);
	    }
	else if (DISPLAYAMODE (cfr))
	    return (DCNumToIndex(dcflags) | (planecode&0xff));
	else if (DISPLAYBMODE (cfr))
	    return (DCNumToIndex(dcflags) | ((planecode>>8)&0xff));
	else
	    return (-1);
	}
    else {
	if (SINGLEBUFMODE (cfr)) {
	    pcode = planecode;
	    lmapcode = 0;
	    if (pcode & BIT(0))  lmapcode |= BIT(0);
	    if (pcode & BIT(8))  lmapcode |= BIT(1);
	    if (pcode & BIT(1))  lmapcode |= BIT(2);
	    if (pcode & BIT(9))  lmapcode |= BIT(3);
	    if (pcode & BIT(2))  lmapcode |= BIT(4);
	    if (pcode & BIT(10)) lmapcode |= BIT(5);
	    if (pcode & BIT(3))  lmapcode |= BIT(6);
	    if (pcode & BIT(11)) lmapcode |= BIT(7);
	    if (pcode & BIT(4))  lmapcode |= BIT(8);
	    if (pcode & BIT(12)) lmapcode |= BIT(9);
	    if (pcode & BIT(5))  lmapcode |= BIT(10);
	    if (pcode & BIT(13)) lmapcode |= BIT(11);
	    return (0+lmapcode);
	    }
	else if (DISPLAYAMODE (cfr))
	    return (((planecode>>8)&0xf00) | (planecode&0xff));
	else if (DISPLAYBMODE (cfr))
	    return (((planecode>>16)&0xf00) | ((planecode>>8)&0xff));
	else
	    return (-1);
	}
    }






mapcolor (mapcode, sigmapbits, red, green, blue)
short		mapcode;	/* Code as seen at input to colormap	*/
short		sigmapbits;	/* Significant bits in above code	*/
short		red, green, blue; /* 0..255 in each color		*/
{
    /*
     *	All colormap entries corresponding to the given mapcode, given
     *    the significant map bits, are loaded with red, green, and blue
     *    as provided.
     */

    short	increment;	/* step through colormap with this 	*/
				/*   address increment			*/
    short	index;
    short	addr;
    short	mapselect;
    short	s_dcr;

    s_dcr = dc_dcr;
    if (s_dcr & DCREGADRMAP)
	sigmapbits |= DCNumToIndex (0xf);
    sigmapbits &= DCSINGLEMASK;
    DCR ((s_dcr & ~(DCIndexToReg(0xf))) | DCBUSOP | DCIndexToReg (mapcode));
    if (sigmapbits == DCSINGLEMASK) {
	addr = mapcode & DCMULTIMASK;
	DCMapColor (addr, red, green, blue);
	}
    else {
	for (increment=1; increment&sigmapbits; increment <<= 1);
	mapcode &= sigmapbits;
	for (index=mapcode; index <= DCSINGLEMASK; index += increment) {
	    if ((index&sigmapbits) == mapcode) {
		DCR ((s_dcr & ~(DCIndexToReg(0xf))) |
			DCBUSOP | DCIndexToReg (index));
		addr = index & DCMULTIMASK;
		DCMapColor (addr, red, green, blue);
		}
	    }
	}
    DCR (s_dcr);
    }
	



boolean	readmap (index, red, green, blue)
short	index, *red, *green, *blue;
{
    /*
     *	Returns the full 16-bit colormap contents corresponding to index.
     *	Index is taken as it is, not mucked with according to dcflags as
     *    it is in mapcolor.  Thus DC2 indexes range 0..1023, and DC3
     *    indexes range 0..4095.
     */

    short s_dcr;
    s_dcr = dc_dcr;
    if ((index < 0) || (index > DCSINGLEMASK))
	return (FALSE);
    else {
	DCR ((s_dcr & ~(DCIndexToReg(0xf))) | DCBUSOP | DCIndexToReg (index));
	index &= DCMULTIMASK;
	DCReadMap (index, *red, *green, *blue)
	DCR (s_dcr);
	return (TRUE);
	}
    }



long		gethexvalue (current, lowbound, highbound, string)
long		current;	/* current value of variable		*/
long		lowbound;	/* low bound for acceptable value	*/
long		highbound;	/* high bound for acceptable value	*/
char		string[];	/* name of the variable			*/
{
    /*
     *	Prompts for a new value for the variable whose current, minimum,
     *	  and maximum values have been indicated.  Only values within the
     *	  specified range are accepted, otherwise prompting continues, after
     *	  the bell is sounded.  The accepted value is returned.
     */

    char	line[25];
    long	value;

    while (TRUE) {
	printf ("%s ", string);
	printf ("[%x,%x], %x: ", lowbound, highbound, current);
	kgetline (line, 20);
	if (sscanf (line, "%lx", &value) == 1) {
	    if ((value >= lowbound) && (value <= highbound) ||
		(value >= lowbound) && (highbound < 0)) {
		return (value);
		}
	    }
	else if (line[0] == '\n') {
	    return (current);
	    }
	else {
	    putchar ('\007');
	    }
	}
    }



printconfig (cfr)
short	cfr;
{
    printf ("  cfr: ");
    if ((cfr&DISPLAYA) || (cfr&DISPLAYB)) {
	printf ("display ");
	if (cfr&DISPLAYA)
	    putchar ('a');
	if (cfr&DISPLAYB)
	    putchar ('b');
	printf (", ");
	}
    if ((cfr&UPDATEA) || (cfr&UPDATEB)) {
	printf ("update ");
	if (cfr&UPDATEA)
	    putchar ('a');
	if (cfr&UPDATEB)
	    putchar ('b');
	printf (", ");
	}
    if (cfr&VIEWPORTMASK)
	printf ("viewport, ");
#ifdef UC3
    if (cfr&ENABLECD)
	printf ("enable cd, ");
#endif UC3
#ifdef UC4
    if (cfr&UC_INVERT)
	printf ("invert, ");
#endif UC4
    if (cfr&BACKLINE)
	printf ("backline, ");
    if (cfr&LDLINESTIP)
	printf ("ldlinestip, ");
#ifdef UC4
    if (cfr&(UC_PFICD|UC_PFIREAD|UC_PFICOLUMN|UC_PFIXDOWN|UC_PFIYDOWN)) {
	printf ("pfi ");
	if (cfr&UC_PFICD)
	    printf ("cd ");
	if (cfr&UC_PFIREAD)
	    printf ("read ");
	if (cfr&UC_PFICOLUMN)
	    printf ("column ");
	if (cfr&UC_PFIXDOWN)
	    printf ("xdown ");
	if (cfr&UC_PFIYDOWN)
	    printf ("ydown ");
	printf ("\010, ");
	}
    if (cfr&UC_ALLPATTERN)
	printf ("allpat, ");
    if (cfr&UC_PATTERN32) {
	if (cfr&UC_PATTERN64)
	    printf ("pat64, ");
	else
	    printf ("pat32, ");
	}
    else if (cfr&UC_PATTERN64)
	printf ("paterror, ");
    if (cfr)
#endif UC4
#ifdef UC3
    if (cfr & (UPDATEA|UPDATEB|DISPLAYA|DISPLAYB|
	    VIEWPORTMASK|ENABLECD|BACKLINE|LDLINESTIP))
#endif UC3
	printf ("\010\010  ");
    printf ("\n");
    }


printmode (mdr)
short	mdr;
{
#ifdef UC4
    printf ("  mdr: ");
    if (mdr & UC_SWIZZLE)
	printf ("swizzle, ");
    if (mdr & UC_DOUBLE)
	printf ("double, ");
    if (mdr & UC_DEPTHCUE)
	printf ("depthcue, ");
    if (mdr)
	printf ("\010\010  ");
    printf ("\n");
#endif UC4
    }



printdcflags (dcflags)
short	dcflags;
{
    short	palctrl;

    printf ("  dcflags: ");
    if (dcflags&DCBUSADRMAP)
	printf ("bus, ");
    if (dcflags&DCREGADRMAP)
	printf ("reg %1x, ", DCRegToNum (dcflags));
    if (dcflags&DCRGBMODE)
	printf ("rgb, ");
#ifdef DC3
    if (dcflags&DCMAINT)
	printf ("maint, ");
#endif DC3
#ifdef DC4
    if (dcflags&DCHIGHMAP)
	printf ("highmap, ");
    if (dcflags&DCOPTCLK)
	printf ("optclk, ");
    if (dcflags&DCPIPE4)
	printf ("pipe4, ");
    if (dcflags&DCPROM)
	printf ("prom, ");
    if (dcflags&DCD1K)
	printf ("1K, ");
    if (dcflags&DCMBIT)
	printf ("mbit, ");
#endif DC4
    palctrl = dcflags & VFSM_CMND_MASK;
    printf ("vfsm-", palctrl);
    switch (palctrl) {
	case VFSM_CLEAR:	printf ("clear, "); break;
	case VFSM_SET:		printf ("set, "); break;
#ifdef DC3
	case VFSM_CYCLE:	printf ("scopecycle, "); break;
	case VFSM_STICKY_CYCLE:	printf ("cycle, "); break;
	case VFSM_EVEN_CHECKSUM:printf ("evenchecksum, "); break;
	case VFSM_ODD_CHECKSUM:	printf ("oddchecksum, "); break;
#endif DC3
#ifdef DC4
	case VFSM_STICKY_EVENT_2:	printf ("event2, "); break;
	case VFSM_STICKY_EVENT_4:	printf ("event4, "); break;
	case VFSM_STICKY_CYCLE_2:	printf ("cycle2, "); break;
	case VFSM_STICKY_CYCLE_4:	printf ("cycle4, "); break;
#endif DC4
	default: printf ("(unknown), ");
	}
    printf ("\010\010  \n");
    }



#define	RECTNUM		8
#define JOE_Y1		(YMAX/3)
#define JOE_Y2		(YMAX/3)
#define JOE_I		255
short	grayscale[RECTNUM] = {0, 37, 73, 109, 146, 183, 219, 255};

joescale () {
    /*
     *	draws 8 128x1024 pixel rectangles with color indexes 0 through
     *    7 from left to right.  The colormap is updated to provide
     *	  indexes 0..7 with gray (equal rgb components) levels from 0
     *	  through ff.
     *	draws the 3 primary and the 3 secondary colors in rectangles in
     *	  the middle of the screen.
     *	draws a white box around the entire screen.
     */

    short	x;
    short	index;
    short	width;
    short	tempmapcode;
    short	sigmapbits;
    short	red, green, blue;

    width = XMAX/RECTNUM;
    sigmapbits = mapcode (sigplanes, dc_dcr, uc_cfb);
    for (x = 0, index = 0; x < XMAX; x += width, index++) {
	setcodes (planecode (index, sigplanes, uc_cfb), wecode);
	drawrect (x, 0, width, YMAX, ONESTIPADDR);
	tempmapcode =mapcode(planecode(index,sigplanes,uc_cfb),dc_dcr,uc_cfb);
	mapcolor (tempmapcode, sigmapbits, 
		  grayscale[index], grayscale[index], grayscale[index]);
	}
    for (x = 0, index = 8; x < XMAX; x += width, index++) {
	setcodes (planecode (index, sigplanes, uc_cfb), wecode);
	drawrect (x, JOE_Y1, width, JOE_Y2, ONESTIPADDR);
	tempmapcode =mapcode(planecode(index,sigplanes,uc_cfb),dc_dcr,uc_cfb);
	mapcolor (tempmapcode, sigmapbits,
		(index&1) ? JOE_I : 0,
		(index&2) ? JOE_I : 0,
		(index&4) ? JOE_I : 0);

	}
    setcodes (planecode (7, sigplanes, uc_cfb), wecode);
    drawline (0, 0, XMAX-1, 0, 0xffff);
    drawline (XMAX-1, 0, XMAX-1, YMAX-1, 0xffff);
    drawline (XMAX-1, YMAX-1, 0, YMAX-1, 0xffff);
    drawline (0, YMAX-1, 0, 0, 0xffff);
    }



long		rgbcode (red, green, blue)
short		red, green, blue;
{
    /*
     *	Returns the appropriate rgb code
     */
    long	code;

		code = (red&0xff) | ((green&0xff)<<8);
		if (blue&0x01) code |= C0;
		if (blue&0x02) code |= D0;
		if (blue&0x04) code |= C1;
		if (blue&0x08) code |= D1;
		if (blue&0x10) code |= C2;
		if (blue&0x20) code |= D2;
		if (blue&0x40) code |= C3;
		if (blue&0x80) code |= D3;
    return (code);
    }

long bits (value, on, off)
long value, on, off;
{
    return (value | on) & ~off;
    }

printplanes (p)
long p;
{
    short i,j;
    printf ("%x, ", p);
    for (i=0; i<4; i++,p>>=8) {
	if (p & 0xff) {
	    printf ("%c:", 'A' + i);
	    for (j=0; j<8; j++) {
		if (p & (1<<j))
		    printf ("%d", j);
		}
	    printf (" ");
	    }
	}
    }

save (s)
Save *s;
{
    s->cfb = uc_cfb;
    s->edb = uc_edb;
    s->ecb = uc_ecb;
    s->xsb = uc_xsb;
    s->xeb = uc_xeb;
    s->ysb = uc_ysb;
    s->yeb = uc_yeb;
    s->fmab = uc_fmab;
#ifdef UC4
    s->rpb = uc_rpb;
    s->mdb = uc_mdb;
    s->ucr = uc_ucr;
#endif UC4
    s->dcr = dc_dcr;
    }

restore (s)
Save *s;
{
    if (s->cfb != uc_cfb) {
	LDCONFIG (s->cfb);
	REQUEST (NOOP, 0);
	}
    else {
	LDCONFIG (s->cfb);
	}
    LDED (s->edb);
    LDEC (s->ecb);
    LDXS (s->xsb);
    LDXE (s->xeb);
    LDYS (s->ysb);
    LDYE (s->yeb);
    LDFMADDR (s->fmab);
#ifdef UC4
    LDREPEAT (s->rpb);
    LDMODE (s->mdb);
    UCR (s->ucr);
#endif UC4
    DCR (s->dcr);
    }

printrestore () {
    printf ("  ED  %4x,        EC %4x\n", uc_edb&0xffff, uc_ecb&0xffff);
    printf ("  XS  %4x,        XE %4x\n", uc_xsb&0xfff, uc_xeb&0xfff);
    printf ("  YS  %4x,        YE %4x\n", uc_ysb&0xfff, uc_yeb&0xfff);
#ifdef UC3
    printf ("  FMA %4x\n", uc_fmab&0xffff);
#endif UC3
#ifdef UC4
    printf ("  FMA %4x,        RP %4x\n", uc_fmab&0xffff, uc_rpb&0xff);
    printmode (uc_mdb);
#endif UC4
    printconfig (uc_cfb);
    printdcflags (dc_dcr);
    }

