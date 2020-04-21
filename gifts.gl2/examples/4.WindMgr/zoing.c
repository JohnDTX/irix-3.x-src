/*
 *	zoing - make a spiral out of circles
 *
 */
#include "device.h"
#include "gl.h"

main()
{
	short val;

	keepaspect(1,1);  /* the graphics port can be any location and
						size, as long as it's square */
	getport("zoing");
	drawit();	  /* image drawn the first time */
	/* the image is redrawn whenever a REDRAW appears in the event queue */
	while(1) {
		if(qread(&val) == REDRAW)
			drawit();
	}
}

drawit()
{
	register int i;

	reshapeviewport();
	color(7);
	clear();
	ortho2(-1.0,1.0,-1.0,1.0);
	color(0);
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
