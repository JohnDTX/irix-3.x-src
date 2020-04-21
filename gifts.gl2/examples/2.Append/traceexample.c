#include "gl.h"

#define BUFFSIZE 400

main()
{
	char 	zflag, depthflag;
	short	buffer[BUFFSIZE];
	short	count;

	noport();
	foreground();
	getport();

	zflag = getzbuffer();
	depthflag = getdcm();
	feedback(buffer, BUFFSIZE);

	color(BLACK);
	clear();
	color(RED);
	circi(200, 200, 200);
	color(GREEN);
	pnt2i(200, 200);
	color(BLUE);
	recti(0, 0, 400, 400);

	count = endfeedback(buffer);

	setfbdebugging(TRUE);

	if (parsefb(buffer, count, zflag, depthflag))
		printf("no errors in parse\n");
	else
		printf("error while parsing\n");
	gexit();
}
