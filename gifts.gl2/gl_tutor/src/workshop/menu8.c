/*  menu8.c -- This program allows the user a pop-up menu	*/
/*  interface to an application program.  In this example,	*/
/*  the RIGHT MOUSE button controls a popup menu.  This popup	*/
/*  menu can be used to exit the program.  Intention to exit	*/
/*  the program must be reconfirmed by the user responding to	*/
/*  a second popup menu.  Popup menu support is ONLY available	*/
/*  in the window manager.					*/
/*  graphics library calls introduced:  defpup(), newpup(),	*/
/*		addtopup(), dopup().				*/

#include "gl.h"
#include "device.h"

/*  new global variables for menus		*/

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

/*  In initialize(), queue the RIGHTMOUSE button for popup menus.	*/
/*  Also the pup calls are used to create popup menus.			*/

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

/*  Now we define two pop-up menus using two different methods. The 1st */
/*  method defines the menu and the text describing the options, all in */
/*  the same step, using a defpup () call. The 2nd method first defines */
/*  the menu using a newpup () call, then puts in the text using a      */
/*  addtopup () all.                                                    */

/*  Note: the '|' character separates menu items, and the %t flag is    */
/*  used to designate the menu title.                                   */

/*  newpup () and defpup () both return the menu id number used later.  */

    killmenu = defpup ("Do you want to exit? %t|yes|no");
    menu = newpup ();
    addtopup (menu, "Baseball %t|exit program");

}

/*  In processinput(), the RIGHTMOUSE button, when it is pressed	*/
/*  down (val == 1), displays a popup menu.  The value returned		*/
/*  by the user's selection is stored in variable menuval.  The		*/
/*  value returned may be:						*/
/*	 1 -- for exit program						*/
/*	-1 -- nothing on menu is chosen					*/
/*  Then the user is asked to reconfirm an exit program decision.	*/
/*	 1 -- for yes							*/
/*	 2 -- for no							*/
/*	-1 -- nothing on menu is chosen					*/

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
	case RIGHTMOUSE:  /*  when RIGHT MOUSE pressed, do popup  */
	    if (val == 1) {
		menuval = dopup(menu); /* returns the menu item chosen */
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

/*  moveball() has not changed since the last example.		*/

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
