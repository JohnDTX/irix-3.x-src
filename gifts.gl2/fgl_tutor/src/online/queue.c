/*  queue.c							*/
/*  April, 1986 -- Thant Tessman				*/
/*  Learning Environment -- to demonstrate the queue		*/

#include <gl.h>
#include <device.h>
#include "tutor.h"

char buffer[80];
long devstack[50];
short valstack[50];
Boolean isitqueued[8];
Boolean tied;
int qcount;
long onqueue;
int dispdev, dispval;
int active;
int backwin,
    helpwin,
    queuewin, 
    valuewin,
    conswin;

Boolean near_exit=FALSE;

main () {
    short   val;
    long    dev;
    int	    pickme();
    int i;
    int	waspicked;

    if(!ismex()){
	printf("You must be running the window manager to use this program.\n");
	printf("Type mex to start.\n");
	exit(0);
    }

    init_windows();

    setupcolors();
    setupqueue ();

    load_correct_help(0);
    draw_menu(-1);
    draw_queue();
    draw_values();
    draw_back();

    while (TRUE) {
	while (qtest ()) {
	    dev = qread (&val);
	    switch (dev) {
		case LEFTMOUSE: 
		    if (val) {
			execute_menu(pickme());
		    }
		    break;
		case MIDDLEMOUSE:
		    if (isitqueued[2])
			put_on_que(dev, val);
		    break;
		case RIGHTMOUSE:
		    if (isitqueued[1])
			put_on_que(dev, val);
		    break;
		case MOUSEX:
		case MOUSEY:
		    if (isitqueued[1] && tied)
			put_on_que(dev, val);
		    break;
		case REDRAW: 
		    redraw_window (val);
		    break;
		case INPUTCHANGE: 
		    active = val;
		    if (active == FALSE) {
			load_correct_help(0);
			draw_menu (-1);
			draw_queue ();
			draw_values();
			draw_back();
		    } else {
			load_correct_help(0);
		    }
		    break;
		case ESCKEY:
		    gexit(); exit(0);
		    break;
		case LEFTSHIFTKEY:
		    if (isitqueued[3])
			put_on_que(dev, val);
		    break;
		case RIGHTSHIFTKEY:
		    if (isitqueued[4])
			put_on_que(dev, val);
		    break;
		case AKEY:
		    if (isitqueued[5])
			put_on_que(dev, val);
		    break;
		case BKEY:
		    if (isitqueued[6])
			put_on_que(dev, val);
		    break;
		case CKEY:
		    if (isitqueued[7])
			put_on_que(dev, val);
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

int pickme () 
{

    short   buffer[100];
    long    numpicked;
    int     i, picked,  old_picked = -1;

    while (getbutton(LEFTMOUSE)) {

	attach_to_cons();
	pushmatrix ();
	pick (buffer, 100);
	initnames ();
	ortho2 (0.0, 300.0, 0.0, 510.0);

	for (i = 1; i < 8; i++) {
	    loadname ((short) i);
	    rectfi (0, 540 - (30 * i), 300, 510 - (30 * i) );
	}

	for (i = 8; i < 10; i++) {
	    loadname ((short) i);
	    rectfi (0, 510 - (i * 30), 300, 480 - (i * 30) );
	}

	for (i = 10; i < 16; i++) {
	    loadname ((short) i);
	    rectfi (0, 480 - (30 * i), 300, 450 - (30 * i));
	}

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

    }

    return(picked);
}

draw_menu (highlighted)
int highlighted;
{
    int i;

    if (highlighted == -1)
        for (i=1; i<16; i++) draw_menu(i);

    attach_to_cons ();
    /* qdevice stuff */

    for (i=1; i<8; i++) {
        if (highlighted==i) {
	    color(NORMCOLOR);
	    rectfi (0, 510 - (30 * i), 300, 540 - (30 * i));
	    color(BLACK);
	} else {
            color(BLACK);
	    rectfi (1, 511 - (30 * i), 299, 539 - (30 * i));
	    if ( isitqueued[i] )
		color(HIGHCOLOR);
	    else
		color(NORMCOLOR);
	}
	cmov2i (4, 519 - (30 * i) );
	switch(i) {

#ifdef FORTRAN
	    case 1:
		charstr("CALL QDEVIC (RIGHTM)");
		break;
	    case 2:
		charstr("CALL QDEVIC (MIDDLE)");
		break;
	    case 3:
		charstr("CALL QDEVIC (LEFTSH)");
		break;
	    case 4:
	        charstr("CALL QDEVIC (RIGHTS)");
		break;
	    case 5:
	        charstr("CALL QDEVIC (AKEY)");
		break;
	    case 6:
	        charstr("CALL QDEVIC (BKEY)");
		break;
	    case 7:
	        charstr("CALL QDEVIC (CKEY)");
		break;
#else
	    case 1:
		charstr("qdevice (RIGHTMOUSE);");
		break;
	    case 2:
		charstr("qdevice (MIDDLEMOUSE);");
		break;
	    case 3:
		charstr("qdevice (LEFTSHIFTKEY);");
		break;
	    case 4:
	        charstr("qdevice (RIGHTSHIFTKEY);");
		break;
	    case 5:
	        charstr("qdevice (AKEY);");
		break;
	    case 6:
	        charstr("qdevice (BKEY);");
		break;
	    case 7:
	        charstr("qdevice (CKEY);");
		break;
#endif
	}
    }

    for (i=8; i<10; i++) {
        if (highlighted==i) {
	    color(NORMCOLOR);
	    rectfi (0, 480 - (30 * i), 300, 510 - (30 * i));
	    color(BLACK);
	} else {
            color(BLACK);
	    rectfi (1, 481 - (30 * i), 299, 509 - (30 * i));
	    if ( tied && i==8 || !tied && i==9)
		color(HIGHCOLOR);
	    else
		color(NORMCOLOR);
	}
	cmov2i (4, 489 - (30 * i) );
	switch(i) {
#ifdef FORTRAN
	    case 8:
		charstr("CALL TIE (RIGHTM, MOUSEX, MOUSEY)");
		break;
	    case 9:
		charstr("CALL TIE (RIGHTM, 0, 0)");
		break;
#else
	    case 8:
		charstr("tie (RIGHTMOUSE, MOUSEX, MOUSEY);");
		break;
	    case 9:
		charstr("tie (RIGHTMOUSE, 0, 0);");
		break;
#endif
	}
    }

    for (i=10; i<16; i++) {
        if (highlighted==i) {
	    color(NORMCOLOR);
	    rectfi (0, 450 - (30 * i), 300, 480 - (30 * i));
	    color(BLACK);
	} else {
            color(BLACK);
	    rectfi (1, 451 - (30 * i), 299, 479 - (30 * i));
	    color(HIGHCOLOR);
	}
	cmov2i (4, 459 - (30 * i) );
	switch(i) {

#ifdef FORTRAN
	    case 10:
		charstr("DEV = QREAD(VAL)");
		break;
	    case 11:
		charstr("ONQUEUE = QTEST()");
		break;
	    case 12:
		charstr("CALL QRESET()");
		break;
	    case 13:
		charstr("CALL QENTER(MIDDLE, 0)");
		break;
	    case 14:
		charstr("CALL QENTER(MIDDLE, 1)");
		break;
#else
	    case 10:
		charstr("dev = qread(&val);");
		break;
	    case 11:
		charstr("onqueue = qtest();");
		break;
	    case 12:
		charstr("qreset();");
		break;
	    case 13:
		charstr("qenter(MIDDLEMOUSE, 0);");
		break;
	    case 14:
		charstr("qenter(MIDDLEMOUSE, 1);");
		break;
#endif
	    case 15:
		if (near_exit)
		    charstr("Press again to confirm exit");
		else
		    charstr("exit program");
		break;
	}
    }
}

execute_menu (chosen)
int chosen;
{
    if(chosen != 15 && near_exit) {
	near_exit=FALSE;
	load_correct_help(0);
    }

    if (chosen>0 && chosen<8)
	isitqueued[chosen] = !isitqueued[chosen];
    else

    switch (chosen) {
	case 8:
	    tied = TRUE;
	    break;
	case 9:
	    tied = FALSE;
	    break;
	case 10:
	    do_qread();
	    break;
	case 11:
	    do_qtest();
	    break;
	case 12:
	    do_qreset();
	    break;
	case 13:
/*	    qenter(MIDDLEMOUSE, 0);  */
	    put_on_que(MIDDLEMOUSE, 0);
	    break;
	case 14:
/*	    qenter(MIDDLEMOUSE, 1);  */
	    put_on_que(MIDDLEMOUSE, 1);
	    break;
	case 15:
	    if (!near_exit) {
		near_exit=TRUE;
		load_correct_help(0);
	    } else {
		tutorrestoremap();
		gexit(); exit(0);
	    }
            break;
    }

    draw_menu(0);
}

draw_queue() {

    int i;

    attach_to_queue ();
    color(NORMCOLOR);
    cmov2i(65, 520);
    charstr("dev");
    cmov2i(216, 520);
    charstr("val");

    for (i=0; i<16; i++) {
	color(UNPICKCOLOR);
	recti(50, 510 - i * 30, 200, 480 - i * 30);
	recti(204, 510 - i * 30, 250, 480 - i * 30);
	color(BLACK);
	rectfi(51, 509 - i * 30, 199, 481 - i * 30);
	rectfi(205, 509 - i * 30, 249, 481 - i * 30);
    }

    color(HIGHCOLOR);
    for (i=0; i<qcount && i<16; i++) {

	cmov2i(55, 487 - i * 30);
	print_dev_name(devstack[i]);

	cmov2i(214, 487 - i * 30);
	sprintf(buffer, "%d", valstack[i]);
	charstr(buffer);
    }
}

draw_values() {

    attach_to_value();
    color(UNPICKCOLOR);
    recti(150, 20, 300, 50);
    recti(150, 100, 200, 130);
    recti(150, 150, 300, 180);

    color(BLACK);
    rectfi(151, 21, 299, 49);
    rectfi(151, 102, 199, 129);
    rectfi(151, 151, 299, 179);

    color(NORMCOLOR);
    cmov2i(80, 160);
    charstr("dev is ");
    cmov2i(80, 110);
    charstr("val is ");
    cmov2i(50, 30);
    charstr("onqueue is ");

    color(HIGHCOLOR);
    cmov2i(158, 160);
    print_dev_name(dispdev);
    cmov2i(158, 110);
    sprintf(buffer, "%d", dispval);
    charstr(buffer);
    cmov2i(158, 30);
    print_dev_name(onqueue);
}

do_qtest() {
    if (qcount)
	onqueue = devstack[0];
    else
	onqueue = 0;
    draw_values();
}

do_qreset() {
    qcount=0;
    draw_queue();
}

do_qread() {

    int i;

    if (qcount==0) {
	load_correct_help(1);
	return;
    }

    dispdev = devstack[0];
    dispval = valstack[0];
    for (i=0; i<qcount; i++) {
	devstack[i] = devstack[i+1];
	valstack[i] = valstack[i+1];
    }
    qcount--;

    draw_queue();
    draw_values();
}

print_dev_name(dev) {

    switch(dev) {
	case LEFTMOUSE:
	    charstr("LEFTMOUSE");
	    break;
	case MIDDLEMOUSE:
	    charstr("MIDDLEMOUSE");
	    break;
	case RIGHTMOUSE:
	    charstr("RIGHTMOUSE");
	    break;
	case LEFTSHIFTKEY:
	    charstr("LEFTSHIFTKEY");
	    break;
	case RIGHTSHIFTKEY:
	    charstr("RIGHTSHIFTKEY");
	    break;
	case AKEY:
	    charstr("AKEY");
	    break;
	case BKEY:
	    charstr("BKEY");
	    break;
	case CKEY:
	    charstr("CKEY");
	    break;
	case MOUSEX:
	    charstr("MOUSEX");
	    break;
	case MOUSEY:
	    charstr("MOUSEY");
	    break;
	case 0:
	    charstr("0");
	    break;
    }
}

put_on_que(dev, val)
long dev;
short val;
{
    if (qcount==50) return;
    devstack[qcount]=dev;
    valstack[qcount]=val;
    qcount++;

    draw_queue();
}

setupqueue () {
    qdevice (LEFTMOUSE);
    qdevice (INPUTCHANGE);
    qdevice (REDRAW);
    qdevice (ESCKEY);

    qdevice (MIDDLEMOUSE);
    qdevice (RIGHTMOUSE);
    tie(RIGHTMOUSE, MOUSEX, MOUSEY);
    qdevice (LEFTSHIFTKEY);
    qdevice (RIGHTSHIFTKEY);
    qdevice (AKEY);
    qdevice (BKEY);
    qdevice (CKEY);

}

setupcolors() {
    int i;

    if (getplanes () < 4) {	/*  if less than 4 bitplanes in mex	 */
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
    helpwin = init_help("Queue -- INFORMATION");
    winattach();
    queuewin = init_queue();
    conswin = init_cons();
    valuewin = init_value();

}

init_cons()
/*---------------------------------------------------------------------------
 * Initialize the console window.
 *---------------------------------------------------------------------------
 */
{
    int res;

    prefposition(675, 975, 120, 630);
    res = winopen("cons");
    wintitle("Queue -- CONSOLE");
    prefsize(300, 510);
    winconstraints();
    setup_cons_environ();

    return(res);
}

init_queue()
/*---------------------------------------------------------------------------
 * Initialize the queue window.
 *---------------------------------------------------------------------------
 */
{
    int res;
    prefposition(15, 315, 5, 555);
    res = winopen("queue");
    wintitle("Queue");
    prefsize(300, 550);
    winconstraints();
    setup_queue_environ();

    return(res);
}

init_value()
/*---------------------------------------------------------------------------
 * Initialize the value window.
 *---------------------------------------------------------------------------
 */
{
    int res;
    prefposition(340, 650, 150, 350);
    res = winopen("value");
    wintitle("Queue -- VARIABLES");
    prefsize(310, 200);
    winconstraints();
    setup_value_environ();

    return(res);
}

setup_cons_environ()
/*---------------------------------------------------------------------------
 * Set the console environment.
 *---------------------------------------------------------------------------
 */
{
    reshapeviewport();
    ortho2(0.0, 300.0, 0.0, 510.0);
}

setup_queue_environ()
/*---------------------------------------------------------------------------
 * Set up the environment used for the queue window.
 *---------------------------------------------------------------------------
 */
{
    reshapeviewport();
    ortho2(0.0, 300.0, 0.0, 550.0);
}

setup_value_environ()
/*---------------------------------------------------------------------------
 * Set up the environment used for the value window.
 *---------------------------------------------------------------------------
 */
{
    reshapeviewport();
    ortho2(0.0, 310.0, 0.0, 200.0);
}

attach_to_value()
/*---------------------------------------------------------------------------
 * Attach to the value window.
 *---------------------------------------------------------------------------
 */
{
    winset(valuewin);
}

attach_to_cons()
/*---------------------------------------------------------------------------
 * Direct graphics output to the console window.
 *---------------------------------------------------------------------------
 */
{
    winset(conswin);
}

attach_to_queue()
/*---------------------------------------------------------------------------
 * Attach to the queue window.
 *---------------------------------------------------------------------------
 */
{
    winset(queuewin);
}

redraw_help()
/*---------------------------------------------------------------------------
 * Routine called in the event of a REDRAW token comming down the queu.
 *---------------------------------------------------------------------------
 */
{
    draw_help();
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
}

redraw_value()
/*---------------------------------------------------------------------------
 * Routine called when a redraw token is sent down the value.
 *---------------------------------------------------------------------------
 */
{
    attach_to_value();
    reshapeviewport();
    color (BLACK);
    clear ();
    draw_values();
}


redraw_queue()
/*---------------------------------------------------------------------------
 * Routine called when a redraw token is sent down the queue.
 *---------------------------------------------------------------------------
 */
{
    attach_to_queue();
    reshapeviewport();
    color (BLACK);
    clear ();
    draw_queue ();
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
    else if (n == queuewin)
	redraw_queue();
    else if (n == valuewin)
	redraw_value();
    else if (n == backwin)
	redraw_back();
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
	draw_help();
	return ;
    }
    switch (message) {
	case 0:
    	    if(near_exit) {
		load_help ("Select again to confirm the request", 
			"to exit the program.  Selecting anywhere",
			"else will cancel the request.",
			"", "", "");
	    } else {
		load_help ("With the LEFT MOUSE, select an action",
			"from the console menu OR press queued devices",
			"(buttons) to put events on the queue.",
			"See IRIS USER'S GUIDE Appendix A for a list",
			"of queueable devices.", "");
	    }
	    break;
	case 1:
	    load_help ("Doing a dev = qread(&val) when the queue",
		"is empty makes the computer wait until",
		"something is entered onto the queue.",
		"A good idea is to do a qtest before a qread",
		"to ensure something is there.", "");
	    break;
	default:
	    break;
    }
    draw_help();
}

