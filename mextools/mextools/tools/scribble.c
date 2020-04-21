/*
 *	scribble - 
 *		Draw anywhere on the screen. This goes into full screen mode.
 *
 *				Paul Haeberli - 1986
 *
 */
#include "image.h"
#include "gl.h"
#include "device.h"
#include "port.h"

main()
{
    register int i;
    short val;
    int x, y;

    prefposition(4,20,4,20);
    winopen("scribble");
    fullscrn();
    qdevice(LEFTMOUSE);
    qdevice(MIDDLEMOUSE);
    wholescreen();
    color(0);
    while(1) {
	switch(qread(&val)) {
	    case LEFTMOUSE:
		if(val) {
		    while(getbutton(LEFTMOUSE)) { 
			x = getvaluator(MOUSEX);
			y = getvaluator(MOUSEY);
			circfi(x,y,6); 
		    } 
		}
		break;
	    case MIDDLEMOUSE:
		if(val) {
		    x = getvaluator(MOUSEX);
		    y = getvaluator(MOUSEY);
		    color(getapixel(x,y));
		}
		break;
	}
    }
}

wholescreen()
{
    int xsize, ysize;

    getsize(&xsize,&ysize);
    viewport(0,xsize-1,0,ysize-1);
    ortho2(-0.5,xsize-0.5,-0.5,ysize-0.5);
}
