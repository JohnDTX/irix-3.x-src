#include "gl.h"

main()
{
	ginit();
	RGBmode();
	gconfig();
	RGBcolor(0,0,0);
	clear();
	RGBcolor(0, 0, 225);
	recti(0, 0, 6, 6);
	RGBcolor(225, 0, 0);
	circi(3, 3, 2);
	gexit();
}
