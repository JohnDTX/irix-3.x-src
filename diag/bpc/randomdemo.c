/*
 *	Kurt Akeley			9/18/82
 *
 *	These routines are intended to be linked with the main routine
 *	"console.b".
 *
 *	Procedures:
 *		randrect (maxdim)
 *		randlines (seed, count, stipple)
 *
 *	Updates:
 *		9/18/82	 KBA	Copied from console.c
 */

#include "uctest.h"
#include "console.h"
#include "ucdev.h"

#define RANDOMIZE(rand)		rand = rand * 5 + 17623

#define FONTADR0		0x100
#define FONTADR1		(FONTADR0+0x10)
#define FONTADR2		(FONTADR0+0x20)
#define FONTADR3		(FONTADR0+0x30)
#define FONTADR4		(FONTADR0+0x40)

randrect (maxdim)
short	maxdim;
{
    /*
     *  Continuously fills the visible screen space with random rectangles
     *    drawn with different colors and stipple patterns.  The argument
     *    specifies an upper bound to the rectangle dimensions.
     */
    long	i, j, k;
    short	x1, x2, y1, y2;
    long	rand;
    long	color;
    long	color0, color1, color2, color3;
    short	cycle;
    long	stipple;
    long	mask;

    rand = 0;
    cycle = 0;
    mask = (~(0xffff << maxdim)) & 0xffff;

    i = 0x33333333;
    for (j=FONTADR0; j<FONTADR1; j++) {
	writefont (j, i);
	i = i >> 1;
	}
    i = 0x33333333;
    for (j=FONTADR1; j<FONTADR2; j++) {
	writefont (j, i);
	i = i << 1;
	if (i & 0x10000)
	    i |= 1;
	}
    for (j=FONTADR2; j<FONTADR3; j++) {
	writefont (j, 0xffff);
	}
    for (j=FONTADR3; j<FONTADR4; j++) {
	writefont (j, 0x3333);
	}	
	
    color0 = planecode (0, sigplanes, uc_cfb);
    color1 = planecode (1, sigplanes, uc_cfb);
    color2 = planecode (2, sigplanes, uc_cfb);
    color3 = planecode (3, sigplanes, uc_cfb);

    while (1) {
	breakcheck ();
	RANDOMIZE (rand);
	x1 = rand & 1023;
	y1 = (rand >> 10) & 1023;
	RANDOMIZE (rand);
	x2 = x1 + (rand & mask);
	y2 = y1 + ((rand >> 10) & mask);

	if (y1 > 820)
	    continue;
	if (y2 > 820)
	    y2 = 820;
	if (x2 > 1024)
	    x2 = 1024;

	switch (cycle & 03) {
	    case 0:
		color = color0;
		break;
	    case 1:
		color = color1;
		break;
	    case 2:
		color = color2;
		break;
	    case 3:
		color = color3;
		break;
	    }

	switch ((cycle >> 2) & 03) {
	    case 0:
		stipple = FONTADR0;
		break;
	    case 1:
		stipple = FONTADR1;
		break;
	    case 2:
		stipple = FONTADR2;
		break;
	    case 3:
		stipple = FONTADR3;
		break;
	    }
	setcodes (color, wecode);
	LDFMADDR (stipple)
	LDXS (x1)
	LDXE (x2)
	LDYS (y1)
	LDYE (y2)
	REQUEST (FILLRECT, 0)
	cycle = (cycle + 1) & 15;
	}
    }



randlines (seed, count, stipple)
long	seed, count, stipple;
{
    /*
     *	Generates <count> contiguous random lines with RNG <seed>.
     */

    long	rand;
    short	x1, y1, x2, y2;
    short	i, j, k;

    rand = seed;
    RANDOMIZE (rand);
    x2 = rand & 1023;
    y2 = (rand >> 10) & 1023;

    for (i=0; i < count; i++) {
	x1 = x2;
	y1 = y2;
	RANDOMIZE (rand);
	x2 = rand & 1023;
	y2 = (rand >> 10) & 1023;

	j = drawline (x1, y1, x2, y2, stipple);
	}
    }



