/*  project10.c -- This program introduces projection		*/
/*  transformations.   The ortho() call in drawimage() fits	*/
/*  the 425.0 x 425.0 field into the opened window. The	        */
/*  field will shrink or stretch to fit the window. This	*/
/*  makes the prefsize() call unnecessary.              	*/
/*  Some adjustments to the moveball() routine were made        */
/*  so that the mouse position corresponds to the location      */
/*  on the field.  						*/
/*  graphics library calls introduced:  ortho(), keepaspect()	*/

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

/*  Using the ortho() call has made the prefsize() call unnecessary     */
/*  so it has been removed.  The keepaspect() call fixes the shape	*/
/*  of the opened window to a square.					*/

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

/*  processinput() has not changed since the last example.	*/

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
    if (domove)
	moveball(mx, my);
}

/*  An ortho() command has been added to drawimage().  The left,	*/
/*  right, top and bottom parameters for ortho specify what the		*/
/*  section of the world is displayed in the opened window.  The	*/
/*  z values are set such that the ball doesn't get clipped no matter	*/
/*  how close or far away the ball is from us. Specifically the z       */
/*  clipping planes have been set to -10000.0 units behind us and       */
/*  10000.0 units in front of us.                                       */

drawimage () {
    ortho (0.0, 425.0, 0.0, 425.0, -10000.0, 10000.0);
    frontbuffer(TRUE);	/*  draw the field twice, once to each buffer	*/
    color(BLACK);
    clear();
    diamond();
    frontbuffer(FALSE);
}

/*  moveball() is substantially different from the last example.	*/
/*  We can no longer pick an absolute (x, y) location on the screen	*/
/*  with the mouse.  We can pick a location on the window and		*/
/*  calculate the relative distance travelled, depending on the size	*/
/*  of the window.  Note that rdistx and rdisty are the relative 	*/
/*  distance:  the absolute distance * 425.0 (world space size of	*/
/*  field) / screen size of window					*/

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
	circf (newx, newy, 3.0);/*  draw ball at position (newx, newy)	*/
	swapbuffers();
    }
	/*  Draw the ball into both front and back buffers.	*/
    frontbuffer (TRUE);	/*  draw final resting place of ball to		*/
    color (BLACK);	/*  both buffers				*/
    clear ();
    color (16);
    circf (newx, newy, 3.0);/*  draw ball at position (newx, newy)	*/
    frontbuffer(FALSE);
    writemask(0xfff);	/*  unprotect all bitplanes		*/
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
