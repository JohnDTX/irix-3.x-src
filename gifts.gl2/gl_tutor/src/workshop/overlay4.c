/*  overlay4.c  -- This program uses double buffering to draw a 	*/
/*		   dynamic image of a ball traveling over a field.	*/
/*		   The ability to disable the bitplanes to which the	*/
/*		   field is drawn allows the field to be drawn only	*/
/*		   once.  The ball is drawn and erased over the field	*/
/*		   without damaging the drawing of the field.		*/
/*  graphics library calls introduced:  writemask()			*/

/*  WARNING:  A minimum of 16 bitplanes are required to run this 	*/
/*  program.								*/

#include "gl.h"
#include "device.h"

/*  The main routine has not changed since the last example.		*/

main () {
    initialize ();
    drawimage ();
    while (TRUE) {	/*  loop forever.  swapbuffers while looping.	*/
	while (!qtest())/*  wait for input     				*/
	    swapbuffers();
	processinput(); /*  process input                               */
    }
}

/*  The entries in the color map are manipulated so that when 	*/
/*  the ball appears over a section of the field, the ball	*/
/*  stays the same RGB color.  The ball will actually appear	*/
/*  as colors 16, 24 and 25 (binary 10000, 11000 and 11001),	*/
/*  on top of 0, 8, 9 (binary 00000, 01000, and 01001), which	*/
/*  are the colors for the field, fences and background.	*/
/*  When the ball (color 16) is over either field, fence or	*/
/*  black background, the writemask/overlay protects the	*/
/*  color values of the underlying object, in effect, adding	*/
/*  16 to the color value.  When the ball moves, only the	*/
/*  16 is erased.  Mapping these three colors to the same RGB	*/
/*  values makes the ball appear as one color the entire trip	*/
/*  of the ball.						*/

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

/*  drawimage() draws the ball moving across the field.  The field	*/
/*  is only drawn ONCE.  The colors of the field utilizes bitplanes	*/
/*  number 1, 2, 3 and 4.  						*/
/*  Then the writemask() call protects bitplanes 1, 2, 3, 4 from being	*/
/*  overwritten any further.  The traveling ball is written to the	*/
/*  fifth bitplane (color 10000 or 16 in decimal).  For each new scene,	*/
/*  only the ball is cleared and drawn.  The field is never drawn	*/
/*  again.								*/

drawimage () {
    int i;

    frontbuffer(TRUE);	/*  draw the field twice, once to each buffer	*/
    color(BLACK);
    clear();
    diamond();
    frontbuffer(FALSE);
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
