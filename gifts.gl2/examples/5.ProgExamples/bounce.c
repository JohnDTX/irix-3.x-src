/* bounce.c */
#include "gl.h"

#define XMIN 100	/* XMIN, ... define the region of the */
#define YMIN 100	/* screen where the ball bounces. */
#define XMAX 900
#define YMAX 700

main(argc, argv)
int argc;
char *argv[];
{
    long xpos = 500, ypos = 500;
    long xvelocity, yvelocity, radius;

    if (argc != 4) {
	printf("Usage: %s <xvelocity> <yvelocity> <radius>\\n", argv[0]);
	exit(0);
    }

    xvelocity = atoi(argv[1]);	/* convert the ascii values of the */
    yvelocity = atoi(argv[2]);	/* parameters to internal integer */
    radius = atoi(argv[3]);	/* format */
    if (radius <= 0)	/* sanity check */
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
    while(1) {
	color(BLACK);
	clear();
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
