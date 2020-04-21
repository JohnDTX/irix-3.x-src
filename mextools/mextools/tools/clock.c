/*
 *	clock - 
 *		A simple desk clock.
 *
 *				Paul Haeberli - 1984
 *
 */
#include <stdio.h>
#include <time.h>
#include "gl.h"
#include "device.h"
#include "port.h"
#include "rect.h"

int hours, minutes, seconds;
XY sechand[4]; 
XY minhand[4]; 
XY hourhand[4]; 
int menu;
int dodot;

main(argc,argv)
int argc;
char **argv;
{
    int i;
    short val;

/* get a port */ 

    if (argc>1)
	dodot = 1;
    keepaspect(1,1);
    winopen("clock");
    makeframe();
    menu = defpup("clock %t|calendar");
    noise(TIMER0,60);
    qdevice(TIMER0);
    qdevice(MENUBUTTON);

/* make the hands */

    hourhand[0] = minhand[0] = xy(-0.5,0.0);
    hourhand[1] = minhand[1] = xy(0.0,-1.5);
    hourhand[2] = minhand[2] = xy( 0.5,0.0);

    minhand[3] = xy(0.0,11.5);
    hourhand[3] = xy(0.0,7.0);

    sechand[0] = xy(-0.05,0.0);
    sechand[1] = xy(0.0,-2.0);
    sechand[2] = xy( 0.05,0.0);
    sechand[3] = xy(0.0,11.5);

/* draw the clock */

    makeface();
    while (1) 
	switch (qread(&val)) {
	    case REDRAW:
		makeframe();
		makeface();
		break;
	    case TIMER0:
		makeface();
		break;
	    case MENUBUTTON:
		if (val) {
		    unqdevice(TIMER0);
		    switch (dopup(menu)) {
			case 1:
			    dosystem("ical");
			    break;
		    }
		    makeface();
		    qdevice(TIMER0);
		}
		break;
	}
}

makeface()
{
    register int i;

    gettime(&hours,&minutes,&seconds);
    makeobj(10);
    pushmatrix();
	color(GREY(2));
	clear();
	scale(8.0,8.0,1.0);
	pushmatrix();
	    color(GREY(14));
	    for (i=0; i<12; i++) {
		if (i==0)
		    rectf(-0.5,9.0,0.5,11.0);
		else if (i==3 || i==6 || i== 9)
		    rectf(-0.5,9.5,0.5,10.5);
		else
		    rectf(-0.25,9.5,0.25,10.5);
		rotate(-300,'z');
	    }
	    pushmatrix();
		translate(0.60,-0.60,0.0);
		showhands(GREY(0),GREY(0));
		if (dodot) {
		    color(GREY(0));
		    circf(0.0,0.0,2.0);
		}
	    popmatrix();
	    showhands(GREY(6),GREY(15));
	    if (dodot) {
		color(GREEN);
		    circf(0.0,0.0,2.0);
	    }
	popmatrix();
    popmatrix();
    closeobj();
    callobj(10);
}

makeframe()
{
    int xsize, ysize;

    reshapeviewport();
    getsize(&xsize,&ysize);
    color(GREY(12));
    clear();
    viewport(2,xsize-2,5,ysize-2);
    color(GREY(0));
    clear();
    viewport(4,xsize-4,7,ysize-4);
    color(GREY(15));
    clear();
    ortho2(-100.0,100.0,-100.0,100.0);
}
		
showhands(fillcolor,bordercolor)
register int fillcolor, bordercolor;
{
    pushmatrix();
        rotate(-(hours*3600/12)-(minutes*5),'z');
        color(fillcolor); 
        polf2(4,hourhand);
        color(bordercolor);
        poly2(4,hourhand);
    popmatrix();

    pushmatrix();
        rotate(-(minutes*3600/60)-(seconds),'z');	
        color(fillcolor); 
        polf2(4,minhand);
        color(bordercolor);
        poly2(4,minhand);
    popmatrix();

    pushmatrix();
        rotate(-seconds*3600/60,'z');
        color(fillcolor); 
        polf2(4,sechand);
        color(bordercolor);
        poly2(4,sechand);
    popmatrix();

    pnt(0.0,0.0,0.0);
}

gettime( h, m, s )
int *h, *m, *s;
{
    long clock;
    struct tm *timeofday;

    time(&clock);
    timeofday = (struct tm *)localtime(&clock);
    *h = timeofday->tm_hour;
    *m = timeofday->tm_min;
    *s = timeofday->tm_sec;
}
