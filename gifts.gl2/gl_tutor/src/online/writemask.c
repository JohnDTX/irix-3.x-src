/*  April, 1986 -- Thant Tessman 				*/
/*  Learning Environment -- to demonstrate writemask and 	*/
/*  how to set up colors to create overlays			*/

#include <gl.h>
#include <device.h>
#include "tutor.h"

#define EXITSOON	5

int     active = FALSE;
Colorindex  colour,
	    mask;

char buffer[80];

Boolean color_bit[3], mask_bit[3];
Boolean near_exit=FALSE;
Object shapes = 1;

int backwin,
    helpwin,
    paintwin, 
    conswin;

main () {
    short   val;
    long    dev;
    int	    mainmenu;
    int	    pickme(), power();
    int     i;

    if(!ismex()){
	printf("You must be running the window manager to use this program.\n");
	printf("Type mex to start.\n");
	exit(0);
    }

    init_windows();

    setupcolors();
    setupqueue ();

    makeobj (shapes);
    closeobj ();

    mainmenu = defpup("Writemask %t|Please read the information window.");

    color_bit[2]=1; color_bit[1]=1; color_bit[0]=1;
    mask_bit[2]=1; mask_bit[1]=1; mask_bit[0]=1;

    load_correct_help();
    draw_menu(-1);
    make_over_map();
    draw_map();

    while (TRUE) {
	while (qtest ()) {
	    dev = qread (&val);
	    switch (dev) {
		case LEFTMOUSE:
		    if (val) {
			execute_menu(pickme());
		    }
		    break;
		case RIGHTMOUSE:
		    if (val) dopup(mainmenu);
		    break;
		case REDRAW: 
		    redraw_window (val);
		    break;
		case INPUTCHANGE: 
		    active = val;
		    if (active == FALSE) {
			load_correct_help();
			draw_menu(-1);
			draw_map();
		    } else {
		        load_correct_help();
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
		    sleep (1);
	}
    }
}

int pickme () {

    short   buffer[100];
    long    numpicked;
    int     i, x, y, picked,  old_picked = -1;

    while (getbutton(LEFTMOUSE)) {

	attach_to_cons();
	pushmatrix ();
	pick (buffer, 100);
	initnames ();
	ortho2(0.0, 300.0, 0.0, 730.0);

	for (i = 1; i < 7; i++) {
	    loadname ((short) i);/*  menu items */
	    rectfi (0, 730 - i * 30, 300, 760 - i * 30);
	}

	for (i = 0; i < 3; i++) {
	    loadname ((short) (i + 7));	/* color bits */
	    rectfi(150 + (i * 30), 460, 170 + (i * 30), 480);
	}

	for (i = 0; i < 3; i++) {
	    loadname ((short) (i + 10));/* mask bits */
	    rectfi (150 + (i * 30), 410, 170 + (i * 30), 430);
	}

	for(y=0; y<2; y++) 
	    for(x=0; x<4; x++) {
		loadname((short) (x + y * 4 + 13) );
		rectfi(20 + y * 180, 20 + x * 70, 50 + y * 180, 50 + x * 70);
	    }

	/* drawing area */

	numpicked = endpick (buffer);
	popmatrix ();
	if (numpicked)
	    picked = buffer[1];
	else
	    picked = 0;

	if (picked != old_picked) {
	    draw_menu(picked);
	    old_picked = picked;
	}

	if (picked == 0) {	/*  nothing from console picked	*/
	    attach_to_paint();
	    pushmatrix ();
	    pick (buffer, 100);
	    initnames ();
	    ortho2 (0.0, 550.0, 0.0, 550.0);
	    loadname ((short) 21);
	    rectfi (1, 1, 549, 549);
	    numpicked = endpick (buffer);
	    popmatrix ();
	    if (numpicked)
		picked = buffer[1];
	    else
		picked = 0;
	    if (picked == 21) draw_stuff();
	}

    }

    return(picked);
}

draw_menu (highlighted)
int highlighted;
{
    int i;

    if (highlighted == -1)
        for (i=1; i<13; i++) draw_menu(i);

    for (i=1; i<7; i++) {
	if (highlighted==i) {
	    color(NORMCOLOR);
	    rectfi (0, 730 - i * 30, 300, 760 - i * 30);
	    color(BLACK);
	} else {
	    color(BLACK);
	    rectfi (1, 731 - i * 30, 299, 759 - i * 30);
	    color(HIGHCOLOR);
	}
	cmov2i (60, 742 - i * 30);
	switch(i) {
	    case 1:
		charstr(" clear drawing area");
		break;
	    case 2:
		charstr("clear using writemask");
		break;
	    case 3:
		charstr(" additive color map");
		break;
	    case 4:
		charstr("  overlay color map");
		break;
	    case 5:
	        charstr("   draw color bars ");
	        break;
	    case 6:
		if (!near_exit)
		    charstr("    exit program   ");
		else
		    charstr(" Press again to exit");
		break;
	}
    }

    /* color bits */

    for (i=0; i<3; i++) {
	if (highlighted==i+7) {
	    color(NORMCOLOR);
    	    rectfi(150 + (i * 30), 460, 170 + (i * 30), 480);
	    color(BLACK);
	} else {
	    color(BLACK);
	    rectfi(151 + (i * 30), 461, 169 + (i * 30), 479);
	    color(HIGHCOLOR);
	}
	cmov2i(157 + (i * 30), 465);
	if (color_bit[2-i])
	    charstr("1");
	else
	    charstr("0");
    }

    /* mask bits */

    for (i=0; i<3; i++) {
	if (highlighted==10+i) {
	    color(NORMCOLOR);
    	    rectfi(150 + (i * 30), 410, 170 + (i * 30), 430);
	    color(BLACK);
	} else {
	    color(BLACK);
	    rectfi(151 + (i * 30), 411, 169 + (i * 30), 429);
	    color(HIGHCOLOR);
	}
	cmov2i(157 + (i * 30), 415);
	if (mask_bit[2-i])
	    charstr("1");
	else
	    charstr("0");
    }

}

execute_menu (chosen)
int chosen;
{

    if(chosen != 6 && near_exit) {
	near_exit=FALSE;
	load_correct_help();
    }

    switch (chosen) {
    case 1:
	attach_to_paint();
	color(BLACK);
	rectfi(1, 1, 549, 549);
	delobj(shapes);		/*  new painted area		*/
	makeobj (shapes);
	closeobj ();
        break;
    case 2:
	attach_to_paint();
	writemask(mask << 4);
	color(BLACK);
	rectfi(1, 1, 549, 549);
	writemask(0xFF);
	editobj (shapes);
	    writemask(mask << 4);
	    color(BLACK);
	    rectfi(1, 1, 549, 549);
	    writemask(0xFF);
	closeobj ();
	break;
    case 3:
	make_add_map();
	draw_map();
	break;
    case 4:
	make_over_map();
	draw_map();
	break;
    case 5:
        draw_color_bars();
	editobj (shapes);
	    draw_color_bars();
	closeobj ();
        break;
    case 6:
	if (!near_exit) {
	    near_exit=TRUE;
	    load_correct_help();
	} else {
	    tutorrestoremap();
	    gexit(); exit(0);
	}
        break;
    case 7:
    case 8:
    case 9:
	attach_to_cons();
        color_bit[9-chosen] = !color_bit[9-chosen];
	color(BLACK);
	rectfi(120, 460, 140, 480);
	color(NORMCOLOR);
	cmov2i(30, 465);
	colour = color_bit[2] * 4 + color_bit[1] * 2 + color_bit[0];
#ifdef FORTRAN
	sprintf(buffer, "CALL COLOR %1d", colour);
#else
	sprintf(buffer, "     color %1d", colour);
#endif
	charstr(buffer);
        break;
    case 10:
    case 11:
    case 12:
	attach_to_cons();
        mask_bit[12-chosen] = !mask_bit[12-chosen];
	color(BLACK);
	rectfi(120, 410, 140, 430);
	color(NORMCOLOR);
	mask = mask_bit[2] * 4 + mask_bit[1] * 2 + mask_bit[0];
	cmov2i(20, 415);
#ifdef FORTRAN
	sprintf(buffer, "CALL WRITEM %1d", mask);
#else
	sprintf(buffer, "  writemask %1d", mask);
#endif
	charstr(buffer);
        break;
    case 13:
	color_bit[0]=0; color_bit[1]=0; color_bit[2]=0;
	break;
    case 14:
	color_bit[0]=1; color_bit[1]=0; color_bit[2]=0;
	break;
    case 15:
	color_bit[0]=0; color_bit[1]=1; color_bit[2]=0;
	break;
    case 16:
	color_bit[0]=1; color_bit[1]=1; color_bit[2]=0;
	break;
    case 17:
	color_bit[0]=0; color_bit[1]=0; color_bit[2]=1;
	break;
    case 18:
	color_bit[0]=1; color_bit[1]=0; color_bit[2]=1;
	break;
    case 19:
	color_bit[0]=0; color_bit[1]=1; color_bit[2]=1;
	break;
    case 20:
	color_bit[0]=1; color_bit[1]=1; color_bit[2]=1;
	break;
    default:
	break;
    }
    draw_result();
    draw_menu(0);
    attach_to_cons();
    color(BLACK);
    rectfi(120, 460, 140, 480);
    draw_map();
}

draw_map() {

    int x, y;
    Colorindex temp;

    attach_to_cons();

    color(NORMCOLOR);
    cmov2i(30, 465);
    colour = color_bit[2] * 4 + color_bit[1] * 2 + color_bit[0];
#ifdef FORTRAN
    sprintf(buffer, "CALL COLOR %1d", colour);
#else
    sprintf(buffer, "     color %1d", colour);
#endif
    charstr(buffer);

    mask = mask_bit[2] * 4 + mask_bit[1] * 2 + mask_bit[0];
    cmov2i(20, 415);
#ifdef FORTRAN
    sprintf(buffer, "CALL WRITEM %1d", mask);
#else
    sprintf(buffer, "  writemask %1d", mask);
#endif
    charstr(buffer);

    cmov2i(40, 365);
    charstr("    writes:");

    draw_result();

    for(y=0; y<2; y++)		/*  palette		*/
	for(x=0; x<4; x++) {
	    color(NORMCOLOR);
	    recti(20 + y * 180, 20 + x * 70, 50 + y * 180, 50 + x * 70);
	    temp = x + y * 4;
	    color(temp << 4);
	    rectfi(21 + y * 180, 21 + x * 70, 49 + y * 180, 49 + x * 70);
	    color(NORMCOLOR);
	    cmov2i(60 + y * 180, 30 + x * 70);
	    sprintf(buffer, "%1d ", temp);
	    charstr(buffer);
	    sprintb(buffer, temp, 3);
	    charstr(buffer);
	}

    color(NORMCOLOR);
    cmov2i(115, 300);
    charstr("Palette");
}

draw_stuff() {

    long mx, my;
    long omx, omy;
    long screenx, screeny;

    attach_to_paint();
    getorigin (&screenx, &screeny);

    color(colour << 4);
    writemask(mask << 4);
    editobj (shapes);
	color(colour << 4);
	writemask(mask << 4);
    closeobj ();
    omx = getvaluator(MOUSEX);
    omy = getvaluator(MOUSEY);

    while (getbutton(LEFTMOUSE)) {
	mx = getvaluator(MOUSEX);
	my = getvaluator(MOUSEY);
	if (omx != mx || omy != my) {	/*  mouse moves		*/
	    circfi(mx - screenx, my - screeny, 10);
	    editobj (shapes);
		circfi(mx - screenx, my - screeny, 10);
	    closeobj ();
	    omx = mx;
	    omy = my;
	}
    }

    writemask(0xFF);
    editobj (shapes);
	writemask(0xFF);
    closeobj ();
}

draw_result() {

    int i;

    attach_to_cons();
    for (i=0; i<3; i++) {
	color(NORMCOLOR);
        recti(150 + (i * 30), 360, 170 + (i * 30), 380);
	color(BLACK);
	rectfi(151 + (i * 30), 361, 169 + (i * 30), 379);
	cmov2i(157 + (i * 30), 365);
	color(HIGHCOLOR);
	if (mask_bit[2-i]) {
	    if(color_bit[2-i])
		charstr("1");
	    else
		charstr("0");
	} else
	    charstr("-");
	    
    }
}

sprintb(string, n, len)
char *string;
int n, len;
{
    int i;

    for (i=len-1; i>=0; i--) {
        if (n >= power(2, i)) {
	    string[len-i-1] = '1';
	    n -= power(2, i);
	} else {
	    string[len-i-1]='0';
	}
    }
    string[len]=0;
}

int power(y, x)
int y, x;
{
    int i, r=1;

    for(i=0; i<x; i++) r = r * y;

    return(r);
}

make_add_map() {
    mapcolor(0, 000, 000, 000);
    mapcolor(1 << 4, 255, 000, 000);
    mapcolor(2 << 4, 000, 255, 000);
    mapcolor(3 << 4, 255, 255, 000);
    mapcolor(4 << 4, 000, 000, 255);
    mapcolor(5 << 4, 255, 000, 255);
    mapcolor(6 << 4, 000, 255, 255);
    mapcolor(7 << 4, 255, 255, 255);
}

make_over_map() {
    mapcolor(0 << 4, 000, 000, 000);
    mapcolor(1 << 4, 255, 000, 000);
    mapcolor(2 << 4, 000, 255, 000);
    mapcolor(3 << 4, 255, 255, 000);
    mapcolor(4 << 4, 000, 000, 255);
    mapcolor(5 << 4, 000, 000, 255);
    mapcolor(6 << 4, 000, 000, 255);
    mapcolor(7 << 4, 000, 000, 255);
}

draw_color_bars() {

    int i;

    attach_to_paint();
    for(i=1;i<8;i++) {
        color( (Colorindex) (i << 4));
	rectfi(40 + i * 50, 100, 80 + i * 50, 500);
    }
}

setupqueue () {
    qdevice (RIGHTMOUSE);
    qdevice (ESCKEY);
    qdevice (LEFTMOUSE);
    qdevice (INPUTCHANGE);
    qdevice (REDRAW);
}

setupcolors() {
    int i;

    if (getplanes () < 7) {	/*  if less than 7 bitplanes in mex	 */
	printf("You do not have enough bitplanes for this program\n");
	gexit();
	exit(0);
    }
    tutorsavemap();
    tutormakemap();
}

init_windows()
/*---------------------------------------------------------------------------
 * Initialize the three windows for the program.
 *---------------------------------------------------------------------------
 */
{
    backwin = init_back();
    singlebuffer();
    gconfig();
    winattach();
    helpwin = init_help("Writemask -- INFORMATION");
    paintwin = init_paint();
    conswin = init_cons();

}

init_paint()
/*---------------------------------------------------------------------------
 * Initialize the paint window.
 *---------------------------------------------------------------------------
 */
{
    int res;
    prefposition(20, 571, 5, 555);
    res = winopen("paint");
    wintitle("Writemask -- PAINT AREA");
    prefsize(550, 550);
    winconstraints();
    setup_paint_environ();

    return(res);
}

init_cons()
/*---------------------------------------------------------------------------
 * Initialize the console window.
 *---------------------------------------------------------------------------
 */
{
    int res;

    prefposition(650, 950, 5, 735);
    res = winopen("cons");
    wintitle("Writemask -- CONSOLE");
    prefsize(300, 730);
    winconstraints();
    setup_cons_environ();

    return(res);
}

setup_cons_environ()
/*---------------------------------------------------------------------------
 * Set the console environment.
 *---------------------------------------------------------------------------
 */
{
    reshapeviewport();
    ortho2(0.0, 300.0, 0.0, 730.0);
}

setup_paint_environ()
/*---------------------------------------------------------------------------
 * Set up the environment used for the paint window.
 *---------------------------------------------------------------------------
 */
{
    reshapeviewport();
    ortho2(0.0, 550.0, 0.0, 550.0);
}

 attach_to_paint()
/*---------------------------------------------------------------------------
 * Attach to the paint window.
 *---------------------------------------------------------------------------
 */
{
    winset(paintwin);
}

attach_to_cons()
/*---------------------------------------------------------------------------
 * Direct graphics output to the console window.
 *---------------------------------------------------------------------------
 */
{
    winset(conswin);
}

/*---------------------------------------------------------------------------
 * Routine called when a redraw token is sent down the queue.
 *---------------------------------------------------------------------------
 */
redraw_help() {

    reshapeviewport();
    draw_help();
}

redraw_paint()
/*---------------------------------------------------------------------------
 * Routine called when a redraw token is sent down the queue.
 *---------------------------------------------------------------------------
 */
{
    attach_to_paint();
    reshapeviewport();
    color (BLACK);
    clear ();
    callobj (shapes);
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
    color (BLACK);
    clear ();
    draw_menu(-1);
    draw_map();
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
    else if (n == paintwin)
	redraw_paint();
    else if (n == backwin)
	redraw_back();
}

load_correct_help()
/*---------------------------------------------------------------------------
 * Load the help box with the correct message according to gammac.
 *---------------------------------------------------------------------------
 */
{
    if (!active)
	load_help ("Press the RIGHT MOUSE button and attach",
		"to the program from the popup menu.", "", "", "", "");
    else if (near_exit)
	load_help ("Select again to confirm the request", 
		"to exit the program.  Selecting anywhere",
		"else will cancel the request.",
		"", "", "");
    else 
	load_help ("Use the LEFT MOUSE button to:",
		"   1. Choose an action from the menu.",
		"   2. Alter the binary color or writemask.",
		"   3. Choose the color from the palette.", 
		"   4. Paint circles into the paint area.", "");
    draw_help();
}
