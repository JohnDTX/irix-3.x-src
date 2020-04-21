/*
 *	Kurt Akeley
 *	22 October 1984
 */

#ifdef UC4

#include "console.h"
#include "ucdev.h"
#include "uctest.h"
#include "bpccodes.h"

#define XSTART	14
#define XEND	17
#define YSTART	14
#define YEND	17

long pixeltest (verbose, teststorun)
boolean verbose;
short teststorun;
{
    short basecfr;
    Save s;

    save (&s);
    basecfr = s.cfb & (UC_UPDATEA|UC_UPDATEB|UC_DISPLAYA|UC_DISPLAYB);
    initerror (2);

    if (teststorun & 1) {
	/* autoincrement test */
	restore (&s);
	if (verbose)
	    putchar ('0');
	if ((sigplanes&(A0|A1|B0|B1)) != (A0|A1|B0|B1)) {
	    if (verbose) {
	        printf ("pixeltest: ");
	        printf ("autoincrement test needs planes A0,A1, and B0,B1\n");
		}
	    }
	else {
	    /*	Use autodraw() and autotest() to test all 8 autoincrement
	     *	  directions.  Always read back in opposite row/column order
	     *	  to be sure of correct operation.
	     */
	    setcodes (0, A0|A1|B0|B1);
	    LDMODE (UC_SWIZZLE);

	    LDCONFIG (basecfr);
	    autodraw (XSTART, YSTART, XEND, YEND);
	    LDCONFIG (basecfr | UC_PFIREAD | UC_PFIXDOWN | UC_PFIYDOWN |
		UC_PFICOLUMN);
	    if (!autotest (XEND, YEND, XSTART, YSTART, verbose)) {
		if (verbose)
		    printf ("ERROR: +x+yrow draw, -x-ycolumn read test\n");
		adderror (verbose);
		}

	    LDCONFIG (basecfr | UC_PFIXDOWN)
	    autodraw (XEND, YSTART, XSTART, YEND);
	    LDCONFIG (basecfr | UC_PFIREAD | UC_PFIYDOWN | UC_PFICOLUMN);
	    if (!autotest (XSTART, YEND, XEND, YSTART, verbose)) {
		if (verbose)
		    printf ("ERROR: -x+yrow draw, +x-ycolumn read test\n");
		adderror (verbose);
		}

	    LDCONFIG (basecfr | UC_PFIYDOWN | UC_PFICOLUMN)
	    autodraw (XSTART, YEND, XEND, YSTART);
	    LDCONFIG (basecfr | UC_PFIREAD | UC_PFIXDOWN);
	    if (!autotest (XEND, YSTART, XSTART, YEND, verbose)) {
		if (verbose)
		    printf ("ERROR: +x-ycolumn draw, -x+yrow read test\n");
		adderror (verbose);
		}

	    LDCONFIG (basecfr | UC_PFIXDOWN | UC_PFIYDOWN | UC_PFICOLUMN)
	    autodraw (XEND, YEND, XSTART, YSTART);
	    LDCONFIG (basecfr | UC_PFIREAD);
	    if (!autotest (XSTART, YSTART, XEND, YEND, verbose)) {
		if (verbose)
		    printf ("ERROR: -x-ycolumn draw, +x+yrow read test\n");
		adderror (verbose);
		}
	    }
	}
    if (teststorun & 2) {
	/* invert draw test */
	restore (&s);
	if (verbose)
	    putchar ('1');
	if ((sigplanes&(A0|A1|B0|B1)) != (A0|A1|B0|B1)) {
	    if (verbose) {
	        printf ("pixeltest: ");
	        printf ("invert test needs planes A0,A1, and B0,B1\n");
		}
	    }
	else {
	    short i;
	    setcodes (0, 0xffffffff);
	    LDMODE (UC_SWIZZLE);
	    LDCONFIG (basecfr);
	    autodraw (XSTART, YSTART, XEND, YEND);
	    setcodes (0, A0|A1|B0|B1);
	    LDCONFIG (basecfr | UC_INVERT);
	    autodraw (XSTART, YSTART, XEND, YEND);
	    REQUEST (UC_SETADDRS, 0);
	    LDCONFIG (basecfr | UC_PFIREAD);
	    for (i=0; i<16; i++) {
		short temp;
		if ((temp= *UCCommandAddr(UC_READPIXELAB)) != (15-i)) {
		    if (verbose) {
			printf ("ERROR: pixel invert failed at pixel %x, ",i);
			printf ("got %x, expected %x\n", temp, 15-i);
			}
		    adderror (verbose);
		    break;
		    }
		}
	    }
	}
    if (teststorun & 4) {
	/* all pattern test */
	short xstart = 0;
	short xend = 0x40;
	short ystart = 0;
	short yend = 0x40;
	short i, x, y;
	short tempcfr;
	restore (&s);
	if (verbose)
	    putchar ('2');
	loadpattern (0x100);
	for (i=0; i<3; i++) {
	    LDCONFIG (basecfr);
	    LDMODE (0);
	    setcodes (0, 0xffffffff);
	    clear (ONESTIPADDR);
	    tempcfr = basecfr;
	    switch (i) {
		case 2: tempcfr |= UC_PATTERN64;
		case 1: tempcfr |= UC_PATTERN32;
		case 0: tempcfr |= UC_ALLPATTERN;
		}
	    LDCONFIG (tempcfr);
	    LDFMADDR (0x100);
	    for (y=ystart; y<=yend; y++) {
		for (x=xstart; x<=xend; x++) {
		    LDXS (x);
		    LDYS (y);
		    REQUEST (UC_SETADDRS, 0);
		    REQUEST (UC_DRAWPIXELAB, 0xffff);
		    }
		}
	    for (y=ystart; y<=yend; y++) {
		for (x=xstart; x<=xend; x++) {
		    short temp, mask;
		    LDXS (x);
		    LDYS (y);
		    REQUEST (UC_SETADDRS, 0);
		    temp = *UCCommandAddr(UC_READPIXELAB);
		    mask = patternbit (0x100, x, y, tempcfr);
		    if (temp && !mask || !temp && mask) {
			if (verbose) {
			    printf ("pattern error at %x,%x.  ", x, y);
			    printf ("size %xx%x.  ",
				(i==0) ? 16 : ((i=1) ? 32 : 64),
				(i==0) ? 16 : ((i=1) ? 32 : 64));
			    if (mask)
				printf ("mask true, pixel not written\n");
			    else
				printf ("mask false, pixel written\n");
			    }
			adderror (verbose);
			}
		    }
		}
	    }
	}
    if (teststorun & 8) {
	/* separate AB and CD write/read
	 *   insure that AB writes do not affect CD contents,
	 *	    that CD writes do not affect AB contents,
	 *	    that AB reads do not require a CD read at that address,
	 *	    that CD reads do not require an AB read at that address.
	 */
	restore (&s);
	if (verbose)
	    putchar ('3');
	if ((sigplanes&(A0|A1|B0|B1|C0|C1|D0|D1))!=(A0|A1|B0|B1|C0|C1|D0|D1)){
	    if (verbose) {
		printf ("AB/CD pixel test requires planes A01,B01,C01,D01\n");
		}
	    }
	else {
	    setcodes (0, 0xffffffff);
	    LDCONFIG (basecfr);
	    clear (ONESTIPADDR);
	    setcodes (0, A0|A1|B0|B1|C0|C1|D0|D1);
	    LDCONFIG (basecfr);
	    LDMODE (UC_SWIZZLE);
	    setpixel (0, 0, UC_DRAWPIXELAB, 1);
	    setpixel (0x100, 0x100, UC_DRAWPIXELCD, 4);
	    setpixel (0, 0, UC_DRAWPIXELCD, 2);
	    setpixel (0x100, 0x100, UC_DRAWPIXELAB, 3);
	    testpixel (0, 0, UC_READPIXELAB, 1, verbose);
	    testpixel (0x100, 0x100, UC_READPIXELCD, 4, verbose);
	    testpixel (0, 0, UC_READPIXELCD, 2, verbose);
	    testpixel (0x100, 0x100, UC_READPIXELAB, 3, verbose);
	    }
	}
    restore (&s);
    return geterrors ();
    }

static autodraw (xs, ys, xe, ye)
short xs, ys, xe, ye;
{
    /*
     *	Draw 16 pixels with values 0..15.  Call with swizzle true and
     *	  with appropriate PFI values in CFR.
     */
    short i;
    LDXS (xs);
    LDYS (ys);
    LDXE (xe);
    LDYE (ye);
    REQUEST (UC_SETADDRS, 0);
    for (i=0; i<16; i++) {
	REQUEST (UC_DRAWPIXELAB, i);
	}
    }

static autotest (xs, ys, xe, ye, verbose)
short xs, ys, xe, ye;
boolean verbose;
{
    /*
     *	Reads 16 pixels and compares their values with those in the
     *	  static array readorder[].  Returns 1 if all values are equal,
     *	  0 otherwise.
     *	Readorder[] is the opposite-x-direction, opposite-y-direction,
     *	  opposite-row/column, opposite-corner-start traversal of the
     *	  4x4 array:
     *
     *		12 13 14 15
     *		 8  9 10 11
     *		 4  5  6  7
     *		 0  1  2  3
     */
    short i;
    static short readorder[16] = {15,11,7,3,14,10,6,2,13,9,5,1,12,8,4,0};
    LDXS (xs);
    LDYS (ys);
    LDXE (xe);
    LDYE (ye);
    REQUEST (UC_SETADDRS, 0);
    for (i=0; i<16; i++) {
	short temp;
	if ((temp= *UCCommandAddr(UC_READPIXELAB)) != readorder[i]) {
	    if (verbose)
		printf ("autotest: expecting %x, got %x\n",readorder[i],temp);
	    return 0;
	    }
	}
    return 1;
    }

patternbit (baseaddr, x, y, cfr)
long baseaddr;	/* base address of a pattern in the font memory */
short x, y;	/* pixel whose mask value is to be returned */
short cfr;	/* includes bits UC_PATTERN32 and UC_PATTERN64 */
{
    /*
     *	Returns the font memory bit that would be used to mask pixel x,y
     *    given baseaddr and cfr.  Returns 0 if UC_PATTERN64 is true and
     *	  UC_PATTERN32 is false (this is an illegal mode).
     */
    long fmaddr;
    if (cfr & UC_PATTERN64) {
	/* 64x64 pattern */
	if (!(cfr & UC_PATTERN32)) {
	    printf ("ERROR: patternbit called with 64 and not 32\n");
	    return 0;
	    }
	if (baseaddr & 0xff) {
	    printf ("WARNING: patternbit: bad 64x64 address %x\n", baseaddr);
	    baseaddr &= 0xff00;
	    }
	fmaddr = (baseaddr & 0xff00) |
		 (y&0x20 ? 0x80 : 0) |
		 (x&0x20 ? 0x40 : 0) |
		 (y&0x10 ? 0x20 : 0) |
		 (x&0x10 ? 0x10 : 0) |
		 (y&0x0f);
	}
    else if (cfr & UC_PATTERN32) {
	/* 32x32 pattern */
	if (baseaddr & 0x3f) {
	    printf ("WARNING: patternbit: bad 32x32 address %x\n", baseaddr);
	    baseaddr &= 0xffc0;
	    }
	fmaddr = (baseaddr & 0xffc0) |
		 (y&0x10 ? 0x20 : 0) |
		 (x&0x10 ? 0x10 : 0) |
		 (y&0x0f);
	}
    else {
	/* 16x16 pattern */
	if (baseaddr & 0xf) {
	    printf ("WARNING: patternbit: bad 16x16 address %x\n", baseaddr);
	    baseaddr &= 0xfff0;
	    }
	fmaddr = (baseaddr & 0xfff0) |
		 (y&0x0f);
	}
    LDFMADDR (fmaddr);
    REQUEST (UC_SETADDRS, 0);
    return (*UCCommandAddr(UC_READFONT) & (1 << (0xf-(x&0xf)))) ? 1 : 0;
    }

loadpattern (baseaddr)
long baseaddr;
{
    /*
     *	Writes a 256-word pattern (enough for 64x64 pixels) into the font
     *	  memory, starting at the given base address.
     */
    static short blockstart[16] = {0x0001, 0x0003, 0x0007, 0x000f,
				   0x001f, 0x003f, 0x007f, 0x00ff,
				   0x01ff, 0x03ff, 0x07ff, 0x0fff,
				   0x1fff, 0x3fff, 0x7fff, 0xffff};
    short i, j;
    long pattern;
    if (baseaddr & 0xff) {
	printf ("WARNING: loadpattern: bad base address %x\n", baseaddr);
	baseaddr &= 0x3f00;
	}
    LDFMADDR (baseaddr);
    REQUEST (UC_SETADDRS, 0);
    for (i=0; i<16; i++) {
	pattern = blockstart[i];
	for (j=0; j<16; j++) {
	    REQUEST (UC_WRITEFONT, pattern);
	    pattern <<= 1;
	    if (pattern & 0x10000) {
		pattern &= 0xffff;
		pattern |= 1;
		}
	    }
	}
    }
	    
static setpixel (x, y, cmnd, value)
short x, y, cmnd, value;
{
    LDXS (x);
    LDYS (y);
    REQUEST (UC_SETADDRS, 0);
    REQUEST (cmnd, value);
    }

static testpixel (x, y, cmnd, value, verbose)
short x, y, cmnd, value;
boolean verbose;
{
    short temp;
    LDXS (x);
    LDYS (y);
    REQUEST (UC_SETADDRS, 0);
    if ((temp= *UCCommandAddr(cmnd)) != value) {
	if (verbose) {
	    printf ("ERROR: testpixel expected %x, got %x\n", value, temp);
	    }
	adderror (verbose);
	}
    }


#endif UC4
