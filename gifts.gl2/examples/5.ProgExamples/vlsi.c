/* "vlsi.c" */
#include "gl.h"
#include "device.h"

main()
{
    register i;
    Device dummy, xend, yend, xstart, ystart, type;
    short wm;

    ginit();
    mapcolor(0, 255, 255, 255);	/* WHITE */
    mapcolor(1, 0, 0, 255);	/* BLUE */
    mapcolor(2, 0, 255, 0);	/* RED */
    mapcolor(3, 0, 150, 255);	/* PURPLE */
    mapcolor(4, 255, 0, 0);	/* GREEN */
    mapcolor(5, 150, 0, 255);	/* LIGHT BLUE */
    mapcolor(6, 255, 255, 0);	/* YELLOW */
    mapcolor(7, 150, 100, 0);	/* BROWN */
    for (i = 8; i < 24; i++)
	mapcolor(i, 0, 0, 0);	/* BLACK */
    for (i = 24; i < 32; i++)
	mapcolor(i, 255, 255, 255);	/* WHITE */
    qdevice(LEFTMOUSE);
    tie(LEFTMOUSE, MOUSEX, MOUSEY);
    qdevice(MIDDLEMOUSE);
    tie(MIDDLEMOUSE, MOUSEX, MOUSEY);
    qdevice(RIGHTMOUSE);
    qdevice(KEYBD);
    setcursor(0, 16, 16);
    restart();
    while (1)
	switch (type = qread(&dummy)) {
	    case KEYBD:
		gexit();
		exit(0);
	    case RIGHTMOUSE:
		qread(&dummy);
		restart();
		break;
	    case MIDDLEMOUSE:
	    case LEFTMOUSE:
		qread(&xstart);
		qread(&ystart);
		if (xstart < 60) {
		    if (10 <= xstart && xstart <= 50) {
			if (10 <= ystart && ystart <= 50)
			    wm = 1;
			else if (60 <= ystart && ystart <= 100)
			    wm = 2;
			else if (110 <= ystart && ystart <= 150)
			    wm = 4;
			else if (160 <= ystart && ystart <= 200)
			    wm = 8;
			writemask(wm);
			qread(&dummy);
			qread(&dummy);
			qread(&dummy);
		    }
		} else {
		    qread(&dummy);
		    qread(&xend);
		    qread(&yend);
		    if (xend > 60) {
			cursoff();
			if (type == LEFTMOUSE)
			    color(31);	/* draw */
			else
			    color(0);	/* erase */
			rectfi(xstart, ystart, xend, yend);
			curson();
			gflush();
		    }
		}
	}
}

restart()
{
    writemask(0xfff);
    cursoff();
    color(0);
    clear();
    color(1);
    rectfi(10, 10, 50, 50);
    color(2);
    rectfi(10, 60, 50, 100);
    color(4);
    rectfi(10, 110, 50, 150);
    color(8);
    rectfi(10, 160, 50, 200);
    move2i(60, 0);
    draw2i(60, 767);
    color(31);
    writemask(0);
    curson();
}
