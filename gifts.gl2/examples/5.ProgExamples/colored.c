/* "colored.c" */
#include "gl.h"
#include "device.h"

#define MYBLACK 255
#define MYWHITE 254
#define CURRENTCOLOR 253

#define indextovalue(index) (4*index + 3)

short redindex = 0, greenindex = 0, blueindex = 0;

main()
{
    register i, j;
    Device dummy, xpos, ypos;

    ginit();
    color(0);
    writemask(0xfff);		/* get zeroes in all planes */
    clear();
    mapcolor(MYBLACK, 0, 0, 0);		/* black */
    mapcolor(MYWHITE, 255, 255, 255);	/* white */
    mapcolor(CURRENTCOLOR, 0, 0, 0);
    qdevice(LEFTMOUSE);
    tie(LEFTMOUSE, MOUSEX, MOUSEY);
    qdevice(RIGHTMOUSE);
    setcursor(0, MYWHITE, 0xfff);
    buildmap();
    displaymap();
    while (1) {
	j = -1;
	switch (qread(&dummy)) {
	    case RIGHTMOUSE:
		greset();
		gexit();
		exit(0);
	    case LEFTMOUSE:
		qread(&xpos);
		qread(&ypos);
		qread(&dummy);
		qread(&dummy);
		qread(&dummy);
		if (650 <= ypos && ypos <= 720)
		    i = 0;	/* red color bar */
		else if (550 <= ypos && ypos <= 620)
		    i = 1;	/* green color bar */
		else if (450 <= ypos && ypos <= 520)
		    i = 2;	/* blue color bar */
		else i = -1;
		if (i != -1) {
		    if (200 <= xpos && xpos < 840)
			j = (xpos - 200)/10;
		}
		if (j != -1) {	/* valid input event */
		    switch (i) {
			case 0:
			    redindex = j;
			    break;
			case 1:
			    greenindex = j;
			    break;
			case 2:
			    blueindex = j;
			    break;
		    }
		    buildmap();
		    displaymap();
		}
	}
    }
}

buildmap()
{
    register i, j;

    blankscreen(TRUE);
    for (i = 0; i < 3; i++)
	for (j = 0; j < 64; j++)
	    switch (i) {
		case 0:		/* red */
		    mapcolor(i*64+j, indextovalue(j),
				     indextovalue(greenindex),
				     indextovalue(blueindex));
		    break;
		case 1:		/* green */
		    mapcolor(i*64+j, indextovalue(redindex),
				     indextovalue(j),
				     indextovalue(blueindex));
		    break;
		case 2:		/* blue */
		    mapcolor(i*64+j, indextovalue(redindex),
				     indextovalue(greenindex),
				     indextovalue(j));
		    break;
	    }
    mapcolor(CURRENTCOLOR, indextovalue(redindex),
			   indextovalue(greenindex),
			   indextovalue(blueindex));
    blankscreen(FALSE);
}

displaymap()
{
    register i, j;
    static initialized = 0;
    char redstr[10], greenstr[10], bluestr[10];

    if (!initialized)
	{
	    makeobj(1);
	    color(MYBLACK);
	    clear();
	    for (i = 0; i < 3; i++)
		for (j = 0; j < 64; j++) {
		    color(i*64 + j);
		    rectfi(200 + 10*j, 700 - i*100, 210 + 10*j, 650 - i*100);
		    color(MYWHITE);
		    recti(200 + 10*j, 700 - i*100, 210 + 10*j, 650 - i*100);
		}
	    color(CURRENTCOLOR);
	    rectfi(400, 200, 600, 300);
	    color(MYWHITE);
	    recti(400, 200, 600, 300);
	    cmov2i(150, 670);
	    charstr("RED");
	    cmov2i(150, 570);
	    charstr("GREEN");
	    cmov2i(150, 470);
	    charstr("BLUE");
	    cmov2i(275, 245);
	    charstr("CURRENT COLOR");
	    cmov2i(380, 100);
	    charstr("Left mouse button: choose a color");
	    cmov2i(380, 84);
	    charstr("Right mouse button: exit");
	    closeobj();
	    initialized = 1;
	}
    cursoff();
    callobj(1);
    move2i(205 + 10*redindex, 700);
    draw2i(205 + 10*redindex, 720);
    cmov2i(210 + 10*redindex, 705);
    sprintf(redstr, "%d", indextovalue(redindex));
    charstr(redstr);
    move2i(205 + 10*greenindex, 600);
    draw2i(205 + 10*greenindex, 620);
    cmov2i(210 + 10*greenindex, 605);
    sprintf(greenstr, "%d", indextovalue(greenindex));
    charstr(greenstr);
    move2i(205 + 10*blueindex, 500);
    draw2i(205 + 10*blueindex, 520);
    cmov2i(210 + 10*blueindex, 505);
    sprintf(bluestr, "%d", indextovalue(blueindex));
    charstr(bluestr);
    cmov2i(450, 310);
    charstr("(");
    charstr(redstr);
    charstr(", ");
    charstr(greenstr);
    charstr(", ");
    charstr(bluestr);
    charstr(")");
    curson();
    gflush();
}
