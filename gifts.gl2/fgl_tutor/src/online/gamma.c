/*  gamma.c							*/
/*  May, 1986 -- Mason Woo					*/
/*  Learning Environment -- to demonstrate gamma correction	*/
/*  and its use to create non-linear color ramps		*/

#include "tutor.h"
#include <stdio.h>
#include <gl.h>
#include <math.h>
#include <device.h>

#define RAMPS 1
#define GRAPH 2
#define CONTRAN	    3

static float    gammac = 1.0;

static  Scoord gpoints[4][2] = {
	142, 155, 142, 200, 462, 200, 462, 155
};
static  Scoord lpoints[4][2] = {
	142, 55, 142, 100, 462, 100, 462, 55
};
float low_pos, high_pos;
long dx;
float wrap;
int holding = FALSE;
int     menu,
        killmenu;

int	helpw,			/* Help window.		*/
	consw,			/* Console window.	*/
	graphw, 		/* Graph window.	*/
	backw;

short oldcolors[48][3];
int   active = FALSE;

main ()
{

    Coord i,j;
    short   val;
    long    dev;
    short mx, my;
    short   pickme ();
    int   resetgam(), really;

    if(!ismex()){
	printf("You must be running the window manager to use this program.\n");
	printf("Type mex to start.\n");
	exit(0);
    }

    init_windows();

    setupcolors();
    makeobjects();
    setupqueue ();
    setupmenus ();

    while(TRUE) {
	while(qtest()) {
	    dev=qread(&val);
	    switch(dev) {
		case LEFTMOUSE :
		    if (val) {
			if (pickme()){
			    holding = val;  /* adjust parameter */
			}
		    } 
		    else
			holding = FALSE;
		    break;
		case RIGHTMOUSE: 
		    if (val)
			switch (dopup (menu)) {
			case 1:
			    gammac = 1.0;
			    gamma_spectrum (gammac, 32, 16, 
				255, 255, 255, 0, 0, 0);
			    break;
			case 2:
			    really = dopup (killmenu);
			    if (really == 1) {
				restore_colors();
				gexit ();
				exit (0);
			    }
			    break;
			default:
			    break;
			}
		    break;
		case INPUTCHANGE:
		    active = val;
		    if (!active) {
			draw_frame();
			swapbuffers();
			draw_frame();
		    }
		    break;
		case ESCKEY:
		    restore_colors();
		    gexit ();
		    exit (0);
		    break;
		case REDRAW:
		    redraw_window(val);
		    break;
		default:
		    break;
	    }
	    if (!active)
		while (!qtest ())
		    swapbuffers ();
	}
	change_parameter();

	draw_frame();
	swapbuffers();
    }
}

short   pickme () {
    short   buffer[100];
    long    numpicked;
    int     i;
    int pos;
    float val;

    val = gammac;

    attach_to_cons();
    pushmatrix ();
	pick (buffer, 100);
	callobj(CONTRAN);
	initnames ();
	rectfi(-150, -3, 149, 3);
	numpicked = endpick (buffer);
    popmatrix ();

    if (numpicked)
	return (buffer[1]);
    else
	return (0);
}

print_float(string, number)
char string[];
float number;
{
    static buffer[20];

    color(UNPICKCOLOR);

    charstr(string);
    sprintf(buffer, "%6.2f", number);
    charstr(buffer);
}

change_parameter() {

    long mx, my;
    long ox, oy;
    long sx, sy;
    Coord wx, wy;
    long iwx;

    wrap = 0.0;

    if (holding) {
	winset(consw);
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
	modify_gamma();
    }
}

modify_gamma() {
    float   k;

    gammac = low_pos + (Coord) dx / 100.0 + (Coord) (wrap * 3.0);
    if (gammac < 0.05) 
	k = 0.05;
    else
	k = gammac;
    gamma_spectrum (k, 32, 16, 255, 255, 255, 0, 0, 0);
}

draw_newline () {
    int     i;
    float   j;
    float   k;

    if (gammac < 0.05) 
	k = 0.05;
    else
	k = gammac;
    color (YELLOW);
    move2i (162, 300);
    for (i = 1; i < 16; i++) {
	j = pow ((float) i / 16.0, 1.0 / k) * 300.0 + 300.0;
	draw2 (162.0 + (i * 300.0 / 16.0), j);
    }
    draw2i (462, 600);
}

makeobjects () {
    int     i,
            j;

    makeobj (RAMPS);
	for (i = 0; i < 16; i++) {
	    color (i + 16);
	    rectfi (142 + (i * 20), 55, 162 + (i * 20), 100);
	}
	color (GREEN);
	poly2s (4, lpoints);
	cmov2i (230, 30);
	charstr ("Linear Color Ramp");

	for (i = 0; i < 16; i++) {
	    color (i + 32);
	    rectfi (142 + (i * 20), 155, 162 + (i * 20), 200);
	}
	color (YELLOW);
	poly2s (4, gpoints);
	cmov2i (180, 130);
	charstr ("Gamma-Corrected Color Ramp");

    closeobj (); /* RAMPS */

    makeobj (GRAPH);
	color(WHITE);
    	recti (162, 300, 462, 600);
	color (UNPICKCOLOR);
	cmov2i (125, 275);
	charstr ("0.0");
	cmov2i (450, 275);
	charstr ("1.0");
	cmov2i (125, 595);
	charstr ("1.0");
	cmov2i (10, 500);
	charstr ("   color");
	cmov2i (10, 480);
	charstr (" brightness");
	cmov2i (10, 460);
	charstr ("(normalized)");
	cmov2i (225, 250);
	charstr ("color ramp location");
	cmov2i (225, 230);
	charstr ("   (normalized)");

	color (GREEN);
	move2i (163, 300);
	draw2i (462, 599);
    closeobj (); /* GRAPH */

    makeobj(CONTRAN);
	ortho2(-250.0, 250.0, -63.0, 63.0);
    closeobj();

}

setupqueue () {
    qdevice (LEFTMOUSE);
    qdevice (RIGHTMOUSE);
    qdevice (INPUTCHANGE);
    qdevice (REDRAW);
    qdevice (ESCKEY);
}

setupcolors() {
    int i;

    if (getplanes () < 6) {	/*  if less than 6 bitplanes in mex	 */
	printf("You do not have enough bitplanes for this program\n");
	gexit();
	exit(0);
    }
    tutorsavemap();
    save_colors();
    tutormakemap();
    gamma_spectrum (1.0, 16, 16, 255, 255, 255, 0, 0, 0);
    gamma_spectrum (gammac, 32, 16, 255, 255, 255, 0, 0, 0);
}


gamma_spectrum (gamma, firstcolor, cuenum,
		brightred, brightgreen, brightblue,
		darkred, darkgreen, darkblue)
/*---------------------------------------------------------------------------
 * 	gamma_spectrum -- This routine creates a gamma-corrected 	
 *	color ramp for depthcuing.  gamma is the strength of gamma
 *	correction.  The location of the beginning of the ramp in the 	
 *	color map is 'firstcolor.'  Cuenum is the number of colors in 	
 *	the ramp.  The next six arguments are the starting and 		
 *	finishing RGB values of the ramp.  All the interim color 	
 *	values on the ramp are determined between these extremes.	
 *---------------------------------------------------------------------------
 */
float   gamma;
Colorindex firstcolor, cuenum;
RGBvalue brightred, brightgreen, brightblue;
RGBvalue darkred, darkgreen, darkblue;
{
    int     i;
    RGBvalue gcred, gcgreen, gcblue;/*  gamma-corrected RGB	 */
    float   gammainc;

    for (i = 0; i < cuenum; i++) {
	gammainc = pow ((float) i / (float) (cuenum - 1), 1.0 / gamma);
	gcred = (RGBvalue) (gammainc * (brightred - darkred) + darkred);
	gcgreen = (RGBvalue) (gammainc * (brightgreen - darkgreen) + darkgreen);
	gcblue = (RGBvalue) (gammainc * (brightblue-darkblue) + darkblue);
	mapcolor (firstcolor + i, gcred, gcgreen, gcblue);
    }
}

/*  pop-up menu initialization		*/

setupmenus () {
    killmenu = defpup ("Exit Confirmation %t|Yes|No");
    menu = defpup ("Gamma %t|Reset gamma to 1.0|Exit");
}

save_colors()
{
    Colorindex i;

    for (i=0;i<48;i++)
	getmcolor(i, oldcolors[i], oldcolors[i]+1, oldcolors[i]+2);
}

restore_colors() {
    Colorindex i;

    for (i=0; i<48; i++) 
	mapcolor(i, oldcolors[i][0], oldcolors[i][1], oldcolors[i][2]);
}

init_windows()
/*---------------------------------------------------------------------------
 * Initialize the three windows for the program.
 *---------------------------------------------------------------------------
 */

{
    backw = init_back();
    doublebuffer();
    gconfig();
    winattach();
    helpw = init_help("Gamma -- INFORMATION");
    graphw = init_graph();
    consw = init_cons();
}

init_graph()
/*---------------------------------------------------------------------------
 * Initialize the graph window.
 *---------------------------------------------------------------------------
 */
{
    int res;
    prefposition(256, 768, 1, 587);
    res = winopen("graph");
    prefsize (512, 586);
    wintitle("Gamma -- GRAPH");
    winconstraints();
    setup_graph_environ();

    return(res);
}

setup_cons_environ()
/*---------------------------------------------------------------------------
 * Set the console environment.
 *---------------------------------------------------------------------------
 */
{
    reshapeviewport();
    ortho2(-250.0, 250.0, -63.0, 63.0);
}

setup_graph_environ()
/*---------------------------------------------------------------------------
 * Set up the environment used for the graph window.
 *---------------------------------------------------------------------------
 */
{
    reshapeviewport();
    ortho2(0.0, 550.0, 0.0, 630.0);
}

attach_to_graph()
/*---------------------------------------------------------------------------
 * Attach to the graph window.
 *---------------------------------------------------------------------------
 */
{
    winset(graphw);
}

attach_to_cons()
/*---------------------------------------------------------------------------
 * Direct graphics output to the console window.
 *---------------------------------------------------------------------------
 */
{
    winset(consw);
}

redraw_help()
/*---------------------------------------------------------------------------
 * Routine called when a redraw token is sent down the queue.
 *---------------------------------------------------------------------------
 */
{
    draw_help();
    swapbuffers();
    draw_help();
}

redraw_graph()
/*---------------------------------------------------------------------------
 * Routine called when a redraw token is sent down the queue.
 *---------------------------------------------------------------------------
 */
{
    attach_to_graph();
    reshapeviewport();
    draw_graph_window();
    swapbuffers();
    draw_graph_window();
}


redraw_cons()
/*---------------------------------------------------------------------------
 * Routine called in the event of a redraw token for the cons window coming
 * down the event queue.
 *---------------------------------------------------------------------------
 */
{
    attach_to_cons();
    reshapeviewport();
    draw_cons_window();
    swapbuffers();
    draw_cons_window();
}

draw_graph_window()
/*---------------------------------------------------------------------------
 * Draw everything in the graph window.
 *---------------------------------------------------------------------------
 */
{
    attach_to_graph();
    color(BLACK);
    clear();
    callobj (RAMPS);
    callobj (GRAPH);
    draw_newline ();
}

draw_cons_window()
/*---------------------------------------------------------------------------
 * Draw everything in the console window.
 *---------------------------------------------------------------------------
 */
{
    attach_to_cons();
    color(BLACK);
    clear();
    color(UNPICKCOLOR);
    cmov2i(-100, 15);
    print_float("gamma constant: ", gammac);
    draw_slider_float(gammac);
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
    wintitle("Gamma -- CONTROL BAR");
    prefsize(500, 126);
    winconstraints();
    setup_cons_environ();

    return(res);
}

draw_frame()
/*---------------------------------------------------------------------------
 * Draw the whole screen.
 *---------------------------------------------------------------------------
 */
{
    load_correct_help();
    draw_help();
    draw_graph_window();
    draw_cons_window();
    draw_back();
}

redraw_window(n)
/*---------------------------------------------------------------------------
 * Redraw window n.
 *---------------------------------------------------------------------------
 */
int n;
{
    if (n == helpw)    
	redraw_help();
    else if (n == consw)
	redraw_cons();
    else if (n == graphw)
	redraw_graph();
    else if (n == backw)
	redraw_back();
}

draw_slider_float(val)
float val;
{
    int pos;

    if (val < 0.0) {
	high_pos = (float) (-0.01 + (int) ((val + 0.01) / 3.0) * 3);
	low_pos = high_pos - 2.99;
	pos = (int) ( (val - low_pos) * 100.0 );
    } else {
        low_pos = (float) ( (int) (val/3.0) * 3 );
	high_pos = low_pos + 2.99;
	pos = (int) ( (val - low_pos) * 100.0 );
    }

    draw_slider(0.0, 0.0, 100, 10, (float)pos, low_pos, high_pos);
}

load_correct_help()
/*---------------------------------------------------------------------------
 * Load the help box with the correct message according to gammac.
 *---------------------------------------------------------------------------
 */
{
    if (!active) {
	load_help("Press the RIGHT MOUSE BUTTON and select ATTACH.",
		  "", "", "", "", "");
	return;
    }

    if(gammac < .05){
	load_help("WARNING:  gamma correction constants of zero or", 
		  "  less than zero create meaningless color ramps", "", "", 
		  "", "");
    } else {
	load_help("Press the LEFT MOUSE button over the Controller bar", 
		  "  to adjust the gamma constant.", "              OR", 
		  " Select a choice from the RIGHT MOUSE pop-up menu.", 
		  "", "");
    }
}
