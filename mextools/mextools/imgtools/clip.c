/*
 * 	clip -	
 *		Save a part of the screen in a window.
 *
 *				Paul Haeberli - 1986
 */
#include "gl.h"
#include "device.h"

int xorg, yorg;
int xsize, ysize;
short **img;

main()
{
    short val;
    register int y;

    winopen("clip");
    getsize(&xsize,&ysize);
    img = (short **)malloc(ysize*sizeof(short *));
    for(y=0; y<ysize; y++) {
    	percentdone((100.0*y)/ysize);
	img[y] = (short *)malloc(xsize*sizeof(short));
	cmov2i(0,y);
	readpixels(xsize,img[y]);
    }
    percentdone(100.0);
    prefsize(xsize,ysize);
    winconstraints();
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
    register int y;

    for(y=0; y<ysize; y++) {
	cmov2i(0,y);
	writepixels(xsize,img[y]);
    }
}
