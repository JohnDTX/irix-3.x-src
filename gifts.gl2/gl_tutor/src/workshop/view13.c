/*  view13.c -- This program introduces an alternative view to the	*/
/*  overhead (blimp) orthographic view.  After displaying the movement	*/
/*  of the ball from overhead, the ball rests using the waitandswap()   */
/*  routine.								*/
/*  Then an instant replay of the movement is shown.  The second time,	*/
/*  there is a different point of view (using polarview) and 		*/
/*  perspective foreshortening.  Notice that the ball becomes larger	*/
/*  as it approaches.							*/
/*  outfieldimage() and waitandswap() are new routines.			*/
/*  processinput() is changed.						*/
/*									*/
/*  graphics library calls introduced:  perspective(), polarview()	*/

#include "gl.h"
#include "device.h"

/*  global variables for menus		*/

int     menu,
        killmenu;

/*  the main() routine has not changed since the last example.	*/

main () {
    initialize ();
    drawimage ();
    while (TRUE) {	/*  loop forever.  swapbuffers and process	*/
	while (!qtest())/*  input while looping				*/
	    swapbuffers();	
	processinput();
    }
}

/*  the initialize() routine has not changed since the last example.	*/

initialize () {
    int i;

    keepaspect (1, 1);
    winopen ("diamond");

    doublebuffer();	/* display mode to become double buffer */
    gconfig();		/* display mode change takes effect	*/

    qdevice(LEFTMOUSE); /* queue the LEFTMOUSE device		*/
    qdevice(RIGHTMOUSE);/* queue the RIGHTMOUSE device		*/
/*  queue the MOUSEX and MOUSEY devices, and tie them to	*/
/*  the LEFT MOUSE						*/
    tie (LEFTMOUSE, MOUSEX, MOUSEY);  
    qdevice(REDRAW);	/* queue the REDRAW device		*/

    mapcolor (8, 240, 240, 240);/*  make color of arcs, lines	*/
    mapcolor (9, 0, 175, 0);	/*  make color of grass		*/
    mapcolor (16, 240, 150, 0);  /*  make overlay (ball) 	*/
    mapcolor (24, 240, 150, 0);  /*  for colors 16, 24, and 25	*/
    mapcolor (25, 240, 150, 0);

    killmenu = defpup ("Do you want to exit? %t|yes|no");
    menu = newpup ();
    addtopup (menu, "Baseball %t|exit program");
}

/*  If a point is chosen, the ball travels twice.  First, it 	*/
/*  is viewed with a familiar overhead view.  waitandswap()	*/
/*  momentarily fixes the position of the ball.  Then the 	*/
/*  instant replay is viewed from a different location.		*/

processinput() {
    short val;
    short mx, my;
    int dev;
    int domove;
    int menuval, kmenuval;

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
	case RIGHTMOUSE:
	    if (val == 1) {
		menuval = dopup(menu);
		if (menuval == 1) {  /*  if exit chosen, ask user */
		    kmenuval = dopup (killmenu); /*  to reconfirm */
		    if (kmenuval == 1) 
			exit (0);
		}
	    }
	    break;
	case REDRAW:
	    reshapeviewport();
	    drawimage();
	    break;
	default:
	    break;
	}
    }
    if (domove) {
	drawimage();		/*  first view		*/
	moveball(mx, my);
	waitandswap();		/*  freeze ball 	*/
	outfieldimage();	/*  instant replay	*/
	moveball(mx, my);
	waitandswap();		/*  freeze ball 	*/
	drawimage();
    }
}

/*  the drawimage() routine has not changed since the last example.	*/

drawimage () {
    ortho (0.0, 425.0, 0.0, 425.0, -10000.0, 10000.0);
    frontbuffer(TRUE);	/*  draw the field twice, once to each buffer	*/
    color(BLACK);
    clear();
    diamond();
    frontbuffer(FALSE);
}

/*  outfieldimage() is introduced.  It specifies the alternate		*/
/*  view for the instant replay.  The parameters for perspective()	*/
/*  and polarview() were largely selected by good trial and error.	*/

/*  The field of view was chosen to be 135 degrees, or ~ 67 degrees to  */
/*  either side. The aspect ratio matches the aspect ratio of the       */
/*  window, namely 1. The near and far clipping planes clip only objects*/
/*  extremely close (within 0.1) or extremely far (beyond 10000.0).     */

/*  The polarview parameters set our view position 400 units from the   */
/*  origin, with a azimith angle of 135 degrees and an inclination of   */
/*  80 degrees producing the view seen from the outfield. The last      */
/*  parameter, twist, is set to zero so that the field remains upright. */

outfieldimage () {

    perspective (1350, 1.0, 0.1, 10000.0);
    polarview (400.0, 1350, 800, 0);
    frontbuffer(TRUE);	/*  draw the field twice, once to each buffer	*/
    color(BLACK);
    clear();
    diamond();
    frontbuffer(FALSE);
}

/*  waitandswap() is introduced.  It causes the program to idle	*/
/*  between views of motion.  When idling, the program swaps 	*/
/*  buffers, which is vital when double buffer programs are	*/
/*  running in the window manager.  The gsync() call delays	*/
/*  execution until the next screen refresh cycle.		*/

waitandswap () {
    int i;

    for (i = 0; i < 60; i++) {
	swapbuffers();
	gsync();
    }
}

/*  moveball() has not changed since the last example.		*/

moveball(mx, my) 
short mx, my;
{
    int i;
    int screenx, screeny;	/*  position of lower left corner	*/
    int sizex, sizey;		/*  size of window			*/
    int distx, disty;		/*  distance ball will travel		*/
    float rdistx, rdisty;	/*  relative distance ball will travel	*/
    float incrx, incry;		/*  amount to move ball in 1 iteration	*/
    float newx, newy;		/*  location for ball in flight		*/

    getorigin (&screenx, &screeny); /* coordinates of lower left corner */
    getsize (&sizex, &sizey);       /* size of window                   */
    distx = mx - screenx;           /* distance to x mouse position     */
    disty = my - screeny;           /* distance to y mouse position     */

    /* This stuff calculates the relative position of the ball in the   */
    /* window as defined by the window size and projection.             */
    rdistx = (float) distx * 425.0 / (float) sizex;
    rdisty = (float) disty * 425.0 / (float) sizey;

    incrx = (float) rdistx / 50.0; /*  do 50 iterations		*/
    incry = (float) rdisty / 50.0;
    newx = 0.0;
    newy = 0.0;
    writemask(16);	/*  protect the first four bitplanes		*/
    for (i = 0; i < 50; i = i + 1) {
	newx = newx + incrx;
	newy = newy + incry;
	color (BLACK);		/*  clear away old ball			*/
	clear ();
	color (16);
	pushmatrix ();
	translate (newx, newy, 0.0);/*  translate local axes to newx, newy*/
	draw_ball ();
	popmatrix ();
	swapbuffers();
    }
	/*  Draw the ball into both front and back buffers.	*/
    frontbuffer (TRUE);	/*  draw final resting place of ball to		*/
    color (BLACK);	/*  both buffers				*/
    clear ();
    color (16);
    pushmatrix ();
    translate (newx, newy, 0.0);/*  translate local axes to newx, newy	*/
    draw_ball ();
    popmatrix ();
    frontbuffer(FALSE);
    writemask(0xfff);	/*  unprotect all bitplanes		*/
}

/*  draw_ball() has not changed since the last example.		*/

draw_ball ()
{
    circ (0.0, 0.0, 3.0);
    rotate (900, 'y');
    circ (0.0, 0.0, 3.0);
    rotate (600, 'x');
    circ (0.0, 0.0, 3.0);
    rotate (600, 'x');
    circ (0.0, 0.0, 3.0);
}

/*  diamond() has not changed since the last example.		*/

diamond () {
    color (9);		/*  make color 9 current color	*/
    arcf (0.0, 0.0, 375.0, 0, 900);	/*  grass	*/
    color (8);		/*  make color 8 current color	*/
    linewidth(2);	/*  change thickness of lines	*/
    arc (0.0, 0.0, 375.0, 0, 900);	/*  fences	*/
    arc (0.0, 0.0, 150.0, 0, 900);	/*  infield	*/

    move (0.0, 0.0, 0.0);	/*  foul lines		*/
    draw (0.0, 400.0, 0.0);
    move (0.0, 0.0, 0.0);
    draw (400.0, 0.0, 0.0);
    linewidth(1);	/*  restore thickness of lines	*/

    circf (43.0, 43.0, 10.0);	/*  pitcher's mound	*/

    draw_base (90.0, 0.0, 0.0);	/*  first, second and third bases	*/
    draw_base (90.0, 90.0, 0.0);
    draw_base (0.0, 90.0, 0.0);

    pmv (0.0, 0.0, 0.0);	/*  draw home plate	*/
    pdr (0.0, 3.0, 0.0);
    pdr (3.0, 6.0, 0.0);
    pdr (6.0, 3.0, 0.0);
    pdr (3.0, 0.0, 0.0);
    pclos ();

    cmov (100.0, 400.0, 0.0);	/*  position to draw scoreboard	*/
    charstr ("New York 3");
    cmov (100.0, 385.0, 0.0);	
    charstr ("Boston   2");
}

/*  draw_base() has not changed since the last example.		*/

draw_base (x, y, z)
float   x,
        y,
	z;
{
    pmv (x, y, z);
    rpdr (5.0, 0.0, 0.0);
    rpdr (0.0, 5.0, 0.0);
    rpdr (-5.0, 0.0, 0.0);
    pclos ();
}
