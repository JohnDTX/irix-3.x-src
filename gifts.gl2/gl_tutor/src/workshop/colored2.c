/*  color2.c  -- This program draws a static image of a baseball	*/
/*		 diamond with several colors.  The field is covered	*/
/*		 with grass.						*/
/*  graphics library calls introduced:  mapcolor(index, r, g, b)	*/

#include "gl.h"
#include "device.h"

/*  The main routine has not changed since the last example.		*/

main () {
    initialize ();
    drawimage ();
    while (TRUE) {	/*  loop forever (killed from window manager).  */
	while (!qtest())/*  wait for input     				*/
	    ;	
	processinput(); /*  process input                               */
    }
}

/*  Initialization now includes defining colors 8 and 9.		*/

initialize () {

    prefsize (450, 450);
    winopen ("diamond");

    qdevice( REDRAW );

    mapcolor (8, 240, 240, 240);/*  make color of fences, lines	*/
    mapcolor (9, 0, 175, 0);	/*  make color of grass		*/
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

/*  drawimage() has not changed since the last example.			*/

drawimage () {
    color (BLACK);
    clear ();
    diamond ();
}

/*  Declare color 9 to be the current color.  Then draw a filled arc	*/
/*  (in color 9) for the grass of the entire field.  Draw remaining	*/
/*  field features with color 8.					*/

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
