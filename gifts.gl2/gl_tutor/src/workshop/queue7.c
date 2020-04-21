/*  queue7.c  -- This program receives all input in an event queue.	*/
/*		 The effect of the program is not different from the	*/
/*		 previous program, aim8.  The user chooses a spot on	*/
/*		 the field with the LEFT MOUSE button, and the ball	*/
/*		 is sent to it.						*/
/*  graphics library calls introduced:  qdevice(), tie(), 		*/
/*					qtest(), qread()		*/

#include "gl.h"
#include "device.h"

/*  While there is nothing on the queue, consider the program idle	*/
/*  and just swapbuffers.  When something gets this programs attention,	*/
/*  generally a queued device, its input will be processed.		*/

main () {
    initialize ();
    drawimage ();
    while (TRUE) {	/*  loop forever.  swapbuffers and process	*/
	while (!qtest())/*  input while looping				*/
	    swapbuffers();	
	processinput();
    }
}

/*  One line of code has been added to initialize() to 		*/
/*  designate the LEFTMOUSE button as a queued device.	Then	*/
/*  the tie() routine queues the MOUSEX and MOUSEY.  Now	*/
/*  when a LEFTMOUSE device entry is put on the queue, 		*/
/*  corresponding MOUSEX and MOUSEY entries are also put	*/
/*  on the queue.						*/

initialize () {
    int i;

    prefsize (450, 450);
    winopen ("diamond");

    doublebuffer();	/* display mode to become double buffer */
    gconfig();		/* display mode change takes effect	*/

    qdevice(LEFTMOUSE); /* queue the LEFTMOUSE device		*/
/*  queue the MOUSEX and MOUSEY devices, and tie them to	*/
/*  the LEFT MOUSE						*/
    tie (LEFTMOUSE, MOUSEX, MOUSEY);  
    qdevice( REDRAW );

    mapcolor (8, 240, 240, 240);/*  make color of arcs, lines	*/
    mapcolor (9, 0, 175, 0);	/*  make color of grass		*/
    mapcolor (16, 240, 150, 0);  /*  make overlay (ball) 	*/
    mapcolor (24, 240, 150, 0);  /*  for colors 16, 24, and 25	*/
    mapcolor (25, 240, 150, 0);
}

/*  Input devices can make entries in the event queue.  The	*/
/*  processinput() routine now handles those events.  The	*/
/*  qtest() call reads the event queue.  qtest() immediately	*/
/*  returns the device number of the first entry of the queue.	*/
/*  If the queue is empty, qtest() returns FALSE, and 	 	*/
/*  processinput() is exited.  qtest() does NOT remove the	*/
/*  entry from the queue.					*/
/*  Each entry into the queue has two fields:  a device name	*/
/*  and an associated data value.  qread(&val) waits until 	*/
/*  there is an entry on the queue, then it reads that entry.	*/
/*  qread(&val) returns the name of the device and also writes	*/
/*  the associated data value into the variable, val.  The	*/
/*  entry on the queue is REMOVED.				*/
/*  With many buttons and keys, the values 1 and 0 are 		*/
/*  associated with pressing down (1) and releasing (0) a key	*/
/*  or button.  With the MOUSEX and MOUSEY devices, the 	*/
/*  associated value is the valuator location of the cursor	*/
/*  on the screen.						*/

processinput() {
    short val;
    short mx, my;
    int dev;
    int domove;

    domove = FALSE;
    while (qtest()) {
	dev = qread(&val);
	switch (dev) { 
	case LEFTMOUSE:
	    if (val == 1) /*  when mouse pressed, will do motion  */
		domove = TRUE;	
	    break;
	case MOUSEX:
	    mx = val;
	    break;
	case MOUSEY:
	    my = val;
	    break;
	case REDRAW:                /* window manager asking for redraw.*/
	    reshapeviewport();      /* This shapes the window.          */
	    drawimage();            /* This redraws the image.          */
	    break;
	default:
	    break;
	}
    }
    if (domove)
	moveball(mx, my);
}

/*  drawimage() has not changed since the last example.		*/

drawimage () {
    frontbuffer(TRUE);	/*  draw the field twice, once to each buffer	*/
    color(BLACK);
    clear();
    diamond();
    frontbuffer(FALSE);
}

/*  moveball() now takes two arguments, which are the final (mx, my)	*/
/*  location of the ball on the field.					*/ 

moveball(mx, my) 
short mx, my;
{
    int i;
    int screenx, screeny;	/*  position of lower left corner	*/
    int distx, disty;		/*  distance ball will travel		*/
    float incrx, incry;		/*  amount to move ball in 1 iteration	*/
    float newx, newy;		/*  location for ball in flight		*/
    int inewx, inewy;		/*  integer location for ball in flight	*/

    getorigin (&screenx, &screeny);
    distx = mx - screenx;
    disty = my - screeny;
    incrx = (float) distx / 50.0;
    incry = (float) disty / 50.0;
    newx = 0.0;
    newy = 0.0;
    writemask(16);	/*  protect the first four bitplanes		*/
    for (i = 0; i < 50; i = i + 1) {
	newx = newx + incrx;
	newy = newy + incry;
	inewx = (int) newx;
	inewy = (int) newy;
	color (BLACK);		/*  clear away old ball			*/
	clear ();
	color (16);
	circfs (inewx, inewy, 3);/*  draw ball at position (inewx, inewy)*/
	swapbuffers();
    }
	/*  Draw the ball into both front and back buffers.	*/
    frontbuffer (TRUE);	/*  draw final resting place of ball to		*/
    color (BLACK);	/*  both buffers				*/
    clear ();
    color (16);
    circfs (inewx, inewy, 3);/*  draw ball at position (inewx, inewy)*/
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

    setpattern(1);	/*  change polygon fill pattern	*/
    circfs (43, 43, 10);	/*  pitcher's mound	*/
    setpattern(0);	/*  restore poly fill pattern	*/

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
