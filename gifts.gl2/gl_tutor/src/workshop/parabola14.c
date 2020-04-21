/*  parabola14.c -- parabola14 is similar to the previous workshop	*/
/*  except that the ball leaves the surface of the playing field.	*/
/*  The ball travels in a parabola from home plate to the selected	*/
/*  point on the field.  This really gives the illusion of 3D.		*/
/*  No new graphics commands introduced here.				*/
/*  parabola() is a completely new procedure.				*/

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

/*  processinput has not changed since the last example.	*/

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

/*  outfieldimage() has not changed since the last example.		*/

outfieldimage () {

    perspective (1350, 1.0, 0.1, 10000.0);
    polarview (400.0, 1350, 800, 0);
    frontbuffer(TRUE);	/*  draw the field twice, once to each buffer	*/
    color(BLACK);
    clear();
    diamond();
    frontbuffer(FALSE);
}

/*  waitandswap() has not changed since the last example.		*/

waitandswap () {
    int i;

    for (i = 0; i < 60; i++) {
	swapbuffers();
	gsync();
    }
}

/*  moveball() has been modified.  The initial position of the	*/
/*  ball (0.0, 0.0), and the relative distance the ball will	*/
/*  travel.  I say "relative" because the distance depends on	*/
/*  how large a window is made.  The parabola() routine is	*/
/*  called to draw the moving ball.				*/

moveball(mx, my) 
short mx, my;
{
    int i;
    int screenx, screeny;	/*  position of lower left corner	*/
    int sizex, sizey;		/*  size of window			*/
    int distx, disty;		/*  distance ball will travel		*/
    float rdistx, rdisty;	/*  relative distance ball will travel	*/

    getorigin (&screenx, &screeny); /* coordinates of lower left corner */
    getsize (&sizex, &sizey);       /* size of window                   */
    distx = mx - screenx;           /* distance to x mouse position     */
    disty = my - screeny;           /* distance to y mouse position     */

    /* This stuff calculates the relative position of the ball in the   */
    /* window as defined by the window size and projection.             */
    rdistx = (float) distx * 425.0 / (float) sizex;
    rdisty = (float) disty * 425.0 / (float) sizey;

    parabola(0.0, 0.0, rdistx, rdisty, 100.0, 100);
}

/*  Given initial conditions, the parabola() routine draws	*/
/*  the moving ball.  (xstart, ystart) are the coordinates	*/
/*  of the starting position of the parabola, assuming z = 0.	*/
/*  (xdone, ydone) is where the ball will meet the ground 	*/
/*  again.  At the apex, the parabola reaches a height of	*/
/*  zmax units.  The position of the ball will be calculated	*/
/*  the number of times specified by the iterates variable.	*/

parabola(xstart, ystart, xdone, ydone, zmax, iterates)
float xstart, ystart, xdone, ydone, zmax;
int iterates;
{
    float t, x, y, z;
    float tincr;

    writemask(16);	/*  protect the first four bitplanes		*/
    tincr = 1.0 / (float) iterates;

    for (t = 0.0; t <= 1.0; t = t + tincr) {
	color(BLACK);
	clear();
	color (16);
	x = (xdone - xstart) * t;     /* velocity constant in x direction   */
	y = (ydone - ystart) * t;     /* velocity constant in y direction   */
	z = 4 * zmax * (t * (1 - t)); /* formula for z motion is a parabola */
	pushmatrix ();
	translate (x, y, z);
	draw_ball ();          /* draw ball at new position          */
	popmatrix ();
	swapbuffers();
    }
	/*  Draw the ball into both front and back buffers.	*/
    frontbuffer (TRUE);	/*  draw final resting place of ball to		*/
    color (BLACK);	/*  both buffers				*/
    clear ();
    color (16);

    pushmatrix ();
    translate (x, y, z);
    draw_ball ();          /* draw ball at new position          */
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
