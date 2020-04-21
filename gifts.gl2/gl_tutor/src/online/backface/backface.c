/*---------------------------------------------------------------------------
 *  backface.c
 *  May, 1986 -- Michael Clark
 *  Learning Environment -- to demonstrate and contrast the
 *  backface removal versus not using backface removal
 *  fixes: redraw_back(), attach help menu, color ramp. VGU 7/7/86
 *---------------------------------------------------------------------------
 */

#include "back.h"

main()
{
    long dev;
    short val;
    init_all();

    while (!exiting) {

	while (qtest()) {		/* is there something on the queue */
	      dev = qread(&val);
 	      switch (dev){

	      case INPUTCHANGE:
		  attached = val;
		  if (attached == FALSE) {
		      load_correct_help();
		      draw_help();
		  }
		  break;

	      case RIGHTMOUSE:
		  respond_to_rightmouse(val);
		  break;

	      case REDRAW:
		  reshapeviewport();
    	    	  draw_frame(val);
	    	  break;

	      }
        }

    update_data();
    draw_newframe();
    swapbuffers();
    }

    clean_up_and_exit();
}

/*---------------------------------------------------------------------------
 * Initialize the graphics commands, the event queue, all the variables used
 * and anything else that needs to be initialized.
 *---------------------------------------------------------------------------
 */
init_all()
{
    init_graphics();		/* Initialize the graphics system. */
    init_queue();		/* Initialize the event queue. */
    init_vars();		/* Initialize all the variables used. */
}

/*---------------------------------------------------------------------------
 * Initialize the graphics system.
 *---------------------------------------------------------------------------
 */
init_graphics()
{
    if (!ismex()){
 	printf("The backface program can only be run under the window manager.\n");
 	printf("Type `mex' to start the window manager.\n");
        exit ();
    }


    init_windows();
    tutorsavemap();
    tutormakemap();

    winset(backw);
    winattach ();

    if (getplanes() == 2) {
	printf("You do not have enough bitplanes to run this program\n");
	exit(0);
    }

    init_pups();	    /* Popup menus. */

    deflinestyle(DASHED, dashed);
    defpattern(HALFTONE, 16, halftone);
}   

/*---------------------------------------------------------------------------
 * Initlialize the pop-up menus.
 *---------------------------------------------------------------------------
 */
init_pups()
{
    exitmen = defpup("Exit Confirmation %t|Yes|No");
    mainmen = newpup();
}

/*---------------------------------------------------------------------------
 * Initialize the event queue.
 *---------------------------------------------------------------------------
 */
init_queue()
{
    qdevice(RIGHTMOUSE);		    /* Pop up menu button. */
    qdevice(REDRAW);			    /* Redraw token.       */
    qdevice(INPUTCHANGE);		    /* Input change token. */
}

/*---------------------------------------------------------------------------
 * Initialize all of the variables.
 *---------------------------------------------------------------------------
 */
init_vars()
{
    exiting = attached = spinning = FALSE;
    arrows = TRUE;

    pushmatrix();
	loadmatrix(ident);
	getmatrix(current);
    popmatrix();
}

/*---------------------------------------------------------------------------
 * Do all the overhead required to exit the program.
 *---------------------------------------------------------------------------
 */
clean_up_and_exit()
{
    doublebuffer(FALSE);
    gconfig();
    tutorrestoremap();
    gexit();
    exit(0);
}

/*---------------------------------------------------------------------------
 * Update all of the current data.
 *---------------------------------------------------------------------------
 */
update_data()
{
    if (spinning) {
        pushmatrix();
	    loadmatrix(current);
	    rotate(30, 'x');
	    rotate(40, 'y');
	    rotate(20, 'z');
	    getmatrix(current);
	popmatrix();
	curexam += 50;
	curexam = (curexam % 3600);
    }
}

/*---------------------------------------------------------------------------
 * Respond to a rightmouse press.
 *---------------------------------------------------------------------------
 */
respond_to_rightmouse(state)
short state;
{
    if (state) {
	make_main_menu();
 	switch (dopup(mainmen)) {

 	case 1:
 	    toggle_spin();
 	    break;

 	case 2:
 	    toggle_arrows();
 	    break;

 	case 3:
 	    toggle_shading();
 	    break;

 	case 4:
 	    do_exit();
 	    break;

 	default:
 	    break;
 	}
    }
}

/*---------------------------------------------------------------------------
 * Draw a frame using all of the current data.
 *---------------------------------------------------------------------------
 */
draw_newframe()
{

    draw_back();
    load_correct_help(); 
    draw_help();		    /* Draw the help window.		*/
    draw_cubes();		    /* Draw the bottom windows.		*/
    draw_example();		    /* Draw the normal example.		*/

}

/*---------------------------------------------------------------------------
 * Redraw the window wich in the event of a redraw token.
 *---------------------------------------------------------------------------
 */
draw_frame(val)
short val;
{

	if (val == backw)
	    draw_back();
	else if (val == helpw) {
            load_correct_help();
	    draw_help();
	}
	else if (val == examw)
	    redraw_exam();
	else if (val == withw)
	    draw_with();
	else if (val == withow)
	    draw_without();
}

/*---------------------------------------------------------------------------
 * Toggle the spin flag.
 *---------------------------------------------------------------------------
 */
toggle_spin()
{
    spinning = !spinning;
}

/*---------------------------------------------------------------------------
 * Toggle the arrows flag.
 *---------------------------------------------------------------------------
 */
toggle_arrows()
{
    arrows = !arrows;
}

/*---------------------------------------------------------------------------
 * Toggle the shading flag.
 *---------------------------------------------------------------------------
 */
toggle_shading()
{
    if (shading)
	tutormakemap();
    shading = !shading;
}

/*---------------------------------------------------------------------------
 * Called from the pop-up menu.
 *---------------------------------------------------------------------------
 */
do_exit()
{
    /* do the confirm exit menu and check for the first item */
    if (dopup(exitmen) == 1)
	clean_up_and_exit();
}

/*---------------------------------------------------------------------------
 * Create the main menu according to the current state of the machine.
 *---------------------------------------------------------------------------
 */
make_main_menu()
{
    freepup(mainmen);

    mainmen = defpup("Backface %t|");

    if (spinning)    
 	addtopup(mainmen, "Stop rotating");
    else
 	addtopup(mainmen, "Rotate the cube");

    if (arrows)    
 	addtopup(mainmen, "Turn arrows OFF");
    else
 	addtopup(mainmen, "Turn arrows ON");

    if (shading)    
	addtopup(mainmen, "Turn shading OFF");
    else
	addtopup(mainmen, "Turn shading ON");

    addtopup(mainmen, "Exit");
}

/*---------------------------------------------------------------------------
 * Initialize all of the windows used in the program.
 *---------------------------------------------------------------------------
 */
init_windows()
{

    /* Grab the background window. */
    backw = init_back();
    doublebuffer();
    gconfig();


    /* Grab the help window. */
    helpw = init_help("Backface");

    /* Grab the example window. */
    examw = init_exam();

    /* Grab the with cube. */
    withw = init_with();

    /* Grab the without cube. */
    withow = init_without();
}

/*---------------------------------------------------------------------------
 * Load the help box with the correct message according to queue.
 *---------------------------------------------------------------------------
 */
load_correct_help()
{
    if (!attached) {
	load_help("In order to use this program:", 
		  "  You must attach to it by", 
		  "  pressing the RIGHT MOUSE button and", 
		  "  selecting ATTACH from the menu.", "", "");
	return;
    } else {
	load_help("Use the RIGHT MOUSE button to:",
		  "   1. Turn the cube spin on and off.",
		  "   2. Turn the NORMALS on and off.",
		  "   3. Turn the cube SHADING on and off.", "", "");
    }
}
