/*
 *	showramp -
 *		Show a ramp of colors.
 *
 *
 */
#include "gl.h"
#include "device.h"

float parray0[][2] = {
    { 10.0, 0.0 },
    { 0.0, 0.0 },
    { 0.0, 2.5 },
    { 10.0, 2.5 }
};

float parray1[][2] = {
    { 0.0, 2.5 },
    { 10.0, 2.5 },
    { 10.0, 7.5 },
    { 0.0, 7.5 }
};

float parray2[][2] = {
    { 0.0, 10.0 },
    { 10.0, 10.0 },
    { 10.0, 7.5 },
    { 0.0, 7.5 }
};

short intens0[] = { 128, 255, 255, 128 };
short intens1[] = { 128, 255, 128, 255 };

main()
{

    int dev, val;
    char selectfirst = TRUE;
    short mousex, mousey;

    winopen("showramp");
    color(BLACK);
    clear();

    ortho2(-0.5, 10.5, -0.5, 10.5);

    qdevice(REDRAW);
    qdevice(LEFTMOUSE);
    tie(LEFTMOUSE, MOUSEX, MOUSEY);
    qdevice(RIGHTMOUSE);

    drawblocks();
    while (1) 
	switch (dev = qread(&val)) {
	    case REDRAW :
		reshapeviewport();
		color(BLACK);
		clear();
		drawblocks();
		break;
	    case LEFTMOUSE :
		dev = qread(&mousex);
		dev = qread(&mousey);
		if (val) {
		    if (selectfirst)
			intens1[0] = intens1[2] = 
			    intens0[3] = intens0[0] = getapixel(mousex, mousey);
		    else
			intens1[1] = intens1[3] = 
			    intens0[2] = intens0[1] = getapixel(mousex, mousey);
		    selectfirst = !selectfirst;
		    drawblocks();
		}
		break;
	}
}

drawblocks()
{

    splf2(4, parray0, intens0);
    splf2(4, parray2, intens0);
    splf2(4, parray1, intens1);

    linewidth(3);
    color(BLACK);
    rect(0.0, 0.0, 10.0, 10.0);
    move2(0.0, 2.5);
    draw2(10.0, 2.5);
    move2(0.0, 7.5);
    draw2(10.0, 7.5);

    linewidth(1);
    color(WHITE);
    rect(0.0, 0.0, 10.0, 10.0);
    move2(0.0, 2.5);
    draw2(10.0, 2.5);
    move2(0.0, 7.5);
    draw2(10.0, 7.5);
}
