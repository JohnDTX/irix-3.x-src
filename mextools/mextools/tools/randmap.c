/*
 *	randmap - 
 *		Randomize a piece of the color map.
 *
 *				Paul Haeberli - 1984
 *
 */
#include "gl.h"
#include "port.h"

main(argc,argv)
int argc;
char **argv;
{
    int seed;
    int start, finish;
    int planes;

    noport();
    winopen("randmap");
    planes = getplanes();

    srand(getpid());
    if (argc == 3) {
	start = atoi(argv[1]);
	finish = atoi(argv[2]);
	srand(getpid());
    } else if (argc == 4) {
	srand(atoi(argv[1]));
	start = atoi(argv[2]);
	finish = atoi(argv[3]);
    } else {
	if (planes <= 8) {
	   start = 16;
	   finish = 31;
	} else {
	   start = 128;
	   finish = 255;
	}
    }
    randmap(start,finish);
    gexit();
}

randmap(start,finish)
int start,finish;
{
    int i;
    int r, g, b;

    for (i=start; i<=finish; i++) {
	do {
	    r = rand() % 255;
	    g = rand() % 255;
	    b = rand() % 255;
	} while ((r+g+b) < 64);
	gammapcolor(i,r,g,b);
    }
}
