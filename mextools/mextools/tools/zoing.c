/*
 *	zoing - 
 *		Make a kind of wacky spiral out of circles.
 *
 *				Paul Haeberli - 1984
 *
 */
#include "port.h"
#include "device.h"
#include "gl.h"

main()
{
    short val;

    keepaspect(1,1);
    winopen("zoing");
    drawit();
    while(1) {
	if(qread(&val) == REDRAW)
	    drawit();
    }
}

drawit()
{
    register int i;

    reshapeviewport();
    color(GREY(15));
    clear();
    ortho2(-1.0,1.0,-1.0,1.0);
    color(GREY(0));
    translate(-0.1,0.0,0.0);
    pushmatrix();
	for(i=0; i<200; i++)  {
	    rotate(170,'z');
	    scale(0.96,0.96,0.0);
	    pushmatrix();
		translate(0.10,0.0,0.0);
		circ(0.0,0.0,1.0);
	    popmatrix();
	}
    popmatrix();
}
