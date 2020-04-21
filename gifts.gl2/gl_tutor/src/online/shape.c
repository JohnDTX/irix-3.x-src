/*---------------------------------------------------------------------------
 *  shape.c							
 *  June, 1986 -- Mason Woo					
 *  Learning Environment -- to demonstrate simple 2D shapes	
 *  using IRIS Graphics Library commands			
 *  this is the modified multiple window version, VGU 6/24/86   
 *---------------------------------------------------------------------------
 */

#include <gl.h>
#include <device.h>
#include "tutor.h"

#define ROW		3	/* Row position for one line comment in the 
				   STATUS window. */

#define MAXPARAM	6 	/* Maximum number of parameters in the STATUS
				 * window per command * / 

/* Tokens used to instigate variouse actions. */

#define POLY		1
#define MOVEDRAW	2
#define POINT		3
#define ARCFILL		4
#define ARC		5
#define CIRCFILL	6
#define CIRC		7
#define RECTFILL	8
#define RECT		9
#define RESET		10	/* Reset the parameter values. */
#define EXITSOON	11	/* Exit SHAPE. */


float	low_pos, 	    /* Current low bound of the controler bar, this
			       will change as the slider is wrapped around to
			       the left or right.			    */

	high_pos;	    /* Current high bound of the controler bar, this
			       will change as the slider is wrapped around to
			       the left or right.			    */

float	wrap;		    /* This varaible is set to 1.0 if the slider
			       needs to wrap to the right, and -1.0 if it
			       needs to wrap to the left.		    */

long	dx;		    /* This is the calculated setting of the
			       controller bar from reading the x coordinate of
			       the mouse.				    */

int	holding = FALSE;    /* This is set to true if the LEFT MOUSE button is
			       pressed on top of the slider bar, and stays
			       false unit the LEFT MOUSE button is released.*/

int	line = 0;	    /* The line number in the status window of the
			       current parameter.			    */

int	parameter = 0;	    /* The column of number in the status window of
			       the current parameter.			    */

int	shapetype = POLY;   /* Current shape being shown.		    */
int	saveval = 0;

int	rectx1,		    /* These are the coordinates defining the	    */
	recty1,		    /* rectangle for the rect() and rectf() 	    */
	rectx2,		    /* commands.				    */
	recty2; 

int	circx,		    /* X coordinate of the circle for the circi() and
			       the circfi() commands. */

	circy,		    /* Y coordinate of the circle for the circi() and
			       the circfi() commands. */

	circrad;	    /* Radius  of the circle for the circi() and
			       the circfi() commands. */

int	arcx,		    /* X coordinate of the arci() for the arcfi() and
			       arcfi() commands. */

	arcy,		    /* Y coordinate of the arci() for the arcfi() and
			       arcfi() commands. */

	arcrad,		    /* Radius of the arci() for the arcfi() and
			       arcfi() commands. */

	arcstart,	    /* Start angle of the arci() for the arcfi() and
			       arcfi() commands. */

	arcend;		    /* End angle of the arci() for the arcfi() and
			       arcfi() commands. */

int	pointx,		    /* X coordinate of the point for the pnti()
			       command. */

	pointy;		    /* Y coordinate of the point for the pnti()
			       command. */

int	movex,		    /* X coordinate of the movei() command.	*/
	movey,		    /* Y coordinate of the movei() command.	*/
	drawx,		    /* X coordinate of the drawi() command.	*/
	drawy;		    /* Y coordinate of the drawi() command.	*/

int	pmvx,		    /* Coordinates for the three points of the  */
	pmvy,		    /* polygon made up of a pmvi() command, 	*/
	pdrx1,		    /* two pdri() commands, and a pclos() 	*/
	pdry1,		    /* command.					*/
	pdrx2,
	pdry2;

int	active = FALSE;	    /* TRUE if this program is attached.	    */

int	primmenu,	    /* ID number of the main popup menu.	    */
	exitmenu;	    /* ID number of the kill conmfirmation menu.    */

int	menu;		    /* Return value of the exit confirmation menu.  */

int	backw,		    /* Background	*/
	helpw,		    /* Help window.	*/
	consw,		    /* Console window.	*/
	graphw,		    /* Graph window.	*/
	statw;		    /* Status window.	*/


main () {

    short	val;
    long	dev;
    short	pickme ();
    short	pickval;	/* Value returned from pickme(). */
    int		i;

    /* If the computer isn't running mex, the program cannot be run.  This
       routine checks to make sure of this. */

    check_for_mex();

    init_windows();

    setupcolors ();
    setupqueue ();
    setupmenus ();
    reset_values ();


    while (1) {

        /* This loops until the queue is empty. */
        while (qtest ()) {

            /* Read the event queue and respond according to the event. */
            dev = qread (&val);
            switch (dev) {

            case LEFTMOUSE:
                /* In this case, the LEFT MOUSE button has been
		   pressed. The program then calls the routine
		   pickme(), which then checks, with picking, to see
		   if the cursor is over any parts of the screen that
		   contain an adjustable parameter. If so, it selects
		   it. */

                if (val) {		/* If the button is PRESSED.  */

                    pickval = pickme();
                    if ((pickval > 0) && (pickval <= MAXPARAM)) {
                        select_parameter((int) pickval);

                    } else if (pickval != 0)
                        holding = val;

                } else {		/* If the button is RELEASED.	*/
                    holding = FALSE;
		}

                break;

            case RIGHTMOUSE:

                /* In this case, the RIGHT MOUSE button has been pressed.
		   The program then calls dopup() to bring up the popup
	           menu. It then responds accordingly to the returned
		   value. */

                if (val) {
                    parameter = 0;	/* Set to 0 so that no paramteter 
					   highlights. */

                    saveval = 0; 	/* Saves the picked value. */

		    /* Select a menu entry respond accordingly.		*/
                    switch (dopup(primmenu)) {

                    case POLY:
                        shapetype = POLY;
                        break;

                    case MOVEDRAW:
                        shapetype = MOVEDRAW;
                        break;

                    case POINT:
                        shapetype = POINT;
                        break;

                    case ARCFILL:
                        shapetype = ARCFILL;
                        break;

                    case ARC:
                        shapetype = ARC;
                        break;

                    case CIRCFILL:
                        shapetype = CIRCFILL;
                        break;

                    case CIRC:
                        shapetype = CIRC;
                        break;

                    case RECTFILL:
                        shapetype = RECTFILL;
                        break;

                    case RECT:
                        shapetype = RECT;
                        break;

                    case RESET:
                        reset_values();
                        break;

                    case EXITSOON:
                        menu = dopup (exitmenu);
                        if (menu == 1) {
                            tutorrestoremap();
                            gexit();
                            exit(0);
                        }
                        break;

                    default:
                        break;

                    }
                    break;
                }

            case REDRAW:
		/* Redraw the appropriate window.	*/
                redraw_window(val);
                break;

            case INPUTCHANGE:
                active = val;
		/* If detaching, draw a picture in both buffers. */
                if (active == FALSE) {
                    draw_frame();
                    swapbuffers();
                    draw_frame();
                }
                break;

            case ESCKEY:
		/* Restore the map and exit. */
                tutorrestoremap();
                gexit();
                exit(0);
                break;

            default:
                break;
            }

            /* Sit idle and wait if de-attached. */
            if (!active)
                while (!qtest ())
                    swapbuffers ();
        }

        change_parameter ();
        draw_frame();

        swapbuffers ();
    }
}


/*---------------------------------------------------------------------------
 * This routine will direct output to the status window with
 * attach_to_status() and enter picking mode.  It will then, in turn, put a
 * number coresponding to each parameter on the name list and draw a rectangle
 * around the parameter.  If the rectangle is drawn on top of the cursor, then
 * that paramter number is put into a buffer.  This parameter number is
 * returned by this routine.  If no hits are registered on the parameter list,
 * then the routine does the same thing with the slider bar.
 *---------------------------------------------------------------------------
 */
short	pickme () {

    short	buffer[100];
    long	numpicked;
    int		i, j;

    attach_to_status();
    pushmatrix ();
        pick (buffer, 100);

        /* The viewing transformation must be re-established because pick
	       destroys the current matrix. */
	ortho2 (0.0, 120.0, 0.0, 6.0);

	initnames ();

        switch (shapetype) {

	case POLY:
	    for (i = 0; i < 2; i++) {
		for (j = 0; j < 3; j++) {
		    loadname (j * 2 + i + 1);
		    rectfi (15 * (i + 2), (j + 2), 15 * (i + 3), (j + 3));
		}
	    }
	    break;

	case MOVEDRAW:
	    for (i = 0; i < 2; i++){
		for (j = 0; j < 2; j++) {
		    loadname (j * 2 + i + 1);
		    rectfi (15 * (i + 2), (j * 3), 15 * (i + 3), 3 + (j * 3));
		}
	    }
	    break;

	case POINT:
	    for (i = 0; i < 2; i++) {
		loadname (i + 1);
		rectfi (15 * (i + 2), 3, 15 * (i + 3), 5);
	    }
	    break;

	case ARCFILL:
	case ARC:
	    for (i = 0; i < 5; i++) {
		loadname (i + 1);
		rectfi (15 * (i + 2), 3, 15 * (i + 3), 5);
	    }
	    break;

	case CIRCFILL:
	case CIRC:
	    for (i = 0; i < 3; i++) {
		loadname (i + 1);
		rectfi (15 * (i + 2), 3, 15 * (i + 3), 5);
	    }
	    break;

	case RECTFILL:
	case RECT:
	    for (i = 0; i < 4; i++) {
		loadname (i + 1);
		rectfi (15 * (i + 2), 3, 15 * (i + 3), 5);
	    }
	    break;

	default:
	    break;
	}

	numpicked = endpick (buffer);
    popmatrix ();

    /* If nothing in the status window was picked, check the controller bar */
    if (numpicked){
        return (buffer[1]);

    } else {
        attach_to_cons();

        pushmatrix ();
	    pick (buffer, 100);
	    ortho2 (-250.0, 250.0, -63.0, 63.0);
	    initnames ();
	    loadname (11);
	    rectfi (-150, -5, 149, 5);
	    numpicked = endpick (buffer);
        popmatrix ();

        if (numpicked)
            return (buffer[1]);
        else
            return (0);
    }
}

/*---------------------------------------------------------------------------
 * This routine reads the mouse location and interprets it's x coordinate as
 * the value of the slider bar.  After wrapping around the ends, it adjusts
 * the correct parameter to the value of the bar.
 *---------------------------------------------------------------------------
 */
change_parameter() 
{

    long	mx, my;
    long	ox, oy;
    long	sx, sy;
    Coord	wx, wy;
    long	iwx;

    wrap = 0.0;
    attach_to_cons();

    if (holding) {

        /* Get the console window size and location. */
        getsize(&sx, &sy);
        getorigin(&ox, &oy);

        /* Get the X value of the cursor. */
        mx = getvaluator(MOUSEX);

        /* Calculate the cursor location relative to the console window. */
        iwx = mx - (ox + sx / 2);

        /* Wrap the cursor around if needed. */
        if (iwx < -150) {
            iwx += 300;
            setvaluator(MOUSEX, iwx + ox + sx / 2, 0, 1023);
            wrap = -1.0;

        } else if (iwx > 149) {
            iwx -= 300;
            setvaluator(MOUSEX, iwx + ox + sx / 2, 0, 1023);
            wrap = 1.0;
        }

        /* Calculate the actual slider value. */
        dx = (iwx + 150);

        /* Modify the correct perameter. */
        switch (shapetype) {
        case POLY:
            modify_poly();
            break;

        case MOVEDRAW:
            modify_line();
            break;

        case POINT:
            modify_point();
            break;

        case ARCFILL:
        case ARC:
            modify_arc();
            break;

        case CIRCFILL:
        case CIRC:
            modify_circ();
            break;

        case RECTFILL:
        case RECT:
            modify_rect();
            break;

        default:
            break;
        }
    }
}

/*---------------------------------------------------------------------------
 * Draw everything in the status window.
 *---------------------------------------------------------------------------
 */
draw_status_window() {

    /* Output to the 'STATUS' window. */
    attach_to_status();

    color(BLACK);
    clear();

    color (NORMCOLOR);

    switch (shapetype) {

    case POLY:
        cmov2i (5, 4);

/* Fortran command compiler flags. */
#ifdef FORTRAN
        charstr ("call pmv2i (");
#else
        charstr ("pmv2i (");
#endif

        print_int (2, 1, "", pmvx, FALSE);
        print_int (2, 2, "", pmvy, TRUE);

        cmov2i (5, 3);
#ifdef FORTRAN
        charstr ("call pdr2i (");
#else
        charstr ("pdr2i (");
#endif

        print_int (3, 1, "", pdrx1, FALSE);
        print_int (3, 2, "", pdry1, TRUE);

        cmov2i (5, 2);
#ifdef FORTRAN
        charstr ("call pdr2i (");
#else
        charstr ("pdr2i (");
#endif
        print_int (4, 1, "", pdrx2, FALSE);
        print_int (4, 2, "", pdry2, TRUE);

        cmov2i (5, 1);
#ifdef FORTRAN
        charstr ("call pclos");
#else
        charstr ("pclos ( );");
#endif
        break;

    case MOVEDRAW:

        cmov2i (5, 3);
#ifdef FORTRAN
        charstr ("call move2i (");
#else
        charstr ("move2i (");
#endif
        print_int (3, 1, "    x", movex, FALSE);
        print_int (3, 2, "    y", movey, TRUE);

        cmov2i (5, 1);
#ifdef FORTRAN
        charstr ("call draw2i (");
#else
        charstr ("draw2i (");
#endif

        print_int (5, 1, "    x", drawx, FALSE);
        print_int (5, 2, "    y", drawy, TRUE);
        break;

    case POINT:

        cmov2i (5, 3);
#ifdef FORTRAN
        charstr ("call pnt2i (");
#else
        charstr ("pnt2i (");
#endif

        print_int (ROW, 1, "    x", pointx, FALSE);
        print_int (ROW, 2, "    y", pointy, TRUE);
        break;

    case ARCFILL:

        cmov2i (5, 3);
#ifdef FORTRAN
        charstr ("call arcfi (");
#else
        charstr ("arcfi (");
#endif

        print_int (ROW, 1, "    x", arcx, FALSE);
        print_int (ROW, 2, "    y", arcy, FALSE);
        print_int (ROW, 3, "radius", arcrad, FALSE);
        print_int (ROW, 4, " start", arcstart, FALSE);
        print_int (ROW, 5, "  end", arcend, TRUE);
        break;

    case ARC:

        cmov2i (5, 3);
#ifdef FORTRAN
        charstr ("call arci (");
#else
        charstr ("arci (");
#endif

        print_int (ROW, 1, "    x", arcx, FALSE);
        print_int (ROW, 2, "    y", arcy, FALSE);
        print_int (ROW, 3, "radius", arcrad, FALSE);
        print_int (ROW, 4, " start", arcstart, FALSE);
        print_int (ROW, 5, "  end", arcend, TRUE);
        break;

    case CIRCFILL:

        cmov2i (5, 3);
#ifdef FORTRAN
        charstr ("call circfi (");
#else
        charstr ("circfi (");
#endif

        print_int (ROW, 1, "    x", circx, FALSE);
        print_int (ROW, 2, "    y", circy, FALSE);
        print_int (ROW, 3, "radius", circrad, TRUE);
        break;

    case CIRC:

        cmov2i (5, 3);
#ifdef FORTRAN
        charstr ("call circi (");
#else
        charstr ("circi (");
#endif

        print_int (ROW, 1, "    x", circx, FALSE);
        print_int (ROW, 2, "    y", circy, FALSE);
        print_int (ROW, 3, "radius", circrad, TRUE);
        break;

    case RECTFILL:

        cmov2i (5, 3);
#ifdef FORTRAN
        charstr ("call rectfi (");
#else
        charstr ("rectfi (");
#endif

        print_int (ROW, 1, "   x1", rectx1, FALSE);
        print_int (ROW, 2, "   y1", recty1, FALSE);
        print_int (ROW, 3, "   x2", rectx2, FALSE);
        print_int (ROW, 4, "   y2", recty2, TRUE);
        break;

    case RECT:

        cmov2i (5, 3);
#ifdef FORTRAN
        charstr ("call recti (");
#else
        charstr ("recti (");
#endif

        print_int (ROW, 1, "   x1", rectx1, FALSE);
        print_int (ROW, 2, "   y1", recty1, FALSE);
        print_int (ROW, 3, "   x2", rectx2, FALSE);
        print_int (ROW, 4, "   y2", recty2, TRUE);
        break;

    default:
        break;
    }
}

/*---------------------------------------------------------------------------
 * Given an integer input, draw the slider with the boundry numbers displayed
 * in integer format.
 *---------------------------------------------------------------------------
 */
draw_slider_int (pos)
short	pos;
{
    static char	buffer[10];

    /* Check to see if pos is out of the boundry values and cycle the slider
       values if needed. */

    if ((pos < 0) || ((pos == 0) && (low_pos < 0))) {
        high_pos = (float) ((pos / 300) * 300);
        low_pos = high_pos - 299.0;
        pos = pos - (int) low_pos + 1;

    } else {
        low_pos = (float) ((pos / 300) * 300);
        high_pos = low_pos + 299.0;
        pos -= (int) low_pos;
    }

    /* Draw the slider. */
    draw_int_slider (0.0, 0.0, 100, 10, (float) pos, low_pos, high_pos);
}

/*---------------------------------------------------------------------------
 * This routine just displays the number number in integer format at 
 * row, column of the parameter list and puts the string string just above it.
 * If last is FALSE, it puts a comma after the number.
 *---------------------------------------------------------------------------
 */
print_int (row, column, string, number, last)
int	row,
	column;
char	string[];
int	number;
Boolean last;
{
    int	x, y;
    static buffer[20];

    x = 15 + 15 * column;
    y = 6 - row;

    /* This displays the number in the highlighted color if it is the current
       parameter. */

    if (parameter == column && line == row)
        color (HIGHCOLOR);
    else
        color (NORMCOLOR);

    cmov2i (x, y + 1);
    charstr (string);

    sprintf (buffer, "%6d", number);
    cmov2i (x, y);
    charstr (buffer);

    color (NORMCOLOR);

    if (last)

#ifdef FORTRAN
        charstr (" )");
#else
    charstr (" );");
#endif

else
    charstr (",");
}

/*---------------------------------------------------------------------------
 * This routine modifies the paramters for the pmv2i() and pdr2i() commands
 * according to the current setting of the slider bar (dx).
 *---------------------------------------------------------------------------
 */
modify_poly () {

    switch (saveval) {
    case 1:
        pdrx2 = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
        break;

    case 2:
        pdry2 = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
        break;

    case 3:
        pdrx1 = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
        break;

    case 4:
        pdry1 = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
        break;

    case 5:
        pmvx = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
        break;

    case 6:
        pmvy = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
        break;

    default:
        break;
    }
}

/*---------------------------------------------------------------------------
 * This routine modifies the paramters for the move2i() and draw2i() commands
 * according to the slider bar value. (dx)
 *---------------------------------------------------------------------------
 */
modify_line () {

    switch (saveval) {
    case 1:
        drawx = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
        break;

    case 2:
        drawy = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
        break;

    case 3:
        movex = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
        break;

    case 4:
        movey = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
        break;

    default:
        break;
    }
}

/*---------------------------------------------------------------------------
 * Modify the paramters for the pnt2i() command.
 *---------------------------------------------------------------------------
 */
modify_point () {

    switch (parameter) {
    case 1:
        pointx = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
        break;

    case 2:
        pointy = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
        break;

    default:
        break;
    }
}

/*---------------------------------------------------------------------------
 * Modify the paramters for the arci() and arcfi() commands.
 *---------------------------------------------------------------------------
 */
modify_arc () {

    switch (parameter) {
    case 1:
        arcx = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
        break;

    case 2:
        arcy = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
        break;

    case 3:
        arcrad = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
        break;

    case 4:
        arcstart = low_pos + (Angle) dx + (Angle) (wrap * 300.0);
        break;

    case 5:
        arcend = low_pos + (Angle) dx + (Angle) (wrap * 300.0);
        break;

    default:
        break;
    }
}

/*---------------------------------------------------------------------------
 * Modify the paramters for the circi() and circfi() commands.
 *---------------------------------------------------------------------------
 */
modify_circ () {
    switch (parameter) {
    case 1:
        circx = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
        break;

    case 2:
        circy = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
        break;

    case 3:
        circrad = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
        break;

    default:
        break;
    }
}

/*---------------------------------------------------------------------------
 * Modify the paramters for the recti() and rectfi() commands.
 *---------------------------------------------------------------------------
 */
modify_rect () {
    switch (parameter) {
    case 1:
        rectx1 = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
        break;

    case 2:
        recty1 = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
        break;

    case 3:
        rectx2 = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
        break;

    case 4:
        recty2 = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
        break;

    default:
        break;
    }
}


/*---------------------------------------------------------------------------
 * Draw the ruler lines that run allong the side of the 'GRAPH' window.
 *---------------------------------------------------------------------------
 */
draw_scale () {

    int	i;

    color (AXESCOLOR);

    for (i = 100; i < 500; i = i + 200) {
        move2i (i, 0);
        draw2i (i, 5);
        move2i (0, i);
        draw2i (5, i);
    }

    for (i = 200; i <= 400; i = i + 200) {
        move2i (i, 0);
        draw2i (i, 10);
        move2i (0, i);
        draw2i (10, i);
    }

    cmov2i (2, 2);
    charstr ("0");

    cmov2i (189, 15);
    charstr ("200");

    cmov2i (15, 195);
    charstr ("200");

    cmov2i (389, 15);
    charstr ("400");

    cmov2i (15, 395);
    charstr ("400");
}

/*---------------------------------------------------------------------------
 * Reset all of the parameters to their default values.
 *---------------------------------------------------------------------------
 */
reset_values () {

    rectx1 = 200;
    recty1 = 200;
    rectx2 = 400;
    recty2 = 400;

    circx = 300;
    circy = 300;
    circrad = 100;

    arcx = 200;
    arcy = 200;
    arcrad = 200;
    arcstart = 0;
    arcend = 900;

    pointx = 300;
    pointy = 300;

    movex = 200;
    movey = 200;
    drawx = 400;
    drawy = 400;

    pmvx = 200;
    pmvy = 200;
    pdrx1 = 300;
    pdry1 = 400;
    pdrx2 = 400;
    pdry2 = 200;
}


/*---------------------------------------------------------------------------
 * Given a paramter slesction number, this routine sets line and paramter to
 * the correct values.
 *---------------------------------------------------------------------------
 */
select_parameter (newparm)
int	newparm;
{
    int	row;

    switch (shapetype) {

    case POLY:
        row = ((newparm - 1) / 2) + 1;

        if (row == 1)
            line = 4;

        else if (row == 2)
            line = 3;

        else if (row == 3)
            line = 2;

        saveval = newparm;
        parameter = ((newparm - 1) % 2) + 1;
        break;

    case MOVEDRAW:
        row = ((newparm - 1) / 2) + 1;

        if (row == 1)
            line = 5;

        else
            line = 3;

        saveval = newparm;
        parameter = ((newparm - 1) % 2) + 1;
        break;

    case POINT:
    case ARCFILL:
    case ARC:
    case CIRCFILL:
    case CIRC:
    case RECTFILL:
    case RECT:
        line = ROW;
        parameter = newparm;
        break;

    default:
        break;
    }
}

/*---------------------------------------------------------------------------
 * This routine sets up the pop-up menus.
 *---------------------------------------------------------------------------
 */
setupmenus () {
    /* Exit confirmation menu. */
    exitmenu = defpup("Exit Confirmation %t|Yes|No");

    /* Primary menu. */
    primmenu = defpup("Shape %t");

#ifdef FORTRAN
    addtopup(primmenu, "call pmv2i/call pdr2i");
    addtopup(primmenu, "call move2i/call draw2i");
    addtopup(primmenu, "call pnt2i");
    addtopup(primmenu, "call arcfi");
    addtopup(primmenu, "call arci");
    addtopup(primmenu, "call circfi");
    addtopup(primmenu, "call circi");
    addtopup(primmenu, "call rectfi");
    addtopup(primmenu, "call recti");
#else
    addtopup(primmenu, "pmv2i()/pdr2i()");
    addtopup(primmenu, "move2i()/draw2i()");
    addtopup(primmenu, "pnt2i()");
    addtopup(primmenu, "arcfi()");
    addtopup(primmenu, "arci()");
    addtopup(primmenu, "circfi()");
    addtopup(primmenu, "circi()");
    addtopup(primmenu, "rectfi()");
    addtopup(primmenu, "recti()");
#endif
    addtopup(primmenu, "Reset parameters");
    addtopup(primmenu, "Exit");
}

/*---------------------------------------------------------------------------
 * This routine initializes the event queue.
 *---------------------------------------------------------------------------
 */
setupqueue () {
    qdevice (RIGHTMOUSE);
    qdevice (LEFTMOUSE);
    qdevice (INPUTCHANGE);
    qdevice (REDRAW);
    qdevice (ESCKEY);
}

/*---------------------------------------------------------------------------
 * This routine makes sure that there are enough bitplanes to run the program,
 * and if so, it save the current colors for later restoration and makes it's
 * own color map.
 *---------------------------------------------------------------------------
 */
setupcolors() {

    if (getplanes() < 4) {
        printf("You do not have enough bitplanes for this program\n");
        gexit();
        exit(0);
    }

    tutorsavemap();
    tutormakemap();
}

/*---------------------------------------------------------------------------
 * Initialize the four windows for the program.
 *---------------------------------------------------------------------------
 */
init_windows() {

    /* Each init_ routine returns the window ID of the window that was opened.
     * This ID number is used with winattach(ID) and winset(ID) to permit the
     * program to draw into the window.*/

    backw = init_back();

    /* All windows must run in the same mode, the first window brought up must
       be initialized with that mode. */
    doublebuffer();
    gconfig();

    /* The program sould come up attached. */
    winattach();

    helpw = init_help("Shape");
    graphw = init_graph();
    consw = init_cons();
    statw = init_status();
}

/*---------------------------------------------------------------------------
 * Initialize the graph window.
 *---------------------------------------------------------------------------
 */
init_graph() {

    int	res;

    /* Don't prompt the user to position the window, just bring it up on the
       screen.  This is done with the prefposition() command. */

    prefposition(5, 505, 75, 575);

    /* winopen() returns the window ID number, used in the winattach() and
       winselect() commands. */

    res = winopen("graph");
    wintitle("Shape -- GRAPH");

    /* winconstraints() simply ignores all of the window constraint commands 
       issued before the winopen() command, prefposition() in this case, 
       and replaces them with all of the commands issued after the 
       winopen() command, keepaspect() in this case. */

    keepaspect(1, 1);
    winconstraints();

    setup_graph_environ();
    return(res);
}


/*---------------------------------------------------------------------------
 * Initialize the status window.
 *---------------------------------------------------------------------------
 */
init_status() {

    int	res;

    prefposition(522, 1022, 480, 580);
    res = winopen("status");
    wintitle("Shape -- STATUS");
    prefsize(500, 100);
    winconstraints();
    setup_status_environ();

    return(res);
}


/*---------------------------------------------------------------------------
 * Initialize the console window.
 *---------------------------------------------------------------------------
 */
init_cons() {
    int	res;

    prefposition(522, 1022, 609, 735);
    res = winopen("cons");
    wintitle("Shape -- CONTROL BAR");
    prefsize(500, 126);
    winconstraints();
    setup_cons_environ();

    return(res);
}

/*---------------------------------------------------------------------------
 * Set up the STATUS window viewing matrix.
 *---------------------------------------------------------------------------
 */
setup_status_environ() {

    reshapeviewport();
    ortho2(0.0, 120.0, 0.0, 6.0);
}

/*---------------------------------------------------------------------------
 * Set up the CONTROL BAR window viewing matrix.
 *---------------------------------------------------------------------------
 */
setup_cons_environ() {

    reshapeviewport();
    ortho2(-250.0, 250.0, -63.0, 63.0);
}

/*---------------------------------------------------------------------------
 * Set up the GRAPH window viewing matrix.
 *---------------------------------------------------------------------------
 */
setup_graph_environ() {

    reshapeviewport();
    ortho2(0.0, 500.0, 0.0, 500.0);
}


/*---------------------------------------------------------------------------
 * Direct graphics output to the STATUS window.
 *---------------------------------------------------------------------------
 */
attach_to_status() {
    winset(statw);
}


/*---------------------------------------------------------------------------
 * Direct graphics output to the GRAPH window.
 *---------------------------------------------------------------------------
 */
attach_to_graph() {
    winset(graphw);
}


/*---------------------------------------------------------------------------
 * Direct graphics output to the CONSOLE window.
 *---------------------------------------------------------------------------
 */
attach_to_cons() {
    winset(consw);
}


/*---------------------------------------------------------------------------
 * Routine called when a redraw token for the HELP window is sent down the 
 * event queue.
 *---------------------------------------------------------------------------
 */
redraw_help() {

    draw_help();
    swapbuffers();
    draw_help();
}

/*---------------------------------------------------------------------------
 * Routine called when a redraw token for the GRAPH window is sent down the 
 * event queue.
 *---------------------------------------------------------------------------
 */
redraw_graph() {
    attach_to_graph();

    reshapeviewport();
    draw_graph_window();
    swapbuffers();
    draw_graph_window();
}


/*---------------------------------------------------------------------------
 * Routine called when a redraw token for the STATUS window is sent down the 
 * event queue.
 *---------------------------------------------------------------------------
 */
redraw_status() {
    attach_to_status();

    reshapeviewport();
    draw_status_window();
    swapbuffers();
    draw_status_window();
}

/*---------------------------------------------------------------------------
 * Routine called when a redraw token for the CONTROL BAR window is sent 
 * down the event queue.
 *---------------------------------------------------------------------------
 */
redraw_cons() {
    attach_to_cons();

    reshapeviewport();
    draw_cons_window();
    swapbuffers();
    draw_cons_window();
}

/*---------------------------------------------------------------------------
 * Routine to draw the GRAPH window.
 *---------------------------------------------------------------------------
 */
draw_graph_window () {

    attach_to_graph();

    color(BLACK);
    clear();

    pushmatrix ();
	ortho2(0.0, 500.0, 0.0, 500.0);

	draw_scale ();

	color (OBJECTCOLOR);
	switch (shapetype) {
	case POLY:
	    pmv2i (pmvx, pmvy);
	    pdr2i (pdrx1, pdry1);
	    pdr2i (pdrx2, pdry2);
	    pclos ();
	    break;

	case MOVEDRAW:
	    move2i (movex, movey);
	    draw2i (drawx, drawy);
	    break;

	case POINT:
	    pnt2i (pointx, pointy);
	    linewidth (2);
	    move2i (pointx + 25, pointy);
	    rdr2i (-15, 0);
	    rdr2i (5, 3);
	    rmv2i (-5, -3);
	    rdr2i (5, -3);
	    move2i (pointx - 25, pointy);
	    rdr2i (15, 0);
	    rdr2i (-5, 3);
	    rmv2i (5, -3);
	    rdr2i (-5, -3);
	    move2i (pointx, pointy + 25);
	    rdr2i (0, -15);
	    rdr2i (3, 5);
	    rmv2i (-3, -5);
	    rdr2i (-3, 5);
	    move2i (pointx, pointy - 25);
	    rdr2i (0, 15);
	    rdr2i (3, -5);
	    rmv2i (-3, 5);
	    rdr2i (-3, -5);
	    linewidth (1);
	    break;

	case ARCFILL:
	    arcfi (arcx, arcy, arcrad, arcstart, arcend);
	    break;

	case ARC:
	    arci (arcx, arcy, arcrad, arcstart, arcend);
	    break;

	case CIRCFILL:
	    circfi (circx, circy, circrad);
	    break;

	case CIRC:
	    circi (circx, circy, circrad);
	    break;

	case RECTFILL:
	    rectfi (rectx1, recty1, rectx2, recty2);
	    break;

	case RECT:
	    recti (rectx1, recty1, rectx2, recty2);
	    break;

	default:
	    break;
	}

    popmatrix ();
}


/*---------------------------------------------------------------------------
 * Routine to draw the COTROL BAR window.
 *---------------------------------------------------------------------------
 */
draw_cons_window() {

    attach_to_cons();
    ortho2(-250.0, 250.0, -63.0, 63.0);

    color(BLACK);
    clear();

    color(UNPICKCOLOR);

    /* Print the name and the type of parameter being worked on. */
    switch (shapetype) {
    case POLY:
        cmov2i(-100, 35);
        charstr ("  filled polygon");
        cmov2i(-100, 15);

        switch (saveval) {
        case 1:
            charstr ("  polygon draw x");
            draw_slider_int (pdrx2);
            break;

        case 2:
            charstr ("  polygon draw y");
            draw_slider_int (pdry2);
            break;

        case 3:
            charstr ("  polygon draw x");
            draw_slider_int (pdrx1);
            break;

        case 4:
            charstr ("  polygon draw y");
            draw_slider_int (pdry1);
            break;

        case 5:
            charstr ("  polygon move x");
            draw_slider_int (pmvx);
            break;

        case 6:
            charstr ("  polygon move y");
            draw_slider_int (pmvy);
            break;

        default:
            charstr ("  Controller Bar   ");
            draw_blank_slider (0.0, 0.0, 100, 10);
            break;
        }
        break;

    case MOVEDRAW:
        cmov2i(-100, 35);
        charstr ("      line");
        cmov2i(-100, 15);

        switch (saveval) {
        case 1:
            charstr ("      draw x");
            draw_slider_int (drawx);
            break;

        case 2:
            charstr ("      draw y");
            draw_slider_int (drawy);
            break;

        case 3:
            charstr ("      move x");
            draw_slider_int (movex);
            break;

        case 4:
            charstr ("      move y");
            draw_slider_int (movey);
            break;

        default:
            charstr ("  Controller Bar   ");
            draw_blank_slider (0.0, 0.0, 100, 10);
            break;
        }
        break;

    case POINT:
        cmov2i(-100, 35);
        charstr ("     point");
        cmov2i(-100, 15);

        switch (parameter) {
        case 1:
            charstr ("         x");
            draw_slider_int (pointx);
            break;

        case 2:
            charstr ("         y");
            draw_slider_int (pointy);
            break;

        default:
            charstr ("  Controller Bar   ");
            draw_blank_slider (0.0, 0.0, 100, 10);
            break;
        }
        break;

    case ARCFILL:
    case ARC:
        cmov2i(-100, 35);

        if (shapetype == ARCFILL)
            charstr ("   filled arc");
        else
            charstr ("  unfilled arc");

        cmov2i(-100, 15);

        switch (parameter) {
        case 1:
            charstr ("         x");
            draw_slider_int (arcx);
            break;

        case 2:
            charstr ("         y");
            draw_slider_int (arcy);
            break;

        case 3:
            charstr ("      radius");
            draw_slider_int (arcrad);
            break;

        case 4:
            charstr ("  starting angle");
            draw_slider_int (arcstart);
            break;

        case 5:
            charstr ("   ending angle");
            draw_slider_int (arcend);
            break;

        default:
            charstr ("  Controller Bar   ");
            draw_blank_slider (0.0, 0.0, 100, 10);
            break;
        }
        break;

    case CIRCFILL:
    case CIRC:
        cmov2i(-100, 35);

        if (shapetype == CIRCFILL)
            charstr ("  filled circle");
        else
            charstr (" unfilled circle");

        cmov2i(-100, 15);

        switch (parameter) {
        case 1:
            charstr ("         x");
            draw_slider_int (circx);
            break;

        case 2:
            charstr ("         y");
            draw_slider_int (circy);
            break;

        case 3:
            charstr ("      radius");
            draw_slider_int (circrad);
            break;

        default:
            charstr ("  Controller Bar   ");
            draw_blank_slider (0.0, 0.0, 100, 10);
            break;
        }
        break;

    case RECTFILL:
    case RECT:
        cmov2i(-100, 35);

        if (shapetype == RECTFILL)
            charstr (" filled rectangle");
        else
            charstr ("unfilled rectangle");

        cmov2i(-100, 15);

        switch (parameter) {
        case 1:
            charstr ("        x1");
            draw_slider_int (rectx1);
            break;

        case 2:
            charstr ("        y1");
            draw_slider_int (recty1);
            break;

        case 3:
            charstr ("        x2");
            draw_slider_int (rectx2);
            break;

        case 4:
            charstr ("        y2");
            draw_slider_int (recty2);
            break;

        default:
            charstr ("  Controller Bar   ");
            draw_blank_slider (0.0, 0.0, 100, 10);
            break;
        }
        break;

    default:
        charstr ("  Controller Bar   ");
        draw_blank_slider (0.0, 0.0, 100, 10);
        break;
    }
}


/*---------------------------------------------------------------------------
 * Draw the whole screen.
 *---------------------------------------------------------------------------
 */
draw_frame() {
    load_correct_help();
    draw_help();
    draw_graph_window();
    draw_cons_window();
    draw_status_window();
}


/*---------------------------------------------------------------------------
 * Redraw window n.
 *---------------------------------------------------------------------------
 */
redraw_window(n)
int	n;
{

    if (n == helpw)
        redraw_help();

    else if (n == consw)
        redraw_cons();

    else if (n == graphw)
        redraw_graph();

    else if (n == statw)
        redraw_status();

    else if (n == backw)
        redraw_back();
}


/*---------------------------------------------------------------------------
 * Load the help box with the correct message according to queue.
 *---------------------------------------------------------------------------
 */
load_correct_help() {

    if (!active) {
        load_attach_help();
        return;
    }

    load_help ("Use the LEFT MOUSE to adjust the highlighted", 
	       "parameter with the CONSOLE controller bar OR",
	       "to select a parameter from the STATUS window.", 
	       "OR use the RIGHT MOUSE for a popup menu.", 
	       "", "");
}


/*---------------------------------------------------------------------------
 * Load the help box with attach stuff.
 *---------------------------------------------------------------------------
 */
load_attach_help() {

    load_help("In order to use this program:", 
        "  You must attach to it by", 
        "  pressing the RIGHT MOUSE button and", 
        "  selecting ATTACH from the menu.", "", "");
}
