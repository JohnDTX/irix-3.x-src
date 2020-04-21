/* "doily.c" */
#include "gl.h"
#include <math.h>

#define PI 3.1415926535

main(argc, argv)
int argc;
char *argv[];
{
    long numpts, i, j;
    float points[100][2];

    /* First figure out how many points there are. */

    if (argc != 2) {
	printf("Usage: %s <point count>\\n", argv[0]);
	exit(0);
    }

    numpts = atoi(argv[1]);	/* convert argument to internal format */

    if (numpts > 100) {
	printf("Too many points\\n");
	exit(0);
    }

    if (numpts < 3) {
	printf("Too few points\\n");
	exit(0);
    }
    /* Now get the x and y coordinates of numpts equally-
     * spaced points around the unit circle.
     */

    for (i = 0; i < numpts; i++) {
	points[i][0] = cos((i*2.0*PI)/numpts);
	points[i][1] = sin((i*2.0*PI)/numpts);
    }

    ginit();
    textport(30,200,30,200);
    cursoff();

    color(WHITE);
    clear();		/* clear the whole screen */

    viewport(200, 800, 100, 700);   /* restrict to a square viewport */
    color(BLACK);
    clear();
    color(RED);

    ortho2(-1.2, 1.2, -1.2, 1.2);

    for (i = 0; i < numpts; i++)
        for (j = i+1; j < numpts; j++) {
	    move2(points[i][0], points[i][1]);
	    draw2(points[j][0], points[j][1]);
	}

    gexit();
}
