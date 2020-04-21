/*
 *	mmon - 
 *		Monitor the buttons of the mouse.  This is useful for
 *		demos sometimes.
 *
 *				Paul Haeberli - 1985
 *
 */
#include "gl.h"
#include "device.h"
#include "port.h"

#define HWIDTH 0.6

int om1, om2, om3;
int m1, m2, m3;

main()
{
    short val;
  
    keepaspect(2,3);
    winopen("mmon");

    m1 = getbutton(MOUSE1);
    m2 = getbutton(MOUSE2);
    m3 = getbutton(MOUSE3);
    qdevice(TIMER0);
    noise(TIMER0,5);
    makeframe();
    while (1) {
	switch(qread(&val)) {
	    case TIMER0:
		drawit();
		break;
	    case REDRAW:
		makeframe();
		break;
 	}	
    }
}

drawit()
{
    m1 = getbutton(MOUSE1);
    m2 = getbutton(MOUSE2);
    m3 = getbutton(MOUSE3);
    if (m3!=om3) {
	drawent(-2.1,m3);	
	om3 = m3;
    }
    if (m2!=om2) {
	drawent(0.0,m2);	
	om2 = m2;
    }
    if (m1!=om1) {
	drawent(2.1,m1);	
	om1 = m1;
    }
}

drawent(x,val)
float x;
int val;
{
    if (val) {
	color(GREY(0));
	rectf(x-HWIDTH,0.6,x+HWIDTH,4.5);
    } else {
	color(GREY(10));
	rectf(x-HWIDTH,0.6,x+HWIDTH,4.5);
	color(GREY(0));
	rect(x-HWIDTH,0.6,x+HWIDTH,4.5);
    }
}

makeframe()
{
    reshapeviewport();
    ortho2(-4.0,4.0,-6.0,6.0);
    color(GREY(10));
    rectf(-4.0,-6.0,4.0,6.0);
    om1 = om2 = om3 = -1;
    drawit();
}
