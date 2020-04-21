/*
 *  	A 3-D field of randomly placed stars
 *	When the program is attached, and the left mouse button
 *	is pressed, the xy movement of the mouse rotates space.
 *	Paul Haeberli - 1984  /  modified by Mason Woo - 1985
 */
#include "gl.h"
#include "device.h"

#define	SPACEOBJ	1
#define	STARCOLOR	8

main()
{
    short x, y, oldx, oldy;
    short dx, dy;		/*  current rotation of object	*/
    short attached;		/*  TRUE if window is attached	*/
    short dev, val;

    keepaspect(3,2);
    winopen("stars");

    qdevice(INPUTCHANGE);
    qdevice(REDRAW);
    qdevice(ESCKEY);

    mapcolor(STARCOLOR, 80, 175, 175);  /*  make color of the stars	*/

    doublebuffer();
    gconfig();
    perspective(400, 3.0/2.0, 0.001, 100000.0);
    translate(0.0, 0.0, -4.0);

    makeobj(SPACEOBJ);
	drawstars(-1.0, 1.0, 100);	/*  draw 100 stars in space	*/
	drawstars(-0.5, 0.5, 400);	/*  draw 400 stars in inner cube*/
	drawcube();			/*  draw outer cube		*/
	pushmatrix();
	    scale(0.1, 0.1, 0.1);	/*  draw inner cube		*/
	    drawcube();
	popmatrix();
    closeobj();

    dx = 0;  dy = 0;
    frontbuffer(TRUE);			/*  draw initial scene		*/
    drawscene(dx, dy);
    frontbuffer(FALSE);

    qenter(INPUTCHANGE, 0);		/*  initial state is unattached */
    while(TRUE) {
	while (qtest()) {	/*  process queued tokens	*/
	    dev = qread(&val);
	    switch(dev) {
	    case ESCKEY:	/*  exit program with ESC	*/
		exit(0);
		break;
	    case INPUTCHANGE:
		attached = val;
		if (!attached) {
		    frontbuffer(TRUE);
		    drawscene(dx, dy);
		    frontbuffer(FALSE);
		}
		break;
	    case REDRAW:
		reshapeviewport();
		frontbuffer(TRUE);
		drawscene(dx, dy);
		frontbuffer(FALSE);
		break;
	    default:
		break;
	    }	/*  end switch(dev)  */
	    if (!attached)	/*  swap buffers if not attached	*/
		while (!qtest())
		    swapbuffers();
	}  /*  end while (qtest())  */

	if (attached) {		/*  process polled devices	*/
	    oldx = x; oldy = y;
	    x = (short) getvaluator(MOUSEX);
	    y = (short) getvaluator(MOUSEY);
	    if (getbutton(LEFTMOUSE)) {	
	/*  if button pressed, change rotation values	*/
	/*  when you get to 3600, start back at 0	*/
		dx = (dx + (x - oldx)) % 3600;
		dy = (dy + (y - oldy)) % 3600;
	    }
	    drawscene(dx, dy);
	}  /*  end if (attached)  */
	swapbuffers();
    }  /*  end while (TRUE)  */
}

/*  drawscene -- draw rotated star field	*/
drawscene(dx, dy)
short dx, dy;
{
    pushmatrix();
	rotate(dx, 'y');
	rotate(-dy, 'x');
	color(BLACK);
	clear();
	callobj(SPACEOBJ);
    popmatrix();
}

/*  drawstars -- randomly make a given number (nstars) of points in space.
    The points should all fall within a cube centered around the origin.
    The minimum and maximum values are the limiting coordinates.
*/
drawstars( minimum, maximum, nstars )
float minimum, maximum;
int nstars;
{
    float range;
    float x, y, z;
    int i;

    color(STARCOLOR);
    srand(getpid());
    range = (maximum-minimum)/1000;
    for( i = 1; i < nstars; i++ ) {
        x = minimum + range * (rand()%1000);
        y = minimum + range * (rand()%1000);
        z = minimum + range * (rand()%1000);
        pnt(x,y,z);
    }
}

drawcube()
{
    color(WHITE);
    move(-0.5, -0.5, -0.5);
    draw(0.5, -0.5, -0.5);
    draw(0.5, 0.5, -0.5);
    draw(-0.5, 0.5, -0.5);
    draw(-0.5, -0.5, -0.5);
    draw(-0.5, -0.5, 0.5);
    move(-0.5, 0.5, -0.5);
    draw(-0.5, 0.5, 0.5);
    move(0.5, -0.5, -0.5);
    draw(0.5, -0.5, 0.5);
    move(0.5, 0.5, -0.5);
    draw(0.5, 0.5, 0.5);
    move(-0.5, -0.5, 0.5);
    draw(-0.5, 0.5, 0.5);
    draw(0.5, 0.5, 0.5);
    draw(0.5, -0.5, 0.5);
    draw(-0.5, -0.5, 0.5);
}
