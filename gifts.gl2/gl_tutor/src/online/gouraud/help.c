#include "help.h"

redraw_help()
/*---------------------------------------------------------------------------
 * Routine called by the main program to display a message in the help window.
 *---------------------------------------------------------------------------
 */
{
    attach_to_help();
    reshapeviewport();
    color(HELPBACK);
    clear();		    /* Clear the back. */
    color(HELPTEXT);
    write_help();	    /* Display the message. */
}

int init_help()
/*---------------------------------------------------------------------------
 * Initialize the help window.
 *---------------------------------------------------------------------------
 */
{
    int res;

    prefposition(50, 50 + 9*HELPLEN + 2*HELPGAP, 675, 675 + 16*NUMLINES);
    res = winopen("help");		/* Open it. */
    wintitle("Gouraud -- INFORMATION");/* Title it. */
    prefsize(9*HELPLEN + 2*HELPGAP, 16*NUMLINES); /* Can't reshape. */
    winconstraints();
    winset(res);			/* Make it current. */
    setup_help_environ();		/* Set up the environment. */

    return(res);
}

setup_help_environ()
/*---------------------------------------------------------------------------
 * Setup the viewing transform an the environment for the help window.
 *---------------------------------------------------------------------------
 */
{
    reshapeviewport();
    ortho2(-(float) (HELPGAP), (float) (HELPLEN)*9.0 + (float) (HELPGAP), 
	    0.0, (float) (NUMLINES)*16.0);
}


load_help(mess1, mess2, mess3)
/*---------------------------------------------------------------------------
 * Load the help box with the message in mess.
 *---------------------------------------------------------------------------
 */
char *mess1, *mess2, *mess3;
{
    strcpy(help_message[0], mess1);
    strcpy(help_message[1], mess2);
    strcpy(help_message[2], mess3);
    redraw_help();
}

write_help()
/*---------------------------------------------------------------------------
 * Display the message in help_message.
 *---------------------------------------------------------------------------
 */
{
    register int i;

    for (i = 0 ; i < NUMLINES ; i++){
	goto_line(i);
	charstr(help_message[i]);
    }
}

goto_line(n)
/*---------------------------------------------------------------------------
 * Position the character cursor to line n of the help box. 
 *---------------------------------------------------------------------------
 */
int n;
{
    cmov2i(0, (NUMLINES - 1 - n) * 16 + 3);
}

attach_to_help()
/*---------------------------------------------------------------------------
 * Attach to the help window to allow the output to the window.
 *---------------------------------------------------------------------------
 */
{
    winset(helpid);
}
