/* "draw.c" */
#include "gl.h"
#include "device.h"

main()
{
    Device val, xpos, ypos;

    ginit();
    color(BLACK);
    cursoff();
    clear();
    curson();
    color(RED);
    qdevice(LEFTMOUSE);
    qdevice(MIDDLEMOUSE);
    qdevice(RIGHTMOUSE);
    tie(LEFTMOUSE, MOUSEX, MOUSEY);
    tie(MIDDLEMOUSE, MOUSEX, MOUSEY);

    while(1) {
	switch(qread(&val)) {	/* wait for mouse down */
	    case RIGHTMOUSE:	/* quit */
		gexit();
		exit(0);
	    case MIDDLEMOUSE:	/* move */
		qread(&xpos);
		qread(&ypos);
		move2i(xpos, ypos);
		qread(&val);	/* these three reads clear out */
		qread(&val);	/* the button up report */
		qread(&val);
		break;
	    case LEFTMOUSE:	/* draw */
		qread(&xpos);
		qread(&ypos);
		cursoff();
		draw2i(xpos, ypos);
		curson();
		qread(&val);
		qread(&val);
		qread(&val);
		break;
	}
    }
}
