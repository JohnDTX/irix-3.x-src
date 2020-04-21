/*  swap.c							*/
/*  March, 1986 -- Mason Woo					*/
/*  Learning Environment -- to demonstrate simulation of	*/
/*  movement by writing images to buffers and swapping buffers.	*/
/*  The frontbuffer(), backbuffer(), and swapbuffers() calls	*/
/*  are demonstrated.						*/

#include <gl.h>
#include <device.h>
#include "tutor.h"

#define EXITSOON	5
#define DISPINCR	0.15
#define DISPSTART	-0.7
#define DISPSIZE	0.25
#define DISPBIG		0.75

float     newdisp;
Colorindex  highcolor,
	    frontcolor,
	    backcolor;
short oldcolors[16][3];
int	backwin,
	helpwin,
	conswin,
	viewwin,
	bufferwin;
int     active;
int     exitswap = -1;

main () {
    short   val;
    long    dev;
    int     frontmenu,
            backmenu,
            mainmenu;
    Boolean frontdrawn, backdrawn;
    float   frontdisp,
            backdisp;
    int     highlight;
    short   chosen;
    short   pickme ();
    int     i;

    if(!ismex()){
	printf("You must be running the window manager to use this program.\n");
	printf("Type mex to start.\n");
	exit(0);
    }

    init_windows();

    setupcolors();
    setupqueue ();

    active = FALSE;
    mainmenu = defpup("Swap %t|Please read the information window.");
    frontmenu = FALSE;
    backmenu = TRUE;
    frontdrawn = FALSE;
    backdrawn = FALSE;
    newdisp = DISPSTART;
    highlight = FALSE;

    while (1) {
	color (BLACK);
	clear ();
	while (qtest ()) {
	    dev = qread (&val);
	    switch (dev) {
		case RIGHTMOUSE:
		    dopup(mainmenu);
		    break;
		case LEFTMOUSE: 
		    if (val == 1) {
			highlight = TRUE;
		    }
		    else if (val == 0) {
			highlight = FALSE;
			chosen = pickme ();
			/*  cancel exit	 */
			if (chosen != EXITSOON)	
			    exitswap = -1;
			if (chosen) {
			    execute_menu (chosen, &frontmenu, &backmenu,
				    &mainmenu, &frontdrawn, &backdrawn,
				    &frontdisp, &backdisp);
			}
			curson ();
		    }
		    break;
		case REDRAW: 
		    redraw_window (val);
		    break;
		case INPUTCHANGE: 
		    active = val;
		    if (active == FALSE) {
			draw_frame(frontdrawn, frontdisp, frontmenu, 
				backdrawn, backdisp, backmenu, 
				mainmenu, highlight, chosen);
			swapbuffers ();
			draw_frame(frontdrawn, frontdisp, frontmenu, 
				backdrawn, backdisp, backmenu, 
				mainmenu, highlight, chosen);
		    }
		    break;
		case ESCKEY:
		    tutorrestoremap();
		    gexit(); exit(0);
		    break;
		default: 
		    break;
	    }
	    if (!active)
		while (!qtest ())
		    swapbuffers ();
	}
	draw_frame(frontdrawn, frontdisp, frontmenu, backdrawn, 
		backdisp, backmenu, mainmenu, highlight, chosen);
	swapbuffers ();
    }
}

short   pickme () {
    short   buffer[100];
    long    numpicked;
    int     i;

    attach_to_cons();
    pushmatrix ();
    pick (buffer, 100);
    ortho2 (0.0, 300.0, 0.0, 300.0);
    initnames ();

    for (i = 0; i <= 1; i++) {
	loadname ((short) (i + 1));/*  menu items 1 and 2	 */
	rectfi (0, 0 + (30 * i), 300, 30 + (30 * i));
    }
    for (i = 0; i <= 1; i++) {
	loadname ((short) (i + 3));/*  menu items 3 and 4	 */
	rectfi (0, 90 + (30 * i), 300, 120 + (30 * i));
    }
    for (i = 0; i <= 3; i++) {
	loadname ((short) (i + 5));
	/*  menu items 5, 6, 7, and 8	 */
	rectfi (0, 180 + (30 * i), 300, 210 + (30 * i));
    }

    numpicked = endpick (buffer);
    popmatrix ();
    if (numpicked)
	return (buffer[1]);
    else
	return (0);
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

    if (getplanes() < 4) {
	printf("You do not have enough bitplanes for this program\n");
	gexit();
	exit(0);
    }
    tutorsavemap();
    tutormakemap();

    frontcolor = 8;
    backcolor = 9;
    highcolor = 10;
}

drawview (frontdrawn, frontdisp)
Boolean frontdrawn;
float   frontdisp;
{
    if (frontdrawn) {
	color (GREEN);
	rectf (frontdisp - DISPSIZE, frontdisp - DISPSIZE,
		frontdisp + DISPSIZE, frontdisp + DISPSIZE);
    }
}

drawmenu (frontmenu, backmenu, mainmenu, highlight, chosen)
int     frontmenu,
        backmenu,
        mainmenu,
	highlight;
short   chosen;
{
    int     i;
    short pickthem;
    short pickme();

    if (highlight)
	pickthem = pickme();

    attach_to_cons ();
    /*  now write the text in the menus		*/
    color(NORMCOLOR);
    if (pickthem == 8) {
	rectfi (0, 270, 300, 300);
	color (BLACK);
    }
    cmov2i (50, 278);
    charstr ("clear enabled buffer(s)");

    color(NORMCOLOR);
    if (pickthem == 7) {
	rectfi (0, 240, 300, 270);
	color (BLACK);
    }
    cmov2i (54, 256);
    charstr (" draw new object into");
    cmov2i (50, 242);
    charstr ("   enabled buffer(s)");

    color(NORMCOLOR);
    if (pickthem == 6) {
	rectfi (0, 210, 300, 240);
	color (BLACK);
    }
    cmov2i (50, 218);
#ifdef FORTRAN
    charstr ("     CALL SWAPBU()");
#else
    charstr ("     swapbuffers()");
#endif

    /*  text for exit selection			*/
    if (exitswap == EXITSOON) {
	color (NORMCOLOR);
	rectfi (0, 180, 300, 210);
	color (BLACK);
	cmov2i (54, 196);
	charstr ("   press here again");
	cmov2i (50, 182);
	charstr ("    to confirm exit");
    }
    else {
	color(NORMCOLOR);
	if (pickthem == 5) {
	    rectfi (0, 180, 300, 210);
	    color (BLACK);
	}
	cmov2i (54, 188);
	charstr ("     exit program");
    }

    /*  highlight menu items	*/

    if (pickthem == 4)
	highfront(TRUE);
    else if (pickthem == 3)
	highfront(FALSE);
    else if (pickthem == 2)
	highback(TRUE);
    else if (pickthem == 1)
	highback(FALSE);
    if ((pickthem != 3) && (pickthem != 4)) {
	if (frontmenu)
	    highfront(TRUE);
	else
	    highfront(FALSE);
    }
    if ((pickthem != 1) && (pickthem != 2)) {
	if (backmenu)
	    highback(TRUE);
	else
	    highback(FALSE);
    }

    /*  backbuffer menu	outline		 */
    color (NORMCOLOR);
    for (i = 0; i <= 1; i++) {
	recti (0, 0 + (30 * i), 300, 30 + (30 * i));
    }
    /*  frontbuffer menu outline	 */
    for (i = 0; i <= 1; i++) {
	recti (0, 90 + (30 * i), 300, 120 + (30 * i));
    }
    /*  main menu outline		 */
    for (i = 0; i <= 3; i++) {
	recti (0, 180 + (30 * i), 300, 210 + (30 * i));
    }

    load_correct_help (pickthem, highlight, chosen);
}

highfront(highup) 
int highup;
{
    if (highup)
	color (NORMCOLOR);
    else
	color (BLACK);
    rectfi (0, 120, 300, 150);
    if (highup)
	color (BLACK);
    else
	color (NORMCOLOR);
#ifdef FORTRAN
    cmov2i (74, 128);
    charstr ("CALL FRONTB(.TRUE.)");
#else
    cmov2i (74, 128);
    charstr (" frontbuffer(TRUE)");
#endif
    rectfi (0, 90, 300, 120);
    if (highup)
	color (NORMCOLOR);
    else
	color (BLACK);
    cmov2i (70, 98);
#ifdef FORTRAN
    charstr ("CALL FRONTB(.FALSE.)");
#else
    charstr ("frontbuffer(FALSE)");
#endif
}

highback(highup)
int highup;
{
    if (highup)
	color (NORMCOLOR);
    else
	color (BLACK);
    rectfi (0, 30, 300, 60);
    if (highup)
	color (BLACK);
    else
	color (NORMCOLOR);
    cmov2i (70, 38);
#ifdef FORTRAN
    charstr ("CALL BACKBU(.TRUE.)");
#else
    charstr (" backbuffer(TRUE)");
#endif
    rectfi (0, 0, 300, 30);
    if (highup)
	color (NORMCOLOR);
    else
	color (BLACK);
#ifdef FORTRAN
    cmov2i (74, 8);
    charstr ("CALL BACKBU(.FALSE.)");
#else
    cmov2i (74, 8);
    charstr (" backbuffer(FALSE)");
#endif
}

drawbuffers (frontdrawn, frontdisp, frontmenu, 
		backdrawn, backdisp, backmenu)
Boolean frontdrawn;
float   frontdisp;
int     frontmenu;
Boolean backdrawn;
float   backdisp;
int     backmenu;
{
    viewport (150, 350, 50, 350);	/*  back buffer		*/
    pushmatrix ();
    ortho(-0.9, 0.9, -1.35, 1.35, 4.0, 10.0);
    lookat(4.0, 4.0, 4.0, 0.0, 0.0, 0.0, 0);

    if (backmenu == TRUE) {
	color(highcolor);
	rectf(-1.1, -1.1, 1.1, 1.1);
    }
    color (backcolor);
    rectf (-1.0, -1.0, 1.0, 1.0);
    if (backdrawn) {
	color (GREEN);
	rectf (backdisp - DISPSIZE, backdisp - DISPSIZE,
		backdisp + DISPSIZE, backdisp + DISPSIZE);
    }
    popmatrix();
    reshapeviewport ();

    viewport (0, 200, 50, 350);	/*  front buffer	*/
    pushmatrix ();
    ortho(-0.9, 0.9, -1.35, 1.35, 4.0, 10.0);
    lookat(4.0, 4.0, 4.0, 0.0, 0.0, 0.0, 0);

    if (frontmenu == TRUE) {
	color(highcolor);
	rectf(-1.1, -1.1, 1.1, 1.1);
    }
    color (frontcolor);
    rectf (-1.0, -1.0, 1.0, 1.0);
    if (frontdrawn) {
	color (GREEN);
	rectf (frontdisp - DISPSIZE, frontdisp - DISPSIZE,
		frontdisp + DISPSIZE, frontdisp + DISPSIZE);
    }
    popmatrix();
    reshapeviewport ();

    color (NORMCOLOR);
    cmov2i (65, 25);
    charstr (" front");
    cmov2i (229, 25);
    charstr (" back");
    if (frontmenu == TRUE) {
	cmov2i (65, 10);
	charstr ("enabled");
    }
    if (backmenu == TRUE) {
	cmov2i (225, 10);
	charstr ("enabled");
    }

}

execute_menu (chosen, frontmenu, backmenu, mainmenu,
    frontdrawn, backdrawn, frontdisp, backdisp)
short   chosen;
int    *frontmenu,
       *backmenu,
       *mainmenu;
Boolean * frontdrawn, *backdrawn;
float  *frontdisp,
       *backdisp;
{
    float   tempdisp;
    Boolean tempdrawn;

    switch (chosen) {
	case 1: 
	    *backmenu = FALSE;
	    break;
	case 2: 
	    *backmenu = TRUE;
	    break;
	case 3: 
	    *frontmenu = FALSE;
	    break;
	case 4: 
	    *frontmenu = TRUE;
	    break;
	case 5: 
	    if (exitswap != EXITSOON)
		exitswap = EXITSOON;
	    else {
	        tutorrestoremap();
		gexit ();
		exit ();
	    }
	    break;
	case 6: 
	    tempdrawn = *frontdrawn;
	    tempdisp = *frontdisp;
	    *frontdrawn = *backdrawn;
	    *frontdisp = *backdisp;
	    *backdrawn = tempdrawn;
	    *backdisp = tempdisp;
	    break;
	case 7: 
	    newdisp = newdisp + DISPINCR;
	    if (newdisp >= DISPBIG)
		newdisp = DISPSTART;
	    if (*frontmenu == TRUE) {
		*frontdrawn = TRUE;
		*frontdisp = newdisp;
	    }
	    if (*backmenu == TRUE) {
		*backdrawn = TRUE;
		*backdisp = newdisp;
	    }
	    break;
	case 8: 
	    if (*frontmenu == TRUE)
		*frontdrawn = FALSE;
	    if (*backmenu == TRUE)
		*backdrawn = FALSE;
	    break;
	default: 
	    break;
    }
}

init_windows()
/*---------------------------------------------------------------------------
 * Initialize the five windows for the program.
 *---------------------------------------------------------------------------
 */
{
    backwin = init_back();
    doublebuffer();
    gconfig();
    winattach();
    helpwin = init_help("Swap -- INFORMATION");
    viewwin = init_view();
    conswin = init_cons();
    bufferwin = init_buffer();
}

init_view()
/*---------------------------------------------------------------------------
 * Initialize the view window.
 *---------------------------------------------------------------------------
 */
{
    int res;
    prefposition(50, 550, 50, 550);
    res = winopen("view");
    prefsize (500, 500);
    wintitle("Swap -- SCREEN VIEW");
    winconstraints();
    setup_view_environ();

    return(res);
}

init_cons()
/*---------------------------------------------------------------------------
 * Initialize the console window.
 *---------------------------------------------------------------------------
 */
{
    int res;

    prefposition(650, 950, 425, 725);
    res = winopen("cons");
    wintitle("Swap -- CONSOLE");
    prefsize(300, 300);
    winconstraints();
    setup_cons_environ();

    return(res);
}

init_buffer()
/*---------------------------------------------------------------------------
 * Initialize the buffer window.
 *---------------------------------------------------------------------------
 */
{
    int res;
    prefposition(625, 975, 25, 375);
    res = winopen("buffer");
    prefsize (350, 350);
    wintitle("Swap -- BUFFERS");
    winconstraints();
    setup_buffer_environ();

    return(res);
}

setup_cons_environ()
/*---------------------------------------------------------------------------
 * Set the console environment.
 *---------------------------------------------------------------------------
 */
{
    reshapeviewport();
    ortho2(0.0, 300.0, 0.0, 300.0);
}

setup_view_environ()
/*---------------------------------------------------------------------------
 * Set up the environment used for the view window.
 *---------------------------------------------------------------------------
 */
{
    reshapeviewport();
    ortho2 (-1.0, 1.0, -1.0, 1.0);
}

setup_buffer_environ()
/*---------------------------------------------------------------------------
 * Set up the environment used for the buffer window.
 *---------------------------------------------------------------------------
 */
{
    reshapeviewport();
    ortho2(0.0, 350.0, 0.0, 350.0);
}

attach_to_view()
/*---------------------------------------------------------------------------
 * Attach to the view window.
 *---------------------------------------------------------------------------
 */
{
    winset(viewwin);
}

attach_to_cons()
/*---------------------------------------------------------------------------
 * Direct graphics output to the console window.
 *---------------------------------------------------------------------------
 */
{
    winset(conswin);
}

attach_to_buffer()
/*---------------------------------------------------------------------------
 * Direct graphics output to the buffer window.
 *---------------------------------------------------------------------------
 */
{
    winset(bufferwin);
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

redraw_view()
/*---------------------------------------------------------------------------
 * Routine called when a redraw token is sent down the queue.
 *---------------------------------------------------------------------------
 */
{
    attach_to_view();
    reshapeviewport();
    draw_view_window();
    swapbuffers();
    draw_view_window();
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

redraw_buffer()
/*---------------------------------------------------------------------------
 * Routine called in the event of a redraw token for the buffer window coming
 * down the event queue.
 *---------------------------------------------------------------------------
 */
{
    attach_to_buffer();
    reshapeviewport();
    draw_buffer_window();
    swapbuffers();
    draw_buffer_window();
}

draw_view_window(frontdrawn, frontdisp)
Boolean frontdrawn;
float   frontdisp;
/*---------------------------------------------------------------------------
 * Draw everything in the view window.
 *---------------------------------------------------------------------------
 */
{
    attach_to_view();
    color(BLACK);
    clear();
    drawview (frontdrawn, frontdisp);
}

draw_cons_window(frontmenu, backmenu, mainmenu, highlight, chosen)
int     frontmenu,
        backmenu,
        mainmenu,
	highlight;
short   chosen;
/*---------------------------------------------------------------------------
 * Draw everything in the console window.
 *---------------------------------------------------------------------------
 */
{
    attach_to_cons();
    color(BLACK);
    clear();
    drawmenu (frontmenu, backmenu, mainmenu, highlight, chosen);
}

draw_buffer_window(frontdrawn, frontdisp, frontmenu, 
		backdrawn, backdisp, backmenu)
Boolean frontdrawn;
float   frontdisp;
int     frontmenu;
Boolean backdrawn;
float   backdisp;
int     backmenu;
/*---------------------------------------------------------------------------
 * Draw everything in the buffer window.
 *---------------------------------------------------------------------------
 */
{
    attach_to_buffer();
    color (BLACK);
    clear ();
    drawbuffers (frontdrawn, frontdisp, frontmenu, 
		backdrawn, backdisp, backmenu);
}

draw_frame(frontdrawn, frontdisp, frontmenu, backdrawn, backdisp, backmenu,
	mainmenu, highlight, chosen)
Boolean frontdrawn;
float   frontdisp;
int     frontmenu;
Boolean backdrawn;
float   backdisp;
int     backmenu;
int     mainmenu,
	highlight;
short   chosen;
/*---------------------------------------------------------------------------
 * Draw the whole screen.
 *---------------------------------------------------------------------------
 */
{
    draw_cons_window(frontmenu, backmenu, mainmenu, highlight, chosen);
    draw_view_window(frontdrawn, frontdisp);
    draw_buffer_window(frontdrawn, frontdisp, frontmenu, 
		backdrawn, backdisp, backmenu);
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
    else if (n == viewwin)
	redraw_view();
    else if (n == bufferwin)
	redraw_buffer();
    else if (n == backwin)
	redraw_back();
}

load_correct_help (pickthem, highlight, chosen)
short   pickthem;
int	highlight;
short	chosen;
/*---------------------------------------------------------------------------
 * Load the help box with the correct message according to queue.
 *---------------------------------------------------------------------------
 */
{
    if (!active) {
	load_help ("", "  Press the RIGHT MOUSE button and attach",
		"  to the program from the popup menu.", "", "", "");
	return ;
    }

    if (highlight == FALSE) {
	switch (chosen) {
	case 0: 
	case 1: 
	case 2: 
	case 3: 
	case 4: 
	case 6: 
	case 7: 
	case 8: 
	    load_help ("", "  Use LEFT MOUSE button to select",
			"  an action from the console window",
			"", "", "");
	    break;
	case 5: 
	    load_help ("", "  Select again to confirm the request", 
			"  to exit the program.  Selecting anywhere",
			"  else will cancel the request.",
			"", "");
	default: 
	    break;
	}
    }
    else {
	switch (pickthem) {
	case 0: 
	    load_help ("", "", 
		"  Select an action from the choices", 
		"  in the console window", 
		"", "");
	    break;
	case 1: 
	    load_help ("", "  backbuffer (FALSE) stops updating", 
		"  the back buffer.  Any drawing commands ", 
		"  no longer draw anything in the back buffer.", "", "");
	    break;
	case 2: 
	    load_help ("", "  backbuffer (TRUE) start updating", 
		"  the back buffer.  Any drawing commands", 
		"  draw those shapes into the back buffer.", 
		"  The contents of the back buffer are not visible", 
		"  to you until they are swapped to the front.");
	    break;
	case 3: 
	    load_help ("", "  frontbuffer (FALSE) stops updating", 
		"  the front buffer.  Any drawing commands", 
		"  no longer draw anything in the front buffer.", 
		"  The contents of the front buffer are what", 
		"  you see on the screen.");
	    break;
	case 4: 
	    load_help ("", "  frontbuffer (TRUE) stops updating", 
		"  the front buffer.  Any drawing commands", 
		"  draw those shapes into the front buffer.", 
		"  The contents of the front buffer are what", 
		"  you see on the screen.");
	    break;
	case 5: 
	    load_help ("", "", "  Select this to exit the program", 
			"", "", "");
	    break;
	case 6: 
	    load_help ("", "  swapbuffers () trades the images ", 
		"  in the front buffer and the back buffer.", 
		"  The contents of the front buffer are what", 
		"  you see on the screen.", "");
	    break;
	case 7: 
	    load_help ("", "", "  Draw a shape into the buffers which", 
		"  are enabled for updating.", "", "");
	    break;
	case 8: 
	    load_help ("", "", "  Clear the contents for all buffers which", 
		"  are enabled for updating.", "", "");
	    break;
	default: 
	    break;
	}
    }
}

