/*  threed9.c -- This is a version of the previous program	*/
/*  using 3D floating point calls instead of 2D integer 	*/
/*  graphics commands.  We did NOT have to do this at all	*/
/*  since the z value for 2D graphics commands defaults to z=0.	*/
/*  However, it is very likely that you will be building	*/
/*  objects in 3D space with floating point, rather than 	*/
/*  integer coordinates.  This change was made to introduce	*/
/*  you to 3D calls.						*/
/*  graphics library calls introduced:  arc, arcf, circf, cmov,	*/
/*	draw, move, pdr, pmv, rpdr.  All these are 3D floating	*/
/*	point versions of their 2D integer relatives.		*/

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

/*  initialize() has not changed since the last example.	*/

initialize () {
    int i;

    prefsize (450, 450);
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

/*  drawimage() has not changed since the last example.		*/

drawimage () {
    frontbuffer(TRUE);	/*  draw the field twice, once to each buffer	*/
    color(BLACK);
    clear();
    diamond();
    frontbuffer(FALSE);
}

/*  moveball() has changed to 3D floating point		*/

moveball(mx, my) 
short mx, my;
{
    int i;
    int screenx, screeny;	/*  position of lower left corner	*/
    int distx, disty;		/*  distance ball will travel		*/
    float incrx, incry;		/*  amount to move ball in 1 iteration	*/
    float newx, newy;		/*  location for ball in flight		*/

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

/*  diamond() has changed to 3D floating point	*/

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

/*  draw_base() has changed to 3D floating point	*/

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
