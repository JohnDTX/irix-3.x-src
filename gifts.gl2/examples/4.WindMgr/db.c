/*
 *	A double buffered window manager program
 *  	Draws a cube which is rotated by movements of the mouse.
 *
 */
#include "gl.h"
#include "device.h"

main()
{
	int x, y;		/*  current rotation of object	*/
	int active;		/*  TRUE if window is attached	*/
	short dev, val;

	keepaspect(3,2);
	getport("cube");
	doublebuffer();
	gconfig();
	qdevice(INPUTCHANGE);
	qdevice(REDRAW);
	qdevice(ESCKEY);
	qdevice(MOUSEX);
	qdevice(MOUSEY);
	perspective(400, 3.0/2.0, 0.001, 100000.0);
	translate(0.0, 0.0, -3.0);

	x = 0;  
	y = 0;
	active = 0;
	while(TRUE) {
		if(active) {
			frontbuffer(FALSE); /*  draw scene in back buffer */
			drawcube(x, y);
			swapbuffers();
		} else {
			frontbuffer(TRUE);  /*  draw scene in both buffers */
			drawcube(x, y);
			while (!qtest())
			    swapbuffers();  /*  swap buffers if not active */
		}
		while (qtest()) {	/*  process queued tokens */
			dev = qread(&val);
			switch(dev) {
			case ESCKEY:	/*  exit program with ESC */
				exit(0);
				break;
			case INPUTCHANGE:
				active = val;
				break;
			case REDRAW:
				reshapeviewport();
				break;
			case MOUSEX:
				x = val;
				break;
			case MOUSEY:
				y = val;
				break;
			default:
				break;
			}
		}
	}
}

drawcube(rotx,roty)
int rotx, roty;
{
	color(0);
	clear();
	color(7);
	pushmatrix();
	rotate(rotx,'x');
	rotate(roty,'y');
	cube();
	scale(0.3,0.3,0.3);
	cube();
	popmatrix();
}

cube() /* make a cube out of 4 squares */
{
	pushmatrix();
	side();
	rotate(900,'x');
	side();
	rotate(900,'x');
	side();
	rotate(900,'x');
	side();
	popmatrix();
}

side() /* make a square translated 0.5 in the z direction */
{
	pushmatrix();
	translate(0.0,0.0,0.5);
	rect(-0.5,-0.5,0.5,0.5);
	popmatrix();
}
