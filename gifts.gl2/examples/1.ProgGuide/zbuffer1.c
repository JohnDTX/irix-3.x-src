#include "gl.h"

main ()
{
	ginit();
	color(BLACK);
	clear();
	ortho(0.0, (float)XMAXSCREEN, 0.0, (float)YMAXSCREEN, 
	    0.0, -(float)XMAXSCREEN);

	setdepth(0xC000,0x3FFF);  /* the minimum and maximum z values are set */

	zbuffer(TRUE);  /* the IRIS enters zbuffering mode */
	zclear();  	/* the zbuffer is cleared to the maximum z value */

	color(YELLOW);
	pmv(0.0, 0.0, 100.0);
	pdr(100.0, 0.0, 100.0);
	pdr(100.0, 100.0, 100.0);
	pdr(0.0, 100.0, 100.0);
	pclos();

	color(RED);
	pmv(0.0, 0.0, 50.0);
	pdr(100.0, 0.0, 50.0);
	pdr(100.0, 100.0, 200.0);
	pdr(0.0, 100.0, 200.0);
	pclos();

	zbuffer(FALSE);  /* the IRIS exits zbuffering mode */
	gexit();
}
