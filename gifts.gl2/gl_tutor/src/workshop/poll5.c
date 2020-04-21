/*  poll5.c  -- This program polls user input from the mouse.		*/
/*		The motion of the ball is initiated by the user.	*/
/*		When the LEFT mouse button is pressed, the motion	*/
/*		begins.							*/
/*  graphics library calls introduced:  getbutton()			*/

#include "gl.h"
#include "device.h"

/*  The main routine has not changed since the last example.             */

main () {
    initialize ();
    drawimage ();
    while (TRUE) {	/*  loop forever.  swapbuffers while looping.	*/
	while (!qtest()) {            /*  wait for input     		*/
	    swapbuffers();
	    if (getbutton (LEFTMOUSE)) /* We check here if the left     */
		moveball();            /* mouse has been pressed.       */
	}
	processinput(); /*  process input from queue.                   */
    }
}

/*  initialize() has not changed since the last example.		*/

initialize () {
    int i;

    prefsize (450, 450);
    winopen ("diamond");

    doublebuffer();	/* display mode to become double buffer */
    gconfig();		/* display mode change takes effect	*/

    qdevice( REDRAW );

    mapcolor (8, 240, 240, 240);/*  make color of arcs, lines	*/
    mapcolor (9, 0, 175, 0);	/*  make color of grass		*/
    mapcolor (16, 240, 150, 0);  /*  make overlay (ball) 	*/
    mapcolor (24, 240, 150, 0);  /*  for colors 16, 24, and 25	*/
    mapcolor (25, 240, 150, 0);
}

/*  processinput() is altered to poll the mouse button		*/

processinput() {
    short val;
    int dev;

    while (qtest()) {               /* while input on queue.            */
	dev = qread(&val);          /* read input device data.          */
	switch (dev) { 
	case REDRAW:                /* window manager asking for redraw.*/
	    reshapeviewport();      /* This shapes the window.          */
	    drawimage();            /* This redraws the image.          */
	    break;
	default:
	    break;
	}
    }
}

/*  The former drawimage() routine is now split into two	*/
/*  routines.  drawimage() draws just the baseball field.	*/
/*  Also see moveball() below.					*/

drawimage () {
    frontbuffer(TRUE);	/*  draw the field twice, once to each buffer	*/
    color(BLACK);
    clear();
    diamond();
    frontbuffer(FALSE);
}

/*  The second part of the former drawimage() routine is now	*/
/*  moveball().  In moveball(), the ball is sent into across	*/
/*  the surface of the playing field until it settles into	*/
/*  a final resting place.  Call moveball() again and again	*/
/*  restarts the motion sequence.				*/

moveball() {
    int i;

    writemask(16);	/*  protect the first four bitplanes		*/
    for (i = 0; i < 300; i = i + 3) {
	color (BLACK);		/*  clear away old ball			*/
	clear ();
	color (16);
	circfs (i, i, 3);	/*  draw ball at position (x, y)	*/
	swapbuffers();
    }
	/*  Draw the ball into both front and back buffers.	*/
    frontbuffer (TRUE);	/*  draw final resting place of ball to		*/
    color (BLACK);	/*  both buffers				*/
    clear ();
    color (16);
    circfs (i, i, 3);	/*  draw ball at position (x, y)	*/
    frontbuffer(FALSE);
    writemask(0xfff);	/*  unprotect all bitplanes		*/
}

/*  diamond() has not changed since the last example.		*/

diamond () {
    color (9);		/*  make color 9 current color	*/
    arcfs (0, 0, 375, 0, 900);	/*  grass		*/
    color (8);		/*  make color 8 current color	*/
    linewidth(2);	/*  change thickness of lines	*/
    arcs (0, 0, 375, 0, 900);	/*  fences		*/
    arcs (0, 0, 150, 0, 900);	/*  infield		*/

    move2s (0, 0);		/*  foul lines		*/
    draw2s (0, 400);
    move2s (0, 0);
    draw2s (400, 0);
    linewidth(1);	/*  restore thickness of lines	*/

    circfs (43, 43, 10);	/*  pitcher's mound	*/

    draw_base (90, 0);		/*  first, second and third bases	*/
    draw_base (90, 90);
    draw_base (0, 90);

    pmv2s (0, 0);		/*  draw home plate	*/
    pdr2s (0, 3);
    pdr2s (3, 6);
    pdr2s (6, 3);
    pdr2s (3, 0);
    pclos ();

    cmov2i (100, 400);	/*  position to draw scoreboard	*/
    charstr ("New York 3");
    cmov2i (100, 385);	
    charstr ("Boston   2");
}

/*  draw_base has not changed since the last example.			*/

draw_base (x, y)
short   x,
        y;
{
    pmv2s (x, y);
    rpdr2s (5, 0);
    rpdr2s (0, 5);
    rpdr2s (-5, 0);
    pclos ();
}
