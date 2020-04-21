/*
 *	Kurt Akeley			4/7/83
 *
 *	This set of routines tests the entire functionality of the 
 *	  update controller (UC2 or UC3).  The previous sentence is a
 *	  big lie.  This package, in conjuction with other test
 *	  routines, provides the basis for a complete UC test.
 *
 *	Functions:
 *		long	linetest (verbose, teststorun)
 *		long	viewporttest (verbose)
 *		long	recttest (verbose)
 *		long	chartest (verbose)
 *		long	traptest (verbose, teststorun)
 *		boolean	checkline (x0, y0, x1, y1, stipple)
 *
 *	Procedures:
 *		uclamptest (cycles)
 *		setviewport (x, y, width, height)
 *
 *	Updates:
 *		4/8/83   KBA	Created
 */

#include "console.h"
#include "uctest.h"
#include "ucdev.h"

#define WAIT	for (wait=0; wait<60; wait++) waitvert (TRUE);

uclamptest (cycles)
short		cycles;		/* number of rotations to do */
{
    /*
     *	Cycles the uc LEDs in a shift register pattern at a visible rate.
     *    Note that two of the UC LEDs are not under direct register
     *    control, and therefore cannot be rotated through.
     */

    short	count;
    short	pattern;
    short	wait;
    Save	s;

    save (&s);
#ifdef UC4
    LDMODE (0);
    REQUEST (UC_NOOP, 0)
#endif UC4
    for (count=0; count<cycles; count++) {
	breakcheck ();
	for (pattern=1; pattern<0x40; pattern<<=1) {
	    LDCONFIG (pattern);
	    REQUEST (LOADXYADDR, 0);
	    WAIT
	    }
#ifdef UC4
	LDCONFIG (UC_ALLPATTERN);
	REQUEST (UC_NOOP, 0)
	WAIT
	LDCONFIG (0);
	LDMODE (UC_SWIZZLE)
	REQUEST (UC_NOOP, 0)
	WAIT
	LDMODE (UC_DOUBLE)
	REQUEST (UC_NOOP, 0)
	WAIT
	LDMODE (UC_DEPTHCUE)
	REQUEST (UC_NOOP, 0)
	WAIT
	LDMODE (0)
	REQUEST (UC_NOOP, 0)
	*UCRAddr = 0;
	WAIT
	*UCRAddr = uc_ucr;
#endif UC4
	}
    restore (&s);
    REQUEST (LOADXYADDR, 0);
    }



#define CONTINUESTIPPLE		0xff18



long	linetest (verbose, teststorun)
boolean	verbose;	/* if true (not 0) errors result in messages	*/
short	teststorun;	/* bits 0..2 enable tests 1..3			*/
{
    /*
     *	Test the line drawing hardware
     */
    short	i;
    short	x, y;
    short	lcfr;
    long	stipple;
    Save	s;
    static short	x1[8] = {200, 200, 200, 200, 200, 200, 200, 200};
    static short	y1[8] = {200, 200, 200, 200, 200, 200, 200, 200};
    static short	x2[8] = {210, 220, 220, 210, 190, 180, 180, 190};
    static short	y2[8] = {220, 210, 190, 180, 180, 190, 210, 220};
    static short	stip[8] = {0xff3c, 0xff3c, 0xff3c, 0xff3c,
				   0xff3c, 0xff3c, 0xff3c, 0xff3c};

    initerror (10);
    save (&s);
    lcfr = s.cfb & (UPDATEA | UPDATEB | DISPLAYA | DISPLAYB);

    if (teststorun & 1) {
	/*
	 *  Check the ER circuit to see that both EDR and ECR are being
	 *    correctly loaded and operated on.  Lines with ECR set to
	 *    1, 2, 4, 8, ... and EDR set to -1, -2, -4, -8, ... are drawn
	 *    into blank screens, then read back pixel by pixel to check
	 *    the circuit operation.
	 */

	restore (&s);
	if (verbose)
	    putchar ('0');
	LDCONFIG (lcfr | LDLINESTIP);
	for (x=1; x<=0x400; x<<=1) {
	    breakcheck ();
	    for (y=1; y<=x; y<<=1) {
		setcodes (0, sigplanes);
		clear (ONESTIPADDR);
		setcodes (sigplanes, sigplanes);
		drawline (0, 0, x, y, 0xffff);
		if (!checkline (0, 0, x, y, 0xffff)) {
		    if (verbose) {
		        printf ("ERROR: er test, ");
			printf ("ed = %3x, ec = %3x\n", -y, x);
			}
		    adderror (verbose);
		    }
		}
	    }
	}

    if (teststorun & 2) {
	/*
	 *  Test the line stipple shift register across a 16-bit boundry
	 */
	restore (&s);
	if (verbose)
	    putchar ('1');
	breakcheck ();
	LDCONFIG (lcfr | LDLINESTIP);
	for (stipple=1; stipple<0x10000; stipple<<=1) {
	    setcodes (0, sigplanes);
	    clear (ONESTIPADDR);
	    setcodes (sigplanes, sigplanes);
	    drawline (0, 0, 31, 0, stipple);
	    if (!checkline (0, 0, 31, 0, stipple)) {
		if (verbose) {
		    printf ("ERROR: stipple test\n");
		    }
		adderror (verbose);
		}
	    }
	/*
	 *  Test stipple continue
	 */
	breakcheck ();
	LDCONFIG (lcfr | LDLINESTIP);
	setcodes (0, sigplanes);
	clear (ONESTIPADDR);
	setcodes (sigplanes, sigplanes);
	drawline (0, 0, 3, 0, CONTINUESTIPPLE);
	LDCONFIG (lcfr);
	drawline (3, 0, 8, 0, CONTINUESTIPPLE);
	drawline (8, 0, 15, 0, CONTINUESTIPPLE);
	drawline (15, 0, 31, 0, CONTINUESTIPPLE);
	if (!checkline (0, 0, 31, 0, CONTINUESTIPPLE)) {
	    if (verbose) {
		printf ("ERROR: stipple continue failed\n");
		}
	    adderror (verbose);
	    }
	/*
	 *  Test stipple backup
	 */
	breakcheck ();
	LDCONFIG (lcfr | LDLINESTIP | BACKLINE);
	setcodes (0, sigplanes);
	clear (ONESTIPADDR);
	setcodes (sigplanes, sigplanes);
	drawline (0, 0, 10, 0, 0);
#ifdef UC3
	if (!checkline (0, 0, 10, 0, 0x700)) {
#endif UC3
#ifdef UC4
	if (!checkline (0, 0, 10, 0, 0x600)) {
#endif UC4
	    if (verbose) {
		printf ("ERROR: stipple backup failed\n");
		}
	    adderror (verbose);
	    }
	LDCONFIG (lcfr | LDLINESTIP);
	setcodes (0, sigplanes);
	clear (ONESTIPADDR);
	setcodes (sigplanes, sigplanes);
	drawline (0, 0, 10, 0, 0);
	if (!checkline (0, 0, 10, 0, 0)) {
	    if (verbose) {
		printf ("ERROR: stipple backup stuck on\n");
		}
	    adderror (verbose);
	    }
	}

    if (teststorun & 4) {
	/*
	 *  Draw one line in each octant
	 */
	restore (&s);
	if (verbose)
	    putchar ('2');
	breakcheck ();
	LDCONFIG (lcfr | LDLINESTIP);
	for (i=0; i<8; i++) {
    	    setcodes (0, sigplanes);
	    clear (ONESTIPADDR);
	    setcodes (sigplanes, sigplanes);
	    drawline (x1[i], y1[i], x2[i], y2[i], stip[i]);
	    if (!checkline (x1[i], y1[i], x2[i], y2[i], stip[i])) {
		if (verbose) {
		    printf ("ERROR: line in octant %d failed\n",
			     (i + 1 + (i>>1)));
		    }
		adderror (verbose);
		}
	    }
	}
#ifdef UC4
    if (teststorun & 8) {
	/* test inverted lines */
	restore (&s);
	if (verbose)
	    putchar ('3');
	breakcheck ();
	LDCONFIG (lcfr);
	for (i=0; i<8; i++) {
    	    setcodes (0, sigplanes);
	    clear (ONESTIPADDR);
	    setcodes (sigplanes, sigplanes);
	    LDCONFIG (lcfr | LDLINESTIP);
	    drawline (x1[i], y1[i], x2[i], y2[i], stip[i]);
	    LDCONFIG (lcfr | LDLINESTIP | UC_INVERT);
	    drawline (x1[i], y1[i], x2[i], y2[i], stip[i]);
	    LDCONFIG (lcfr);
	    if (!checkline (x1[i], y1[i], x2[i], y2[i], 0)) {
		if (verbose) {
		    printf ("ERROR: inverted line in octant %d failed\n",
			     (i + 1 + (i>>1)));
		    }
		adderror (verbose);
		}
	    }
	}
    if (teststorun & 0x10) {
	/* depthcue test - assumes that ddatest gets most errors */
	restore (&s);
	if (verbose) 
	    putchar ('4');
	if ((sigplanes & 0x303) != 0x303) {
	    if (verbose) {
	printf ("\nWARNING:Depthcue line test requires bitplanes A01, B01\n");
		printf ("\tTest not performed if bitplanes not present\n");
	    }
	    }
	else {
	    short i;
	    LDXS (0); LDXE (0xf); LDYS (0); LDYE (0);
	    LDED (0); LDEC (0);
	    LDDDASAF (0);
	    LDDDASAI (0);
	    LDDDASDF (0);
	    LDDDASDI (1);
	    LDCONFIG (lcfr | UC_LDLINESTIP);
	    REQUEST (UC_NOOP, 0xffff);
	    LDCONFIG (lcfr);
	    LDMODE (UC_SWIZZLE | UC_DEPTHCUE);
	    REQUEST (UC_DRAWLINE2, 0);
	    LDCONFIG (lcfr | UC_PFIREAD);
	    REQUEST (UC_SETADDRS, 0);
	    for (i=0; i<16; i++) {
		short temp;
		if ((temp = (*UCCommandAddr(UC_READPIXELAB))&0xf) != i) {
		    if (verbose) {
			printf ("Failed depthcue test at pixel %x, ", i);
			printf ("expected %x, got %x\n", i, temp);
			}
		    adderror (verbose);
		    break;
		    }
		}
	    }
	LDCONFIG (lcfr);
	LDMODE (0);
	}
    if (teststorun & 0x20) {
	/* line stipple readback */
	short i;
	restore (&s);
	if (verbose)
	    putchar ('5');
	for (i=0; i<16; i++) {
	    long temp;
	    LDCONFIG (lcfr | UC_LDLINESTIP);
	    REQUEST (UC_NOOP, 1<<i);
	    LDCONFIG (lcfr);
	    if ((temp = (*UCCommandAddr(UC_READLSTIP))&0xffff) != (1<<i)) {
		if (verbose) {
		    printf ("Failed stipple readback at pixel %x, ", i);
		    printf ("expected %x, got %x\n", 1<<i, temp);
		    }
		adderror (verbose);
		}
	    }
	}
    if (teststorun & 0x40) {
	/* stipple repeat test - draw and register readback */
	short i, j;
	restore (&s);
	if (verbose)
	    putchar ('6');
	LDMODE (0);
	LDXS (0); LDXE (0x3ff); LDYS (0x10); LDYE (0x10); LDED (0); LDEC (0);
	for (i=0; i<256; i++) {
	    short temp;
	    long stip;
	    short repeat;
	    setcodes (0, sigplanes);
	    LDCONFIG (lcfr | UC_LDLINESTIP);
	    REQUEST (UC_DRAWLINE2, 0xffff);
	    LDREPEAT (i);
	    REQUEST (UC_NOOP, 0x5555);
	    LDCONFIG (lcfr);
	    if ((temp=(*UCCommandAddr(UC_READREPEAT)&0xff)) != i) {
		if (verbose) {
		    printf ("Failure of stipple repeat readback, ");
		    printf ("expected %x, got %x\n", i, temp);
		    }
		adderror (verbose);
		continue;
		}
	    setcodes (sigplanes, sigplanes);
	    REQUEST (DRAWLINE2, 0);
	    REQUEST (UC_SETADDRS, 0);
	    LDCONFIG (lcfr | UC_PFIREAD);
	    for (j=0,stip=0x5555,repeat=i; j<0x400; j++) {
		long temp;
		temp = *(long*)UCCommandAddr(UC_READPIXELCD);
		if ((stip&1) && (temp!=sigplanes)) {
		    if (verbose) {
			printf ("Failed stipple repeat test at pixel %x, ",j);
			printf ("repeat=%x, expected %x, got %x\n", repeat,
				sigplanes, temp);
			}
		    adderror (verbose);
		    }
		else if ((stip&1==0) && (temp!=0)) {
		    if (verbose) {
			printf ("Failed stipple repeat test at pixel %x, ",j);
			printf ("repeat=%x, expected 0, got %x\n", repeat,
				temp);
			}
		    adderror (verbose);
		    }
		if (repeat==0) {
		    repeat = i;
		    if (stip&1)
			stip = (stip>>1) | 0x8000;
		    else
			stip = stip>>1;
		    }
		else
		    repeat -= 1;
		}
	    }
	}
#endif UC4	
    restore (&s);
    return geterrors ();
    }



boolean	checkline (x0, y0, x1, y1, stipple)
short	x0, y0, x1, y1, stipple;
{
    /*
     *	Simulates the line drawing algorithm used by the IRIS update
     *	  controller.  Each line pixel is read back from the bitplane
     *	  and compared to the correct stipple value.  It is assumed that
     *	  stipple-zero pixels will be zero in all planes, and that
     *	  stipple-one pixels will be one in all significant planes.
     *  BACKLINE is not implemented.
     *	LDLINESTIP is assumed to be true.
     */
    short	x, y;
    short	dx, dy;
    short	absdx, absdy;
    short	xincr, yincr;
    short	edr, ecr, er;
    long	stip;
    long	pixels[16];

    dx = x1 - x0;
    if (dx >= 0) {
	absdx = dx;
	xincr = 1;
	}
    else {
	absdx = -dx;
	xincr = -1;
	}
    dy = y1 - y0;
    if (dy >= 0) {
	absdy = dy;
	yincr = 1;
	}
    else {
	absdy = -dy;
	yincr = -1;
	}

    x = x0;
    y = y0;
    stip = stipple & 0xffff;
    if (absdy >= absdx) {
	edr = -(absdx<<1);
	ecr = absdy<<1;
	er  = absdy;
	while (1) {
	    readpixels (pixels, x>>4, y, sigplanes);
	    if (stip&1) {
		if (pixels[x&15] != sigplanes) {
		    if ((x>=0)&&(x<0x400)&&(y>=0)&&(y<0x400)) {
			return (FALSE);
			}
		    }
		stip = (stip>>1) | 0x8000;
		}
	    else {
		if (pixels[x&15] != 0) {
		    if ((x>=0)&&(x<0x400)&&(y>=0)&&(y<0x400))
			return (FALSE);
		    }
		stip = (stip>>1) & 0x7fff;
		}
	    if (y == y1)
		break;
	    y += yincr;
	    er += edr;
	    if (er < 0) {
		er += ecr;
		x += xincr;
		}
	    }
	}
    else {
	edr = -(absdy<<1);
	ecr = absdx<<1;
	er  = absdx;
	while (1) {
	    readpixels (pixels, x>>4, y, sigplanes);
	    if (stip&1) {
		if (pixels[x&15] != sigplanes) {
		    if ((x>=0)&&(x<0x400)&&(y>=0)&&(y<0x400)) {
			return (FALSE);
			}
		    }
		stip = (stip>>1) | 0x8000;
		}
	    else {
		if (pixels[x&15] != 0) {
		    if ((x>=0)&&(x<0x400)&&(y>=0)&&(y<0x400))
			return (FALSE);
		    }
		stip = (stip>>1) & 0x7fff;
		}
	    if (x == x1)
		break;
	    x += xincr;
	    er += edr;
	    if (er < 0) {
		er += ecr;
		y += yincr;
		}
	    }
	}
    return (TRUE);
    }



#define LARGEDIM		100

long	viewporttest (verbose)
boolean	verbose;
{
    /*
     *	Test all viewport functionality.
     */

    short	x, y;
    short	x2, y2;
    short	lcfr;
    long	pixels[16];
    Save	s;

    save (&s);
    initerror (10);
    lcfr = s.cfb & (UPDATEA | UPDATEB | DISPLAYA | DISPLAYB);

    /*
     *	Test the 8/16-bit left and right viewport masks.
     */

    for (x=0; x<16; x++) {
	breakcheck ();
	LDCONFIG (lcfr);
	setcodes (0, sigplanes);
	drawrect (0, 0, 16, LARGEDIM, ONESTIPADDR);
	LDCONFIG (lcfr | VIEWPORTMASK);
	setcodes (sigplanes, sigplanes);
	setviewport (x, 0, 1, LARGEDIM);
	drawrect (0, 0, 16, LARGEDIM, ONESTIPADDR);
	readpixels (pixels, 0, 0, sigplanes);
	for (x2=0; x2<16; x2++) {
	    if (x == x2) {
		if (pixels[x2] != sigplanes) {
		    if (verbose) {
			printf ("ERROR: screenmask failure\n");
			printf ("    expected %8x, received %8x\n",
				sigplanes, pixels[x2]);
			}
		    adderror (verbose);
		    }
		}
	    else {
		if (pixels[x2] != 0) {
		    if (verbose) {
			printf ("ERROR: screenmask failure\n");
			printf ("    expected %8x, received %8x\n",
				0, pixels[x2]);
			}
		    adderror (verbose);
		    }
		}
	    }
	}

    /*
     *	Test x byte viewport masking
     */

    for (x=0; x<1024; x+=8) {
	breakcheck ();
	LDCONFIG (lcfr);
	setcodes (0, sigplanes);
	drawrect (0, 0, 1024, LARGEDIM, ONESTIPADDR);
	LDCONFIG (lcfr | VIEWPORTMASK);
	setcodes (sigplanes, sigplanes);
	setviewport (x, 0, 8, LARGEDIM);
	drawrect (0, 0, 1024, LARGEDIM, ONESTIPADDR);
	for (x2=0; x2<1024; x2+=8) {
	    if ((x2&8)==0)	/* we get 16 pixels per call */
		readpixels (pixels, x2>>4, 0, sigplanes);
	    if (x == x2) {	/* check only one pixel per byte */
		if (pixels[x2&8] != sigplanes) {
		    if (verbose) {
			printf ("ERROR: viewport x address failure\n");
			}
		    adderror (verbose);
		    }
		}
	    else {
		if (pixels[x2&8] != 0) {
		    if (verbose) {
			printf ("ERROR: viewport x address failure\n");
			}
		    adderror (verbose);
		    }
		}
	    }
	}


    /*
     *	Test y viewport masking
     */

    for (y=1; y<1024; y<<=1) {
	breakcheck ();
	LDCONFIG (lcfr);
	setcodes (0, sigplanes);
	drawrect (0, 0, LARGEDIM, 1024, ONESTIPADDR);
	LDCONFIG (lcfr | VIEWPORTMASK);
	setcodes (sigplanes, sigplanes);
	setviewport (0, y, LARGEDIM, 1);
	drawrect (0, 0, LARGEDIM, 1024, ONESTIPADDR);
	for (y2=0; y2<1024; y2+=1) {
	    readpixels (pixels, 0, y2, sigplanes);
	    if (y == y2) {
		if (pixels[0] != sigplanes) {
		    if (verbose) {
			printf ("ERROR: viewport y address failure\n");
			}
		    adderror (verbose);
		    }
		}
	    else {
		if (pixels[0] != 0) {
		    if (verbose) {
			printf ("ERROR: viewport y address failure\n");
			}
		    adderror (verbose);
		    }
		}
	    }
	}
    restore (&s);
    return geterrors ();
    }



setviewport (x, y, width, height)
short	x, y, width, height;
{
    Save s;
    save (&s);
    LDXS (x);
    LDYS (y);
    LDXE (x + width - 1);
    LDYE (y + height - 1);
#ifdef UC3
    REQUEST (LOADVIEWPORT, 0);
#endif UC3
#ifdef UC4
    REQUEST (UC_SETSCRMASKX, 0);
    REQUEST (UC_SETSCRMASKY, 0);
#endif UC4
    restore (&s);
    }



long	recttest (verbose)
boolean	verbose;
{
    /*
     *	Insure correct operation of rectfill command
     */

    short	x, y;
    short	x2;
    short	lcfr;
    long	pixels[16];
    Save	s;
    static short stipple[16] =	{0x8080, 0x8080, 0x4040, 0x4040,
				 0x2020, 0x2020, 0x1010, 0x1010,
				 0x0808, 0x0808, 0x0404, 0x0404,
				 0x0202, 0x0202, 0x0101, 0x0101};

    save (&s);
    breakcheck ();
    initerror (10);
    lcfr = s.cfb & (UPDATEA | UPDATEB | DISPLAYA | DISPLAYB);

    /*
     *	Test mask alignment
     */

    if (verbose)
	putchar ('0');
    LDCONFIG (lcfr);
    setcodes (0, sigplanes);
    clear (ONESTIPADDR);
    setstipple (STIPADDR, stipple);
    setcodes (sigplanes, sigplanes);
    for (x=0, y=0; x<16; x++, y++)
	drawrect (x, y, 16-x, 16-y, STIPADDR);
    for (y=0; y<16; y++) {
	readpixels (pixels, 0, y, sigplanes);
	for (x=0; x<16; x++) {
	    if ((x == (y>>1)) || (x == ((y>>1)+8))) {
		if (pixels[x] != sigplanes) {
		    if (verbose) {
			printf ("ERROR: rect stip alignment off, ");
			printf ("x = %3x, y = %3x\n", x, y);
			}
		    adderror (verbose);
		    }
		}
	    else {
		if (pixels[x] != 0) {
		    if (verbose) {
			printf ("ERROR: rect stip alignment off, ");
			printf ("x = %3x, y = %3x\n", x, y);
			}
		    adderror (verbose);
		    }
		}
	    }
	}

    /*
     *	Test the 8/16-bit left and right update masks.
     */

    if (verbose)
	putchar ('1');
    for (x=0; x<16; x++) {
	setcodes (0, sigplanes);
	drawrect (0, 0, 16, 1, ONESTIPADDR);
	setcodes (sigplanes, sigplanes);
	drawrect (x, 0, 1, 1, ONESTIPADDR);
	readpixels (pixels, 0, 0, sigplanes);
	for (x2=0; x2<16; x2++) {
	    if (x == x2) {
		if (pixels[x2] != sigplanes) {
		    if (verbose) {
			printf ("ERROR: rectangle mask failure, ");
			printf ("x = %3x\n", x2);
			}
		    adderror (verbose);
		    }
		}
	    else {
		if (pixels[x2] != 0) {
		    if (verbose) {
			printf ("ERROR: rectangle mask failure\n");
			}
		    adderror (verbose);
		    }
		}
	    }
	}
#ifdef UC4
    /* test all 3 pattern sizes */
    {
    short xstart = 0;
    short xend = 0x40;
    short ystart = 0;
    short yend = 0x40;
    short i, x, y;
    short tempcfr;
    if (verbose)
	putchar ('2');
    restore (&s);
    loadpattern (0x100);
    for (i=0; i<3; i++) {
	LDCONFIG (lcfr);
	LDMODE (0);
	setcodes (0, 0xffffffff);
	clear (ONESTIPADDR);
	tempcfr = lcfr;
	switch (i) {
	    case 2: tempcfr |= UC_PATTERN64;
	    case 1: tempcfr |= UC_PATTERN32;
	    case 0: tempcfr |= UC_ALLPATTERN;
	    }
	LDCONFIG (tempcfr);
	LDFMADDR (0x100);
	setcodes (0xffffffff, 0xffffffff);
	clear (0x100);
	for (y=ystart; y<=yend; y++) {
	    for (x=xstart; x<=xend; x++) {
		drawrect (x, y, xend-xstart+1, yend-ystart+1, 0x100);
		}
	    }
	for (y=ystart; y<=(2*yend); y++) {
	    for (x=xstart; x<=(2*xend); x++) {
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
	}}
    /* test invert mode in all 3 pattern sizes */
    {
    short xstart = 0;
    short xend = 0x40;
    short ystart = 0;
    short yend = 0x40;
    short i, x, y;
    short tempcfr;
    if (verbose)
	putchar ('3');
    restore (&s);
    loadpattern (0x100);
    for (i=0; i<3; i++) {
	LDCONFIG (lcfr);
	LDMODE (0);
	setcodes (0, 0xffffffff);
	clear (ONESTIPADDR);
	tempcfr = lcfr;
	switch (i) {
	    case 2: tempcfr |= UC_PATTERN64;
	    case 1: tempcfr |= UC_PATTERN32;
	    case 0: tempcfr |= UC_ALLPATTERN;
	    }
	LDCONFIG (tempcfr);
	LDFMADDR (0x100);
	setcodes (0xffffffff, 0xffffffff);
	for (y=ystart; y<=yend; y++) {
	    for (x=xstart; x<=xend; x++) {
		drawrect (x, y, xend-xstart+1, yend-ystart+1, 0x100);
		LDCONFIG (tempcfr | UC_INVERT);
		drawrect (x, y, xend-xstart+1, yend-ystart+1, 0x100);
		LDCONFIG (tempcfr);
		}
	    }
	for (y=ystart; y<=(2*yend); y++) {
	    for (x=xstart; x<=(2*xend); x++) {
		short temp;
		LDXS (x);
		LDYS (y);
		REQUEST (UC_SETADDRS, 0);
		temp = *UCCommandAddr(UC_READPIXELAB);
		if (temp) {
		    if (verbose) {
			printf ("invert pattern error at %x,%x.  ", x, y);
			printf ("size %xx%x.  ",
			    (i==0) ? 16 : ((i=1) ? 32 : 64),
			    (i==0) ? 16 : ((i=1) ? 32 : 64));
			}
		    adderror (verbose);
		    }
		}
	    }
	}}
#endif UC4
    restore (&s);
    return geterrors ();
    }



long	chartest (verbose)
boolean	verbose;
{
    /*
     *  Check barrel shifter operation.
     *  Check xgte comparison operation.
     */

    short	xoffset;
    short	x, y;
    short	lcfr;
    long	pixels[16];
    Save s;
    static short stipple[8] =	{0x8080, 0xc0c0, 0xe0e0, 0xf0f0,
				 0xf8f8, 0xfcfc, 0xfefe, 0xffff};

    save (&s);
    breakcheck ();
    initerror (10);
    lcfr = s.cfb & (UPDATEA | UPDATEB | DISPLAYA | DISPLAYB);

    LDCONFIG (lcfr);
    setstipple (STIPADDR, stipple);
    for (xoffset=0; xoffset<8; xoffset++) {
	setcodes (0, sigplanes);
	drawrect (0, 0, 16, 8, ONESTIPADDR);
	setcodes (sigplanes, sigplanes);
	drawchar (xoffset, 0, 8, 8, STIPADDR);
	for (y=0; y<8; y++) {
	    readpixels (pixels, 0, y, sigplanes);
	    for (x=0; x<7; x++) {
		if ((x<xoffset) || (x > (xoffset+y))) {
		    if (pixels[x] != 0) {
			if (verbose) {
			    printf ("ERROR: character problem, ");
			    printf ("xoffset = %3x, x = %3x, y = %3x\n",
				    xoffset, x, y);
			    }
			adderror (verbose);
			}
		    }
		else {
		    if (pixels[x] != sigplanes) {
			if (verbose) {
			    printf ("ERROR: character problem, ");
			    printf ("xoffset = %3x, x = %3x, y = %3x\n",
				    xoffset, x, y);
			    }
			adderror (verbose);
			}
		    }
		}
	    }
	}
    restore (&s);
    return geterrors ();
    }

#ifdef UC4
long traptest (verbose, teststorun)
boolean verbose;
short teststorun;
{
    Save s;
    short lcfr;

    initerror (2);
    save (&s);
    lcfr = s.cfb & (UPDATEA|UPDATEB|DISPLAYA|DISPLAYB);

    if (teststorun & 1) {
	/* edge test - fill various width parallelograms */
	short width;
	short height = 0x20;	
	short x, y;
	if (verbose)
	    putchar ('0');
	for (width=1; width<40; width++) {	
	    LDMODE (0);
	    LDCONFIG (lcfr);
	    setcodes (0, 0xffffffff);
	    clear (ONESTIPADDR);
	    setcodes (0xffffffff, 0xffffffff);
	    filltrap(width,0,width,height,width+height-1,width,ONESTIPADDR);
	    for (y=0; y<height; y++) {
		for (x=0; x<(2*width+height+10); x++) {
		    short temp, mask;
		    LDXS (x);
		    LDYS (y);
		    REQUEST (UC_SETADDRS, 0);
		    temp = *UCCommandAddr(UC_READPIXELAB);
		    mask = (x >= (width+y) && x < (2*width+y)) ? 1 : 0;
		    if (temp && !mask || !temp && mask) {
			if (verbose) {
			    printf ("parallelogram error at %x,%x, ", x, y);
			    if (mask)
				printf ("pixel should be drawn\n");
			    else
				printf ("pixel should not be drawn\n");
			    }
			adderror (verbose);
			}
		    }
		}
	    }
	}
    if (teststorun & 2) {
	/* invert test - fill various width parallelograms */
	short width;
	short height = 0x20;
	short x, y;
	if (verbose)
	    putchar ('1');
	for (width=1; width<40; width++) {
	    LDMODE (0);
	    LDCONFIG (lcfr);
	    setcodes (0, 0xffffffff);
	    clear (ONESTIPADDR);
	    setcodes (0xffffffff, 0xffffffff);
	    filltrap(width,0,width,height,width+height-1,width,ONESTIPADDR);
	    LDCONFIG (lcfr | UC_INVERT);
	    filltrap(width,0,width,height,width+height-1,width,ONESTIPADDR);
	    LDCONFIG (lcfr);	    
	    for (y=0; y<height; y++) {
		for (x=0; x<(2*width+height+10); x++) {
		    short temp;
		    LDXS (x);
		    LDYS (y);
		    REQUEST (UC_SETADDRS, 0);
		    temp = *UCCommandAddr(UC_READPIXELAB);
		    if (temp) {
			if (verbose) {
			    printf ("invert error at %x,%x, ", x, y);
			    printf ("pixel should not be drawn\n");
			    }
			adderror (verbose);
			}
		    }
		}
	    }
	}
    if (teststorun & 4) {
	/* pattern test - fill various width parallelograms */
	short width;
	short height = 0x40;
	short x, y;
	if (verbose)
	    putchar ('2');
	LDMODE (0);
	LDCONFIG (lcfr);
	setcodes (0, 0xffffffff);
	clear (ONESTIPADDR);
	loadpattern (0x100);
	setcodes (0xffffffff, 0xffffffff);
	LDCONFIG (lcfr | UC_PATTERN32 | UC_PATTERN64);
	clear (0x100);
	for (width=1; width<0x40; width++) {
	    filltrap(width,0,width,height,width+height-1,width,0x100);
	    }
	for (y=0; y<height; y++) {
	    for (x=0; x<(2*width+height+10); x++) {
		short temp, mask;
		LDXS (x);
		LDYS (y);
		REQUEST (UC_SETADDRS, 0);
		temp = *UCCommandAddr(UC_READPIXELAB);
		mask = patternbit (0x100, x, y, uc_cfb);
		if (temp && !mask || !temp & mask) {
		    if (verbose) {
			printf ("pattern error at %x,%x, ", x, y);
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
    restore (&s);
    return geterrors ();
    }
#endif UC4
