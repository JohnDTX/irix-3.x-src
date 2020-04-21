/* "iobounce.c" */
#include "gl.h"
#include "device.h"

#define XMIN 100
#define YMIN 100
#define XMAX 900
#define YMAX 700

long xvelocity, yvelocity;

main(argc, argv)
int argc;
char *argv[];
{
    long xpos = 500, ypos = 500;
    long radius;

    xvelocity = yvelocity = 0;
    radius = 10;

    ginit();
    doublebuffer();
    gconfig();
    color(WHITE);
    frontbuffer(TRUE);
    clear();
    frontbuffer(FALSE);
    viewport(XMIN, XMAX, YMIN, YMAX);
    ortho2(XMIN - 0.5, XMAX + 0.5, YMIN - 0.5, YMAX + 0.5);

    qdevice(LEFTMOUSE);
    qdevice(MIDDLEMOUSE);
    qdevice(RIGHTMOUSE);

    while(1) {
	color(BLACK);
	clear();
	checkmouse();
	xpos += xvelocity;
	ypos += yvelocity;
	if (xpos > XMAX - radius ||
	    xpos < XMIN + radius) {
	    xpos -= xvelocity;
	    xvelocity = -xvelocity;
	}
	if (ypos > YMAX - radius ||
	    ypos < YMIN + radius) {
	    ypos -= yvelocity;
	    yvelocity = -yvelocity;
	}
	color(YELLOW);
	circfi(xpos, ypos, radius);
	swapbuffers();
    }
}

#define LEFT 1
#define MIDDLE 2
#define RIGHT 4

checkmouse()
{
    static buttons = 0, pressed = 0;
    Device val;

    while (qtest()) {
	switch (qread(&val)) {
	    case LEFTMOUSE:
		buttons ^= LEFT;
		pressed |= LEFT;
		break;
	    case MIDDLEMOUSE:
		buttons ^= MIDDLE;
		pressed |= MIDDLE;
		break;
	    case RIGHTMOUSE:
		buttons ^= RIGHT;
		pressed |= RIGHT;
		break;
	}
	if (buttons == 0) {
	    switch (pressed) {
		case LEFT:		/* increase xvelocity */
		    if (xvelocity >= 0)
			xvelocity++;
		    else
			xvelocity--;
		    break;
		case MIDDLE:	/* increase yvelocity */
		    if (yvelocity >= 0)
			yvelocity++;
		    else
			yvelocity--;
		    break;
		case RIGHT: 	/* stop ball */
		    xvelocity = yvelocity = 0;
		    break;
		case LEFT+RIGHT:	/* exit */
		    gexit();
		    exit(0);
	    }
	pressed = 0;
	}
    }
}
