#include "gl.h"
#include "device.h"

main()
{
	initialize();
	drawimage();
	while(TRUE) {
		while (!qtest())
			drawimage();
		processinput();
	}
}


initialize()
{
	winopen("matrix");

	doublebuffer();
	gconfig();
	frontbuffer(TRUE);
	color(BLACK);
	clear();
	frontbuffer(FALSE);

	ortho(-100.0, 100.0, -100.0, 100.0, -100.0, 100.0);
}

processinput()
{
	short val;

	switch(qread(&val)) {
		case REDRAW:
			reshapeviewport();
			drawimage();
			break;
	}
}

drawimage()
{
	color(BLACK);
	clear();

	drawobjects();

	swapbuffers();
}

drawobjects()
{

	color(RED);	

	drawone();

	drawtwo();

	drawthree();

	drawfour();

	drawfive();
	
}

drawone()
{
	arcf(10.0, 30.0, 10.0, 0, 300);
}

drawtwo()
{
	rectf(10.0, 10.0, 20.0, 20.0);
}

drawthree()
{
	rectf(-10.0, -50.0, -5.0, -20.0);
}

drawfour()
{
	rect(30.0, 30.0, 40.0, 40.0);
}

drawfive()
{
	rect(-10.0, -10.0, 10.0, 10.0); 
}

