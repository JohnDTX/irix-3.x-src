/*  patch.c							*/
/*  May, 1986 -- Michael Clark					*/
/*  Learning Environment -- to demonstrate parametric bicubic	*/
/*  surface patches with different bases, precisions, and	*/
/*  control points						*/

#include "patch.h"

static Matrix beziermat = {
	{ -1, 3, -3, 1}, 
	{3, -6, 3, 0}, 
	{-3, 3, 0, 0}, 
	{1, 0, 0, 0}
    };

static Matrix cardinalmat = {
	{ -0.5, 1.5, -1.5, 0.5}, 
	{ 1.0, -2.5, 2.0, -0.5}, 
	{ -0.5, 0.0, 0.5, 0.0}, 
	{ 0, 1, 0, 0}
    };

static Matrix bsplinemat = {
	{-1.0/6.0, 3.0/6.0, -3.0/6.0, 1.0/6.0}, 
	{3.0/6.0, -1.0, 0.5, 0}, 
	{-0.5, 0.0, 0.5, 0.0}, 
	{1.0/6.0, 4.0/6.0, 1.0/6.0, 0}
    };

static Matrix geomxi = {
	{-.75, -.75, -.75, -.75}, 
	{-.25, -.25, -.25, -.25}, 
	{.25, .25, .25, .25}, 
	{.75, .75, .75, .75}};

static Matrix geomyi = {
	{0.0, 0.0, 0.0, 0.0}, 
	{0.0, 0.0, 0.0, 0.0}, 
	{0.0, 0.0, 0.0, 0.0}, 
	{0.0, 0.0, 0.0, 0.0}};

static Matrix geomzi = {
	{.75, .25, -.25, -.75}, 
	{.75, .25, -.25, -.75}, 
	{.75, .25, -.25, -.75}, 
	{.75, .25, -.25, -.75}};

main()
{
    init_all();

    while(!exiting){
	respond_to_input();

	if (attached){
	    draw_frame();
	    swapbuffers();
        if (moving)
		update_sliders();
	}
    }

    clean_up_and_exit();
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

init_graphics()
/*---------------------------------------------------------------------------
 * Initialize the graphics system.
 *---------------------------------------------------------------------------
 */
{
    check_for_mex();

    init_windows();
    if (getplanes() == 2) {
	printf("You do not have enough bitplanes to run this program\n");
	exit(0);
    }

    picksize(20, 20);	    /* Picking sensitivity.			*/
    init_color_map();	    /* Initialize the color map and make ramps. */  
    init_pups();	    /* Popup menus. */
    init_display_list();    /* Make all objects. */

}

init_color_map()
/*---------------------------------------------------------------------------
 * Initialize the color map.
 *---------------------------------------------------------------------------
 */
{
    tutorsavemap();
    tutormakemap();
}

set_ucard()
/*---------------------------------------------------------------------------
 * Set the current basis to Cardinal.
 *---------------------------------------------------------------------------
 */
{
    curbasisu = CARDINAL;
}

set_ubspl()
/*---------------------------------------------------------------------------
 * Set the current basis to bspline.
 *---------------------------------------------------------------------------
 */
{
    curbasisu = BSPLINE;
}

set_ubez()
/*---------------------------------------------------------------------------
 * Set the current basis to bezier.
 *---------------------------------------------------------------------------
 */
{
    curbasisu = BEZIER;
}

set_vcard()
/*---------------------------------------------------------------------------
 * Set the current basis to Cardinal.
 *---------------------------------------------------------------------------
 */
{
    curbasisv = CARDINAL;
}

set_vbspl()
/*---------------------------------------------------------------------------
 * Set the current basis to bspline.
 *---------------------------------------------------------------------------
 */
{
    curbasisv = BSPLINE;
}

set_vbez()
/*---------------------------------------------------------------------------
 * Set the current basis to bezier.
 *---------------------------------------------------------------------------
 */
{
    curbasisv = BEZIER;
}

reset_surface()
/*---------------------------------------------------------------------------
 * Reset the surface to the initial conditions.
 *---------------------------------------------------------------------------
 */
{
    matrix_cpy(geomx, geomxi);
    matrix_cpy(geomy, geomyi);
    matrix_cpy(geomz, geomzi);
}

init_pups()
/*---------------------------------------------------------------------------
 * Initialize the pop-up menus.
 *---------------------------------------------------------------------------
 */
{
    exitmen = defpup("Exit Confirmation %t|Yes|No");
    basisumen = defpup("Basis %t|Bezier %f|Cardinal %f|B-Spline %f",set_ubez, 
							set_ucard, set_ubspl);
    basisvmen = defpup("Basis %t|Bezier %f|Cardinal %f|B-Spline %f",set_vbez, 
							set_vcard, set_vbspl);
    mainmen = defpup("Patches %t|Change Basis U|Change Basis V|Reset paramters%f|Exit", reset_surface);
}


init_display_list()
/*---------------------------------------------------------------------------
 * Initialize the display list.
 *---------------------------------------------------------------------------
 */
{
}


init_queue()
/*---------------------------------------------------------------------------
 * Initialize the event queue.
 *---------------------------------------------------------------------------
 */
{
    qdevice(RIGHTMOUSE);		    /* Pop up menu button. */
    qdevice(LEFTMOUSE);			    /* Interaction button. */
    qdevice(REDRAW);			    /* Redraw token.       */
    qdevice(INPUTCHANGE);		    /* Input change token. */
    qdevice(ESCKEY);			    /* Escape token.       */

    qenter(REDRAW, 0);
}

init_vars()
/*---------------------------------------------------------------------------
 * Initialize all of the variables.
 *---------------------------------------------------------------------------
 */
{
    exiting = attached = FALSE;	    /* Not leaving or attached.		*/
    curbasisu = BEZIER;		    /* Starts with this basis.		*/
    curbasisv = BEZIER;		    /* Starts with this basis.		*/
    curprecu = 25;		    /* At the top  of the range.	*/
    curprecv = 25;		    /* At the top of the range. 	*/
    curcursu = 4;		    /* Middle of range.			*/
    curcursv = 4;		    /* Middle of range.			*/
    curcont = NONE;		    /* None at the moment.		*/

    load_attach_help();

    defbasis(BEZIER, beziermat);
    defbasis(CARDINAL, cardinalmat);
    defbasis(BSPLINE, bsplinemat);

    strcpy(basis_names[0], "BEZIER");
    strcpy(basis_names[1], "CARDINAL");
    strcpy(basis_names[2], "BSPLINE");

    makeobj(SLIDERTRANS);
	ortho2(-200.0, 200.0, -88.0, 88.0);
    closeobj();

    reset_surface();

    pushmatrix();
	perspective(450, 1.0, 1.0, 30.0);
	lookat(.5, .5, 3.0, 0.0, 0.0, 0.0, 0);
	getmatrix(downz);
    popmatrix();

    pushmatrix();
	perspective(450, 1.0, 1.0, 30.0);
	polarview(3.082, -800, 800, 0);
/*	lookat(3.0, .5, .5, 0.0, 0.0, 0.0, 0);	*/
	getmatrix(downx);
    popmatrix();

    pushmatrix();
	perspective(450, 1.0, 1.0, 30.0);
	polarview(3.082, 100, 800, 0);
/*	lookat(.5, 3.0, .5, 0.0, 0.0, 0.0, 1350);  */
	getmatrix(downy);
    popmatrix();

    pushmatrix();
	ortho2(-10.0, 360.0, -15.0, 85.0);
	getmatrix(state);
    popmatrix();
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
	    case ESCKEY:
		clean_up_and_exit();
		break;
	    case REDRAW:
		redraw_window(val);
		break;
	    case INPUTCHANGE:
		attached = val;
		if (!attached){
		    load_attach_help();
		    redraw_help();
		} else {
		    load_correct_help();
		}
		break;
	    case LEFTMOUSE:
		respond_to_leftmouse(val);
		break;
    	    case RIGHTMOUSE:
		respond_to_rightmouse(val);
		break;
	}
    }
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

redraw_help()
/*---------------------------------------------------------------------------
 * Redraw the HELP window.
 *---------------------------------------------------------------------------
 */
{
    draw_help();
    swapbuffers();
    draw_help();
}

redraw_window(n)
/*---------------------------------------------------------------------------
 * Redraw the window in the event of a redraw token.
 *---------------------------------------------------------------------------
 */
int n;
{
    if (n == helpw)    
	redraw_help();		    /* Draw the help winow.	*/
    else if (n == consw)
	redraw_cons();
    else if (n == statw)
	redraw_state();
    else if (n == backw)
	redraw_back();
    else if (n == frontxw)
	redraw_front(frontxw);		    /* Draw the curves.		*/
    else if (n == frontyw)
	redraw_front(frontyw);		    /* Draw the curves.		*/
    else if (n == frontzw)
	redraw_front(frontzw);		    /* Draw the curves.		*/
}

respond_to_rightmouse(state)
/*---------------------------------------------------------------------------
 * Respond to a rightmouse press.
 *---------------------------------------------------------------------------
 */
short state;
{
    int val;

    if (state) {
	switch(val = dopup(mainmen)) {
	    case 1:
		dopup(basisumen);
		break;
	    case 2:
		dopup(basisvmen);
		break;
	    case 4:
		if (dopup(exitmen)==1)
		    clean_up_and_exit();
		break;
	}
    }

}

respond_to_leftmouse(state)
/*---------------------------------------------------------------------------
 * Respond to someone pressing the left mouse button.
 *---------------------------------------------------------------------------
 */
short state;
{
    int what, where;

    if (state){
	what = check_for_hit(&where);
	switch(what){
	case CONTROL:
	    handle_control(where);
	    break;
	case SLIDERS:
	    handle_sliders(where);
	    break;
	default:
	    break;
        }

    } else {
	moving = FALSE;
    }

    load_correct_help();
}

int check_for_hit(vert)
/*---------------------------------------------------------------------------
 * Check the control points, slider bars and the precision for a pick hit and
 * return the values if apropriate.
 *---------------------------------------------------------------------------
 */
int *vert;
{
    retnumber = 0;

    pick_control();			    /* Check the surface. */
    if (retnumber != 0){
        *vert = (int) pick_buffer[3];
	return((int) pick_buffer[2]);
    }

    pick_precision();			    /* Check the precision. */
    if (retnumber != 0){
        *vert = (int) pick_buffer[3];
	return((int) pick_buffer[2]);
    }

    pick_number();			    /* Check the patchcurve. */
    if (retnumber != 0){
        *vert = (int) pick_buffer[3];
	return((int) pick_buffer[2]);
    }

    pick_sliders();
    if (retnumber != 0){
        *vert = (int) pick_buffer[3];
	return((int) pick_buffer[2]);
    }

    *vert = NONE;
    return(NONE);
}   

init_windows()
/*---------------------------------------------------------------------------
 * Initialize all of the windows in the system.
 *---------------------------------------------------------------------------
 */
{
    backw = init_back();
    doublebuffer();
    gconfig();
    winattach(helpw);
    helpw = init_help("Patch");
    statw = init_state();
    frontxw = init_front(DOWNX);
    frontyw = init_front(DOWNY);
    frontzw = init_front(DOWNZ);
    consw = init_cons();
}

draw_windata()
{
    long ox, oy;
    long sx, sy;

    winset(helpw);
    getorigin(&ox, &oy);
    getsize(&sx, &sy);
    printf("help: ox = %d oy = %d sx = %d sy = %d\n", ox, oy, sx, sy);

    winset(consw);
    getorigin(&ox, &oy);
    getsize(&sx, &sy);
    printf("cons: ox = %d oy = %d sx = %d sy = %d\n", ox, oy, sx, sy);

    winset(statw);
    getorigin(&ox, &oy);
    getsize(&sx, &sy);
    printf("state: ox = %d oy = %d sx = %d sy = %d\n", ox, oy, sx, sy);

    winset(frontxw);
    getorigin(&ox, &oy);
    getsize(&sx, &sy);
    printf("fx: ox = %d oy = %d sx = %d sy = %d\n", ox, oy, sx, sy);

    winset(frontyw);
    getorigin(&ox, &oy);
    getsize(&sx, &sy);
    printf("fy: ox = %d oy = %d sx = %d sy = %d", ox, oy, sx, sy);

    winset(frontzw);
    getorigin(&ox, &oy);
    getsize(&sx, &sy);
    printf("fz: ox = %d oy = %d sx = %d sy = %d\n", ox, oy, sx, sy);

}

draw_frame()
/*---------------------------------------------------------------------------
 * Draw a frame using all of the current data.
 *---------------------------------------------------------------------------
 */
{
    draw_help();		    /* Draw the help winow.	*/
    draw_cons();		    /* Draw the console.	*/
    draw_fronts();		    /* Draw the curves.		*/
    draw_state();
}
