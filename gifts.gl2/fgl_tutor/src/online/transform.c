/*  transform.c							*/
/*  May, 1986 -- Mason Woo					*/
/*  Learning Environment -- to demonstrate modeling transfor-	*/
/*  mations:  rotate, translate and scale			*/

#include <gl.h>
#include <device.h>
#include "tutor.h"

#define XROT		3
#define YROT		2
#define ZROT		1
#define ROTATE		1
#define TRANSLATE	2
#define SCALE		3
#define X		0
#define Y		1
#define Z		2
#define ROW		3
#define CONTROL		10
#define RESET		4
#define EXIT		5
#define MAXPARAM	3	/* maximum # of parameters for longest command line */

float   low_pos,
        high_pos;
float   wrap;
long    dx;
int     holding = FALSE;
int 	comptrans = -1;
int	transtype = 1;
int	parameter = 0;
int     line = 0;
int     active = FALSE;
int	xrot, yrot, zrot;
float	xtrans, ytrans, ztrans;
float	xscale, yscale, zscale;
int     menu,
        killmenu;
short oldcolors[16][3];

int	backw,			/* background		*/
	helpw,			/* Help window.		*/
	consw,			/* Console window.	*/
	graphw,			/* Graph window.	*/
	statw,			/* Status window.	*/
	viewyw,
	viewxw,
	back;


main () {
    short   qval;
    long    dev;
    short   chosen;
    short   pickme (), pickval;

    if (!ismex ()) {
	printf("The projection program can only be run under the window manager.\n");
 	printf("Type `mex' to start the window manager.\n");
        exit ();
    }

    init_windows();

    setupcolors ();
    setupqueue ();
    reset_values ();
    setupmenus ();

    while (TRUE) {
	while (qtest ()) {
	    dev = qread (&qval);
	    switch (dev) {
		case LEFTMOUSE: 
		    if (qval) {
			pickval = pickme();
			if ((pickval > 0) && (pickval <= MAXPARAM)) {
			    select_parameter(pickval) ;/* pick parameter   */
			}
			else if (pickval != 0)
			    holding = qval;	/* adjust parameter */
		    }
		    else
			holding = FALSE;
		    break;
		case RIGHTMOUSE: 
		    if (qval) {
		        comptrans = dopup (menu);	
			if (comptrans != transtype && comptrans != RESET)
			    parameter = line = 0; 
			switch (comptrans) {
			case ROTATE:
			    transtype = ROTATE;
			    break;
			case TRANSLATE:
			    transtype = TRANSLATE;
			    break;
			case SCALE:
			    transtype = SCALE;
			    break;
			case RESET:
			    reset_values();
			    break;
			case EXIT:
			    if (dopup (killmenu) == 1) {
				tutorrestoremap();
				gexit ();
				exit (0);
			    }
			    break;
			default:
			    break;
			}
		    }	/* end if (qval) */
		    break;
		case ESCKEY: 
		    tutorrestoremap();
		    gexit ();
		    exit ();
		    break;

		case INPUTCHANGE: 
		    active = qval;
		    if (active == FALSE) {
			draw_frame ();
		    }
		    break;
		default: 
		    break;
	    }
	}
	change_parameter();
	draw_frame ();
    }
}

setupcolors() {


    if (getplanes() < 4) {
	printf("You do not have enough bitplanes for this program\n");
	gexit();
	exit(0);
    }

    tutorsavemap();
    tutormakemap();
}

setupqueue () {
    qdevice (LEFTMOUSE);
    qdevice (RIGHTMOUSE);
    qdevice (INPUTCHANGE);
    qdevice (REDRAW);
    qdevice (ESCKEY);
}


/*  when values are reset, draw entire screen again		*/
reset_values () {
    xrot = 0;
    yrot = 0;
    zrot = 0;
    xtrans = 0.0;
    ytrans = 0.0;
    ztrans = 0.0;
    xscale = 1.0;
    yscale = 1.0;
    zscale = 1.0;
}

setupmenus () {
    killmenu = defpup ("Exit Confirmation %t|Yes|No");

#ifdef FORTRAN
    menu = defpup ("Transform %t|CALL ROTATE|CALL TRANSL|CALL SCALE");
#else
    menu = defpup ("Transform %t|rotate()|translate()|scale()");
#endif

    addtopup(menu, "Reset parameters|Exit");
}

short   pickme () {
    short   buffer[100];
    long    numpicked;
    int     i, j;

    winset(statw);
    pushmatrix ();
    pick (buffer, 100);
    ortho2(0.0, 120.0, 0.0, 6.0);
    initnames ();

    if (transtype == ROTATE) {
	for (i = 0; i < MAXPARAM; i++) {
		loadname (i + 1);
		rectfi (25, (i*2), 60, (i + i + 2));
	}
    }
    else if ((transtype == TRANSLATE) || (transtype == SCALE))
	for (i = 0; i < MAXPARAM; i++) {  /*  translate/scale arguments	*/
	    loadname (i + 1);
	    rectfi (15 * (i + 2), 3, 15 * (i + 3), 5);
	}

    numpicked = endpick (buffer);
    popmatrix ();

    /* if nothing in the status window was picked check the controller bar */
    if (numpicked)
	return (buffer[1]);
    else {
	winset (consw);
	pushmatrix ();
	pick (buffer, 100);
    	ortho2(-250.0, 250.0, -63.0, 63.0);
	initnames ();
	loadname (CONTROL);
	rectfi (-150, -5, 149, 5);    /*  controller bar		*/
	numpicked = endpick (buffer);
	popmatrix ();

        if (numpicked)
	    return (buffer[1]);
	else
	    return (0);
    }
}

select_parameter (newparm) 
/*---------------------------------------------------------------------------
 * select the parameter as positioned in the status window 
 *---------------------------------------------------------------------------
 */
int newparm;
{
int row;

	switch (transtype) {
	case ROTATE:
	/* only one parameter per line in rotate */
    	    line = newparm;
    	    parameter = newparm; 
    	    break;
	case TRANSLATE:
	case SCALE:
    	    line = ROW;
    	    parameter = newparm; 
	    break;
	default:
	    break;
	}
}


change_parameter() {

    long mx, my;
    long ox, oy;
    long sx, sy;
    Coord wx, wy;
    long iwx;

    wrap = 0.0;

    winset(consw);
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

	switch (transtype) {
	case ROTATE:
	    modify_rot();
	    break;
	case TRANSLATE:
	    modify_trans();
	    break;
	case SCALE:
	    modify_scale();
	    break;
	default:
	    break;
	}
    }
}





draw_status_window () {

    color (NORMCOLOR);
    switch (transtype) {
    case ROTATE:
	cmov2i (5, 5);
#ifdef FORTRAN
	charstr ("CALL ROTATE (");
#else
	charstr ("rotate (");
#endif
	print_int (XROT, xrot);
#ifdef FORTRAN
	charstr (", 'x')");
#else
	charstr (", 'x');");
#endif
	cmov2i (5, 3);
#ifdef FORTRAN
	charstr ("CALL ROTATE (");
#else
	charstr ("rotate (");
#endif
	print_int (YROT, yrot);
#ifdef FORTRAN
	charstr (", 'y')");
#else
	charstr (", 'y');");
#endif
	cmov2i (5, 1);
#ifdef FORTRAN
	charstr ("CALL ROTATE (");
#else
	charstr ("rotate (");
#endif
	print_int (ZROT, zrot);
#ifdef FORTRAN
	charstr (", 'z')");
#else
	charstr (", 'z');");
#endif
    	break; 

    case TRANSLATE:
	cmov2i (5, 3);
#ifdef FORTRAN
	charstr ("CALL TRANSL (");
#else
	charstr ("translate (");
#endif
	print_float (3, 1, "   x", xtrans, FALSE);
	print_float (3, 2, "   y", ytrans, FALSE);
	print_float (3, 3, "   z", ztrans, TRUE);
   	break; 

    case SCALE:
	cmov2i (5, 3);
#ifdef FORTRAN
	charstr ("CALL SCALE (");
#else
	charstr ("    scale (");
#endif
	print_float (3, 1, "   x", xscale, FALSE);
	print_float (3, 2, "   y", yscale, FALSE);
	print_float (3, 3, "   z", zscale, TRUE);
  	break; 

    default:
	break;
    }
}

draw_cons_window () 
/*---------------------------------------------------------------------------
 * Draw everything in the console window.
 *---------------------------------------------------------------------------
 */
{
    winset (consw);
    ortho2(-250.0, 250.0, -63.0, 63.0);
    color(BLACK);
    clear();
    color(UNPICKCOLOR);


    switch (transtype) {
	case ROTATE: 
            cmov2i(-80, 35);
#ifdef FORTRAN
	    charstr ("  CALL ROTATE");
#else
	    charstr ("    rotate");
#endif
    	    cmov2i(-100, 15);
	    switch (parameter) {
		case XROT: 
		    charstr ("    x rotation    ");
		    draw_knob_int (xrot);
		    break;
		case YROT: 
		    charstr ("    y rotation    ");
		    draw_knob_int (yrot);
		    break;
		case ZROT: 
		    charstr ("    z rotation    ");
		    draw_knob_int (zrot);
		    break;
		default: 
		    charstr ("  Controller Bar   ");
		    draw_blank_slider (0.0, 0.0, 100, 10);
		    break;
	    }
	    break;
	case TRANSLATE: 
            cmov2i(-80, 35);
#ifdef FORTRAN
	    charstr (" CALL TRANSL");
#else
	    charstr ("  translate");
#endif
    	    cmov2i(-100, 15);
	    switch (parameter) {
		case 1: 
		    charstr ("  x translation   ");
		    draw_knob_float (xtrans);
		    break;
		case 2: 
		    charstr ("  y translation   ");
		    draw_knob_float (ytrans);
		    break;
		case 3: 
		    charstr ("  z translation   ");
		    draw_knob_float (ztrans);
		    break;
		default: 
		    charstr ("  Controller Bar   ");
		    draw_blank_slider (0.0, 0.0, 100, 10);
		    break;
	    }
	    break;
	case SCALE: 
            cmov2i(-80, 35);
#ifdef FORTRAN
	    charstr ("  CALL SCALE");
#else
	    charstr ("    scale");
#endif
    	    cmov2i(-100, 15);
	    switch (parameter) {
		case 1: 
		    charstr ("  x scale factor   ");
		    draw_knob_float (xscale);
		    break;
		case 2: 
		    charstr ("  y scale factor   ");
		    draw_knob_float (yscale);
		    break;
		case 3: 
		    charstr ("  z scale factor   ");
		    draw_knob_float (zscale);
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


draw_knob_int (pos)
short   pos;
{
    static char buffer[10];

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
   
    draw_int_slider (0.0, 0.0, 100, 10, (float) pos, low_pos, high_pos);

}


draw_knob_float(val)
float val;
{
    int pos;
    static char buffer[10];

    if (val < 0.0) {
	high_pos = (float) (-0.01 + (int) ((val + 0.01) / 3.0) * 3);
	low_pos = high_pos - 2.99;
	pos = (int) ( (val - low_pos) * 100.0 );
    } else {
        low_pos = (float) ( (int) (val/3.0) * 3 );
	high_pos = low_pos + 2.99;
	pos = (int) ( (val - low_pos) * 100.0 );
    }

    draw_slider(0.0, 0.0, 100, 10, (float) pos, low_pos, high_pos);
}

print_float(row, column, string, number, last)
int row, column;
char string[];
float number;
Boolean last;	/* last parameter in function (print comma?) */
{
    int x, y;
    static buffer[20];

    x = 15 + 15 * column;
    y = 6 - row;

    if (parameter==column && line==row) 
	color(HIGHCOLOR);
    else 
	color(NORMCOLOR);
	
    cmov2i(x, y+20); charstr(string);
    sprintf(buffer, "%6.2f", number);
    cmov2i(x, y); charstr(buffer);
    color(NORMCOLOR);
#ifdef FORTRAN
    if (last) charstr(" )");
#else
    if (last) charstr(" );");
#endif
    else charstr(",");
}

print_int(row, number)
int row, number;
{
    static buffer[20];

    if (line == row) 
	color(HIGHCOLOR);
    else 
	color(NORMCOLOR);
	
    sprintf(buffer, "%6d", number);
    charstr (buffer);
    color(NORMCOLOR);
}


modify_rot () {

    switch (parameter) {
	case XROT: 
	    xrot = low_pos + (Angle) dx + (Angle) (wrap * 300.0);
	    break;
	case YROT: 
	    yrot = low_pos + (Angle) dx + (Angle) (wrap * 300.0);
	    break;
	case ZROT: 
	    zrot = low_pos + (Angle) dx + (Angle) (wrap * 300.0);
	    break;
	default: 
	    break;
    }
}

modify_trans() {

    switch (parameter) {
	case 1:
	    xtrans = low_pos + (Coord) dx / 100.0 + (Coord) (wrap * 3.0);
	    break;
	case 2:
	    ytrans = low_pos + (Coord) dx / 100.0 + (Coord) (wrap * 3.0);
	    break;
	case 3:
	    ztrans = low_pos + (Coord) dx / 100.0 + (Coord) (wrap * 3.0);
	    break;
	default:
	    break;
    }
}

modify_scale() {

    switch (parameter) {
	case 1:
	    xscale = low_pos + (Coord) dx / 100.0 + (Coord) (wrap * 3.0);
	    break;
	case 2:
	    yscale = low_pos + (Coord) dx / 100.0 + (Coord) (wrap * 3.0);
	    break;
	case 3:
	    zscale = low_pos + (Coord) dx / 100.0 + (Coord) (wrap * 3.0);
	    break;
	default:
	    break;
    }
}

draw_top_view() {

    pushmatrix ();
/* test with perspective ()
    ortho(-2.5, 2.5, -2.5, 2.5, 1.1, 10000.0);
*/
    perspective(450, 1.0, 0.01, 100.0);
    polarview(5.0, 0, -900, 0);

    if (transtype == ROTATE) {
	rotate(xrot, 'x');
	rotate(yrot, 'y');
	rotate(zrot, 'z');
    }
    else if (transtype == TRANSLATE) 
	translate (xtrans, ytrans, ztrans);
    else if (transtype == SCALE) 
	scale (xscale, yscale, zscale);

    color(OBJECTCOLOR);
    draw_fedges(.75);
    popmatrix ();

}

draw_side_view() {

    pushmatrix ();
/* test with perspective ()
    ortho(-2.5, 2.5, -2.5, 2.5, 1.1, 10000.0);
*/
    perspective(450, 1.0, 0.01, 100.0);
    polarview(5.0, 900, 900, 0);
    rotate (900, 'x');	/*  to make it upright	*/

    if (transtype == ROTATE) {
	rotate(xrot, 'x');
	rotate(yrot, 'y');
	rotate(zrot, 'z');
    }
    else if (transtype == TRANSLATE) 
	translate (xtrans, ytrans, ztrans);
    else if (transtype == SCALE) 
	scale (xscale, yscale, zscale);

    color(OBJECTCOLOR);
    draw_fedges(.75);
    popmatrix ();
}

draw_graph_window () {

    pushmatrix ();
/* test with perspective ()
    ortho(-2.5, 2.5, -2.5, 2.5, 1.1, 10000.0);
*/
    perspective(400, 1.00, 0.01, 100.0);
    polarview(5.0, 0, 0, 0);

    if (transtype == ROTATE) {
	rotate(xrot, 'x');
	rotate(yrot, 'y');
	rotate(zrot, 'z');
    }
    else if (transtype == TRANSLATE) 
	translate (xtrans, ytrans, ztrans);
    else if (transtype == SCALE) 
	scale (xscale, yscale, zscale);

    color(OBJECTCOLOR);
    draw_fedges(.75);
    popmatrix ();
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


init_windows()
/*---------------------------------------------------------------------------
 * Initialize all of the windows to be used.
 *---------------------------------------------------------------------------
 */
{
    backw = init_back();
    doublebuffer();
    gconfig();
    winattach(backw);
    helpw = init_help("Transform -- INFORMATION");
    statw = init_stat();
    consw = init_cons();
    graphw = init_graph();
    viewxw = init_view(X);
    viewyw = init_view(Y);
}

init_stat()
/*---------------------------------------------------------------------------
 * Initialize the status window.
 *---------------------------------------------------------------------------
 */
{
    int res;

    prefposition(516, 1016, 487, 587);
    res = winopen("status");
    wintitle("Transform -- STATUS");
    prefsize(500, 100);
    winconstraints();
    ortho2(0.0, 120.0, 0.0, 6.0);

    return(res);
}

init_cons()
/*---------------------------------------------------------------------------
 * Initialize the console window.
 *---------------------------------------------------------------------------
 */
{
    int res;

    prefposition(516, 1016, 609, 735);
    res = winopen("cons");
    wintitle("Transform -- CONTROL BAR");
    prefsize(500, 100);
    winconstraints();
    ortho2(-250.0, 250.0, -63.0, 63.0);

    return(res);
}

init_graph()
/*---------------------------------------------------------------------------
 * Initialize the graph window.
 *---------------------------------------------------------------------------
 */
{
    int res;

    prefposition(530, 980, 5, 455);
    res = winopen("graph");
    wintitle("Transform -- VIEWPORT");
    keepaspect(1, 1);
    winconstraints();

    return(res);
}

init_view(n)
/*---------------------------------------------------------------------------
 * Initialize the view window, X or Y.
 *---------------------------------------------------------------------------
 */
int n;
{
    int res;

    switch(n){
    case X:
	prefposition(85, 363, 5, 283);
	res = winopen("X");
	wintitle("Transform -- DOWN X AXIS");
	keepaspect(1, 1);
	winconstraints();
	break;
    case Y:
	prefposition(85, 363, 307, 585);
	res = winopen("Y");
	wintitle("Transform -- DOWN Y AXIS");
	keepaspect(1, 1);
	winconstraints();
	break;
    }

    return(res);
}

draw_frame()
/*---------------------------------------------------------------------------
 * Draw all of the windows.
 *---------------------------------------------------------------------------
 */
{
    draw_back();
    load_correct_help();
    draw_help();	/* library function called after load_corr_help() */
    draw_state();
    draw_cons();
    draw_graph();
    draw_view(X);
    draw_view(Y);

    swapbuffers();
}

draw_state()
/*---------------------------------------------------------------------------
 * Draw the status window.
 *---------------------------------------------------------------------------
 */
{
    winset(statw);
    color(BLACK);
    clear();
    draw_status_window ();
}

draw_cons()
/*---------------------------------------------------------------------------
 * Draw the console window.
 *---------------------------------------------------------------------------
 */
{
    winset(consw);
    color(BLACK);
    clear();
    draw_cons_window();
}

draw_graph()
/*---------------------------------------------------------------------------
 * Draw the graph window.
 *---------------------------------------------------------------------------
 */
{
    winset(graphw);
    color(BLACK);
    clear();
    reshapeviewport();
    draw_graph_window();
}

draw_view(n)
/*---------------------------------------------------------------------------
 * Draw the view window, X or Y.
 *---------------------------------------------------------------------------
 */
int n;
{
    switch(n){
    case X:
	winset(viewxw);
	color(BLACK);
	clear();
	reshapeviewport();
	draw_side_view();
	break;
    case Y:
	winset(viewyw);
	color(BLACK);
	clear();
	reshapeviewport();
	draw_top_view();
	break;
    }
}

load_correct_help()
/*---------------------------------------------------------------------------
 * Load the help box with the correct message according to queue.
 *---------------------------------------------------------------------------
 */
{
    if (!active) {
    load_help("In order to use this program:", 
	      "  You must attach to it by", 
	      "  pressing the RIGHT MOUSE button and", 
	      "  selecting ATTACH from the menu.", "", "");
	return;
    }

    load_help ("Use the LEFT MOUSE to adjust the highlighted", 
	"parameter with the CONSOLE controller bar OR",
	"to select a parameter from the STATUS window.", 
	"OR press the RIGHT MOUSE for a popup menu", 
	"to select ROTATE, TRANSLATE, or SCALE.", "");
}

