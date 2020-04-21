#include "gl.h"

main()
{
	ginit();
	color(BLUE);
	recti(0, 0, 100, 100);
	color(RED);
	circi(50, 50, 50);
	gexit();
}
