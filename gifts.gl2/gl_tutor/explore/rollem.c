

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
	keepaspect(1,1);
	winopen("rollem");

	doublebuffer();
	gconfig();
	frontbuffer(TRUE);
	color(BLACK);
	clear();
	frontbuffer(FALSE);
	
	ortho(-400.0, 400.0, -400.0, 400.0, -400.0, 400.0);
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
	rollem();

	swapbuffers();
}

rollem()
{
	rotate(150, 'x');
	scale(1.05, 1.05, 1.05);
	translate(0.0,-.25, 1.0);
 
	drawsphere();
}
	
drawsphere()
{
	int i;
	
	pushmatrix();

	for (i=0; i<=24; i=i+1)
	{		
		rotate(150,'y');
		circ(0.0, 0.0, 20.0);
	}

	pushmatrix();
	rotate(-150,'y');
	
	for (i=0; i<=24; i=i+1)
	{ 	
		rotate(150, 'x');
		circ(0.0, 0.0, 20.0);
	}

	popmatrix();	
	popmatrix();
}	
