/*
 *	sunflower - 
 *		make a sunflower-like pattern out of circles.
 *
 *		try "% sunflower 40 0.05 1.1" and then try something else.	
 *
 *				Paul Haeberli - 1984
 *
 */
#include "gl.h"
#include "device.h"
#include "port.h"
#include "stdio.h"

float atof();

int seeds = 0;

main(argc,argv)
int argc;
char **argv;
{
    int nseeds;
    float seedsize, grow;
    short val;

    if (argc<4) {
	fprintf(stderr,"usage: sunflower <nseeds> <seedsize> <grow>\n");
	exit(1);
    }
    nseeds = atoi(argv[1]);
    seedsize = atof(argv[2]);
    grow = atof(argv[3]);
    winopen("sunflower");
    wintitle("sunflower");
    makeframe();
    sunflower(nseeds,seedsize,grow);
    while (1) {
	if (qread(&val) == REDRAW)  {
	    makeframe();
	    sunflower(nseeds,seedsize,grow);
  	}
    }
}

sunflower(nseeds,seedsize,grow)
int nseeds;
float seedsize, grow;
{
    float rad = 20.0;
    int parity = 0;

    scale(10.0,10.0,0.0);
    pushmatrix();
    while (rad < 100.0) {
	rotate(1800/nseeds,'z');
	scale(grow,grow,0.0);
	makering(nseeds,seedsize);
	rad = rad * grow;
    }
    popmatrix();
}

makering(nseeds,seedsize)
int nseeds;
float seedsize;
{
    int i;
    
    for (i=0; i<nseeds; i++) {
	pushmatrix();
	    rotate((i*3600)/nseeds,'z');
	    drawseed(seedsize);
	popmatrix();
    }
}

drawseed(seedsize)
float seedsize;
{
    seeds++;
    circ(1.0,0.0,seedsize); 
}

makeframe()
{
    int xsize, ysize;
    float aspect;

    reshapeviewport();
    getsize(&xsize,&ysize);
    color(GREY(14));
    clear();
    color(GREY(4));
    aspect = xsize/(float)ysize;
    ortho2(-50.0,50.0,-50.0/aspect,50.0/aspect);
}
