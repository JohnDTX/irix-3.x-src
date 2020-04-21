/*  double3.c  --  This program uses double buffering to draw a 	*/
/*		   dynamic image of a ball traveling over a field	*/
/*		   without flashing.  The motion is accomplished by	*/
/*		   redrawing the scene to the back buffer.  When an	*/
/*		   image is completed, the entire buffer is swapped	*/
/*		   to the fore, and its image is displayed.		*/
/*  calls introduced:  doublebuffer(), gconfig(), swapbuffers(),	*/
/*		       frontbuffer()					*/

/*  WARNING:  For machines with a minimum configuration of bitplanes,	*/
/*  there may not be enough bitplanes to run all the colors in this	*/
/*  and subsequent double buffered programs.  A minimum of 12		*/
/*  bitplanes are required for this program.  For subsequent programs,	*/
/*  a minimum of 16 bitplanes will be required.				*/

#include "gl.h"
#include "device.h"

/*  In the window manager, double buffered programs MUST swapbuffers,	*/
/*  even when idling.  The window manager waits for all active double	*/
/*  double buffered programs to issue a swapbuffers() call before	*/
/*  actually swapping buffers.  If a program idles without issuing	*/
/*  a swapbuffers() call, all other double buffered programs will	*/
/*  wait for it.							*/

main () {
    initialize ();
    drawimage ();
    while (TRUE) {	/*  loop forever.  swapbuffers while looping.	*/
	while (!qtest())/*  wait for input     				*/
	    swapbuffers();
	processinput(); /*  process input                               */
    }
}

/*  To initiate double buffering, the doublebuffer() call is made.	*/
/*  The display mode change to double buffering does not take place	*/
/*  until the gconfig() call is made.					*/

initialize () {

    prefsize (450, 450);
    winopen ("diamond");

    doublebuffer();	/* display mode to become double buffer */
    gconfig();		/* display mode change takes effect	*/

    qdevice( REDRAW );

    mapcolor (8, 240, 240, 240);/*  make color of arcs, lines	*/
    mapcolor (9, 0, 175, 0);	/*  make color of grass		*/
    mapcolor (10, 240, 150, 0);	/*  make color of ball		*/
}

/*  Process input from the window manager                               */

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

/*  drawimage() creates motion which doesn't flash.  The for loop	*/
/*  changes the value of i, which in turn, changes the position of	*/
/*  the ball.  Each iteration through the loop, the field is redrawn	*/
/*  and the ball is redrawn in a new position.				*/

drawimage () {
    int i;

    for (i = 0; i < 300; i = i + 3) {
	color (BLACK);		/*  clear away old field		*/
	clear ();
	diamond ();		/*  draw new field			*/
	color (10);
	circfs (i, i, 3);	/*  draw ball at position (x, y)	*/
	swapbuffers();
    }
	/*  At the end of the loop, the ball will stop moving.  However,*/
	/*  the two buffers will have the ball in different positions. 	*/
	/*  You want to be certain that the ball is in the same		*/
	/*  position in both buffers.  Otherwise, when at rest, the	*/
	/*  program will swap between the two buffers, and the ball	*/
	/*  will appear to bounce back and forth between the disparate	*/
	/*  positions in the two buffers.  To draw the same scene	*/
	/*  into both buffers, use frontbuffer(TRUE).  Then draw	*/
	/*  the scene--that draws it into both front and back buffers.	*/
	/*  frontbuffer(FALSE) restores the default state, where only	*/
	/*  the back buffer is drawn into.				*/
    frontbuffer(TRUE);	/*  draw final resting place of ball to		*/
    color(BLACK);	/*  both buffers				*/
    clear();
    diamond();
    color (10);
    circfs (i, i, 3);	/*  draw ball at position (x, y)	*/
    frontbuffer(FALSE);
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
