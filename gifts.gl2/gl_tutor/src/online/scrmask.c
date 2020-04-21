/*---------------------------------------------------------------------------
 *  scrmask.c
 *  May, 1986 -- Mason Woo
 *  Learning Environment -- to demonstrate coarse and fine clipping using
 *  the scrmask and viewport commands.
 *---------------------------------------------------------------------------
 */

#include <gl.h>
#include <device.h>
#include "tutor.h"

float   low_pos,	/* Current low value returned by the controler bar
			   when the knob is as far left as it will go.  This 
			   value will change as the slider knob wraps around
			   to the other side of the slider.		*/

        high_pos;	/* Current high value ...			*/

float   wrap;		/* This variable is set to 1.0 if the slider
			   needs to wrap to the right, and -1.0 if it
			   needs to wrap to the left.			*/

long    dx;		/* This is the calculated setting of the
			   controller bar from reading the x coordinate of
			   the mouse.				    */

int     holding = FALSE;    /* This is set to true if the LEFT MOUSE button is
			       pressed on top of the slider bar, and stays
			       true util the LEFT MOUSE button is released.*/

int	line = 0;	/* The line number in the status window of the 
			   current parameter.				*/

int	parameter = 0;	/* The column of number in the status window of the 
			   currently selected parameter.		*/

			/* These are the screen coordinates for the scrmask
			   command. */
Screencoord sleft, sright, sbottom, stop;

			/* Ditto for the viewport command. */
Screencoord vleft, vright, vbottom, vtop;

int     menu,		/* Place for the ID number of the main popup menu. */
        killmenu;	/* Ditto for the kill confirmation menu. */

Boolean scron;		/* FALSE if the program is using the default scrmask*/

int	backwin,	/* These are the ID numbers for the various windows */
	helpwin,
	viewwin,
	statuswin,
	conswin;

int     active;		/* TRUE if this program is attached. */

main () {
    short   qval;
    long    dev;
    short   chosen;
    short   pickme (), pickval;
    int     really;

    /* If mex isn't running, then the program won't work. */

    check_for_mex();

    active = FALSE;
    init_windows();

    setupcolors();
    setupqueue ();
    reset_values ();
    setupmenus ();
    load_correct_help (7);	/* Make sure an appropriate help message is 
				   displayed. */

    while (1) {

	color (BLACK);
	clear ();

	/* Loop until the queue is empty. */

	while (qtest ()) {
	    dev = qread (&qval);
	    switch (dev) {

		case LEFTMOUSE: 
		    if (qval) {

			/* pickme() returns a value that coresponds to 
			   the parameter or function the mouse cursor
			   is on.  If the value doesn't corespond to a
			   parameter and it isn't zero then it
			   is assumed that the slider bar knob was
			   grabbed.  Else they let go of the button.  */

			pickval = pickme();
			if ((pickval > 0) && (pickval < 9))
			    select_parameter(pickval);
			else if (pickval != 0)
			    holding = qval;
		    }
		    else
			holding = FALSE;
		    break;

		case RIGHTMOUSE: 
		    if (qval) {

			/* The rightmouse brings up a popup menu. */

			remake_menu();
			switch (dopup (menu)) {

			    /* toggle between default and
			       user controlled screenmask */
			    case 1:
				scron = !scron;
				break;

			    /* Selected reset. */
			    case 2:
				reset_values();
				break;

			    /* Selected exit. Do confirm menu. */
			    case 3:
			        really = dopup (killmenu);
			        if (really == 1) {
				    tutorrestoremap();
				    gexit ();
				    exit (0);
				}
				break;

			    default:
				break;
			}
		    }
		    break;

		case REDRAW: 

		    /* A redraw token is generated when a window has been 
		       uncovered or reshaped and it needs redrawing.  */

		    redraw_window(qval);
		    break;

		case INPUTCHANGE:
		    active = qval;
		    if (!active) {

			/* When the user selects a window belonging to
			   another process, the window manager tells the
			   program it is on hold by sending it an
			   INPUTCHANGE with qval == FALSE.  
			   Draw everything in both buffers and
			   swap buffers until the user re-attaches and an
			   INPUTCHANGE with qval == TRUE is on the queue. */

			draw_frame();
			swapbuffers();
			draw_frame();
		    }
		    break;

		case ESCKEY:
		    tutorrestoremap();
		    gexit();
		    exit(0);

		default: 
		    break;
	    }

	    /* Twiddle thumbs while on hold.  It is important to swap buffers
	       because the computer won't swap buffers until all of the 
	       graphics processes have gotten to a swapbuffers() command. */

	    if (!active)
		while (!qtest ())
		    swapbuffers ();
	}
	change_parameter();

	draw_frame();
	swapbuffers();
    }
}

/*---------------------------------------------------------------------------
 * This routine informs the computer of the devices (buttons, valuators, etc)
 * that we will be using for interaction.
 *---------------------------------------------------------------------------
 */
setupqueue () 
{
    qdevice (LEFTMOUSE);
    qdevice (RIGHTMOUSE);
    qdevice (INPUTCHANGE);
    qdevice (REDRAW);
    qdevice (ESCKEY);
}

/*---------------------------------------------------------------------------
 * This routine first saves the current values of all of the color indicies
 * that it will use.  And the creates it's own colors for the program
 * according to how many bitplanes are available on the machine.
 *---------------------------------------------------------------------------
 */
setupcolors ()
{
    if (getplanes() < 4) {
	printf("You do not have enough bitplanes for this program\n");
	gexit();
	exit(0);
    }

    tutorsavemap();
    tutormakemap();
}

/*---------------------------------------------------------------------------
 * This routine sets all of the adjustable parameters in the program back to
 * their default values.
 *---------------------------------------------------------------------------
 */
reset_values ()
{
    sleft = 100;
    sright = 800;
    sbottom = 100;
    stop = 300;
    vleft = 50;
    vright = 850;
    vbottom = 50;
    vtop = 350;
    scron = TRUE;
}

/*---------------------------------------------------------------------------
 * This routine defines what all of the menus in the program wil say.
 *---------------------------------------------------------------------------
 */
setupmenus ()
{
    killmenu = defpup ("Exit Confirmation %t|Yes|No");
    menu = newpup ();
}

/*---------------------------------------------------------------------------
 * This makes the menus.
 *---------------------------------------------------------------------------
 */
remake_menu() 
{
    freepup (menu);
    menu = defpup ("Scrmask %t");

    if (scron)
	addtopup (menu, "Use default screenmask");
    else
	addtopup (menu, "Choose screenmask values");
    addtopup (menu, "Reset parameters|Exit");
}

/*---------------------------------------------------------------------------
 * This routine, given a paramter number, sets line to the line that the
 * parameter is on, and paramter to the paramter number on that line.
 *---------------------------------------------------------------------------
 */
select_parameter (newparm) 
int newparm;
{
    line = ((newparm - 1) / 4) + 1;
    parameter = ((newparm - 1) % 4) + 1;
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
short   pickme ()
{
    short   buffer[100];
    long    numpicked;
    int     i;

    attach_to_status();
    pushmatrix ();
    pick (buffer, 100);

	    /* The viewing transformation must be re-established because pick
	       destroys the current matrix. */

    ortho2 (0.0, 100.0, 0.0, 6.0);

    initnames ();

    for (i = 0; i < 4; i++) {	/*  viewport arguments		*/
	loadname (i + 1);
	rectfi (15 * (i + 2), 3, 15 * (i + 3), 5);
    }
    if (scron)
	for (i = 0; i < 4; i++) {/*  scrmask arguments		*/
	    loadname (i + 5);
	    rectfi (15 * (i + 2), 1, 15 * (i + 3), 3);
	}

    numpicked = endpick (buffer);
    popmatrix ();

    if (numpicked) {
	return (buffer[1]);
    } else {
	attach_to_cons();
	pushmatrix ();

	pick (buffer, 100);

	   /* The viewing transformation must be re-established because pick
	  destroys the current matrix. */

	ortho2 (-250.0, 250.0, -63.0, 63.0);
	initnames ();
	loadname (9);
	rectfi (-150, -5, 149, 5);    /*  controller bar		*/
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

    long mx, my;
    long ox, oy;
    long sx, sy;
    Coord wx, wy;
    long iwx;

    wrap = 0.0;

    attach_to_cons();
    if (holding) {

	/* Get the size and origin of the window relative to the screen. */
	getsize(&sx, &sy);
	getorigin(&ox, &oy);

	/* Get the x coordinate of the cursor (in screen coordinates). */
    	mx = getvaluator(MOUSEX);

	/* The corsor location relative to the console window. */
	iwx = mx - (ox + sx/2);

	/* Wrap the cursor around if needed. */
	if (iwx < -150) {
	    iwx += 300;
	    setvaluator(MOUSEX, iwx+ox+sx/2, 0, 1023);
	    wrap = -1.0;
	} else if (iwx > 149) {
	    iwx -= 300;
	    setvaluator(MOUSEX, iwx+ox+sx/2, 0, 1023);
	    wrap = 1.0;
	}

	dx = (iwx+150);
	if (line == 1)
	    modify_view ();
	else if ((line == 2) && scron)
	    modify_scr ();
    }
}

/*---------------------------------------------------------------------------
 * This routine just displays the number number in integer format at 
 * row, column of the parameter list.
 * If last is FALSE, it puts a comma after the number.
 *---------------------------------------------------------------------------
 */
print_int (row, column, string, number, last)
int     row,
        column;
char    string[];
int     number;
{
    int     x,
            y;
    static  buffer[20];

    x = 15 + 15 * column;
    y = 5 - 2 * row;

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
 * Modify the current parameter in the viewport command.
 *---------------------------------------------------------------------------
 */
modify_view ()
{
    switch (parameter) {

	case 1: 
	    vleft = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
	    break;

	case 2: 
	    vright = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
	    break;

	case 3: 
	    vbottom = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
	    break;

	case 4: 
	    vtop = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
	    break;

	default: 
	    break;
    }
}

/*---------------------------------------------------------------------------
 * Modify the current parameter in the scrmask command.
 *---------------------------------------------------------------------------
 */
modify_scr ()
{
    switch (parameter) {

	case 1: 
	    sleft = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
	    break;

	case 2: 
	    sright = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
	    break;

	case 3: 
	    sbottom = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
	    break;

	case 4: 
	    stop = low_pos + (Scoord) dx + (Scoord) (wrap * 300.0);
	    break;

	default: 
	    break;
    }
}

/*---------------------------------------------------------------------------
 * Draw the reference points.
 *---------------------------------------------------------------------------
 */
draw_refpts()
{
    draw_pt (200, 50);
    draw_pt (600, 50);
    draw_pt (50, 100);
    draw_pt (300, 100);
    draw_pt (700, 100);
    draw_pt (100, 150);
    draw_pt (550, 150);
    draw_pt (50, 200);
    draw_pt (300, 200);
    draw_pt (650, 200);
    draw_pt (250, 250);
    draw_pt (600, 250);
    draw_pt (100, 300);
    draw_pt (400, 300);
    draw_pt (750, 300);
    draw_pt (200, 350);
    draw_pt (500, 350);
}

/*---------------------------------------------------------------------------
 * Draw the reference point and its coordinates.
 *---------------------------------------------------------------------------
 */
draw_pt (x, y)
int x, y;
{
    static char buffer[20];

    cmov2i (x, y);
#ifdef FORTRAN
    sprintf (buffer, "call cmov2i (%4d, %4d)", x, y);
#else
    sprintf (buffer, "cmov2i (%4d, %4d)", x, y);
#endif
    charstr (buffer);
}


/*---------------------------------------------------------------------------
 * Initialize the four windows for the program.
 *---------------------------------------------------------------------------
 */
init_windows()
{
    backwin = init_back();
    doublebuffer();
    gconfig();
    winattach();
    helpwin = init_help("Scrmask");
    viewwin = init_view();
    conswin = init_cons();
    statuswin = init_status();

}

/*---------------------------------------------------------------------------
 * Initialize the view window.
 *---------------------------------------------------------------------------
 */
init_view()
{
    int res;
    prefposition(60, 960, 50, 450);
    res = winopen("view");
    prefsize (900, 400);
    wintitle("Scrmask -- VIEW");
    winconstraints();
    setup_view_environ();

    return(res);
}

/*---------------------------------------------------------------------------
 * Initialize the status window.
 *---------------------------------------------------------------------------
 */
init_status()
{
    int res;

    prefposition(522, 1022, 480, 580);
    res = winopen("status");
    wintitle("Scrmask -- STATUS");
    prefsize(500, 100);
    winconstraints();
    setup_status_environ();

    return(res);
}

/*---------------------------------------------------------------------------
 * Initialize the console window.
 *---------------------------------------------------------------------------
 */
init_cons()
{
    int res;

    prefposition(522, 1022, 609, 735);
    res = winopen("cons");
    wintitle("Scrmask -- CONTROL BAR");
    prefsize(500, 126);
    winconstraints();
    setup_cons_environ();

    return(res);
}

/*---------------------------------------------------------------------------
 * Set the console environment.
 *---------------------------------------------------------------------------
 */
setup_cons_environ()
{
    reshapeviewport();
    ortho2(-250.0, 250.0, -63.0, 63.0);
}

/*---------------------------------------------------------------------------
 * Set the status environment.
 *---------------------------------------------------------------------------
 */
setup_status_environ()
{
    reshapeviewport();
    ortho2(0.0, 100.0, 0.0, 6.0);
}

/*---------------------------------------------------------------------------
 * Set up the environment used for the view window.
 *---------------------------------------------------------------------------
 */
setup_view_environ()
{
    reshapeviewport();
    ortho2(0.0, 900.0, 0.0, 400.0);
}

/*---------------------------------------------------------------------------
 * Attach to the view window.
 *---------------------------------------------------------------------------
 */
attach_to_view()
{
    winset(viewwin);
}

/*---------------------------------------------------------------------------
 * Direct graphics output to the status window.
 *---------------------------------------------------------------------------
 */
attach_to_status()
{
    winset(statuswin);
}

/*---------------------------------------------------------------------------
 * Direct graphics output to the console window.
 *---------------------------------------------------------------------------
 */
attach_to_cons()
{
    winset(conswin);
}
/*---------------------------------------------------------------------------
 * Routine called when a redraw token is sent down the queue.
 *---------------------------------------------------------------------------
 */
redraw_help() {

    draw_help();
    swapbuffers();
    draw_help();
}

/*---------------------------------------------------------------------------
 * Routine called when a redraw token is sent down the queue.
 *---------------------------------------------------------------------------
 */
redraw_view()
{
    attach_to_view();
    reshapeviewport();
    draw_view_window();
    swapbuffers();
    draw_view_window();
}

/*---------------------------------------------------------------------------
 * Routine called when a redraw token is sent down the queue.
 *---------------------------------------------------------------------------
 */
redraw_status()
{
    attach_to_status();
    reshapeviewport();
    draw_status_window();
    swapbuffers();
    draw_status_window();
}

/*---------------------------------------------------------------------------
 * Routine called in the event of a redraw token for the cons window coming
 * down the event queue.
 *---------------------------------------------------------------------------
 */
redraw_cons()
{
    attach_to_cons();
    reshapeviewport();
    draw_cons_window();
    swapbuffers();
    draw_cons_window();
}

/*---------------------------------------------------------------------------
 * Draw everything in the view window.
 *---------------------------------------------------------------------------
 */
draw_view_window()
{
    int check_parameters();
    long ox, oy;

    attach_to_view();
    getorigin(&ox, &oy);
    color(BLACK);
    clear();

    if (check_parameters()) {
	pushmatrix ();
	    if (scron) {
		color (GREEN);
		rects (sleft, sbottom, sright, stop);
	    }
	    color (UNPICKCOLOR);
	    rects (vleft, vbottom, vright, vtop);

	    viewport (vleft, vright, vbottom, vtop);
	    ortho2 ((float) vleft, (float) vright,
		    (float) vbottom, (float) vtop);

	    if (scron) {
		scrmask (sleft, sright, sbottom, stop);
	    }

	    color (OBJECTCOLOR);
	    draw_refpts();

	popmatrix ();
	reshapeviewport();
    }
}

/*---------------------------------------------------------------------------
 * Draw everything in the status window.
 *---------------------------------------------------------------------------
 */
draw_status_window()
{
    attach_to_status();
    color(BLACK);
    clear();

    color (NORMCOLOR);
    cmov2i (5, 3);
#ifdef FORTRAN
    charstr ("call viewport (");
#else
    charstr ("viewport (");
#endif
    print_int (1, 1, "  left", vleft, FALSE);
    print_int (1, 2, " right", vright, FALSE);
    print_int (1, 3, "bottom", vbottom, FALSE);
    print_int (1, 4, "   top", vtop, TRUE);

    if (scron) {
	cmov2i (5, 1);
#ifdef FORTRAN
	charstr ("call scrmask  (");
#else
	charstr ("scrmask  (");
#endif
	print_int (2, 1, "  left", sleft, FALSE);
	print_int (2, 2, " right", sright, FALSE);
	print_int (2, 3, "bottom", sbottom, FALSE);
	print_int (2, 4, "   top", stop, TRUE);
    }

}

/*---------------------------------------------------------------------------
 * Draw everything in the console window.
 *---------------------------------------------------------------------------
 */
draw_cons_window()
{
    attach_to_cons();
    ortho2(-250.0, 250.0, -63.0, 63.0);
    color(BLACK);
    clear();
    color(UNPICKCOLOR);
    cmov2i(-100, 15);
    if (line == 1)
	switch (parameter) {

	    case 1: 
		charstr (" left - viewport  ");
		draw_slider_int (vleft);
		break;

	    case 2: 
		charstr (" right - viewport ");
		draw_slider_int (vright);
		break;

	    case 3: 
		charstr ("bottom - viewport ");
		draw_slider_int (vbottom);
		break;

	    case 4: 
		charstr ("  top - viewport  ");
		draw_slider_int (vtop);
		break;

	    default: 
		charstr ("  Controller Bar   ");
		draw_blank_slider(0.0, 0.0, 100, 10);
		break;
	}
    else if ((line == 2) && scron)
	switch (parameter) {

	    case 1: 
		charstr ("  left - scrmask  ");
		draw_slider_int (sleft);
		break;

	    case 2: 
		charstr ("  right - scrmask ");
		draw_slider_int (sright);
		break;

	    case 3: 
		charstr (" bottom - scrmask ");
		draw_slider_int (sbottom);
		break;

	    case 4: 
		charstr ("   top - scrmask  ");
		draw_slider_int (stop);
		break;

	    default: 
		charstr ("  Controller Bar   ");
		draw_blank_slider(0.0, 0.0, 100, 10);
		break;
	}
    else {
	charstr ("  Controller Bar   ");
	draw_blank_slider(0.0, 0.0, 100, 10);
    }

}

/*---------------------------------------------------------------------------
 * Draw the whole screen.
 *---------------------------------------------------------------------------
 */
draw_frame()
{
    draw_status_window();
    draw_cons_window();
    draw_view_window();
    draw_help();
}

/*---------------------------------------------------------------------------
 * Redraw window n.
 *---------------------------------------------------------------------------
 */
redraw_window(n)
int n;
{
    if (n == helpwin)    
	redraw_help();
    else if (n == conswin)
	redraw_cons();
    else if (n == statuswin)
	redraw_status();
    else if (n == viewwin)
	redraw_view();
    else if (n == backwin)
	redraw_back();
}

draw_slider_int(pos)
int pos;
{

    if ((pos < 0) || ((pos == 0) && (low_pos < 0))) {
	high_pos = (float) ((pos / 300) * 300);
	low_pos = high_pos - 299.0;
	pos = pos - (int) low_pos + 1;
    } else {
	low_pos = (float) ((pos / 300) * 300);
	high_pos = low_pos + 299.0;
	pos -= (int) low_pos;
    }

    draw_int_slider(0.0, 0.0, 100, 10, (float)pos, low_pos, high_pos);
}

/*---------------------------------------------------------------------------
 * Load the help box with the correct message according to queue.
 *---------------------------------------------------------------------------
 */
load_correct_help(message)
int message;
{
    if (!active) {
	load_help ("Press the RIGHT MOUSE button and attach",
		"to the program from the popup menu.", "", "", "", "");
	return ;
    }

    switch (message) {

	case 1:
	    load_help ("viewport--values for the left and right",
			"gross clipping planes eliminate the entire", 
			"area of the screen.  The left edge is not",
			"allowed to be greater than or equal to the",
			"right edge.", "");
	    break;

	case 2:
	    load_help ("viewport--values for the bottom and top",
			"gross clipping planes eliminate the entire",
			"area of the screen.  The bottom edge is not",
			"allowed to be greater than or equal to the",
			"top edge.", "");
	    break;

	case 3:
	    load_help ("scrmask--values for the left and right",
			"fine clipping planes eliminate the entire", 
			"area of the screen.  The left edge is not",
			"allowed to be greater than or equal to the",
			"right edge.", "");
	    break;

	case 4:
	    load_help ("scrmask--values for the bottom and top",
			"fine clipping planes eliminate the entire",
			"area of the screen.  The bottom edge is not",
			"allowed to be greater than or equal to the",
			"top edge.", "");
	    break;

	case 5:
	    load_help ("viewport parameters outside (1, 899, 1, 399)",
			"do not fit in this viewing window, but",
			"they ARE LEGAL AND OFTEN USEFUL",
			"in your own programs.", "", "");
	    break;

	case 6:
	    load_help ("scrmask parameters outside (1, 899, 1, 399)",
			"do not fit in this viewing window, but",
			"they ARE LEGAL AND OFTEN USEFUL",
			"in your own programs.", "", "");
	    break;

	case 7:
	    load_help ("Use the LEFT MOUSE to select a parameter",
			"from the STATUS window, OR use the RIGHT MOUSE",
			"to bring up the popup menu.", "", "", "");
	    break;

	case 8:
	    load_help ("Use the LEFT MOUSE to adjust the highlighted", 
			"parameter with the CONSOLE controller bar OR",
			"to select a parameter from the STATUS window.", 
			"OR use the RIGHT MOUSE for a popup menu.", 
			"", "");
	    break;

	default:
	    break;
    }
}

/*---------------------------------------------------------------------------
 * Examines all the values of all of the parameters and display an
 * appropriate message in the 'INFORMATION' window.
 *---------------------------------------------------------------------------
 */
int check_parameters()
{
    int toreturn;
    int earlyreturn;

    toreturn = TRUE;
    earlyreturn = FALSE;

    if ((sleft >= sright) || (sbottom >= stop)
	|| (vleft >= vright) || (vbottom >= vtop)) {
	toreturn = FALSE;	/*  error, blank out screen	*/
	earlyreturn = TRUE;
    }
    else if ((vleft < 1) || (vright > 899) || (vbottom < 1) || (vtop > 399) 
	|| (sleft < 1) || (sright > 899) || (sbottom < 1) || (stop > 399)) {
	toreturn = TRUE;	/*  warning, not error		*/
	earlyreturn = TRUE;
    }

    /*  check viewport parameters			*/
    if (vleft >= vright) {
	load_correct_help (1);
    } else if (vbottom >= vtop) {
	load_correct_help (2);
    /*  check scrmask parameters			*/
    } else if (sleft >= sright) {
	load_correct_help (3);
    }
    else if (sbottom >= stop) {
	load_correct_help (4);
    }
/*  warn about boundary conditions for viewport and scrmask		*/
    else if ((vleft < 1) || (vright > 899) || (vbottom < 1) || (vtop > 399)){
	load_correct_help (5);
    }
    else if ((sleft < 1) || (sright > 899) || (sbottom < 1) || (stop > 399)){
	load_correct_help (6);
    }

    if (earlyreturn)	/*  if message other than normal printed, exit early*/
	return (toreturn);

    if (parameter == 0) {	/*  no parameter chosen yet	*/
	load_correct_help (7);
    }
    else {
	load_correct_help (8);
    }
    return (TRUE);
}

