/*  zbuffer.c							*/
/*  April, 1986 -- Mason Woo					*/
/*  Learning Environment -- to demonstrate z-buffering hidden	*/
/*  surface removal and to compare it to no hidden surface	*/
/*  removal whatsoever.						*/

#include <gl.h>
#include <device.h>
#include "tutor.h"

#define  NOFILL	0
#define  FLATFILL 1
#define  ZFILL  2

int	whitecolor,
	ffacecolor, 
	bfacecolor, 
	sidecolor;
float   low_pos,
        high_pos;
float   wrap;
long    dx;
int     holding = FALSE;
int	line = 0;
int	parameter = 0;
int	xrot, yrot, zrot;
int     menu,
        killmenu;
short oldcolors[24][3];
int	fillstate;
int     active;
int	backwin,
	helpwin,
	conswin,
	statuswin,
	topwin,
	sidewin,
	viewwin;

main () {
    short   qval;
    long    dev;
    short   chosen;
    short   pickme (), pickval;

    if(!ismex()){
	printf("You must be running the window manager to use this program.\n");
	printf("Type mex to start.\n");
	exit(0);
    }

    active = FALSE;
    init_windows();
    setdepth(0xc000, 0x3fff);
    setupcolors ();
    setupqueue ();
    reset_values ();
    setupmenus ();
    load_correct_help();

    while (1) {
	while (qtest ()) {
	    dev = qread (&qval);
	    switch (dev) {
		case ESCKEY:
		    gexit(); exit();
		    break;
		case LEFTMOUSE: 
		    if (qval) {
			pickval = pickme();
			if ((pickval > 0) && (pickval < 4)) {
			    parameter = pickval;/* pick parameter   */
			    draw_status_window ();
			    draw_cons_window ();
			    load_correct_help();
			}
			else if (pickval != 0)
			    holding = qval;	/* adjust parameter */
		    }
		    else
			holding = FALSE;
		    break;
		case RIGHTMOUSE: 
		    if (qval)
			switch (dopup (menu)) {
			case 1:
			    fillstate = ZFILL;
			    load_correct_help();
			    draw_frame ();
			    break;
			case 2:
			    fillstate = FLATFILL;
			    load_correct_help();
			    draw_frame ();
			    break;
			case 3:
			    reset_values();
			    break;
			case 4:
			    if (dopup (killmenu) == 1) {
				tutorrestoremap();
				gexit ();
				exit (0);
			    }
			    break;
			default:
			    break;
			}
		    break;
		case REDRAW: 
		    redraw_window (qval);
		    break;
		case INPUTCHANGE: 
		    active = qval;
		    if (active == FALSE) {
			fillstate = NOFILL;
			load_correct_help();
			draw_frame ();
		    }
		    else 
			load_correct_help();
		    break;
		default: 
		    break;
	    }
	    if (!active)
		while (!qtest ())
		    ;
	}
	change_parameter();
	if (holding) {
	    fillstate = NOFILL;
	    load_correct_help();
	    draw_frame ();
	}
	
    }
}

setupqueue () {
    qdevice (LEFTMOUSE);
    qdevice (RIGHTMOUSE);
    qdevice (INPUTCHANGE);
    qdevice (REDRAW);
    qdevice (ESCKEY);
}

setupcolors () {
    int i;

    if (getplanes () < 10) {	/*  if less than 10 bitplanes in mex	 */
	printf("You do not have enough bitplanes for z-buffering\n");
	exit(0);
    }
    tutorsavemap();
    tutormakemap();

    whitecolor = WHITE;
	ffacecolor = 16;
	bfacecolor = 17;
	sidecolor = 18;
	mapcolor (ffacecolor, 0, 225, 225);
	mapcolor (bfacecolor, 0, 150, 150);

	mapcolor (sidecolor, 225, 150, 0);
	mapcolor (sidecolor+1, 225, 0, 150);
	mapcolor (sidecolor+2, 225, 0, 0);
	mapcolor (sidecolor+3, 150, 225, 0);
	mapcolor (sidecolor+4, 150, 0, 225);

}

/*  when values are reset, draw entire screen again		*/

reset_values () {
    fillstate = FALSE;
    xrot = 0;
    yrot = 0;
    zrot = 0;
    draw_frame();
}

setupmenus () {
    killmenu = defpup ("Do you want to exit? %t|yes|no");

    menu = defpup ("Zbuffer %t|z-buffer filled object");
    addtopup(menu, "fill object without z-buffer");
    addtopup(menu, "reset values|exit program");
}

short   pickme () {
    short   buffer[100];
    long    numpicked;
    int     i;

    attach_to_status();
    pushmatrix ();
    pick (buffer, 100);
    ortho2 (0.0, 40.0, 0.0, 7.0);
    initnames ();

    for (i = 0; i < 3; i++) {
	loadname (i + 1);
	rectfi (0, 6 - (2 * i), 40, 5 - (2 * i));
    }

    numpicked = endpick (buffer);
    popmatrix ();

    if (numpicked)
	return (buffer[1]);
    else {
	attach_to_cons();
	pushmatrix ();
	pick (buffer, 100);
	ortho2 (-250.0, 250.0, -63.0, 63.0);
	initnames ();
	loadname (4);
	rectfi (-150, -5, 149, 5);    /*  controller bar		*/
	numpicked = endpick (buffer);
	popmatrix ();

	if (numpicked)
	    return (buffer[1]);
	else 
	    return (0);
    }
}

change_parameter () {

    long mx, my;
    long ox, oy;
    long sx, sy;
    Coord wx, wy;
    long iwx;

    wrap = 0.0;

    attach_to_cons();
    if (holding) {
	getsize(&sx, &sy);
	getorigin(&ox, &oy);
    	mx = getvaluator(MOUSEX);

	iwx = mx - (ox + sx/2);

	if (iwx < -150) {
	    iwx += 300;
	    setvaluator(MOUSEX, iwx+ox+sx/2, 0, 1023);
	    wrap = -1.0;
	}
	else if (iwx > 149) {
	    iwx -= 300;
	    setvaluator(MOUSEX, iwx+ox+sx/2, 0, 1023);
	    wrap = 1.0;
	}
	dx = (iwx+150);
	if (parameter != 0)
	    modify_rot ();
    }
}

print_int (row, number)
int     row;
int     number;
{
    int     x,
            y;
    static  buffer[20];

    if (parameter == row)
	color (HIGHCOLOR);
    else
	color (NORMCOLOR);

    sprintf (buffer, "%6d", number);
    charstr (buffer);
    color (NORMCOLOR);
}


modify_rot () {
    switch (parameter) {
	case 1: 
	    xrot = low_pos + (float) dx + (wrap * 300.0);
	    break;
	case 2: 
	    yrot = low_pos + (float) dx + (wrap * 300.0);
	    break;
	case 3: 
	    zrot = low_pos + (float) dx + (wrap * 300.0);
	    break;
	default: 
	    break;
    }
}

draw_fedges(deep) 
float deep;
{
    horizpoly(-.6, -1., -.2, deep);	/*  A to J lower corner		*/
    vertpoly(-.6, -1., 1., deep);	/*  A to B left side		*/
    horizpoly(-.6, 1., .6, deep);	/*  B to C top edge		*/
    vertpoly(.6, 1., .6, deep);		/*  C to D right edge		*/
    horizpoly(.6, .6, -.2, deep);	/*  D to E lower part of top	*/
    vertpoly(-.2, .6, .2, deep);	/*  E to F upright above mid 	*/
    horizpoly(-.2, .2, .2, deep);	/*  F to G upper part of mid 	*/
    vertpoly(.2, .2, -.2, deep);	/*  G to H upright right of mid */
    horizpoly(.2, -.2, -.2, deep);	/*  H to I lower part of mid 	*/
    vertpoly(-.2, -.2, -1., deep);	/*  I to J upright below mid 	*/
    color(AXESCOLOR);			/*  draw axes			*/
    move (0.0, 0.0, 0.0); draw(0.0, 0.0, 1.5);
    cmov(0.0, 0.0, 1.55); charstr("Z");
    move (0.0, 0.0, 0.0); draw(0.0, 1.5, 0.0);
    cmov(0.0, 1.55, 0.0); charstr("Y");
    move (0.0, 0.0, 0.0); draw(1.5, 0.0, 0.0);
    cmov(1.55, 0.0, 0.0); charstr("X");
}

horizpoly(x, y, nx, deep)
float x, y, nx, deep;
{
    move(x, y, deep);
    draw(x, y, -deep);
    draw(nx, y, -deep);
    draw(nx, y, deep);
    draw(x, y, deep);
}

vertpoly(x, y, ny, deep)
float x, y, ny, deep;
{
    move(x, y, deep);
    draw(x, y, -deep);
    draw(x, ny, -deep);
    draw(x, ny, deep);
    draw(x, y, deep);
}

outlines(deep) 
float deep;
{
    color(bfacecolor);
    pmv(-.6, -1., -deep);    /*  draw F upright poly at z=-deep	*/
    pdr(-.6, 1., -deep);
    pdr(-.2, 1., -deep);
    pdr(-.2, -1., -deep);
    pclos();
    pmv(-.2, .6, -deep);	    /*  draw F topbar poly at z=-deep	*/
    pdr(-.2, 1., -deep);
    pdr(.6, 1., -deep);
    pdr(.6, .6, -deep);
    pclos();
    pmv(-.2, -.2, -deep);    /*  draw F middlebar poly at z=-deep	*/
    pdr(-.2, .2, -deep);
    pdr(.2, .2, -deep);
    pdr(.2, -.2, -deep);
    pclos();
    color(ffacecolor);
    pmv(-.6, -1., deep);    /*  draw F upright poly at z=deep 	*/
    pdr(-.6, 1., deep);
    pdr(-.2, 1., deep);
    pdr(-.2, -1., deep);
    pclos();
    pmv(-.2, .6, deep);	    /*  draw F topbar poly at z=deep 	*/
    pdr(-.2, 1., deep);
    pdr(.6, 1., deep);
    pdr(.6, .6, deep);
    pclos();
    pmv(-.2, -.2, deep);    /*  draw F middlebar poly at z=deep 	*/
    pdr(-.2, .2, deep);
    pdr(.2, .2, deep);
    pdr(.2, -.2, deep);
    pclos();
}

sidefill(deep) 
float deep;
{
    color(sidecolor);
    horizfill(-.6, -1., -.2, deep);	/*  A to J lower corner		*/
    color(sidecolor+1);
    vertfill(-.6, -1., 1., deep);	/*  A to B left side		*/
    color(sidecolor+2);
    horizfill(-.6, 1., .6, deep);	/*  B to C top edge		*/
    color(sidecolor+3);
    vertfill(.6, 1., .6, deep);		/*  C to D right edge		*/
    color(sidecolor+4);
    horizfill(.6, .6, -.2, deep);	/*  D to E lower part of top	*/
    color(sidecolor);
    vertfill(-.2, .6, .2, deep);	/*  E to F upright above mid 	*/
    color(sidecolor+1);
    horizfill(-.2, .2, .2, deep);	/*  F to G upper part of mid 	*/
    color(sidecolor+2);
    vertfill(.2, .2, -.2, deep);	/*  G to H upright right of mid */
    color(sidecolor+3);
    horizfill(.2, -.2, -.2, deep);	/*  H to I lower part of mid 	*/
    color(sidecolor+4);
    vertfill(-.2, -.2, -1., deep);	/*  I to J upright below mid 	*/
}

horizfill(x, y, nx, deep)
float x, y, nx, deep;
{
    pmv(x, y, deep);
    pdr(x, y, -deep);
    pdr(nx, y, -deep);
    pdr(nx, y, deep);
    pclos();
}

vertfill(x, y, ny, deep)
float x, y, ny, deep;
{
    pmv(x, y, deep);
    pdr(x, y, -deep);
    pdr(x, ny, -deep);
    pdr(x, ny, deep);
    pclos();
}

init_windows()
/*---------------------------------------------------------------------------
 * Initialize the six windows for the program.
 *---------------------------------------------------------------------------
 */
{
    backwin = init_back();
    singlebuffer();
    gconfig();
    helpwin = init_help("Zbuffer -- INFORMATION");
    winattach();
    conswin = init_cons();
    statuswin = init_status();
    topwin = init_top();
    sidewin = init_side();
    viewwin = init_view();
}

init_cons()
/*---------------------------------------------------------------------------
 * Initialize the console window.
 *---------------------------------------------------------------------------
 */
{
    int res;

    prefposition(522, 1022, 609, 735);
    res = winopen("cons");
    wintitle("Zbuffer -- CONSOLE");
    prefsize(500, 126);
    winconstraints();
    setup_cons_environ();

    return(res);
}

init_status()
/*---------------------------------------------------------------------------
 * Initialize the status window.
 *---------------------------------------------------------------------------
 */
{
    int res;
    prefposition(618, 918, 465, 585);
    res = winopen("status");
    wintitle("Zbuffer -- STATUS");
    prefsize(300, 120);
    winconstraints();
    setup_status_environ();

    return(res);
}

init_top()
/*---------------------------------------------------------------------------
 * Initialize the top window.
 *---------------------------------------------------------------------------
 */
{
    int res;
    prefposition(135, 385, 325, 575);
    res = winopen("top");
    wintitle("Zbuffer -- TOP VIEW");
    prefsize(250, 250);
    winconstraints();
    setup_top_environ();

    return(res);
}

init_side()
/*---------------------------------------------------------------------------
 * Initialize the side window.
 *---------------------------------------------------------------------------
 */
{
    int res;
    prefposition(135, 385, 25, 275);
    res = winopen("side");
    wintitle("Zbuffer -- SIDE VIEW");
    prefsize(250, 250);
    winconstraints();
    setup_side_environ();

    return(res);
}

init_view()
/*---------------------------------------------------------------------------
 * Initialize the view window.
 *---------------------------------------------------------------------------
 */
{
    int res;
    prefposition(570, 970, 25, 425);
    res = winopen("view");
    wintitle("Zbuffer -- MAIN VIEW");
    prefsize(400, 400);
    winconstraints();
    setup_view_environ();

    return(res);
}

setup_cons_environ()
/*---------------------------------------------------------------------------
 * Set the console environment.
 *---------------------------------------------------------------------------
 */
{
    reshapeviewport();
    ortho2 (-250.0, 250.0, -63.0, 63.0);
}

setup_status_environ()
/*---------------------------------------------------------------------------
 * Set up the environment used for the status window.
 *---------------------------------------------------------------------------
 */
{
    reshapeviewport();
    ortho2(0.0, 40.0, 0.0, 7.0);
}

setup_top_environ()
/*---------------------------------------------------------------------------
 * Set up the environment used for the top window.
 *---------------------------------------------------------------------------
 */
{
    reshapeviewport();
    perspective(450, 1.00, 2.5, 7.5);
    polarview(5.0, 0, -900, 0);
}

setup_side_environ()
/*---------------------------------------------------------------------------
 * Set up the environment used for the side window.
 *---------------------------------------------------------------------------
 */
{
    reshapeviewport();
    perspective(450, 1.00, 2.5, 7.5);
    polarview(5.0, 900, 900, 0);
    rotate (900, 'x');
}

setup_view_environ()
/*---------------------------------------------------------------------------
 * Set up the environment used for the view window.
 *---------------------------------------------------------------------------
 */
{
    reshapeviewport();
    perspective(450, 1.00, 2.5, 7.5);
    polarview(5.0, 0, 0, 0);
}

attach_to_cons()
/*---------------------------------------------------------------------------
 * Direct graphics output to the console window.
 *---------------------------------------------------------------------------
 */
{
    winset(conswin);
}

attach_to_status()
/*---------------------------------------------------------------------------
 * Attach to the status window.
 *---------------------------------------------------------------------------
 */
{
    winset(statuswin);
}

attach_to_top()
/*---------------------------------------------------------------------------
 * Attach to the top window.
 *---------------------------------------------------------------------------
 */
{
    winset(topwin);
}

attach_to_side()
/*---------------------------------------------------------------------------
 * Attach to the side window.
 *---------------------------------------------------------------------------
 */
{
    winset(sidewin);
}

attach_to_view()
/*---------------------------------------------------------------------------
 * Attach to the view window.
 *---------------------------------------------------------------------------
 */
{
    winset(viewwin);
}

redraw_cons()
/*---------------------------------------------------------------------------
 * Routine called in the event of a redraw token for the cons window coming
 * down the event status.
 *---------------------------------------------------------------------------
 */
{
    attach_to_cons();
    reshapeviewport();
    draw_cons_window();
}

redraw_status()
/*---------------------------------------------------------------------------
 * Routine called when a redraw token is sent down the queue.
 *---------------------------------------------------------------------------
 */
{
    attach_to_status();
    reshapeviewport();
    draw_status_window ();
}

redraw_top()
/*---------------------------------------------------------------------------
 * Routine called when a redraw token is sent down the top.
 *---------------------------------------------------------------------------
 */
{
    attach_to_top();
    reshapeviewport();
    draw_top_window();
}


redraw_side()
/*---------------------------------------------------------------------------
 * Routine called when a redraw token is sent down the side.
 *---------------------------------------------------------------------------
 */
{
    attach_to_side();
    reshapeviewport();
    draw_side_window();
}


redraw_view()
/*---------------------------------------------------------------------------
 * Routine called when a redraw token is sent down the view.
 *---------------------------------------------------------------------------
 */
{
    attach_to_view();
    reshapeviewport();
    draw_view_window();
}

/*---------------------------------------------------------------------------
 * Redraw the HELP window.
 *---------------------------------------------------------------------------
 */
redraw_help()
{
    reshapeviewport();
    draw_help();
}


redraw_window(n)
/*---------------------------------------------------------------------------
 * Redraw window n.
 *---------------------------------------------------------------------------
 */
int n;
{
    if (n == helpwin)    
	redraw_help();
    else if (n == conswin)
	redraw_cons();
    else if (n == statuswin)
	redraw_status();
    else if (n == topwin)
	redraw_top();
    else if (n == sidewin)
	redraw_side();
    else if (n == viewwin)
	redraw_view();
    else if (n == backwin)
	redraw_back();
}

draw_frame()
/*---------------------------------------------------------------------------
 * Draw the whole screen.
 *---------------------------------------------------------------------------
 */
{
    draw_cons_window();
    draw_status_window();
    draw_top_window();
    draw_side_window();
    draw_view_window();
}

draw_cons_window()
/*---------------------------------------------------------------------------
 * Draw everything in the console window.
 *---------------------------------------------------------------------------
 */
{
    attach_to_cons();
    ortho2(-250.0, 250.0, -63.0, 63.0);
    color(BLACK);
    clear();
    color(UNPICKCOLOR);
    cmov2i(-100, 15);

    switch (parameter) {
	case 1: 
	    charstr ("    x rotation    ");
	    draw_slider_int (xrot);
	    break;
	case 2: 
	    charstr ("    y rotation    ");
	    draw_slider_int (yrot);
	    break;
	case 3: 
	    charstr ("    z rotation    ");
	    draw_slider_int (zrot);
	    break;
	default: 
	    charstr ("  Controller Bar   ");
	    draw_ruling (100, 10);
	    break;
    }
}

draw_status_window()
/*---------------------------------------------------------------------------
 * Draw everything in the status window.
 *---------------------------------------------------------------------------
 */
{
    attach_to_status();
    color(BLACK);
    clear();

    color (NORMCOLOR);
    cmov2i (5, 5);
#ifdef FORTRAN
    charstr ("CALL ROTATE (");
#else
    charstr ("     rotate (");
#endif
    print_int (1, xrot);
#ifdef FORTRAN
    charstr (", 'x')");
#else
    charstr (", 'x');");
#endif

    cmov2i (5, 3);
#ifdef FORTRAN
    charstr ("CALL ROTATE (");
#else
    charstr ("     rotate (");
#endif
    print_int (2, yrot);
#ifdef FORTRAN
    charstr (", 'y')");
#else
    charstr (", 'y');");
#endif

    cmov2i (5, 1);
#ifdef FORTRAN
    charstr ("CALL ROTATE (");
#else
    charstr ("     rotate (");
#endif
    print_int (3, zrot);
#ifdef FORTRAN
    charstr (", 'z')");
#else
    charstr (", 'z');");
#endif
}

draw_top_window()
/*---------------------------------------------------------------------------
 * Draw everything in the top view window.
 *---------------------------------------------------------------------------
 */
{
    attach_to_top ();
    color (BLACK);
    clear ();

    pushmatrix ();
    rotate(xrot, 'x');
    rotate(yrot, 'y');
    rotate(zrot, 'z');
    if (fillstate == NOFILL) {
	color(whitecolor);
	draw_fedges(.75);
    }
    else if (fillstate == FLATFILL) {
	sidefill(.75);
	outlines(.75);
    }
    else if (fillstate == ZFILL) {
	zbuffer(TRUE);
	zclear();
	sidefill(.75);
	outlines(.75);
	zbuffer(FALSE);
    }
    popmatrix ();
}

draw_side_window()
/*---------------------------------------------------------------------------
 * Draw everything in the side view window.
 *---------------------------------------------------------------------------
 */
{
    attach_to_side ();
    color (BLACK);
    clear ();

    pushmatrix ();
    rotate(xrot, 'x');
    rotate(yrot, 'y');
    rotate(zrot, 'z');
    if (fillstate == NOFILL) {
	color(whitecolor);
	draw_fedges(.75);
    }
    else if (fillstate == FLATFILL) {
	sidefill(.75);
	outlines(.75);
    }
    else if (fillstate == ZFILL) {
	zbuffer(TRUE);
	zclear();
	sidefill(.75);
	outlines(.75);
	zbuffer(FALSE);
    }
    popmatrix ();
}

draw_view_window()
/*---------------------------------------------------------------------------
 * Draw everything in the main view window.
 *---------------------------------------------------------------------------
 */
{
    attach_to_view ();
    color (BLACK);
    clear ();

    pushmatrix ();
    rotate(xrot, 'x');
    rotate(yrot, 'y');
    rotate(zrot, 'z');
    if (fillstate == NOFILL) {
	color(whitecolor);
	draw_fedges(.75);
    }
    else if (fillstate == FLATFILL) {
	sidefill(.75);
	outlines(.75);
    }
    else if (fillstate == ZFILL) {
	zbuffer(TRUE);
	zclear();
	sidefill(.75);
	outlines(.75);
	zbuffer(FALSE);
    }
    popmatrix ();
}

draw_slider_int (pos)
short   pos;
{

    if ((pos < 0) || ((pos == 0) && (low_pos < 0))) {
	high_pos = (float) ((pos / 300) * 300);
	low_pos = high_pos - 299.0;
	pos = pos - (int) low_pos + 1;
    }
    else {
	low_pos = (float) ((pos / 300) * 300);
	high_pos = low_pos + 299.0;
	pos -= (int) low_pos;
    }

    draw_int_slider(0.0, 0.0, 100, 10, (float)pos, low_pos, high_pos);
}

load_correct_help(message)
/*---------------------------------------------------------------------------
 * Load the help box with the correct message according to queue.
 *---------------------------------------------------------------------------
 */
int message;
{
    if (!active) {
	load_help ("Press the RIGHT MOUSE button and attach",
		"to the program from the popup menu.", "", "", "", "");
    }

    else if (getdisplaymode()==2) {
	load_help ("Z-buffering is not compatible with",
		   "double-buffering.",
                   "exit menu program then re-exucte", "", "", "");
    }

    else if (parameter == 0) {	/*  no parameter chosen yet	*/
	    load_help ("Use the LEFT MOUSE to select a parameter",
			"from the STATUS window, OR use the RIGHT MOUSE",
			"to select choices from the popup menu to fill", 
			"the polygons in the wireframe F with and", 
			"without zbuffering", "");
    }
    else if (fillstate) {	/*  F is filled		*/
	    load_help ("If you use the LEFT MOUSE to change the value", 
			"associated with the CONSOLE controller bar, the",
			"object will go back to wireframe.  Or you can",
			"use the RIGHT MOUSE button to select a choice", 
			"from the popup menu to fill the object with and",
			"without zbuffering.", "");
    }
    else {			/*  !fillstate	*/
	    load_help ("Use the LEFT MOUSE to adjust the highlighted", 
			"parameter with the CONSOLE controller bar OR",
			"to select a parameter from the STATUS window", 
			"OR use the RIGHT MOUSE for a popup menu to", 
			"fill the object with and without zbuffering", "");
    }
    draw_help();
}

