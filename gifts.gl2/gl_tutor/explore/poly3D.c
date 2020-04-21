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
	winopen("c1");

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

	color(RED);
	drawpoly();

	swapbuffers();

}

drawpoly()
{
	
	{
	Coord parray [4][3];
	
	parray [0][0] = -10.0;
	parray [0][1] = -10.0;
	parray [0][2] = -10.0;
	parray [1][0] = 10.0;
	parray [1][1] = -10.0;
	parray [1][2] = -10.0;
	parray [2][0] = 10.0;
	parray [2][1] = 10.0;
	parray [2][2] = -10.0;
	parray [3][0] = -10.0;
	parray [3][1] = 10.0;
	parray [3][2] = -10.0;
	
	poly(4, parray);
	}
	
	{	
	Coord parray [4][3];
	
	parray [0][0] = -10.0;
	parray [0][1] = -10.0;
	parray [0][2] = -10.0;
	parray [1][0] = -10.0;
	parray [1][1] = -10.0;
	parray [1][2] = 10.0;
	parray [2][0] = 10.0;
	parray [2][1] = -10.0;
	parray [2][2] = 10.0;
	parray [3][0] = 10.0;
	parray [3][1] = -10.0;
	parray [3][2] = -10.0;

	poly(4, parray);
	}

	{
	Coord parray[4][3];
	
	parray [0][0] = -10.0;
	parray [0][1] = -10.0;
	parray [0][2] = 10.0;
	parray [1][0] = 10.0;
	parray [1][1] = -10.0;
	parray [1][2] = 10.0;
	parray [2][0] = 10.0;
	parray [2][1] = 10.0;
	parray [2][2] = 10.0;
	parray [3][0] = -10.0;
	parray [3][1] = 10.0;
	parray [3][2] = 10.0;

	poly(4, parray);
	}

	{
	Coord parray[4][3];
	
	parray [0][0] = -10.0;
	parray [0][1] = 10.0;
	parray [0][2] = -10.0;
	parray [1][0] = -10.0;
	parray [1][1] = 10.0;
	parray [1][2] = 10.0;
	parray [2][0] = 10.0;
	parray [2][1] = 10.0;
	parray [2][2] = 10.0;
	parray [3][0] = 10.0;
	parray [3][1] = 10.0;
	parray [3][2] = -10.0;

	poly(4, parray);
	}
}
