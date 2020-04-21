	/* this program creates a color ramp of 240 shades of red
		and displays them with their color indices written in white */
#include "gl.h"
#include "device.h"

/* set up some constants */

#define BASEC	8	/* the base color index */
#define ENDC	248		/* the end color index */
#define NXBOXES	24		/* number of boxes in the x direction */
#define NYBOXES	10		/* number of boxes in the y direction */
#define SIZE	(XMAXSCREEN / NXBOXES) /* size (in pixels) of each box */

char buf[100];		/* buf temporarily holds the value of color index
			variable so it can be printed by charstr */

main()
{
	initialize();
	while(TRUE) {
		drawimage();
		checkinput();
		}
}


initialize()
{
	/* declare i as an integer variable */
	int i;

	prefposition(0, 1023, 0, 767);
	winopen("color.c");

	/* load color map locations 8-247 with RGB values of
		increasing red component */
	for (i=BASEC; i<ENDC; i=i+1)
		mapcolor(i, i, 0, 0);
}


drawimage()
{
	int i,j;

	color(BLACK);
	clear();

	for (i=0; i<NYBOXES; i++)
		for (j=0; j<NXBOXES; j++){
			color(i*NXBOXES+j+BASEC);
			rectfi(j*SIZE, i*SIZE, (j+1)*SIZE, (i+1)*SIZE);
			color(WHITE);
			cmov2i(5+j*SIZE, 5+i*SIZE);
			sprintf(buf,"%d ",i*NXBOXES+j+BASEC);
			charstr(buf);
			}
}

checkinput()
{
	Device val;

	switch(qread(&val)) {
		case REDRAW:
			reshapeviewport();
			break;
		}
}
