#include "gl.h"

main()
{
	ginit();
	cursoff();
	color(BLACK);
	clear();
	color(BLUE);

	move2i(200,200);
	draw2i(200,300);
	draw2i(300,300);
	draw2i(300,200);
	draw2i(200,200);
	sleep(3);
	gexit();
}
