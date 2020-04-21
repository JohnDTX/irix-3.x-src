#include <gl.h>
#include "tutor.h"

static short	    map[32][3];

/*---------------------------------------------------------------------------
 * Make the color map for the on-line tutorial, checks the bitplanes.
 *---------------------------------------------------------------------------
 */
tutormakemap()
{
    if (getplanes() == 2){
	fprintf("Not enough bitplanes to make the map!\n");
	exit(0);
    }

    mapcolor(HELPCOLOR, 50, 0, 255);
    mapcolor(HIGHCOLOR, 200, 255, 255);
    mapcolor(NORMCOLOR, 190, 160, 160);
    mapcolor(UNPICKCOLOR, 100, 200, 255);
    mapcolor(AXESCOLOR, 255, 255, 100);
    mapcolor(CBARCOLOR, 100, 100, 50);
    mapcolor(CBAR2COLOR, 50, 50, 250);
    mapcolor(OBJECTCOLOR, 50, 255, 50);
}

/*---------------------------------------------------------------------------
 * Save the colors used for the tutorial demos.
 *---------------------------------------------------------------------------
 */
tutorsavemap()
{
    register Colorindex i;
    short r, g, b;

    if (getplanes() == 2){
	fprintf("Not enough bitplanes to make the map!\n");
	exit(0);
    }

    for (i = 0 ; i <= 32 ; i++){
	getmcolor(i, &r, &g, &b);
	map[i][0] = r;
	map[i][1] = g;
	map[i][2] = b;
    }
}

/*---------------------------------------------------------------------------
 * Restore the color map to it's original values.
 *---------------------------------------------------------------------------
 */
tutorrestoremap()
{
    register Colorindex i;

    for (i = 0 ; i <= 32 ; i++)
	mapcolor(i, map[i][0], map[i][1], map[i][2]);
}

/*---------------------------------------------------------------------------
 * Make sure the machine is running mex, if not exit.
 *---------------------------------------------------------------------------
 */
check_for_mex() {

    if (!ismex()) {
        printf(
            "You must be running the window manager to use this program.\n");
        printf("Type mex to start.\n");

        exit(0);
    }
}
