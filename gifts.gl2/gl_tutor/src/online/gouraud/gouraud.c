#include "gouraud.h"

/*---------------------------------------------------------------------------
 * gouraud - A classroom demo to demonstrate the way to do Gouraud
 * shaded polygons.
 *---------------------------------------------------------------------------
 */

main()
{
    init_all();			/* Initialize everything in existance. */
    
    while(!exiting){
	if (!(selecting || moving))
	    check_help_status();

	respond_to_input();	/* Change parameters according to input. */

	if (selecting)
	    do_selection();
	if (moving)
	    do_moving();
    }

    clean_up_and_exit();	/* Self explanitory. */
}

init_all()
/*---------------------------------------------------------------------------
 * Initialize the graphics commands, the event queue, all the variables used
 * and anything else that needs to be initialized.
 *---------------------------------------------------------------------------
 */
{
    init_graphics();		/* Initialize the graphics system. */
    init_queue();		/* Initialize the event queue. */
    init_vars();		/* Initialize all the variables used. */
}

init_polygon()
/*----------------------------------------------------------------------------
 * Init the polygon.
 *----------------------------------------------------------------------------
 */
{
    /* Initialize a polygon. */
    polyarray[0][X] = 5.0;
    polyarray[0][Y] = 0.0;
    polyarray[0][I] = 0.0;

    polyarray[1][X] = 5.0;
    polyarray[1][Y] = 15.0;
    polyarray[1][I] = 28.0;

    polyarray[2][X] = 15.0;
    polyarray[2][Y] = 19.0;
    polyarray[2][I] = 0.0;

    polyarray[3][X] = 15.0;
    polyarray[3][Y] = 5.0;
    polyarray[3][I] = 28.0;

    make_edge_list();
    qenter(REDRAW, gridid);
}

clean_up_and_exit()
/*---------------------------------------------------------------------------
 * Do all the overhead required to exit the program.
 *---------------------------------------------------------------------------
 */
{
    tutorrestoremap();
    gexit();
    exit(0);
}

init_graphics()
/*---------------------------------------------------------------------------
 * Initialize the graphics system.
 *---------------------------------------------------------------------------
 */
{

    check_for_mex();

    init_windows();	    /* This is a multiple window program. */
    init_color_map();	    /* Initialize the color map and make ramps. */
    init_pups();	    /* Popup menus. */
    init_display_list();    /* Make all objects. */
}   

init_pups()
/*---------------------------------------------------------------------------
 * Initialize the pop-up menus used in the program.
 *---------------------------------------------------------------------------
 */
{
    exitmen = defpup("Exit Confirmation %t|Yes|No");
    mainmen = defpup("Gouraud %t|Reset parameters%f|Exit", init_polygon);
}

init_queue()
/*---------------------------------------------------------------------------
 * Initialize the event queue.
 *---------------------------------------------------------------------------
 */
{
    qdevice(RIGHTMOUSE);	/* Pop up menu button. */
    qdevice(LEFTMOUSE);		/* Interaction button. */
    qdevice(REDRAW);		/* Redraw token.       */
    qdevice(INPUTCHANGE);	/* Input change token. */
    qdevice(ESCKEY);

    qenter(REDRAW, consid);
    qenter(REDRAW, helpid);
}

init_vars()
/*---------------------------------------------------------------------------
 * Initialize all the variables that need initializing in the program.
 *---------------------------------------------------------------------------
 */
{
    exiting = FALSE;		    /* We've only begun!! */
    current_vertex = -1;	    /* Not editing any. */
    moving = selecting = FALSE;	    /* We're Idle. */
    moved = selected = FALSE;
    cursloc = -1;
    attached = FALSE;

    polyarray[0][X] = 5.0;
    polyarray[0][Y] = 0.0;
    polyarray[0][I] = 0.0;

    polyarray[1][X] = 5.0;
    polyarray[1][Y] = 15.0;
    polyarray[1][I] = 28.0;

    polyarray[2][X] = 15.0;
    polyarray[2][Y] = 19.0;
    polyarray[2][I] = 0.0;

    polyarray[3][X] = 15.0;
    polyarray[3][Y] = 5.0;
    polyarray[3][I] = 28.0;

    make_edge_list();

}

init_color_map()
/*---------------------------------------------------------------------------
 * Initialize the color map.  Uses only the bottom 16 bit planes.
 *---------------------------------------------------------------------------
 */
{
    if (getplanes() < 6) {
	printf("You do not have enough bitplanes for this program\n");
	gexit();
	exit(0);
    }
    tutorsavemap();
    tutormakemap();
    make_ramp(RAMPBOT, RAMPTOP, 0, 121, 169, 255, 255, 255);
    mapcolor(CONSBACK, 128, 128, 128);
}

init_windows()
/*---------------------------------------------------------------------------
 * Initialize the multiple windows.  At this point we assume we're in mex.
 *---------------------------------------------------------------------------
 */
{
    backid = init_back();
    winattach ();

    /* Help window */
    helpid = init_help();

    /* Main control panel. */
    consid = init_console();

    /* Polygon grid. */
    gridid = init_grid();

}

init_display_list()
/*---------------------------------------------------------------------------
 * Initialize the display list.
 *---------------------------------------------------------------------------
 */
{
    makeobj(PIXEL);
	circfi(0, 0, UNITSIZE/2);
    closeobj();

    makeobj(HIGHLIGHT);
	linewidth(2);
	circi(0, 0, UNITSIZE/2);
    closeobj();

    makeobj(HPIXEL);
	circfi(0, 0, UNITSIZE/2);
	color(RED);
	callobj(HIGHLIGHT);
    closeobj();

    makeobj(CONSTRAN);
	ortho2(-SQRSIZE, SQRSIZE*33.0, -2.0*SQRSIZE, SQRSIZE*2.0);
    closeobj();

    makeobj(GRIDTRAN);
	ortho2(-((float)UNITSIZE),
	(float)(UNITSIZE*GRIDSIZE), 
	-((float)UNITSIZE),
	(float)(UNITSIZE*GRIDSIZE));
    closeobj();

}


respond_to_input()
/*---------------------------------------------------------------------------
 * Do all the mouse input.
 *---------------------------------------------------------------------------
 */
{
    long dev;
    short val;

    while(qtest()){
	dev = qread(&val);
	switch (dev){
	    case REDRAW:
		redraw_window(val);
		break;
	    case INPUTCHANGE:
		attached = val;
		break;
	    case LEFTMOUSE:
		respond_to_leftmouse(val);
		break;
	    case RIGHTMOUSE:
		respond_to_rightmouse(val);
		break;
	    case ESCKEY:
		clean_up_and_exit();
		break;
	}
    }
}

rampcolor(n)
/*---------------------------------------------------------------------------
 * Execute a color command for  the nth color on the color ramp.
 *---------------------------------------------------------------------------
 */
Colorindex n;
{
    color(RAMPBOT + n);
}

make_ramp(bi, ei, rb, gb, bb, re, ge, be)
/*---------------------------------------------------------------------------
 * Create a color ramp from index bi with rgb of rb,gr,bb to color index be
 * with rgb of re,ge,be.
 *---------------------------------------------------------------------------
 */
Colorindex bi, ei;
RGBvalue rb, gb, bb, re, ge, be;
{
    register float dr, dg, db;
    register int i;

    dr = ((float) (re - rb)/(float) (ei - bi));
    dg = ((float) (ge - gb)/(float) (ei - bi));
    db = ((float) (be - bb)/(float) (ei - bi));

    for (i = bi ; i <= ei ; i++){
	mapcolor(i, rb, gb, bb);
	rb = (RGBvalue) ((float) rb + dr);
	gb = (RGBvalue) ((float) gb + dg);
	bb = (RGBvalue) ((float) bb + db);
    }
}

redraw_window(n)
/*---------------------------------------------------------------------------
 * Redraw the window identified by n.
 *---------------------------------------------------------------------------
 */
{
    if (n == helpid)
	redraw_help();
    else if (n == consid)
	redraw_console();
    else if (n == gridid)
	redraw_grid();
    else if (n == backid)
	redraw_back();
}

respond_to_leftmouse(state)
/*---------------------------------------------------------------------------
 * Given the state of the left mouse button event (up or down) respond
 * accordingly.
 *---------------------------------------------------------------------------
 */
int state;
{
    if (!state)			    /* If up. */
	do_left_up();
    else
	do_left_down();
}

respond_to_rightmouse(state)
/*---------------------------------------------------------------------------
 * Do the pup.
 *---------------------------------------------------------------------------
 */
int state;
{
    if (state)
	if (dopup(mainmen) == 2)
	    if (dopup(exitmen) == 1)
		exiting = TRUE;
}

do_left_up()
/*---------------------------------------------------------------------------
 * Respond to the left mouse button comming up.
 *---------------------------------------------------------------------------
 */
{
    moving = selecting = FALSE;	    /* Not doing anything. */

    attach_to_grid();
    if (selected)
	draw_big_poly(GRIDX, GRIDY);
    else if (moved) {
	make_edge_list();
        redraw_grid();
    }

    display_correct_menu(cursor_location());

    moved = selected = FALSE;
}

do_left_down()
/*---------------------------------------------------------------------------
 * Respond to the left mouse button going down. 
 *---------------------------------------------------------------------------
 */
{
    register int what;
    int where;

    what = check_for_hit(&where);

    switch (what){
	case GRID:
	    respond_at_grid(where);
	    break;
	case CONSOLE:
	    respond_at_console();
	    break;
    }
}

int check_for_hit(vert)
/*---------------------------------------------------------------------------
 * Check both the grid and the console window for a hit and return a tolken
 * stating such.  If it's a hit on the grid, the vertex number of the hit is
 * returned.
 *---------------------------------------------------------------------------
 */
int *vert;
{
    pick_grid();			    /* Check the grid. */
    if (retnumber != 0){
        *vert = (int) pick_buffer[3];
	return((int) pick_buffer[2]);
    }
	
    pick_console();			    /* Check the console. */
    if (retnumber == 0)
	return(FALSE);
    else
        return((int) pick_buffer[2]);
}   

init_pick_buffer()
/*---------------------------------------------------------------------------
 * Initilize the pick buffer to zeros.
 *---------------------------------------------------------------------------
 */
{
    register int i;

    for (i = 0 ; i < MAXPICK ; i++)
	pick_buffer[i] = 0;
}

print_pick_list()
{
    register int i, j, k;

		printf ("hits: %d; ", retnumber);
		j = 0;
		for (i = 0; i < retnumber; i++) {/* for each entry */
		    printf (" ");
		    k = pick_buffer[j++];/* get number of objects */
		    printf ("%d ", k);
		    for (; k; k--)	/* for each object */
			printf ("%d ", pick_buffer[j++]);/* print it */
		    printf ("|");
		}
		printf ("\n");
}

grab_mice(mx, my)
/*---------------------------------------------------------------------------
 * Read the mouse valuator and put the values in mx and my.
 *---------------------------------------------------------------------------
 */
short *mx, *my;
{
    
    *mx = getvaluator(MOUSEX);
    *my = getvaluator(MOUSEY);
}

cursor_location()
/*---------------------------------------------------------------------------
 * Grab the location of the cursor and return the window (if any) it is
 * positioned over.
 *---------------------------------------------------------------------------
 */
{
    short mx, my;

    grab_mice(&mx, &my);

    if (!attached)
	return(NOTATT);
    else if (on_console((long) mx, (long) my))
	return(CONSOLE);
    else if (on_grid((long) mx, (long) my))
	return(GRID);

    return(FALSE);
}

display_correct_menu(loc)
/*---------------------------------------------------------------------------
 * Given the cursor location loc, display the apropriate menu.
 *---------------------------------------------------------------------------
 */
int loc;
{
    switch (loc) {
    case CONSOLE:
	if (current_vertex != -1)
	    load_help("Use the LEFT MOUSE button to:",
		      "  1. (If held down) Grab the color index slider.",
		      "");
	else
	    load_help("Before you edit a VERTEX,  you must:", 
		      "  Select one to edit in the GRID window.", 
		      "");
	break;
    case GRID:
	load_help("Use the LEFT MOUSE button to:", 
		  "  1. Select a vertex to edit.", 
		  "  2. (If held down) Move a vertex about the grid.");
	break;
    case NOTATT:
	load_help("In order to use this program you must:", 
		  "  Press the RIGHT MOUSE BUTTON and select ATTACH.", 
		  "");
	break;
    default:
	load_help("To work this program:", 
		  "  Move the cursor onto either the GRID window.", 
		  "  Or the CONSOLE window.");
	break;
    }
}

check_help_status()
/*---------------------------------------------------------------------------
 * Checks to see if the status of the cursor location has changed.  If it has,
 * then redraws the help menu.
 *---------------------------------------------------------------------------
 */
{
    int curloc;

    curloc = cursor_location();
    if (curloc != cursloc){
	cursloc = curloc;
	display_correct_menu(curloc);
    }
}


