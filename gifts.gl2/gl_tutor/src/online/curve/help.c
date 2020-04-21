#include "curve.h"

load_attach_help()
/*---------------------------------------------------------------------------
 * Load the help box with attach stuff.
 *---------------------------------------------------------------------------
 */
{
    load_help("In order to use this program:", 
	      "  You must attach to it by", 
	      "  pressing the RIGHT MOUSE button and", 
	      "  selecting ATTACH from the menu.", "", "", "");
}

load_init_help()
/*---------------------------------------------------------------------------
 * Draw the initial help message.
 *---------------------------------------------------------------------------
 */
{
 load_help("Use the LEFT MOUSE button to:", 
	   "  1 - Select the CURVE PRECISION function", 
	   "      in the STATUS window.", 
	   "  2 - Select a CONTROL POINT to move around.",
	   "  3 - Select and drag a CONTROL BAR knob.",
	   "OR use the RIGHT MOUSE to change curve basis.");
}

load_prec_help()
/*---------------------------------------------------------------------------
 * Load the help box with a precision message.
 *---------------------------------------------------------------------------
 */
{
 load_help("Use the LEFT MOUSE button on the CONTROL BAR to:", 
	   "  Adjust the curve precision.", "", "", "", "");
}

load_cont_help()
/*---------------------------------------------------------------------------
 * Load the help box with control point instructions.
 *---------------------------------------------------------------------------
 */
{
    char buff[40];
    sprintf( buff, "  control point number %d.", curcont);
    
 load_help("Use the CONTROL BARS to:", 
	   "  Adjust the X, Y, and Z coordinates", 
	   "  (top to bottom respectively) of", buff, "", "");
}

load_correct_help()
/*---------------------------------------------------------------------------
 * Load the correct help message based on curcont.
 *---------------------------------------------------------------------------
 */
{
    switch(curcont){
    case NONE:
	load_init_help();
	break;
    case PRECISION:
	load_prec_help();
	break;
    }

    if ((curcont > NONE) && (curcont < NUMPNTS))
	load_cont_help();
}


