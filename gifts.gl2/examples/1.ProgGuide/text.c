#include "gl.h"

main()
{
	ginit();
	cursoff();
	color(BLACK);
	clear();
	color(RED);
	cmov2i(300,380);
	charstr("The first line is drawn ");
	charstr("in two parts. ");
	cmov2i(300, 368);
	charstr("This line is 12 pixels lower. ");
	sleep(5);
	curson();
	gexit();
}
