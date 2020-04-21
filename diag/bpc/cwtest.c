/*
 *	Kurt Akeley			4/6/83
 *
 *	Tests the colorcode and wecode bits on each bitplane board.
 *
 *	Functions:
 *		long	colorwetest (verbose)
 */

#include "console.h"
#ifdef UC4
#include "ucdev.h"
#include "uctest.h"
short SinNout[32] = {0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15,
	16, 24, 17, 25, 18, 26, 19, 27, 20, 28, 21, 29, 22, 30, 23, 31};
short NinSout[32] = {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15,
	16, 18, 20, 22, 24, 26, 28, 30, 17, 19, 21, 23, 25, 27, 29, 31};
#define BASECFR		(UPDATEA|UPDATEB|DISPLAYA|DISPLAYB)
#endif UC4

long		colorwetest (verbose)
boolean		verbose;
{
    long	code;
    long	pixels[16];
    short	bit;			/* uc4 only */
    long	incode, outcode;	/* uc4 only */
    short	i;
    Save	s;

    initerror (10);
    save (&s);

    /* Colorcode test - walking 1 */
    restore (&s);
    if (verbose)
	putchar ('0');
    for (code = 1; code != 0; code <<= 1) {
	if (code & sigplanes) {
	    setcodes (code, 0xffffffff);
	    drawrect (0, 0, 16, 1, ONESTIPADDR);
	    readpixels (pixels, 0, 0, 0xffffffff);
	    if ((pixels[0]&sigplanes) != code) {
		if (verbose) {
		    printf ("ERROR: colorcode %8x read back as %8x\n",
			    code, pixels[0]&sigplanes);
		    }
		adderror (verbose);
		}
	    }
	}

    /* Colorcode test - walking 0 */
    restore (&s);
    if (verbose)
	putchar ('1');
    for (code = 1; code != 0; code <<= 1) {
	if (code & sigplanes) {
	    setcodes (~code, 0xffffffff);
	    drawrect (0, 0, 16, 1, ONESTIPADDR);
	    readpixels (pixels, 0, 0, 0xffffffff);
	    if (((~pixels[0])&sigplanes) != code) {
		if (verbose) {
		    printf ("ERROR: colorcode %8x read back as %8x\n",
			    (~code)&sigplanes, pixels[0]&sigplanes);
		    }
		adderror (verbose);
		}
	    }
	}

    /* Wecode test - walking 1 */
    restore (&s);
    if (verbose)
	putchar ('2');
    for (code = 1; code != 0; code <<= 1) {
	if (code & sigplanes) {
	    setcodes (0, 0xffffffff);
	    drawrect (0, 0, 16, 1, ONESTIPADDR);
	    setcodes (0xffffffff, code);
	    drawrect (0, 0, 16, 1, ONESTIPADDR);
	    readpixels (pixels, 0, 0, 0xffffffff);
	    if ((pixels[0]&sigplanes) != code) {
		if (verbose) {
		    printf ("ERROR: wecode %8x read back as %8x\n",
			    code, pixels[0]&sigplanes);
		    }
		adderror (verbose);
		}
	    }
	}


    /* Wecode test - walking 0 */
    restore (&s);
    if (verbose)
	putchar ('3');
    for (code = 1; code != 0; code <<= 1) {
	if (code & sigplanes) {
	    setcodes (0, 0xffffffff);
	    drawrect (0, 0, 16, 1, ONESTIPADDR);
	    setcodes (0xffffffff, ~code);
	    drawrect (0, 0, 16, 1, ONESTIPADDR);
	    readpixels (pixels, 0, 0, 0xffffffff);
	    if (((~pixels[0])&sigplanes) != code) {
		if (verbose) {
		    printf ("ERROR: wecode %8x read back as %8x\n",
			    (~code)&sigplanes, pixels[0]&sigplanes);
		    }
		adderror (verbose);
		}
	    }
	}
#ifdef UC4
    /* walking 1 color test - swizzle during write, normal read */
    restore (&s);
    if (verbose)
	putchar ('4');
    for (bit=0; bit<32; bit++) {
	incode = 1<<bit;
	outcode = 1<<SinNout[bit];
	if (outcode & sigplanes) {
	    swizzlecodes (incode, 0xffffffff);
	    drawrect (0, 0, 16, 1, ONESTIPADDR);
	    readpixels (pixels, 0, 0, 0xffffffff);
	    if (pixels[0] != outcode) {
		if (verbose) {
		    printf ("ERROR: colorcode %x written swizzled as %x ",
			incode, outcode);
		    printf ("read back as %x\n", pixels[0]);
		    }
		adderror (verbose);
		}
	    }
	}

    /* walking 1 we test - swizzle during write, normal read */
    restore (&s);
    if (verbose)
	putchar ('5');
    for (bit=0; bit<32; bit++) {
	incode = 1<<bit;
	outcode = 1<<SinNout[bit];
	if (outcode & sigplanes) {
	    setcodes (0, 0xffffffff);
	    drawrect (0, 0, 16, 1, ONESTIPADDR);
	    swizzlecodes (0xffffffff, incode);
	    drawrect (0, 0, 16, 1, ONESTIPADDR);
	    readpixels (pixels, 0, 0, 0xffffffff);
	    if (pixels[0] != outcode) {
		if (verbose) {
		    printf ("ERROR: wecode %x written swizzled as %x ",
			incode, outcode);
		    printf ("read back as %x\n", pixels[0]);
		    }
		adderror (verbose);
		}
	    }
	}
    /* walking 1 color test - normal write, swizzle during read */
    restore (&s);
    if (verbose)
	putchar ('6');
    for (bit=0; bit<32; bit++) {
	incode = 1<<bit;
	outcode = 1<<NinSout[bit];
	if (incode & sigplanes) {
	    setcodes (incode, 0xffffffff);
	    drawrect (0, 0, 16, 1, ONESTIPADDR);
	    swizzlepixels (pixels, 0, 0, 0xffffffff);
	    if (pixels[0] != outcode) {
		if (verbose) {
		    printf ("ERROR: colorcode %x read swizzled as %x ",
			incode, pixels[0]);
		    printf ("should have read %x\n", outcode);
		    }
		adderror (verbose);
		}
	    }
	}

    /* walking 1 we test - normal write, swizzle during read */
    restore (&s);
    if (verbose)
	putchar ('7');
    for (bit=0; bit<32; bit++) {
	incode = 1<<bit;
	outcode = 1<<NinSout[bit];
	if (incode & sigplanes) {
	    setcodes (0, 0xffffffff);
	    drawrect (0, 0, 16, 1, ONESTIPADDR);
	    setcodes (0xffffffff, incode);
	    drawrect (0, 0, 16, 1, ONESTIPADDR);
	    swizzlepixels (pixels, 0, 0, 0xffffffff);
	    if (pixels[0] != outcode) {
		if (verbose) {
		    printf ("ERROR: wecode %x read swizzled as %x ",
			incode, pixels[0]);
		    printf ("should have read %x\n", outcode);
		    }
		adderror (verbose);
		}
	    }
	}

    /* walking 1 color test - double mode, no swizzle */
    restore (&s);
    if (verbose)
	putchar ('8');
    for (bit=0; bit<16; bit++) {
	incode = 1<<bit;
	outcode = ((bit<8) ? (0x101<<bit) : (0x10100<<bit)) & sigplanes;
	setcodes (0, 0xffffffff);
	LDMODE (UC_DOUBLE);
	*UCCommandAddr(UC_SETCOLORAB) = incode;
	drawrect (0, 0, 16, 1, ONESTIPADDR);
	LDMODE (0);				/* clear the double bits */
	*UCCommandAddr(UC_SETCOLORAB) = 0;	/*   on the bitplane boards */
	readpixels (pixels, 0, 0, 0xffffffff);
	if (pixels[0] != outcode) {
	    if (verbose) {
		printf ("ERROR: colorcode %x written double-mode read as %x ",
		    incode, pixels[0]);
		printf ("should have read %x\n", outcode);
		}
	    adderror (verbose);
	    }
	}

    /* walking 1 we test - double mode, no swizzle */
    restore (&s);
    if (verbose)
	putchar ('9');
    for (bit=0; bit<16; bit++) {
	incode = 1<<bit;
	outcode = ((bit<8) ? (0x101<<bit) : (0x10100<<bit)) & sigplanes;
	setcodes (0, 0xffffffff);
	drawrect (0, 0, 16, 1, ONESTIPADDR);
	setcodes (0xffffffff, 0);
	LDMODE (UC_DOUBLE);
	*UCCommandAddr(UC_SETWEAB) = incode;
	drawrect (0, 0, 16, 1, ONESTIPADDR);
	LDMODE (0);				/* clear the double bits */
	*UCCommandAddr(UC_SETCOLORAB) = 0;	/*   on the bitplane boards */
	readpixels (pixels, 0, 0, 0xffffffff);
	if (pixels[0] != outcode) {
	    if (verbose) {
		printf ("ERROR: wecode %x written double-mode read as %x ",
		    incode, pixels[0]);
		printf ("should have read %x\n", outcode);
		}
	    adderror (verbose);
	    }
	}
#endif UC4
    restore (&s);
    setcodes (colorcode, wecode);	/* restore machine state */
    return geterrors ();
    }

#ifdef UC4
swizzlecodes (colorcode, wecode)
long	colorcode;	/* 0..ffffffff, known on BPM as color */
long	wecode;		/* 0..ffffffff, enable or disable plane */
{
    short s_mdb;
    s_mdb = uc_mdb;
    LDMODE (UC_SWIZZLE);
    *((long*)(UCCommandAddr(UC_SETCOLORCD))) = colorcode;
    *((long*)(UCCommandAddr(UC_SETWECD))) = wecode;
    LDMODE (s_mdb);
    }

swizzlepixels (pixels, x, y, sigplanes)
long	*pixels;	/* array of 16 32-bit words */
short	x;		/* x word address, 0..63 */
short	y;		/* y line address, 0..1023 */
long	sigplanes;
{
    register i;
    short s_cfb, s_mdb;
    s_cfb = uc_cfb;
    s_mdb = uc_mdb;
    LDCONFIG (s_cfb | BASECFR | UC_PFIREAD);
    LDMODE (UC_SWIZZLE);
    x <<= 4;
    LDXS (x);
    LDXE (x+15);
    LDYS (y);
    REQUEST (UC_SETADDRS, 0);
    for (i=0; i<16; i++) {
	*pixels++ = *((long*)UCCommandAddr(UC_READPIXELCD)) & sigplanes;
	}
    LDCONFIG (s_cfb);
    LDMODE (s_mdb);
    }
#endif UC4
