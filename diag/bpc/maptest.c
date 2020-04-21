/*
 *	Kurt Akeley			9/20/82
 *
 *	These routines are intended to be linked with the main routine
 *	"console.b".
 *
 *	Functions:
 *		long	maptest (verbose, teststorun, diagnostic)
 *		long	comparemap (data, mode, verbose)
 *		long	dcrtest (verbose)
 *
 *	Procedures:
 *		loadmap (data, mode)
 *		dclamptest (cycles)
 *		addmaperrors ()
 *		ramptest (red, green, blue, rgbmode)
 *		steptest (red, green, blue, rgbmode)
 *
 *	Updates:
 *		10/25/82 KBA	Changed error information printed
 *		4/7/83   KBA	Added dcrtest and dclamptest
 *		4/11/83	 KBA	Added ramptest and steptest
 *		5/13/83  KBA	Added random data test
 *		5/17/83  KBA	Used vfsm.h
 */

#include "console.h"
#include "uctest.h"
#include "ucdev.h"
#include "dcdev.h"
#include "vfsm.h"

#define DATAMODE	0
#define LOWADDRMODE	1
#define HIGHADDRMODE	2
#define RANDOMMODE	3
#define RED		0x00
#define GREEN		0x40
#define BLUE		0x80

#define SEED		433
#define RANDOMIZE(r)	r = ((r<<2) + r + 17623) & 0xffff



long maptest (verbose, teststorun, diagnostic)
boolean	verbose;	/* forces verbose error reporting	*/
short teststorun;
boolean diagnostic;	/* tests the code itself		*/
{
    /*
     *	A sequence of 5 tests of the colormap is performed.  These are:
     *	  1.  All zero write and readback
     *	  2.  All one write and readback
     *	  3.  Lower 8 address bit write and readback
     *    4.  Upper 8 address bit write and readback
     *	  5.  Random data write and readback
     *  Individual errors are reported as they occur if verbose if true.
     *  The total number of errors is returned.
     */

    long	errortotal;
    Save	s;
    errortotal = 0;

    save (&s);
    /* all zero test */
    if (teststorun & 1) {
	if (verbose)
	    putchar ('0');
	loadmap (0, DATAMODE);
	if (diagnostic) {
	    addmaperrors ();
	    }
	errortotal += comparemap (0, DATAMODE, verbose);
	restore (&s);
	}

    /* all one test */
    if (teststorun & 2) {
	if (verbose)
	    putchar ('1');
	loadmap (0xff, DATAMODE);
	if (diagnostic) {
	    addmaperrors ();
	    }
	errortotal += comparemap (0xff, DATAMODE, verbose);
	restore (&s);
	}

    /* low address test */
    if (teststorun & 4) {
	if (verbose)
	    putchar ('2');
	loadmap (0, LOWADDRMODE);
	errortotal += comparemap (0, LOWADDRMODE, verbose);
	restore (&s);
	}

    /* high address test */
    if (teststorun & 8) {
	if (verbose)
	    putchar ('3');
	loadmap (0, HIGHADDRMODE);
	errortotal += comparemap (0, HIGHADDRMODE, verbose);
	restore (&s);
	}

    /* random data test */
    if (teststorun & 16) {
	if (verbose)
	    putchar ('4');
	loadmap (0, RANDOMMODE);
	errortotal += comparemap (0, RANDOMMODE, verbose);
	restore (&s);
	}

    return errortotal;
    }


loadmap (data, mode)
short		data;		/* written if mode is DATAMODE		*/
short		mode;		/* controls data			*/
{
    /*
     *	Writes the entire colormap with either the data provided
     *    (DATAMODE) or with one of two functions of the map index.
     *    Each RGB triplet is treated as a single location.
     *  LOWADDRMAP loads each location with the 8-bit map index of that
     *    location.  Red, green, and blue are equal at all locations.
     *	HIGHADDRMODE constructs 8-bit values that include the color
     *    (red:0, green:1, blue:2) and the mapnumber (0..3 for DC2, 0..15 for
     *    DC3).
     */

    short	i;		/* map address index			*/
    short	mapnum;		/* colormap number, 0..3 for dc2, 0..15 */
				/*   for dc3				*/
    short	mapindex;	/* dc2 map index			*/
    short	red,green,blue; /* intensities to be written		*/
    short	rand;		/* random value				*/
    short	s_dcr;

    s_dcr = dc_dcr;
    rand = SEED;
    for (mapnum=0; mapnum < DCMAPNUM; mapnum++) {
	breakcheck ();
	for (i=0; i < 0x100; i++) {
	    switch (mode) {
		case DATAMODE:
		    red = (data&0xff);
		    green = (data&0xff);
		    blue = (data&0xff);
		    break;
		case LOWADDRMODE:
		    red = (i&0xff);
		    green = (i&0xff);
		    blue = (i&0xff);
		    break;
		case HIGHADDRMODE:
		    red = RED | mapnum;
		    green = GREEN | mapnum;
		    blue = BLUE | mapnum;
		    break;
		case RANDOMMODE:
		    red = rand & 0xff;
		    RANDOMIZE (rand);
		    green = rand & 0xff;
		    RANDOMIZE (rand);
		    blue = rand & 0xff;
		    RANDOMIZE (rand);
		    break;
		}
	    DCR ((dc_dcr & ~(DCNumToReg(0xF))) | DCBUSOP |DCNumToReg(mapnum));
	    DCMapColor (i, red, green, blue);
	    }
	}
    DCR (s_dcr);
    }



long 		comparemap (data, mode, verbose)
short		data;
short		mode;
boolean		verbose;
{
    /*
     *  Compares the contents of the entire colormap with with
     *    the (presumed) desired contents.  Errors are reported as
     *    as they occur if verbose is true.  The total number of
     *    errors is returned as a long integer.
     */

    short	i;		/* colormap address index		*/
    short	mapindex;
    short	mapnum;
    short	expectedred, expectedgreen, expectedblue;
    short	actualred, actualgreen, actualblue;
    short	highred, highgreen, highblue;		/* for DC4 only */
    short	rand;
    short	s_dcr;

    s_dcr = dc_dcr;
    rand = SEED;
    initerror (10);
    for (mapnum=0; mapnum < DCMAPNUM; mapnum++) {
	breakcheck ();
	for (i=0; i<0x100; i++) {
	    switch (mode) {
		case DATAMODE:
		    expectedred = (data&0xff);
		    expectedgreen = (data&0xff);
		    expectedblue = (data&0xff);
		    break;
		case LOWADDRMODE:
		    expectedred = (i&0xff);
		    expectedgreen = (i&0xff);
		    expectedblue = (i&0xff);
		    break;
		case HIGHADDRMODE:
		    expectedred = RED | mapnum;
		    expectedgreen = GREEN | mapnum;
		    expectedblue = BLUE | mapnum;
		    break;
		case RANDOMMODE:
		    expectedred = rand & 0xff;
		    RANDOMIZE (rand);
		    expectedgreen = rand & 0xff;
		    RANDOMIZE (rand);
		    expectedblue = rand & 0xff;
		    RANDOMIZE (rand);
		    break;
		}
#ifdef DC3

	    DCR ((dc_dcr & ~(DCNumToReg(0xF))) | DCBUSOP |DCNumToReg(mapnum));
	    DCReadMap (i, actualred, actualgreen, actualblue);
#endif DC3
#ifdef DC4
	    DCR ((dc_dcr & ~(DCHIGHMAP|DCNumToReg(0xF))) |
		DCBUSOP |DCNumToReg(mapnum));
	    DCReadMap (i, actualred, actualgreen, actualblue);
	    DCR ((dc_dcr & ~(DCNumToReg(0xF))) | DCBUSOP |
		DCHIGHMAP | DCNumToReg(mapnum));
	    DCReadMap (i, highred, highgreen, highblue);
#endif DC4
	    if ((expectedred != (actualred&0xff)) ||
		(expectedred != ((actualred>>8)&0xff)) ||
		(expectedgreen != (actualgreen&0xff)) ||
		(expectedgreen != ((actualgreen>>8)&0xff)) ||
		(expectedblue != (actualblue&0xff)) ||
		(expectedblue != ((actualblue>>8)&0xff))
#ifdef DC4
		||
		(expectedred != (highred&0xff)) ||
		(expectedred != ((highred>>8)&0xff)) ||
		(expectedgreen != (highgreen&0xff)) ||
		(expectedgreen != ((highgreen>>8)&0xff)) ||
		(expectedblue != (highblue&0xff)) ||
		(expectedblue != ((highblue>>8)&0xff))
#endif DC4
		) {
		if (verbose) {
		   printf ("ERROR: map=%1x, index=%2x, value=%2x,%2x,%2x, ",  
		       mapnum, i, expectedred, expectedgreen, expectedblue);
#ifdef DC3
		   printf ("return=%4x,%4x,%4x\n",
		       actualred&0xffff,actualgreen&0xffff,actualblue&0xffff);
#endif DC3
#ifdef DC4
		   printf ("return=%4x%4x,%4x%4x,%4x%4x\n",
		       highred&0xffff, actualred&0xffff, highgreen&0xffff,
		       actualgreen&0xffff, highblue&0xffff,actualblue&0xffff);
#endif DC4
		   }
		adderror (verbose);
		}
	    }
	}
    DCR (s_dcr);
    return geterrors ();
    }



addmaperrors ()
{
    short i;
    short s_dcr;
    s_dcr = dc_dcr;
    printf ("\nWARNING: adding errors !!!\n\007");
    DCR (s_dcr|DCBUSOP);
    for (i=0; i<8; i++) {
	DCMapColor (i, i, 0x10+i, 0x20+i);
	}
    DCR (s_dcr);
    }



#ifdef DC3
#define DCBITS 8
#endif DC3
#ifdef DC4
#define DCBITS 7
#endif DC4

long		dcrtest (verbose)
boolean		verbose;
{
    /*
     *	Checks all 256 possible combinations of the 8 low-order dcr bits.
     *	DC2 ignores the PAL outputs, DC3 checks for all zeros and all ones.
     */

    short	i;
    short	j;
    short	s_dcr;

    s_dcr = dc_dcr;
    initerror (10);
    for (i=0; i<(1<<DCBITS); i++) {
	DCR (i);
	j = DCflags & ((1<<DCBITS) - 1);
	if (i != j) {
	    if (verbose) {
		printf ("ERROR: dcr %2x read back as %2x, xor %x\n",
		    i, j, i^j);
		}
	    adderror (verbose);
	    }
	}
    DCR (VFSM_SET);
    i = VFSM_ADJUST (DCflags);
    if (i != 0xff) {
	if (verbose) {
	    printf ("ERROR: expected vfsm ff read back as %x\n", i);
	    }
	adderror (verbose);
	}
    DCR (VFSM_CLEAR);
    i = VFSM_ADJUST (DCflags);
    if (i != 0) {
	if (verbose) {
	    printf ("ERROR: vfsm expected  0 read back as %x\n", i);
	    }
	adderror (verbose);
	}
    DCR (s_dcr);
    return geterrors ();
    }



#ifdef DC3
#define DCLAMPMAX	8
static long dclampbit[DCLAMPMAX] = {0, 1, 2, 3, 4, 5, 6, 7};
#endif DC3
#ifdef DC4
#define DCLAMPMAX	12
static long dclampbit[DCLAMPMAX] = {4, 5, 6, 7, 0, 1, 2, 3, 11, 12, 13, 14};
#endif DC4

dclamptest (cycles)
short		cycles;		/* number of rotations to do */
{
    /*
     *	Cycles the dc LEDs in a shift register pattern at a visible rate.
     */

    short	count;
    short	checkbit;
    short	wait;
    short	s_dcr;

    s_dcr = dc_dcr;
    for (count=0; count<cycles; count++) {
	breakcheck ();
	for (checkbit=0; checkbit<DCLAMPMAX; checkbit++) {
	    DCR (1 << dclampbit[checkbit]);
	    for (wait=0; wait<60; wait++)
		waitvert (checkbit < 8);
	    }
	}
    DCR (s_dcr);
    }



ramptest (red, green, blue, rgbmode)
boolean		red, green, blue;
boolean		rgbmode;
{
    /*
     *	To be used with an oscilloscope to check all DAC circuitry.
     *    If rgbmode is false, the colormap is used to create an
     *	  increasing intensity in each color on each scanline.  Pixels
     *	  0..3 have intensity zero, 4..7 intensity 1, 1020..1023 intensity
     *    255.  The visible effect is a grayscale of 256 levels.
     *  If rgbmode is true, the same pattern is created using the direct
     *    mapping.  This also tests the rgb data paths.
     *  The three booleans red, green, and blue specify whether that color
     *    is to be ramped (true) or left at zero (false).
     */

    long	code;
    long	sigmapbits;
    long	tempmapcode;
    short	i;
    char	s[10];
    short	s_cfb;
    short	s_dcr;

    s_cfb = uc_cfb;
    s_dcr = dc_dcr;
    LDCONFIG (s_cfb|UPDATEA|UPDATEB|DISPLAYA|DISPLAYB);
    if (rgbmode) {
	DCR (bits (s_dcr, DCRGBMODE, DCBUSOP));
	for (i=0; i<256; i++) {
	    setcodes (rgbcode (red?i:0, green?i:0, blue?i:0), 0xffffffff);
	    drawrect (i<<2, 0, 4, 1024, ONESTIPADDR);
	    }
	}
    else {
	DCR (bits (s_dcr, 0, DCBUSOP|DCRGBMODE));
	for (i=0; i<256; i++) {
	    code = planecode (i, sigplanes, s_cfb);
	    if (code == -1) {
		printf ("  ramptest: index %d cannot be coded, abort\n", i);
		return;
		}
	    setcodes (code, sigplanes);
	    drawrect (i<<2, 0, 4, 1024, ONESTIPADDR);
	    sigmapbits = mapcode (sigplanes, s_dcr, s_cfb);
	    tempmapcode = mapcode (code, s_dcr, s_cfb);
	    mapcolor (tempmapcode, sigmapbits, red?i:0, green?i:0, blue?i:0);
	    }
	}
    LDCONFIG (s_cfb);
    }



steptest (red, green, blue, rgbmode)
boolean		red, green, blue;
boolean		rgbmode;
{
    /*
     *	To be used with an oscilloscope to check all DAC circuitry.
     *    If rgbmode is false, the colormap is used to create an
     *	  increasing intensity in each color on each scanline.  Intensities
     *	  are increased int the binary pattern 0,1,2,4,8,16,32,64,128,255.
     *	  Pixels 0..99 have intensity 0, 100..199 intensity 1, etc.  The
     *	  last 124 pixels have intensity 255.
     *  If rgbmode is true, the same pattern is created using the direct
     *    mapping.  This also tests the rgb data paths.
     *  The three booleans red, green, and blue specify whether that color
     *    is to be ramped (true) or left at zero (false).
     */

    short	i;
    short	inten;
    long	code;
    long	sigmapbits;
    long	tempmapcode;
    short	colwidth;
    short	x;
    char	s[10];
    short	s_dcr;
    short	s_cfb;

    s_dcr = dc_dcr;
    s_cfb = uc_cfb;
    colwidth = 100;
    LDCONFIG (s_cfb|UPDATEA|UPDATEB|DISPLAYA|DISPLAYB);
    if (rgbmode) {
	DCR (bits (s_dcr, DCRGBMODE, DCBUSOP));
	for (i=0, x=0; i<10; i++, x+=colwidth) {
	    inten = (1<<i)>>1;
	    if (inten > 255)
		inten = 255;
	    setcodes (rgbcode (red?inten:0, green?inten:0, blue?inten:0), 
		      0xffffffff);
	    drawrect (x, 0, (i==9)?124:100, 1024, ONESTIPADDR);
	    }
	}
    else {
	DCR (bits (s_dcr, 0, DCBUSOP|DCRGBMODE));
	for (i=0, x=0; i<10; i++, x+=colwidth) {
	    code = planecode (i, sigplanes, s_cfb);
	    if (code == -1) {
		printf ("  ramptest: index %d cannot be coded, abort\n", i);
		return;
		}
	    setcodes (code, sigplanes);
	    drawrect (x, 0, (i==9)?124:100, 1024, ONESTIPADDR);
	    inten = (1<<i)>>1;
	    if (inten > 255)
		inten = 255;
	    sigmapbits = mapcode (sigplanes, s_dcr, s_cfb);
	    tempmapcode = mapcode (code, s_dcr, s_cfb);
	    mapcolor (tempmapcode, sigmapbits,
		      red?inten:0, green?inten:0, blue?inten:0);
	    }
	}
    LDCONFIG (s_cfb);
    }




