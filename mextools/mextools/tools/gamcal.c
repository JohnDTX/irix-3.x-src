/*
 *	gamcal -
 *		See if the gamma correction value is correct by comparing
 *		the intensity of a full on, full off checker board to a
 *		50 percent grey.
 *
 *				Paul Haeberli - 1985
 *
 */
#include "port.h"

short tex1[] = {
    0xcccc, 0x3333, 0xcccc, 0x3333,
    0xcccc, 0x3333, 0xcccc, 0x3333,
    0xcccc, 0x3333, 0xcccc, 0x3333,
    0xcccc, 0x3333, 0xcccc, 0x3333,
};

short tex[] = {
    0xcccc, 0xcccc, 0x3333, 0x3333,
    0xcccc, 0xcccc, 0x3333, 0x3333,
    0xcccc, 0xcccc, 0x3333, 0x3333,
    0xcccc, 0xcccc, 0x3333, 0x3333,
};

main()
{
    short val;

    getport("gamcal");
    defpattern(2,16,tex);
    drawit();
    while(1) {
	switch(qread(&val)) {
	    case REDRAW:
		drawit();
		break;
	}
    }
}

drawit()
{
    int i;

    reshapeviewport();
    ortho2(0.0,16.0,0.0,16.0);
    for (i=0; i<8; i++) {
	setpattern(0);
	color(GREY(0));
	rectf(0.0,0.0,16.0,1.0);

	setpattern(2);
	color(GREY(15));
	rectf(0.0,0.0,16.0,1.0);
	translate(0.0,1.0,0.0);

	setpattern(0);
	color(GREY(7));
	rectf(0.0,0.0,16.0,1.0);
	translate(0.0,1.0,0.0);
    }
}
