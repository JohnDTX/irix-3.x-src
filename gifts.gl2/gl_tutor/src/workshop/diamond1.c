/*  diamond1.c  --  This program draws a static image of a baseball	*/
/*		    diamond.  From this simple starting block, a	*/
/*		    more sophisticated series of programs with color,	*/
/*		    motion, user interaction and 3-D will be built.	*/

/*  Every graphics program which utilizes the IRIS graphics library	*/
/*  should include the gl.h and device.h files.				*/

#include "gl.h"
#include "device.h"

/*  For each program in the series, new graphics library routines	*/
/*  will be used.  Each new routine will be introduced in a short	*/
/*  header comment.  For more information on each routine, see the	*/
/*  Reference Manual section of the User's Guide.			*/
/*									*/
/*  graphics library calls introduced:					*/
/*	keepaspect (x, y) -- constrain any window opened in the future	*/
/* 		to an aspect ratio of y divided by x units		*/
/*      prefsize (x, y) -- constrain any window opened in the future to */
/*              a size of x units by y units                            */
/*	winopen ("diamond") -- open a window, name it "diamond"		*/
/*	color (colorname) -- change the current color to colorname	*/
/*	clear () -- clear every pixel to the current color		*/
/*	arcs (x, y, rad, startang, endang) -- draw an UNfilled arc	*/
/*		with a center at (x, y), a radius of length rad.	*/
/*		The arc begins at angle startang and ends at endang,	*/
/*		where startang and endang are measured in tenths of 	*/
/*		degrees, counterclockwise from the x-axis.		*/
/*		arcs is different from arc, because arcs specifies	*/
/*		the x, y and rad arguments to be 16-bit integers	*/
/*		(Screen coordinates) rather than 32-bit floating point	*/
/*		real numbers, which are used for arc.  arcf is used	*/
/*		for filled arcs.					*/
/*	move2s (x, y) -- move, without drawing, the current graphics	*/
/*		position to the point (x, y).  move2 (x, y) without	*/
/*		the s means to move to an (x, y) location specified	*/
/*		by 32-bit real numbers.  move (x, y, z) without the 2	*/
/*		means to move to a 3-D location.  Other calls in the	*/
/*		move family are move2i (2-D integer), moves (3-D short	*/
/*		integer), movei (3-D integer).				*/
/*	draw2s (x, y) -- draws a line from the current graphics 	*/
/*		position (specified by move) to (x, y).  The current	*/
/*		graphics position is then moved to (x, y).  draw2s	*/
/*		also has its sibling calls:  draw, draw2, drawi,	*/
/*		draw2i, draws.						*/
/*	circfs (x, y, rad) -- draw a FILLED circle with a center	*/
/*		at (x, y) and a radius of length rad.  An unfilled	*/
/*		(hollow) circle would be drawn with the circ, circi,	*/
/*		and circs commands.					*/
/*	pmv2s (x, y)							*/
/*	pdr2s (x, y)							*/
/*	pclos () -- The pmv, pdr and pclos are used in conjunction	*/
/*		with one another to draw a FILLED polygon.  The 	*/
/*		location of the first point is determined by pmv2s	*/
/*		(polygon move).  The location of the next points	*/
/*		are determined by a sequence of pdr2s's.  The polygon	*/
/*		is closed by pclos, which connects the final point	*/
/*		(last pdr) with the first point (pmv).  		*/
/*		Also see the polf command for another method of		*/
/*		drawing filled polygons.				*/
/*		A concave polygon will not look right.			*/
/*	rpdr2s (rx, ry) -- relative polygon draw			*/
/*		rpdr is similar to pdr, except that the edge of a	*/
/*		polygon is drawn from the current graphics position	*/
/*		to a point a distance away.  You are no longer 		*/
/*		specifying the point you are drawing to, but how	*/
/*		far away to draw FROM your current position.		*/
/*                                                                      */
/*  The following calls are used to handle input from the window        */
/*  manager and other input sources. They will be discussed in gory     */
/*  detail in the queue7 workshop.                                      */
/*      qdevice () -- Establish input device.                           */
/*      qtest () -- check input queue for any input.                    */
/*      qread () -- Read data from the input queue.                     */
/*  Some lines are drawn twice their previous thickness with the 	*/
/*  linewidth() command.						*/

/*  In the main routine of the program, initialize() is called to	*/
/*  open a window.  Then, drawimage() draws the baseball diamond.	*/
/*  The while(TRUE) loop keeps the window open forever.  The user	*/
/*  can kill the program from the window manager menu.			*/

main () {
    initialize ();
    drawimage ();
    while (TRUE) {	/*  loop forever (killed from window manager).  */
	while (!qtest())/*  wait for input     				*/
	    ;	
	processinput(); /*  process input                               */
    }
}

/*  Open the window.  The prefsize() call forbids changing the window	*/
/*  size from the size designated.  The qdevice call establishes	*/
/*  contact with the window manager in regard to screen refreshes.      */

initialize () {

    prefsize (450, 450);
    winopen ("diamond");

    qdevice( REDRAW );
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

/*  clear the window to BLACK.  Draw the baseball diamond.		*/

drawimage () {
    color (BLACK);
    clear ();
    diamond ();
}

/*  Draw the baseball field in yellow.  Use arcs, circles, lines	*/
/*  and polygons.  Note that we are drawing to a 2-D screen and all	*/
/*  values are integers, so far.					*/

diamond () {
    color (YELLOW);
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
}

/*  Draw a base.  Note the use of relative draws to create the base.	*/
/*  If absolute draws were used, the code would look like this:		*/
/*	pmv2s (x, y);							*/
/*	pdr2s (x + 5, y);						*/
/*	pdr2s (x + 5, y + 5);						*/
/*	pdr2s (x, y + 5);						*/
/*	pclos ();							*/

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
