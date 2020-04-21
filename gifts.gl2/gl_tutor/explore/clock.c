/*
 *	clock - a simple desk clock.
 *
 *			Paul Haeberli - 1984
 *
 */
#define SYSTEM5		/* this is system 5 no? */

#include <stdio.h>
#ifdef UNIX4_2
#include <sys/time.h>
#endif
#ifdef SYSTEM5
#include <time.h>
#endif
#include "gl.h"
#include "device.h"

typedef struct
{
    float x;
    float y;
} XY;

XY xy(x,y)
float x, y;
{
    XY temp;

    temp.x = x;
    temp.y = y;
    return temp;
}

XY sechand[4]; 
XY minhand[4]; 
XY hourhand[4]; 
int hours, minutes, seconds;
int nhours, nminutes, nseconds;

#define FACE	101
#define SEC	102
#define MINU	103
#define HOUR	104

int makeframe();

main(argc,argv)
int argc;
char **argv;
{
    int i;

/*    keepaspect(1, 1);*/
    winopen("clock");
    makeframe();

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
	
    makeface();

/* draw the clock */

    while(1) {
	redraw(makeframe);
	gettime(&nhours,&nminutes,&nseconds);
	if (nseconds != seconds || nminutes != minutes || nhours != hours ) {
	    seconds = nseconds;
	    minutes = nminutes;
	    hours = nhours;

	    makeobj(10);
            pushmatrix();
		color(BLACK);
                clear();
	        scale(8.0,8.0,1.0);
	        pushmatrix();
		    callobj(FACE);
		    pushmatrix();
			translate(-0.35,-0.35,0.0);
			showhands(RED, RED);
		    popmatrix();
	            showhands(YELLOW, GREEN);
	        popmatrix();
            popmatrix();
	    closeobj();
	    callobj(10);
	    gflush();
	}
	sleep(1);
    }
}

makeframe()
{
    int xsize, ysize;

    getsize(&xsize,&ysize);
    viewport(0, xsize, 0, ysize);
    color(GREEN);
    clear();
    viewport(2,xsize-2,5,ysize-2);
    color(YELLOW);
    clear();
    viewport(4,xsize-4,7,ysize-4);
    color(RED);
    clear();
    ortho2(-100.0,100.0,-100.0,100.0);
}
		
showhands(fillcolor,bordercolor)
int fillcolor, bordercolor;
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

makeface()
{
    int i;

    makeobj(FACE);
	color(YELLOW);
	for(i=0; i<12; i++) {
	    if( i==0 )
		rectf(-0.5,9.0,0.5,11.0);
	    else if( i==3 || i==6 || i== 9 )
		rectf(-0.5,9.5,0.5,10.5);
	    else
		rectf(-0.25,9.5,0.25,10.5);
	    rotate(-300,'z');
	}
    closeobj();
}


/*
 * redraw:
 *	 - test for a redraw token in the queue, and if present, swallow it
 *	   and call the optionally supplied user function as well as the
 *	   code to reshape the viewport based on the new size
 */
redraw(f)
    int (*f)();
{
    short v;

    if (qtest() == REDRAW) {
	(void) qread(&v);
	reshapeviewport();
	if (f)
		(*f)();
	return 1;
    }
    return 0;
}
