/*  backface.c							*/
/*  May, 1986 -- Michael Clark					*/
/*  Learning Environment -- to demonstrate and contrast the	*/
/*  backface removal versus not using backface removal		*/
/*  fixes: redraw_back(), attach help menu, color ramp. VGU 7/7/86 */

#include "back.h"

main()
{
    long dev;
    short val;
    init_all();

    while(!exiting){

	while(qtest()){
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

init_pups()
/*---------------------------------------------------------------------------
 * Initlialize the pop-up menus.
 *---------------------------------------------------------------------------
 */
{
    exitmen = defpup("Exit Confirmation %t|Yes|No");
    mainmen = newpup();
}

init_queue()
/*---------------------------------------------------------------------------
 * Initialize the event queue.
 *---------------------------------------------------------------------------
 */
{
    qdevice(RIGHTMOUSE);		    /* Pop up menu button. */
    qdevice(REDRAW);			    /* Redraw token.       */
    qdevice(INPUTCHANGE);		    /* Input change token. */
}

init_vars()
/*---------------------------------------------------------------------------
 * Initialize all of the variables.
 *---------------------------------------------------------------------------
 */
{
    exiting = attached = spinning = FALSE;
    arrows = TRUE;

    pushmatrix();
	loadmatrix(ident);
	getmatrix(current);
    popmatrix();
}

clean_up_and_exit()
/*---------------------------------------------------------------------------
 * Do all the overhead required to exit the program.
 *---------------------------------------------------------------------------
 */
{
    doublebuffer(FALSE);
    gconfig();
    tutorrestoremap();
    gexit();
    exit(0);
}


update_data()
/*---------------------------------------------------------------------------
 * Update all of the current data.
 *---------------------------------------------------------------------------
 */
{
    if (spinning){
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

respond_to_rightmouse(state)
/*---------------------------------------------------------------------------
 * Respond to a rightmouse press.
 *---------------------------------------------------------------------------
 */
short state;
{
    if (state){
	make_main_menu();
 	switch(dopup(mainmen)) {
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



draw_newframe()
/*---------------------------------------------------------------------------
 * Draw a frame using all of the current data.
 *---------------------------------------------------------------------------
 */
{

    draw_back();
    load_correct_help(); 
    draw_help();		    /* Draw the help window.		*/
    draw_cubes();		    /* Draw the bottom windows.		*/
    draw_example();		    /* Draw the normal example.		*/

}

draw_frame(val)
/*---------------------------------------------------------------------------
 * Redraw the window wich in the event of a redraw token.
 *---------------------------------------------------------------------------
 */
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

toggle_spin()
/*---------------------------------------------------------------------------
 * Toggle the spin flag.
 *---------------------------------------------------------------------------
 */
{
    spinning = !spinning;
}

toggle_arrows()
/*---------------------------------------------------------------------------
 * Toggle the arrows flag.
 *---------------------------------------------------------------------------
 */
{
    arrows = !arrows;
}

toggle_shading()
/*---------------------------------------------------------------------------
 * Toggle the shading flag.
 *---------------------------------------------------------------------------
 */
{
    if (shading)
	tutormakemap();
    shading = !shading;
}

do_exit()
/*---------------------------------------------------------------------------
 * Called from the pop-up menu.
 *---------------------------------------------------------------------------
 */
{
    if (dopup(exitmen) == 1)
	clean_up_and_exit();
}

make_main_menu()
/*---------------------------------------------------------------------------
 * Create the main menu according to the current state of the machine.
 *---------------------------------------------------------------------------
 */
{
    freepup(mainmen);

    mainmen = defpup("Backface Removal %t|");

    if (spinning)    
 	addtopup(mainmen, "Stop Rotating");
    else
 	addtopup(mainmen, "Rotate The Cube");

    if (arrows)    
 	addtopup(mainmen, "Turn Arrows OFF");
    else
 	addtopup(mainmen, "Turn Arrows ON");

    if (shading)    
	addtopup(mainmen, "Turn Shading OFF");
    else
	addtopup(mainmen, "Turn Shading ON");


    addtopup(mainmen, "Exit");

}

init_windows()
/*---------------------------------------------------------------------------
 * Initialize all of the windows used in the program.
 *---------------------------------------------------------------------------
 */
{

    /* Grab the background window. */
    backw = init_back();
    doublebuffer();
    gconfig();


    /* Grab the help window. */
    helpw = init_help("Backface -- INFORMATION");

    /* Grab the example window. */
    examw = init_exam();

    /* Grab the with cube. */
    withw = init_with();

    /* Grab the without cube. */
    withow = init_without();

}

load_correct_help()
/*---------------------------------------------------------------------------
 * Load the help box with the correct message according to queue.
 *---------------------------------------------------------------------------
 */
{
    if (!attached) {
    load_help("In order to use this program:", 
	      "  You must attach to it by", 
	      "  pressing the RIGHT MOUSE button and", 
	      "  selecting ATTACH from the menu.", "", "");
	return;
    }
    else {
    load_help ("Use the RIGHT MOUSE button to:",
    "   1. Turn the cube spin on and off.",
    "   2. Turn the NORMALS on and off.",
    "   3. Turn the cube SHADING on and off.", "", "");
    }
}
