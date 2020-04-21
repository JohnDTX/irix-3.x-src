/*
 *	mouse - 
 *		A mouse motion tester.  See if the mouse buttons work too!!
 *
 *				Paul Haeberli - 1984
 *
 */
#include "gl.h"
#include "device.h"
#include "port.h"

Matrix C_spline = {
    {-0.5, 1.5, -1.5, 0.5},
    {1.0, -2.5, 2.0, -0.5},
    {-0.5, 0.0, 0.5, 0.0},
    {0.0, 1.0, 0.0, 0.0},
};

#define MOUSE_OBJ 	1
#define LEN		300.0

main()
{
    int i;
    float sc;
    short butval;
    int button;
    short dev;
    short val;
    float x, y, ox, oy;
    int m1, m2, m3;
    int om1, om2, om3;
    float xoff, yoff;
    float geom[4][3];
  
    keepaspect(1,1);
    winopen("mouse");

    makeframe();
    makeobj(MOUSE_OBJ);
	color(GREY(15));
	rectf(-4.0,-6.0,4.0,6.0);
	color(GREY(0));
	rect(-4.0,-6.0,4.0,6.0);
	rect(-2.75,0.0,-1.25,4.5);
	rect(-0.75,0.0, 0.75,4.5);
	rect( 1.25,0.0, 2.75,4.5);
    closeobj();

    qdevice(MOUSE1);
    qdevice(MOUSE2);
    qdevice(MOUSE3);
    qdevice(MOUSEX);
    qdevice(MOUSEY);

    om1 = om2 = om3 = -1;
    ox = oy = -1.0;
    xoff = yoff = 0.0;
    m1 = getbutton(MOUSE1);
    m2 = getbutton(MOUSE2);
    m3 = getbutton(MOUSE3);
    x = getvaluator(MOUSEX);
    y = getvaluator(MOUSEY);
    geom[0][0] = 0.0;
    geom[0][1] = yoff+6.0-LEN;
    geom[0][2] = 0.0; 
    geom[1][0] = xoff;
    geom[1][1] = yoff+6.0; 
    geom[1][2] = 0.0; 
    geom[2][0] = 0.0;
    geom[2][1] = 110.0;
    geom[2][2] = 0.0; 
    geom[3][0] = xoff;
    geom[3][1] = 110.0+LEN;
    geom[3][2] = 0.0; 
    defbasis(100,C_spline);
    curveprecision(20);
    curvebasis(100);
    while (1) {
	if (x != ox || y != oy || m1 != om1 || m2 != om2 || m3 != om3) {
	    xoff = x*(100.0)/1024.0;
	    yoff = y*(100.0)/767.0;
	    color(GREY(10));
	    clear();
	    color(GREY(0));
	    geom[0][1] = yoff+6.0-LEN;
	    geom[1][0] = xoff;
	    geom[1][1] = yoff+6.0; 
	    geom[3][0] = xoff;
	    crv(geom);
	    pushmatrix();
	        translate(x*(100.0/1024.0),y*(100.0/767.0),0.0);
	        callobj(MOUSE_OBJ);
		if (m3)
		    rectf(-2.75,0.0,-1.25,4.5);
		if (m2)
		    rectf(-0.75,0.0, 0.75,4.5);
		if (m1)
		    rectf( 1.25,0.0, 2.75,4.5);
	    popmatrix();
	    ox = x; oy = y;
	    ox = x; oy = y;
	    om1 = m1; om2 = m2; om3 = m3; 
	}
	do { 
	    dev = qread(&val);
	    switch (dev) {
		case MOUSE1: m1 = val;
			     break;
		case MOUSE2: m2 = val;
			     break;
		case MOUSE3: m3 = val;
			     break;
		case MOUSEX: x = val;
			     break;
		case MOUSEY: y = val;
			     break;
		case REDRAW: reshapeviewport();
			     makeframe();
			     ox = -1.0;
			     break;
  	    }
	} while (qtest());
    }
}

makeframe()
{
    ortho2(-10.0,110.0,-10.0,110.0);
    color(GREY(0));
}
