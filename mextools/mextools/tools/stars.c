/*
 *  	stars - 
 *		Create a random field of stars in threespace and move
 *	 	them around with the mouse.
 *
 *				Paul Haeberli - 1984
 *
 */
#include "gl.h"
#include "device.h"
#include "port.h"

main()
{
    int xsize, ysize;
    int i;
    short dev, val;

    keepaspect(3,2);
    winopen("stars");
    wintitle("stars");

    perspective(400,3.0/2.0,0.001,100000.0);
    translate(0.0,0.0,-4.0);
    makeobj(1);
	drawstars(-1.0,1.0,-1.0,1.0,-1.0,1.0,100);
	drawstars(-0.5,0.5,-0.5,0.5,-0.5,0.5,400);
	drawcube();
	pushmatrix();
	    scale(0.1,0.1,0.1);
	    drawcube();
	popmatrix();
    closeobj();
    curson();
    while (1) {
	    redraw(0);
	    pushmatrix();
		rotate((getvaluator(MOUSEX)-XMAXSCREEN/2),'y');
		rotate(-(getvaluator(MOUSEY)-YMAXSCREEN/2),'x');
		color(GREY(0));
		clear();
		callobj(1);
		gsync();
		gsync();
		gsync();
	    popmatrix();
    }
}

int drawstars( xmin, xmax, ymin, ymax, zmin, zmax, nstars )
float xmin, xmax, ymin, ymax, zmin, zmax;
int nstars;
{
    float xrange, yrange, zrange;
    float x, y, z;
    int i;

    srand(getpid());
    xrange = (xmax-xmin)/1000;
    yrange = (ymax-ymin)/1000;
    zrange = (zmax-zmin)/1000;
    for (i = 1; i < nstars; i++) {
        x = xmin+xrange*(rand()%1000);
        y = ymin+yrange*(rand()%1000);
        z = zmin+zrange*(rand()%1000);
	if (i%16 == 0)
	    color(GREY(8+(rand()%8)));
        pnt(x,y,z);
    }
}

drawcube()
{
    int i;

    color(GREY(15));
    pushmatrix();
    for (i=0; i<4; i++) {
	rotate(900,'y');
	pushmatrix();
	    translate(0.0,0.0,0.5);
	    rect(-0.5,-0.5,0.5,0.5);
	popmatrix();
    }
    popmatrix();
}
