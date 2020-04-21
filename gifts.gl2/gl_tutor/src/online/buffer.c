/*  buffer.c							*/
/*  March, 1986 -- Mason Woo					*/
/*  Learning Environment -- to demonstrate and contrast the	*/
/*  effect of single buffer mode versus double buffer mode	*/
/*  in scenes with motion					*/

#include <gl.h>
#include <device.h>
#include "tutor.h"

#define PLANET		1
#define MOON		2
#define COMET		3

#define SINGLE	1
#define DOUBLE	2
#define EXIT	3

Colorindex  cometcolor,
	    mooncolor,
	    planetcolor;

int menu, killmenu;
int backwin, helpwin, bufferwin;
int motion = FALSE;
int degree = 0;

main () {
    short   val;
    long    dev;
    int     active = FALSE;
    short   pickme ();
    int     i;
    int     really;

    check_for_mex();

    init_windows ();

    setupcolors();
    setupqueue();
    makemenus();

    makeobjects();

    load_help ("Use the RIGHT MOUSE button to:", 
	"   1. Animate the scene with double buffering.",
	"   2. Animate the scene with a single buffer.",
	"   3. Exit the program.", "", "");

    while (1) {
	while (qtest ()) {
	    dev = qread (&val);
	    switch (dev) {
		case RIGHTMOUSE: 
		    if (val) {
			switch (dopup(menu)) {
			case SINGLE:
			    orbitsingle();
			    break;
			case DOUBLE:
			    orbitdouble();
			    break;
			case EXIT:
			    really = dopup (killmenu);
			    if (really == 1) {
			   	tutorrestoremap();
				gexit ();
				exit (0);
			    }
			    break;
			default:
			    break;
			}
		    }
		    break;
		case INPUTCHANGE: 
		    active = val;
		    if (active == FALSE) {
load_help("Press the RIGHT MOUSE BUTTON and select ATTACH.",
	  "", "", "", "", "");
		    } else {
    load_help ("Use the RIGHT MOUSE button to:", 
	"   1. Animate the scene with double buffering.",
	"   2. Animate the scene with a single buffer.",
	"   3. Exit the program.", "", "");
		    }
		    draw_frame ();
		    break;
		case ESCKEY:
		    tutorrestoremap();
		    gexit ();
		    exit (0);
		    break;
		default: 
		    break;
	    }
	}
	draw_frame ();
    }
}

setupqueue () {
    qdevice(RIGHTMOUSE);
    qdevice(REDRAW);
    qdevice(INPUTCHANGE);
    qdevice(ESCKEY);
}

setupcolors () {
    int i;

    if (getplanes() < 4) {
	printf("You do not have enough bitplanes for this program\n");
	gexit();
	exit(0);
    }
    tutorsavemap();
    tutormakemap();

    cometcolor = 8;
    mooncolor = 9;
    planetcolor = 10;
}

makeobjects()
{
Angle i;

    makeobj(PLANET);
    pushmatrix();
    color(planetcolor);
    for (i = 0; i < 1800; i = i + 300) {
	rotate (i, 'y');
	circi (0, 0, 15);
    }
    popmatrix();
    closeobj();

    makeobj(MOON);
    pushmatrix();
    color(mooncolor);
    translate (25.0, 25.0, 0.0);
    for (i = 150; i < 1950; i = i + 300) {
	rotate (i, 'y');
	circi (0, 0, 5);
    }
    popmatrix();
    closeobj();

    makeobj(COMET);
    pushmatrix();
    color(cometcolor);
    translate (75.0, 0.0, 0.0);
    circfi (0, 0, 2);
    move2i (3, 0);
    draw2i (12, 0);
    move2i (3, 1);
    draw2i (14, 2);
    move2i (3, -1);
    draw2i (14, -2);
    popmatrix();
    closeobj();

}

orbitsingle()
{
/* Henry says this is what the GLII does behind your back anyway */
    motion = SINGLE;
}

orbitdouble()
{
    motion = DOUBLE;
}

orbit()
{
    color(BLACK);
    clear();

    /* draw the objects */
	pushmatrix();
	rotate (degree, 'y');
	callobj(PLANET);
	popmatrix();
	pushmatrix();
	rotate (degree, 'z');
	callobj(MOON);
	popmatrix();
	pushmatrix();
	translate (0.0, -30.0, 0.0);
	rotate (degree >> 1, 'z');
	callobj(COMET);
	popmatrix();

    if (degree >= 3600) {
	degree = 0;
	motion = FALSE;
    }

    if (motion)
	degree += 30;
}

makemenus() {
    int orbitsingle(), orbitdouble();

    menu = defpup ("Buffer %t");
    addtopup(menu, "Single buffer animation");
    addtopup(menu, "Double buffer animation");
    addtopup(menu, "Exit");
    killmenu = defpup ("Exit Confirmation %t|Yes|No");

}

init_windows () 
{
    backwin = init_back ();
    doublebuffer ();
    gconfig ();
    winattach ();
    helpwin = init_help("Buffer");
    bufferwin = init_buffer();
}

init_buffer ()
{
    int res;

    prefposition (237, 787, 25, 575);
    res = winopen ("buffer");
    prefsize (550, 550);
    wintitle ("Buffer -- Animated Scene");
    winconstraints ();
    perspective (600, 1.0, 25.0, 175.0);
    translate (0.0, 0.0, -100.0);
    return (res);
}


draw_frame()
/*---------------------------------------------------------------------------
 * Draw the whole screen.
 *---------------------------------------------------------------------------
 */
{
    draw_buffer_window ();
    draw_back();
    draw_help();

    swapbuffers();
}

draw_buffer_window ()
{
    winset(bufferwin);
    reshapeviewport();
    if (motion == SINGLE)
    	frontbuffer(TRUE);
    else
    	frontbuffer(FALSE);
    orbit();
}
