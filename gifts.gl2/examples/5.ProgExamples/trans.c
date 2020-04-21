/* "trans.c" */
#include "gl.h"
#include "device.h"
#include "popup.h"

#define CUBEOBJ 1
#define AXISOBJ 2

#define TRANSX 1
#define TRANSY 2
#define TRANSZ 3
#define ROTX 4
#define ROTY 5
#define ROTZ 6
#define SCALEX 7
#define SCALEY 8
#define SCALEZ 9
#define EXITTRANS 10

popupentry mainmenu[] = {
    TRANSX, "Translate X",
    TRANSY, "Translate Y",
    TRANSZ, "Translate Z",
    ROTX, "Rotate X",
    ROTY, "Rotate Y",
    ROTZ, "Rotate Z",
    SCALEX, "Scale X",
    SCALEY, "Scale Y",
    SCALEZ, "Scale Z",
    EXITTRANS, "Quit",
    0, 0
};

main()
{
    Device val;
    int command, i;
    float transparam;

    ginit();
    doublebuffer();
    gconfig();
    initpopup();
    frontbuffer(TRUE);
    writemask(0xfff);
    color(BLACKDRAW);
    cursoff();
    clear();
    curson();
    frontbuffer(FALSE);
    viewport(100, 900, 100, 700);
    perspective(400, 1.3333333, 0.1, 1000.0);
    lookat(10.0, 10.0, 10.0, 0.0, 0.0, 0.0, 0);

    while (1) {
	frontbuffer(TRUE); /* make the menu visible */
	switch (command = popup(mainmenu)) {
	    case 0:
		continue;
	    case TRANSX:
	    case TRANSY:
	    case TRANSZ:
	    case SCALEX:
	    case SCALEY:
	    case SCALEZ:
		drawbar(-10.0, 10.0);
		break;
	    case ROTX:
	    case ROTY:
	    case ROTZ:
		drawbar(0.0, 3600.0);
		break;
	    case EXITTRANS:
		greset();
		gexit();
		exit(0);
	}
	frontbuffer(FALSE);
	color(BLACKDRAW);
	clear();
	color(YELLOWDRAW);
	drawaxes();
	color(REDDRAW);
	drawcube();
	swapbuffers();
	gflush();
	while (qtest() == 0)	/* while no buttons pressed */ {
	    if (readbar(&transparam)) {
		color(BLACKDRAW);
		clear();
		color(YELLOWDRAW);
		drawaxes();
		pushmatrix();
		switch (command) {
		    case TRANSX:
			translate(transparam, 0.0, 0.0);
			break;
		    case TRANSY:
			translate(0.0, transparam, 0.0);
			break;
		    case TRANSZ:
			translate(0.0, 0.0, transparam);
			break;
		    case SCALEX:
			scale(transparam, 1.0, 1.0);
			break;
		    case SCALEY:
			scale(1.0, transparam, 1.0);
			break;
		    case SCALEZ:
			scale(1.0, 1.0, transparam);
			break;
		    case ROTX:
			rotate((int)transparam, 'x');
			break;
		    case ROTY:
			rotate((int)transparam, 'y');
			break;
		    case ROTZ:
			rotate((int)transparam, 'z');
			break;
		}
		color(REDDRAW);
		drawcube();
		popmatrix();
		swapbuffers();
		gflush();
	    }
	}
	for (i = 0; i < 6; i++)
	    qread(&val);	/* throw away down and up strokes */
				/* of the exit button press */
    }
}

float barmin, barmax, bardelta;

drawbar(minval, maxval)
float minval, maxval;
{
    register i;
    char str[20];

    barmin = minval;
    barmax = maxval;
    bardelta = (barmax - barmin)/800.0;
    pushmatrix();
    pushviewport();
    ortho2(-0.5, 1023.5, -0.5, 767.5);
    viewport(0, 1023, 0, 767);
    frontbuffer(TRUE);
    cursoff();
    color(BLACKDRAW);
    rectfi(99, 19, 1000, 70);
    color(REDDRAW);
    recti(100, 20, 900, 40);
    for (i = 0; i < 5; i++) {
	move2i(100 + i*200, 40);
	draw2i(100 + i*200, 50);
	cmov2i(103 + i*200, 44);
	sprintf(str, "%6.2f", minval + i*(maxval - minval)/4.0);
	charstr(str);
    }
    curson();
    frontbuffer(FALSE);
    popviewport();
    popmatrix();
}

/* The readbar routine returns 1 if the value stored in retval is valid,
 * and zero otherwise.
 */

readbar(retval)
float *retval;
{
    int xmouse, ymouse;

    ymouse = getvaluator(MOUSEY);
    if (20 <= ymouse && ymouse <= 40) {
	xmouse = getvaluator(MOUSEX);
	if (100 <= xmouse && xmouse <= 900) {
	    *retval = barmin + bardelta*(xmouse - 100);
	    return 1;
	}
    }
    return 0;
}

drawcube()
{
    static short initialized = 0;

    if (!initialized) {
	makeobj(CUBEOBJ);

	/* First draw the outline of the cube */

	move(-1.0, -1.0, -1.0);
	draw(1.0, -1.0, -1.0);
	draw(1.0, 1.0, -1.0);
	draw(-1.0, 1.0, -1.0);
	draw(-1.0, -1.0, -1.0);
	draw(-1.0, -1.0, 1.0);
	draw(1.0, -1.0, 1.0);
	draw(1.0, 1.0, 1.0);
	draw(-1.0, 1.0, 1.0);
	draw(-1.0, -1.0, 1.0);
	move(-1.0, 1.0, -1.0);
	draw(-1.0, 1.0, 1.0);
	move(1.0, 1.0, -1.0);
	draw(1.0, 1.0, 1.0);
	move(1.0, -1.0, 1.0);
	draw(1.0, -1.0, -1.0);

	/* now draw the letters 'X', 'Y', and 'Z' on the faces: */

	move(1.0, -0.6666666, -0.5);
	draw(1.0, 0.6666666, 0.5);
	move(1.0, 0.6666666, -0.5);
	draw(1.0, -0.6666666, 0.5);

	move(0.0, 1.0, 0.6666666);
	draw(0.0, 1.0, 0.0);
	draw(0.5, 1.0, -0.6666666);
	move(0.0, 1.0, 0.0);
	draw(-0.5, 1.0, -0.6666666);

	move(-0.5, 0.6666666, 1.0);
	draw(0.5, 0.6666666, 1.0);
	draw(-0.5, -0.6666666, 1.0);
	draw(0.5, -0.6666666, 1.0);
	closeobj();
	initialized = 1;
    }
    callobj(CUBEOBJ);
    gflush();
}

drawaxes()
{
    static initialized = 0;

    if (!initialized) {
	makeobj(AXISOBJ);
	movei(0, 0, 0);
	drawi(0, 0, 2);
	movei(0, 2, 0);
	drawi(0, 0, 0);
	drawi(2, 0, 0);
	cmovi(0, 0, 2);
	charstr("z");
	cmovi(0, 2, 0);
	charstr("y");
	cmovi(2, 0, 0);
	charstr("x");
	closeobj();
	initialized = 1;
    }
    callobj(AXISOBJ);
    gflush();
}
