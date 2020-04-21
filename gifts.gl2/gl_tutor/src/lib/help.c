
#include "tutor.h"

static char helpmess[6][80] = {"", "", "", "", "", ""};
static int hid;

/*---------------------------------------------------------------------------
 * Open the help window and return the window ID.
 *---------------------------------------------------------------------------
 */
init_help(title)

char *title;
{
    int res;
    char ft[80];

    prefposition(2, 512, 609, 735);
    res = winopen("help");
    prefsize(510, 126);
    strcpy(ft, title);
    strcat(ft, " -- INFORMATION");
    wintitle(ft);
    winconstraints();
    setup_help_environ();

    hid = res;
    return(res);
}

/*---------------------------------------------------------------------------
 * Setup the environment for help.
 *---------------------------------------------------------------------------
 */
setup_help_environ()

{
    reshapeviewport();
    ortho2(-2.0, 30.0, 6.0, -1.0);
}


/*---------------------------------------------------------------------------
 * Draw the help box according to currne state of the demo.
 *---------------------------------------------------------------------------
 */
draw_help()

{
    attach_to_help();
    color(7);
    clear();
    color(HELPCOLOR);
    print_message();
}

/*---------------------------------------------------------------------------
 * Attach to the help section.
 *---------------------------------------------------------------------------
 */
attach_to_help()

{
    winset(hid);
}

/*---------------------------------------------------------------------------
 * Load the help message.
 *---------------------------------------------------------------------------
 */
load_help(mess0, mess1, mess2, mess3, mess4, mess5)

char *mess0, *mess1, *mess2, *mess3, *mess4, *mess5;
{
    strcpy(helpmess[0], mess0);
    strcpy(helpmess[1], mess1);
    strcpy(helpmess[2], mess2);
    strcpy(helpmess[3], mess3);
    strcpy(helpmess[4], mess4);
    strcpy(helpmess[5], mess5);
}

/*---------------------------------------------------------------------------
 * Print the help message.
 *---------------------------------------------------------------------------
 */
print_message()

{
    register int i;

    for(i = 0 ; i < 6 ; i++){
	cmov2i(0, i);
	charstr(helpmess[i]);
    }
}
