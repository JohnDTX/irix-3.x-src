#include "gl.h"
#include "gl2cmds.h"
#include "stdio.h"

#define BUFFSIZE 100

int plotpoint();
int plotmove();
int plotdraw();

main()
{
	char	zflag, depthflag;
	short	buffer[BUFFSIZE];
	short	count;

	foreground();
	noport();
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

	bindfbfunc(FBCpoint, plotpoint);
	bindfbfunc(FBCmove, plotmove);
	bindfbfunc(FBCdraw, plotdraw);

	openpl();
	if (parsefb(buffer, count, zflag, depthflag))
		fprintf(stderr,"no errors in parse\n");
	else
		fprintf(stderr,"error while parsing\n");
	closepl();
	gexit();
}

int	lastx, lasty;

plotpoint(count, command, string)
int	count;
int	command;
char	*string;
{
	int i, x, y;

	x = getfbword();
	y = getfbword();
	point(x, y);
	for (i = 2; i < count; i++)
		getfbword();
}

plotmove(count, command, string)
int	count;
int	command;
char	*string;
{
	int i, x, y;

	lastx = getfbword();
	lasty = getfbword();
	for (i = 2; i < count; i++)
		getfbword();
}

plotdraw(count, command, string)
int	count;
int	command;
char	*string;
{
	int i, x, y;

	x = getfbword();
	y = getfbword();
	line(x, y, lastx, lasty);
	lastx = x;
	lasty = y;
	for (i = 2; i < count; i++)
		getfbword();
}
