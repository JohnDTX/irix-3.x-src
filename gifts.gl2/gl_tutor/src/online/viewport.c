/*---------------------------------------------------------------------------
 *  viewport.c							
 *  April, 1986 -- Mason Woo					
 *  Learning Environment -- to demonstrate mapping a 3D viewing	
 *  volume, in this case defined by the projection 		
 *  transformation of ortho, to a 2D area of the screen.  The	
 *  screen boundary is defined by the viewport command.		
 *---------------------------------------------------------------------------
 */


#include <gl.h>
#include <device.h>
#include "tutor.h"

#define DOTTED	1	    /* Reference number for the linestyle table.    */
/*---------------------------------------------------------------------------
 * These colors will be set to different values depending on the number of
 * bitplanes that are on the system.  If there are not enough bitplanes to
 * display all of the colors required by the program, some of these numbers
 * will be set eqaul to others, thus using less colors.
 *---------------------------------------------------------------------------
 */



int	volcolor, 	    /* Color of the box representing the orthographics
			       viewing volume.				    */

	maplcolor, 	    /* Color of the dotted lines mapping the viewport
			       to the orthographics viewing volume.	    */

	screencolor, 	    /* Color of the square representing the viewport
			       window.					    */

	obj1color, 	    /* One of the three colors the F's are drawn in.*/

	obj2color, 	    /* One of the three colors the F's are drawn in.*/

	obj3color, 	    /* One of the three colors the F's are drawn in.*/

	ptcolor;	    /* Color of the small squares and their
			       coordinates are drawn in.		    */

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

Screencoord vleft, 	    /* X coordinate of the left hand side of the
			       viewport window in the 'SCREEN SPACE' window.*/

	    vright, 	    /* X coordinate of the right hand side of the
			       viewport window in the 'SCREEN SPACE' window.*/

	    vbottom, 	    /* Y coordinate of the bottom  side of the
			       viewport window in the 'SCREEN SPACE' window.*/

	    vtop;	    /* Y coordinate of the bottom side of the
			       viewport window in the 'SCREEN SPACE' window.*/

Coord	    left, 	    /* X coordinate of the left hand side of the box
			       representing the orthographics viewing volume
			       in the 'WORLD SPACE' window.		    */

	    right, 	    /* X coordinate of the right hand side of the box
			       representing the orthographics viewing volume
			       in the 'WORLD SPACE' window.		    */

	    bottom, 	    /* Y coordinate of the bottom side of the box
			       representing the orthographics viewing volume
			       in the 'WORLD SPACE' window.		    */

	    top, 	    /* Y coordinate of the top side of the box
			       representing the orthographics viewing volume
			       in the 'WORLD SPACE' window.		    */

	    near, 	    /* Z coordinate of the front side of the box
			       representing the orthographics viewing volume
			       in the 'WORLD SPACE' window.		    */

	    far; 	    /* Z coordinate of the back side of the box
			       representing the orthographics viewing volume
			       in the 'WORLD SPACE' window.		    */

int	    menu,	    /* ID number of the main popup menu.	    */
	    killmenu;	    /* ID number of the kill conmfirmation menu.    */
    
int	    backwin, 	    /* ID number of the background (GREY) window.   */
	    helpwin, 	    /* ID number of the 'INFORMATION' window.	    */
	    viewwin, 	    /* ID number of the 'SCREEN SPACE' window. 	    */
	    volwin,	    /* ID number of the 'WORLD SPACE' window.	    */
	    statuswin, 	    /* ID number of the 'STATUS' window.	    */
	    conswin;	    /* ID number of the 'CONTROL BAR' window.	    */

int	    active;	    /* TRUE if this program is attached.	    */

main()
{
    short	qval;
    long	dev;
    short	chosen;
    short	pickme(), pickval;
    int	really;

    /* If the computer isn't running mex, the program cannot be run.  This
       routine checks to make sure of this. */

    if (!ismex()) {
        printf(
            "You must be running the window manager to use this program.\n");
        printf("Type mex to start.\n");

        exit(0);
    }

    active = FALSE;

    init_windows();

    setupcolors();
    setupqueue();
    reset_values();
    setupmenus();

    while (1) {

        /* This loops until the queue is empty. */
        while (qtest()) {

            /* Read the event queue and respond according to the event. */
            dev = qread(&qval);
            switch (dev) {

            case LEFTMOUSE:
                /* In this case, the LEFT MOUSE button has been
		   pressed. The program then calls the routine
		   pickme(), which then checks, with picking, to see
		   if the cursor is over any parts of the screen that
		   contain an adjustable parameter. If so, it selects
		   it. */

                if (qval) {		/* If PRESSED. */

                    pickval = pickme();
                    if ((pickval > 0) && (pickval < 11))
                        select_parameter(pickval);

		     else if (pickval != 0)
                        /* The cursor is over the slider bar. */
                        holding = qval;

                } else {		/* If released. */
                    holding = FALSE;
		}

                break;

            case RIGHTMOUSE:

                /* In this case, the RIGHT MOUSE button has been pressed.
		   The program then calls dopup() to bring up the popup
	           menu. It then responds accordingly to the returned
		   value. */

                if (qval)	    /* If pressed. */
                    switch (dopup(menu)) {

                    case 1:
			/* If first item is selected, reset the 
			   paramters to the default values. */

                        reset_values();
                        break;

                    case 2:
                        /* Confirm the exit request. */
                        really = dopup(killmenu);

                        if (really == 1) {
			    tutorrestoremap();
                            gexit();
                            exit(0);
                        }
                        break;

                    default:
                        break;
                    }

                break;

            case REDRAW:

                /* In this case, a REDAW token has been sent down the
		   queue. A REDRAW is generated when a window has been
		   uncovered or reshaped and it is requiered that the
		   program redraw the scene to fill in the previously
		   covered area. */

                redraw_window(qval);
                break;

            case INPUTCHANGE:

                active = qval;
                if (active == FALSE) {

                    /* If the program has been de-attached, then draw
			   everything in both buffers swap buffers until an
			   attach token comes down the queue, this is an
			   INPUTCHAGE with qval == TRUE. */

                    draw_frame();
                    swapbuffers();
                    draw_frame();

                }

                break;

                case (ESCKEY):
                    gexit();
		    exit(0);
	            break;

            default:
                break;

            }

            /* Sit idle and wait if de-attached. */
            if (!active)
                while (!qtest())
                    swapbuffers();
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
setupqueue() {
    qdevice(LEFTMOUSE);
    qdevice(RIGHTMOUSE);
    qdevice(INPUTCHANGE);
    qdevice(REDRAW);
    qdevice(ESCKEY);
}

/*---------------------------------------------------------------------------
 * This routine first saves the current values of all of the color indicies
 * that it will use.  And then creates it's own colors for the program
 * according to how many bitplanes are available on the machine.
 *---------------------------------------------------------------------------
 */
setupcolors() {

    int	i;
    Linestyle dotted = 0xaaaa;

    deflinestyle(DOTTED, dotted);

    if (getplanes() < 4) {
        printf("You do not have enough bitplanes for this program\n");
        gexit();
        exit(0);
    }

    tutorsavemap();		/* Save the current color map. 	*/
    tutormakemap();		/* Create a new color map.	*/

    if (getplanes() < 6) {
        volcolor = GREEN;
        maplcolor = YELLOW;
        screencolor = RED;
        obj1color = GREEN;
        obj2color = GREEN;
        obj3color = GREEN;
        ptcolor = YELLOW;

    } else {
        volcolor = 16;
        maplcolor = 17;
        screencolor = 18;
        obj1color = 19;
        obj2color = 20;
        obj3color = 21;
        ptcolor = 22;
        mapcolor(volcolor, 200, 200, 0);
        mapcolor(maplcolor, 200, 100, 0);
        mapcolor(screencolor, 100, 250, 50);
        mapcolor(obj1color, 0, 200, 200);
        mapcolor(obj2color, 175, 175, 100);
        mapcolor(obj3color, 200, 0, 200);
        mapcolor(ptcolor, 200, 200, 200);
    }
}

/*---------------------------------------------------------------------------
 * This routine sets all of the adjustable parameters in the program back to
 * their default values.
 *---------------------------------------------------------------------------
 */
reset_values() {

    left = 100.0;
    right = 500.0;
    bottom = 100.0;
    top = 500.0;
    near = 0.0;
    far = 600.0;
    vleft = 50;
    vright = 350;
    vbottom = 50;
    vtop = 350;
}

/*---------------------------------------------------------------------------
 * This routine defines what all of the menus in the program wil say.
 *---------------------------------------------------------------------------
 */
setupmenus() {

    /* The main menu of the program. */
    menu = defpup("Viewport %t|reset values|exit program");

    /* The kill confirmation menu. */
    killmenu = defpup("Do you want to exit? %t|yes|no");

    /* When defining a menu, the %t to the right of an entry signifies that it
       is the title bar of the menu.  "Veiwport %t|..." is the title section 
       of the main menu, and "Do you want to exit? %t|..." is the title bar of
       the exit confirmation menu. */

    /* See the IRIS Users Guide for more information on popup menus. */
}

/*---------------------------------------------------------------------------
 * This routine, given a paramter number, sets line to the line that the
 * parameter is on, and paramter to the paramter number on that line.
 *---------------------------------------------------------------------------
 */
select_parameter(newparm)
int	newparm;
{
    if (newparm < 0) {
        line = 0;
        parameter = 0;

    } else {
        line = ((newparm - 1) / 4) + 1;
        parameter = ((newparm - 1) % 4) + 1;

        if (line == 3) {
            parameter = parameter + 4;
            line = 2;
        }
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
short	pickme()
{

    short	buffer[100];
    long	numpicked;
    int		i;

    attach_to_status();

    pushmatrix();
        pick(buffer, 100);

        /* The viewing transformation must be re-established because pick
	       destroys the current matrix. */
        ortho2(0.0, 140.0, 0.0, 6.0);

        initnames();

        for (i = 0; i < 4; i++) {	/*  viewport arguments		*/
            loadname(i + 1);
            rectfi(15 * (i + 2), 3, 15 * (i + 3), 5);
        }

        for (i = 0; i < 6; i++) {	/*  ortho arguments		*/
            loadname(i + 5);
            rectfi(15 * (i + 2), 1, 15 * (i + 3), 3);
        }

        numpicked = endpick(buffer);
    popmatrix();

    if (numpicked){
        return(buffer[1]);

    } else {
        attach_to_cons();

        pushmatrix();
            pick(buffer, 100);

            ortho2(-250.0, 250.0, -63.0, 63.0);

            initnames();
            loadname(11);

            rectfi(-150, -5, 149, 5);    /*  Controller bar		*/

            numpicked = endpick(buffer);
        popmatrix();

        if (numpicked)
            return(buffer[1]);
        else
            return(0);
    }
}

/*---------------------------------------------------------------------------
 * This routine reads the mouse location and interprets it's x coordinate as
 * the value of the slider bar.  After wrapping around the ends, it adjusts
 * the correct parameter to the value of the bar.
 *---------------------------------------------------------------------------
 */
change_parameter(){

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
        dx  = iwx + 150;

        /* Modify the correct perameter. */
        if (line == 1)
            modify_view();
        else if (line == 2)
            modify_ortho();
    }
}

/*---------------------------------------------------------------------------
 * This routine just displays the number number in floating point format at 
 * row, column of the parameter list and puts the string string just above it.
 * If last is FALSE, it puts a comma after the number.
 *---------------------------------------------------------------------------
 */
print_float(row, column, string, number, last)
int	row,
	column;
char	string[];
float	number;
Boolean last;
{
    int	    x, y;
    static buffer[20];

    x = 15 + 15 * column;
    y = 5 - 2 * row;

    /* This displays the number in the highlighted color if it is the current
       parameter. */

    if ((parameter == column) && (line == row))
        color(HIGHCOLOR);
    else
        color(NORMCOLOR);


    cmov2i(x, y + 1);
    charstr(string);

    sprintf(buffer, "%6.1f", number);
    cmov2i(x, y);
    charstr(buffer);

    color(NORMCOLOR);

    if (last)

/* This #ifdef command checks to see if FORTRAN is defined, if so it compiles
   in the fortran version of the command, otherwiseit compiles in the C
   version of the command. */

#ifdef FORTRAN
	charstr (" )");
#else
	charstr (" );");
#endif

    else
        charstr(",");
}

/*---------------------------------------------------------------------------
 * This routine just displays the number number in integer format at 
 * row, column of the parameter list and puts the string string just above it.
 * If last is FALSE, it puts a comma after the number.
 *---------------------------------------------------------------------------
 */
print_int(row, column, string, number, last)

int	row,
	column;
char	string[];
int	number;
{
    int	x, y;

    static buffer[20];

    x = 15 + 15 * column;
    y = 5 - 2 * row;

    /* This displays the number in the highlighted color if it is the current
       parameter. */

    if (parameter == column && line == row)
        color(HIGHCOLOR);
    else
        color(NORMCOLOR);

    cmov2i(x, y + 1);
    charstr(string);

    sprintf(buffer, "%6d", number);
    cmov2i(x, y);
    charstr(buffer);

    color(NORMCOLOR);

    if (last)
#ifdef FORTRAN
	charstr (" )");
#else
	charstr (" );");
#endif
    else
        charstr(",");
}

/*---------------------------------------------------------------------------
 * Modify the current parameter in the ortho command.
 *---------------------------------------------------------------------------
 */
modify_ortho() {

    switch (parameter) {

    case 1:
        left = low_pos + (float) dx + (wrap * 300.0);
        break;

    case 2:
        right = low_pos + (float) dx + (wrap * 300.0);
        break;

    case 3:
        bottom = low_pos + (float) dx + (wrap * 300.0);
        break;

    case 4:
        top = low_pos + (float) dx + (wrap * 300.0);
        break;

    case 5:
        near = low_pos + (float) dx + (wrap * 300.0);
        break;

    case 6:
        far = low_pos + (float) dx + (wrap * 300.0);
        break;

    default:
        break;
    }
}

/*---------------------------------------------------------------------------
 * Modify the current paramter in the viewport command.
 *---------------------------------------------------------------------------
 */
modify_view() {

    switch (parameter) {

    case 1:
        vleft = low_pos + (Scoord) dx + (Scoord)(wrap * 300.0);
        break;

    case 2:
        vright = low_pos + (Scoord) dx + (Scoord)(wrap * 300.0);
        break;

    case 3:
        vbottom = low_pos + (Scoord) dx + (Scoord)(wrap * 300.0);
        break;

    case 4:
        vtop = low_pos + (Scoord) dx + (Scoord)(wrap * 300.0);
        break;

    default:
        break;
    }
}

/*---------------------------------------------------------------------------
 * Draw the reference 'F's.
 *---------------------------------------------------------------------------
 */
draw_refobjs() {

    color(obj2color);
    draw_f(200.0, 100.0, 100.0);

    color(obj1color);
    draw_f(300.0, 200.0, -100.0);

    color(obj2color);
    draw_f(600.0, 400.0, -300.0);

    color(obj3color);
    draw_f(100.0, 350.0, -500.0);

    color(obj1color);
    draw_f(400.0, 50.0, -700.0);
}

/*---------------------------------------------------------------------------
 * Draw the reference squares.
 *---------------------------------------------------------------------------
 */
draw_refsqs() {

    draw_sq(400.0, 800.0, -500.0);
    draw_sq(200.0, 600.0, -200.0);
    draw_sq(100.0, 500.0, 100.0);
    draw_sq(400.0, 450.0, -500.0);
    draw_sq(-200.0, 400.0, -100.0);
    draw_sq(200.0, 350.0, -400.0);
    draw_sq(100.0, 300.0, -700.0);
    draw_sq(300.0, 200.0, -300.0);
    draw_sq(500.0, 100.0, -300.0);
    draw_sq(0.0, 0.0, 0.0);
    draw_sq(300.0, -200.0, -200.0);
}

/*---------------------------------------------------------------------------
 * Draw the reference square coordinates.
 *---------------------------------------------------------------------------
 */
draw_refpts() {

    draw_pt(400.0, 800.0, -500.0);
    draw_pt(200.0, 600.0, -200.0);
    draw_pt(100.0, 500.0, 100.0);
    draw_pt(400.0, 450.0, -500.0);
    draw_pt(-200.0, 400.0, -100.0);
    draw_pt(200.0, 350.0, -400.0);
    draw_pt(100.0, 300.0, -700.0);
    draw_pt(300.0, 200.0, -300.0);
    draw_pt(500.0, 100.0, -300.0);
    draw_pt(0.0, 0.0, 0.0);
    draw_pt(300.0, -200.0, -200.0);
}

/*---------------------------------------------------------------------------
 * Routine that draws an 'F' at (x, y, z).
 *---------------------------------------------------------------------------
 */
draw_f(x, y, z)
float	x, y, z;
{
    pmv(x + 20.0, y, z);    /*  draw F upright poly  	*/
    pdr(x + 20.0, y + 100.0, z);
    pdr(x + 40.0, y + 100.0, z);
    pdr(x + 40.0, y, z);
    pclos();

    pmv(x + 40.0, y + 80.0, z); /*  draw F topbar poly 	*/
    pdr(x + 40.0, y + 100.0, z);
    pdr(x + 80.0, y + 100.0, z);
    pdr(x + 80.0, y + 80.0, z);
    pclos();

    pmv(x + 40.0, y + 40.0, z); /*  draw F middlebar poly	*/
    pdr(x + 40.0, y + 60.0, z);
    pdr(x + 60.0, y + 60.0, z);
    pdr(x + 60.0, y + 40.0, z);
    pclos();
}

/*---------------------------------------------------------------------------
 * Routine to draw a square at (x, y, z).
 *---------------------------------------------------------------------------
 */
draw_sq(x, y, z)
float	x, y, z;
{
    pmv(x - 3.0, y - 3.0, z);
    pdr(x + 3.0, y - 3.0, z);
    pdr(x + 3.0, y + 3.0, z);
    pdr(x - 3.0, y + 3.0, z);
    pclos();
}

/*---------------------------------------------------------------------------
 * Routine to draw the coordinates (x, y, z) at (x, y, z).
 *---------------------------------------------------------------------------
 */
draw_pt(x, y, z)
float	x, y, z;
{

    static char	buffer[20];

    cmov(x + 4.0, y - 5.0, z);
    charstr("(");

    sprintf(buffer, "%5.1f,", x);
    charstr(buffer);

    sprintf(buffer, "%5.1f,", y);
    charstr(buffer);

    sprintf(buffer, "%5.1f", z);
    charstr(buffer);

    charstr(")");
}

/*---------------------------------------------------------------------------
 * Initialize the five windows for the program.
 *---------------------------------------------------------------------------
 */
init_windows() {

    /* Each init_ routine returns the window ID of the window that was opened.
     * This ID number is used with winattach(ID) and winset(ID) to permit the
     * program to draw into the window.*/

    backwin = init_back();

    /* All windows must run in the same mode, the first window brought up must
       be initialized with that mode. */

    doublebuffer();
    gconfig();

    /* The program sould come up attached. */
    winattach();

    helpwin = init_help("Viewport -- INFORMATION");
    viewwin = init_view();
    conswin = init_cons();
    statuswin = init_status();
    volwin = init_vol();
}

/*---------------------------------------------------------------------------
 * Initialize the status window.
 *---------------------------------------------------------------------------
 */
init_status() {

    int	res;

    /* Don't prompt the user to position the window, just bring it up on the
       screen.  This is done with the prefposition() command. */

    prefposition(422, 1022, 480, 580);

    /* winopen() returns the window ID number, used in the winattach() and
       winselect() commands. */

    res = winopen("status");
    wintitle("Viewport -- STATUS");
    prefsize(600, 100);

    /* winconstraints() simply ignores all of the window constraint commands 
       issued before the winopen() command, prefposition() in this case, 
       and replaces them with all of the commands issued after the 
       winopen() command, prefsize() in this case. */

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
    wintitle("Viewport -- CONTROL BAR");
    prefsize(500, 126);
    winconstraints();
    setup_cons_environ();

    return(res);
}

/*---------------------------------------------------------------------------
 * Initialize the view window.
 *---------------------------------------------------------------------------
 */
init_view() {

    int	res;

    prefposition(112, 512, 50, 450);
    res = winopen("view");
    prefsize(400, 400);
    wintitle("Viewport -- SCREEN SPACE");
    winconstraints();
    setup_view_environ();

    return(res);
}

/*---------------------------------------------------------------------------
 * Initialize the vol window.
 *---------------------------------------------------------------------------
 */
init_vol() {

    int	res;

    prefposition(522, 922, 50, 450);
    res = winopen("vol");
    prefsize(400, 400);
    wintitle("Viewport -- WORLD SPACE");
    winconstraints();
    setup_vol_environ();

    return(res);
}

/*---------------------------------------------------------------------------
 * Set the console environment.
 *---------------------------------------------------------------------------
 */
setup_cons_environ() {

    reshapeviewport();

    /* Set up the viewing transformation for this window. */
    ortho2(-250.0, 250.0, -63.0, 63.0);
}

/*---------------------------------------------------------------------------
 * Set the status environment.
 *---------------------------------------------------------------------------
 */
setup_status_environ() {

    reshapeviewport();
    ortho2(0.0, 140.0, 0.0, 6.0);
}

/*---------------------------------------------------------------------------
 * Set up the environment used for the view window.
 *---------------------------------------------------------------------------
 */
setup_view_environ() {

    reshapeviewport();
    ortho2(0.0, 400.0, 0.0, 400.0);
}

/*---------------------------------------------------------------------------
 * Set up the environment used for the vol window.
 *---------------------------------------------------------------------------
 */
setup_vol_environ() {

    reshapeviewport();
    ortho(-1000.0, 1000.0, -1000.0, 1000.0, 1.0, 10000.0);

    /* Position the eye. */
    lookat(-550.0, -450.0, 450.0, 0.0, 0.0, 0.0, 0);

}

/*---------------------------------------------------------------------------
 * Attach to the view window.
 *---------------------------------------------------------------------------
 */
attach_to_view(){

    winset(viewwin);
}

/*---------------------------------------------------------------------------
 * Direct graphics output to the status window.
 *---------------------------------------------------------------------------
 */
attach_to_status() {

    winset(statuswin);
}

/*---------------------------------------------------------------------------
 * Direct graphics output to the console window.
 *---------------------------------------------------------------------------
 */
attach_to_cons(){

    winset(conswin);
}

/*---------------------------------------------------------------------------
 * Direct graphics output to the vol window.
 *---------------------------------------------------------------------------
 */
attach_to_vol() {

    winset(volwin);
}

/*---------------------------------------------------------------------------
 * Routine called when a redraw token is sent down the queue.
 *---------------------------------------------------------------------------
 */
redraw_help() {
    reshapeviewport();

    /* Draw the scene in both buffers.*/
    draw_help();
    swapbuffers();
    draw_help();
    
}

/*---------------------------------------------------------------------------
 * Routine called when a redraw token is sent down the queue.
 *---------------------------------------------------------------------------
 */
redraw_view() {

    attach_to_view();

    reshapeviewport();

    /* Draw the scene in both buffers.*/
    draw_view_window();
    swapbuffers();
    draw_view_window();
}

/*---------------------------------------------------------------------------
 * Routine called when a redraw token is sent down the queue.
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
 * Routine called in the event of a redraw token for the cons window coming
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
 * Routine called in the event of a redraw token for the vol window coming
 * down the event queue.
 *---------------------------------------------------------------------------
 */
redraw_vol() {

    attach_to_vol();

    reshapeviewport();
    draw_vol_window();
    swapbuffers();
    draw_vol_window();
}

/*---------------------------------------------------------------------------
 * Draw everything in the view window.
 *---------------------------------------------------------------------------
 */
draw_view_window() {

    int	check_parameters();
    int	vwidth, vheight;
    int	vtemptop, vtempright;

    float	newtop, newright;
    float	hratio, wratio;
    float	temptop, tempright;
    float	owidth, oheight;

    /* Output to the 'SCREEN SPACE' window. */
    attach_to_view();

    /* Clear the screen. */
    color(BLACK);
    clear();

    if (check_parameters()) {
        pushmatrix();
            color(screencolor);

            /*  Draw the viewport rectangle in the window. */
		rects(vleft, vbottom, vright, vtop);

            vwidth = vright - vleft;
            vheight = vtop - vbottom;

            /*  If the viewport is larger than allotted area provided by the 
		'SCREEN SPACE' window, then make  the viewport maximum size
		and adjust the ortho() command to compensate. */

            vtempright = vright;
            vtemptop = vtop;
            tempright = right;
            temptop = top;

            /* Adjust left to right. */

            if (vright > 399) {
                vtempright = 399;
                wratio = (399.0 - (float) vleft) / (float) vwidth;
                tempright = right * wratio;
            }

            /* Adjust bottom to top. */

            if (vtop > 399) {
                vtemptop = 399;
                hratio = (399.0 - (float) vbottom) / (float) vheight;
                temptop = top * hratio;
            }

            /* Create new view. */
            viewport(vleft, vtempright, vbottom, vtemptop);
            ortho(left, tempright, bottom, temptop, near, far);

            /* Draw all of the reference objects. */
            draw_refobjs();
            color(ptcolor);
            draw_refsqs();

            /* Adjust the viewport and screenmask to make sure that the text 
	       is finely clipped, not grossly clipped.  If the viewport is 
	       already substantially off-screen, then don't bother trying to
	       calculate a good screenmask.  For more information on 
	       scrmask(), refer to the Iris Users Guide. */

            if (( vleft > -100 ) && ( vbottom > -20 )) {
                vwidth = vtempright - vleft;
                vheight = vtemptop - vbottom;
                owidth = tempright - left;
                oheight = temptop - bottom;

                reshapeviewport();
                viewport(vleft - vwidth, vtempright, 
                    vbottom - vheight, vtemptop);
                ortho(left - owidth, tempright, bottom - oheight, 
                    temptop, near, far);
                scrmask(vleft, vtempright, vbottom, vtemptop);
            }

            color(ptcolor);

            /* Draw the square coordinates. */
            draw_refpts();
        popmatrix();

	/* This restores everything scrmask() might have screwed up. */
        reshapeviewport();
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

    color(NORMCOLOR);

    /* Display the viewport() command. */
    cmov2i(5, 3);

#ifdef FORTRAN
    charstr ("CALL VIEWPO (");
#else
    charstr ("viewport (");
#endif

    print_int(1, 1, "  left", vleft, FALSE);
    print_int(1, 2, " right", vright, FALSE);
    print_int(1, 3, "bottom", vbottom, FALSE);
    print_int(1, 4, "   top", vtop, TRUE);

    /* Display the ortho() command. */
    cmov2i(5, 1);

#ifdef FORTRAN
    charstr ("CALL ORTHO (");
#else
    charstr ("ortho    (");
#endif

    print_float(2, 1, "  left", left, FALSE);
    print_float(2, 2, " right", right, FALSE);
    print_float(2, 3, "bottom", bottom, FALSE);
    print_float(2, 4, "   top", top, FALSE);
    print_float(2, 5, "  near", near, FALSE);
    print_float(2, 6, "   far", far, TRUE);
}

/*---------------------------------------------------------------------------
 * Draw everything in the console window.  The slider bar,  basically.
 *---------------------------------------------------------------------------
 */
draw_cons_window() {

    /* Output to the 'CONSOLE' window. */
    attach_to_cons();

    color(BLACK);
    clear();

    color(UNPICKCOLOR);
    cmov2i(-100, 15);

    if (line == 1) {
        switch (parameter) {

        case 1:
            /* This is the lable to display above the slider bar. */
            charstr(" left - viewport  ");
            draw_slider_int(vleft);
            break;

        case 2:
            /* This is the lable to display above the slider bar. */
            charstr(" right - viewport ");
            draw_slider_int(vright);
            break;

        case 3:
            /* This is the lable to display above the slider bar. */
            charstr("bottom - viewport ");
            draw_slider_int(vbottom);
            break;

        case 4:
            /* This is the lable to display above the slider bar. */
            charstr("  top - viewport  ");
            draw_slider_int(vtop);
            break;

        default:
            /* This is the lable to display above the slider bar. */
            charstr("  Controller Bar   ");
            draw_blank_slider(0.0, 0.0, 100, 10);
            break;
        }

    } else if (line == 2) {
        switch (parameter) {

        case 1:
            /* This is the lable to display above the slider bar. */
            charstr("  left - ortho   ");
            draw_slider_float(left);
            break;

        case 2:
            /* This is the lable to display above the slider bar. */
            charstr("  right - ortho  ");
            draw_slider_float(right);
            break;

        case 3:
            /* This is the lable to display above the slider bar. */
            charstr(" bottom - ortho  ");
            draw_slider_float(bottom);
            break;

        case 4:
            /* This is the lable to display above the slider bar. */
            charstr("   top - ortho   ");
            draw_slider_float(top);
            break;

        case 5:
            /* This is the lable to display above the slider bar. */
            charstr("  near - ortho   ");
            draw_slider_float(near);
            break;

        case 6:
            /* This is the lable to display above the slider bar. */
            charstr("   far - ortho   ");
            draw_slider_float(far);
            break;

        default:
            /* This is the lable to display above the slider bar. */
            charstr("  Controller Bar   ");
            draw_blank_slider(0.0, 0.0, 100, 10);
            break;
        } 
    } else {

        /* This is the lable to display above the slider bar. */
        charstr("  Controller Bar   ");
        draw_blank_slider(0.0, 0.0, 100, 10);
    }
}

/*---------------------------------------------------------------------------
 * Draw the 'WORLD SPACE' window.
 *---------------------------------------------------------------------------
 */
draw_vol_window() {

    /* Output to the 'WORLD SPACE' window. */
    attach_to_vol();

    color(BLACK);
    clear();

    pushmatrix();
        /*  Draw the rectangualar volume representing the orthographic 
	    projection transformation volume. */

        color(volcolor);

        /*  Draw near clipping plane. */
        move(left, bottom, near);
        draw(left, top, near);
        draw(right, top, near);
        draw(right, bottom, near);
        draw(left, bottom, near);

        /*  Draw the rest of the box. */
        move(right, bottom, near);
        draw(right, bottom, far);
        draw(right, top, far);
        draw(right, top, near);
        move(right, top, far);
        draw(left, top, far);
        draw(left, top, near);

        cmov(100.0, bottom - 500.0, near);
        charstr("3-D world space");

        /* Draw the reference objects in the 3-D orthographic volume. */
        pushmatrix();
            scale(1.0, 1.0, -1.0);
            draw_refobjs();
            color(ptcolor);
            draw_refsqs();
        popmatrix();

        /* Draw the dotted lines showing mapping between the orthogonal 
	   volume, the near clipping plane,  and the screen. */

        /* The DOTTED linestye was defined in setupcolors(). */

        setlinestyle(DOTTED);
        color(maplcolor);

        move(left, bottom, near);
        draw((float) vleft, (float) vbottom, -1000.0);

        move(left, top, near);
        draw((float) vleft, (float) vtop, -1000.0);

        move(right, bottom, near);
        draw((float) vright, (float) vbottom, -1000.0);

        move(right, top, near);
        draw((float) vright, (float) vtop, -1000.0);

        /* Linestyle 0 always refers to the solid, default, linestyle. */

        setlinestyle(0);

        /* Draw the rectangel representing the screen. */

        translate(0.0, 0.0, -1000.0);

        color(screencolor);
        rects(vleft, vbottom, vright, vtop);

        cmov2(0.0, (float) vbottom - 500.0);
        charstr("2-D screen space");
    popmatrix();
}

/*---------------------------------------------------------------------------
 * Cycle through each window and draw the most current contents of each.
 *---------------------------------------------------------------------------
 */
draw_frame() {

    draw_status_window();
    draw_cons_window();
    draw_view_window();
    draw_vol_window();
    draw_help();
}

/*---------------------------------------------------------------------------
 * Given a window ID n, call the correct redraw routine to redraw the
 * appropriate window.
 *---------------------------------------------------------------------------
 */
redraw_window(n)
int	n;
{
    if (n == helpwin)
        redraw_help();

    else if (n == conswin)
        redraw_cons();

    else if (n == statuswin)
        redraw_status();

    else if (n == viewwin)
        redraw_view();

    else if (n == volwin)
        redraw_vol();

    else if (n == backwin)
        redraw_back();
}

/*---------------------------------------------------------------------------
 * Given an integer input, draw the slider with the boundry numbers displayed
 * in integer format.
 *---------------------------------------------------------------------------
 */
draw_slider_int(pos)
int	pos;
{

    /* Check to see if pos is out of the boundry values and cycle the slider
       values if needed. */

    if ( (pos < 0) || ((pos == 0) && (low_pos < 0)) ) {
        high_pos = (float)((pos / 300) * 300);
        low_pos = high_pos - 299.0;
        pos = pos - (int) low_pos + 1;

    } else {
        low_pos = (float)((pos / 300) * 300);
        high_pos = low_pos + 299.0;
        pos -= (int) low_pos;
    }

    /* Draw the slider. */
    draw_int_slider(0.0, 0.0, 100, 10, (float) pos, low_pos, high_pos);
}

/*---------------------------------------------------------------------------
 * Given an floating point input, draw the slider with the boundry numbers 
 * displayed in floating point format.
 *---------------------------------------------------------------------------
 */
draw_slider_float(val)
float	val;
{

    int	pos;
    static char	buffer[10];

    /* Check to see if pos is out of the boundry values and cycle the slider
       values if needed. */

    if ( (val < 0.0) || ((val == 0.0) && (low_pos < 0.0)) ) {
        high_pos = (float)((int)((val - 0.01) / 300.0) * 300.0);
        low_pos = high_pos - 299.0;
        pos = (int)(val - low_pos) + 1;

    } else {
        low_pos = (float)((int)(val / 300.0) * 300.0);
        high_pos = low_pos + 299.0;
        pos = (int)(val - low_pos);
    }

    /* Draw the slider. */
    draw_slider(0.0, 0.0, 100, 10, (float)pos, low_pos, high_pos);
}

/*---------------------------------------------------------------------------
 * Load the 'INFORMATION' window with the appropriate message given the 
 * current state of the machine, as indicated by the integer message.
 *---------------------------------------------------------------------------
 */
load_correct_help(message)
int	message;
{

    if (!active) {
        /* Message to print if the program is not attached. */

        load_help("Press the RIGHT MOUSE button and attach",
            "to the program from the popup menu.", "", "", "", "");
        return;
    }

    switch (message) {

    case 1:
        load_help("viewport--values for the left and right",
		  "clipping planes eliminate the entire area", 
		  "of the screen.  The left edge is not allowed",
		  "to be greater than or equal to the right edge.",
		  "", "");
        break;

    case 2:
        load_help("viewport--values for the bottom and top",
		  "clipping planes eliminate the entire area", 
		  "of the screen.  The bottom edge is not allowed",
		  "to be greater than or equal to the top edge.",
		  "", "");
        break;

    case 3:
        load_help("viewport parameters outside(1, 399, 1, 399)",
		  "do not fit in this viewing window, but",
		  "they ARE LEGAL AND OFTEN USEFUL",
		  "in your own programs.", "", "");
        break;

    case 4:
        load_help("ortho -- the left edge cannot equal the right",
		  "edge.", "", "", "", "");
        break;

    case 5:
        load_help("ortho -- the bottom edge cannot equal the top",
		  "edge.", "", "", "", "");
        break;

    case 6:
        load_help("ortho -- the near edge cannot equal the far",
		  "edge.", "", "", "", "");
        break;

    case 7:
        load_help("Use the LEFT MOUSE to select a parameter",
		  "from the STATUS window, OR use the RIGHT MOUSE",
		  "to bring up the popup menu.", "", "", "");
        break;

    case 8:
        load_help("Use the LEFT MOUSE to adjust the highlighted", 
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
 * This routine examines the values of all of the parameters and displays the
 * appropriate message in the 'INFORMATION' window.
 *---------------------------------------------------------------------------
 */
int	check_parameters() {

    int	toreturn;
    int	earlyreturn;

    toreturn = TRUE;
    earlyreturn = FALSE;

    if ((vleft >= vright) || (vbottom >= vtop) || 
        (left == right) || (bottom == top) || (near == far)) {

        toreturn = FALSE;
        earlyreturn = TRUE;

    } else if ((vleft < 1) || (vright > 399) || 
        (vbottom < 1) || (vtop > 399)) {

        toreturn = TRUE;
        earlyreturn = TRUE;
    }

    /* Check the viewport() parameters. */
    if (vleft >= vright) {
        load_correct_help(1);

    } else if (vbottom >= vtop) {
        load_correct_help(2);

    } else if ((vleft < 1) || (vright > 399) || 
        (vbottom < 1) || (vtop > 399)) {
        load_correct_help(3);

    }

    /* Check ortho2() parameters. */

    if (left == right) {
        load_correct_help(4);

    } else if (bottom == top) {
        load_correct_help(5);

    } else if (near == far) {
        load_correct_help(6);
    }

    /* If a message other than a normally printed one, exit the 
       routine early. */

    if (earlyreturn)
        return(toreturn);

    /* No parameter has been chosen yet. */
    if (parameter == 0) {
        load_correct_help(7);

    } else {
        load_correct_help(8);
    }

    return(TRUE);
}
