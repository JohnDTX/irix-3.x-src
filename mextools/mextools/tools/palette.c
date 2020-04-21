/*
 *	palette - 
 *		Display a pallete of colors.  A new palette may be selected 
 *		by pointing with the mouse and clicking the left mouse button.
 *
 *				Paul Haeberli - 1984
 *
 */
#include "gl.h"
#include "device.h"
#include "port.h"
#include "stdio.h"

int c1, c2;

main(argc,argv)
int argc;
char **argv;
{
    short dev, val;
    int one, two;

    c1 = 128;
    c2 = c1+127;
    winopen("palette");
    qdevice(LEFTMOUSE);
    drawit();
    while (1) {
	switch(qread(&val)) {
	    case REDRAW:
		reshapeviewport();
		drawit();
		break;
	    case LEFTMOUSE:
		if (!val) {
		    c1 = c2;
		    c2 = getapixel(getvaluator(MOUSEX),getvaluator(MOUSEY));
		}
		drawit();
		break;
	}
    }
}

drawit()
{
    register int i;

    if (c1<c2) {
	ortho2((float)c1,(float)c2+1,0.0,1.0);
	for (i=c1; i<=c2; i++) {
	    color(i);
	    rectfi(i,0,i+1,1);
	}
    } else {
	ortho2((float)c1+1,(float)c2,0.0,1.0);
	for (i=c2; i<=c1; i++) {
	    color(i);
	    rectfi(i,0,i+1,1);
	}
    }
}
