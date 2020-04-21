#include "patch.h"

init_state()
/*---------------------------------------------------------------------------
 * Open the window for the status window.
 *---------------------------------------------------------------------------
 */
{
    int res;
    
    prefposition(569, 569+370, 377+72, 477+72);
    res = winopen("state");
    reshapeviewport();
    wintitle("Patch -- STATUS");
    prefsize(370, 100);
    winconstraints();
    ortho2(-10.0, 360.0, -15.0, 85.0);


    return(res);

}

redraw_state()
/*---------------------------------------------------------------------------
 * Routine called if a REDRAW token comes down the event queue.
 *---------------------------------------------------------------------------
 */
{
    winset(statw);

    color(BLACK);
    clear();
    print_basis();
    print_number();
    print_precision();
    color(WHITE);
    linewidth(2);
    line2(-10.0, 25.0, 360.0, 25.0);

    swapbuffers();
    
    color(BLACK);
    clear();
    print_basis();
    print_number();
    print_precision();
    color(WHITE);
    linewidth(2);
    line2(-10.0, 25.0, 360.0, 25.0);
}

draw_state()
/*---------------------------------------------------------------------------
 * Draw the current state of the curve stuff.  Called from the main program.
 *---------------------------------------------------------------------------
 */
{
    winset(statw);

    color(BLACK);
    clear();

    print_basis();
    print_number();
    print_precision();
    color(WHITE);
    linewidth(2);
    line2(-10.0, 25.0, 360.0, 25.0);
}

print_basis()
/*---------------------------------------------------------------------------
 * Display the current basis in the state window.
 *---------------------------------------------------------------------------
 */
{
    char buf[80];

#ifdef FORTRAN
    sprintf(buf, "call patchb (%s, %s)", 
		  basis_names[curbasisu-1], basis_names[curbasisv-1]);
#else
    sprintf(buf, "patchbasis (%s, %s);", 
		  basis_names[curbasisu-1], basis_names[curbasisv-1]);
#endif
    color(UNPICKCOLOR);
    cmov2(0.0, 0.0);
    charstr(buf);
}

print_number()
/*---------------------------------------------------------------------------
 * Print the number of curves per patch in the state window.
 *---------------------------------------------------------------------------
 */
{
    char buf[80];

#ifdef FORTRAN
    sprintf(buf, "call patchc (%d, %d)", curcursu, curcursv);
#else
    sprintf(buf, "patchcurves (%d, %d);", curcursu, curcursv);
#endif
    if (curcont == NUMBER)
	color(HIGHCOLOR);
    else
	color(NORMCOLOR);

    cmov2(0.0, 37.0);
    charstr(buf);
}

print_precision()
/*---------------------------------------------------------------------------
 * Print the current curve precision in the state window.
 *---------------------------------------------------------------------------
 */
{
    char buf[80];

#ifdef FORTRAN
    sprintf(buf, "call patchp (%d, %d)", curprecu, curprecv);
#else
    sprintf(buf, "patchprecision (%d, %d);", curprecu, curprecv);
#endif
    if (curcont == PRECISION)
	color(HIGHCOLOR);
    else
	color(NORMCOLOR);

    cmov2(0.0, 60.0);
    charstr(buf);
}

pick_number()
/*---------------------------------------------------------------------------
 * Attach to and check the curve for a picking hit.
 *---------------------------------------------------------------------------
 */
{
    int i;

    winset(statw);

    pushmatrix();
        pick(pick_buffer, MAXPICK);
	    initnames();
	    loadmatrix(state);
	    pushname(CONTROL);
	    pushname(NUMBER);
	    rectfi(-10, 37, 360, 53);
	retnumber = endpick(pick_buffer);
    popmatrix();
}

pick_precision()
/*---------------------------------------------------------------------------
 * Attach to and check the curve for a picking hit.
 *---------------------------------------------------------------------------
 */
{
    int i;

    winset(statw);

    pushmatrix();
        pick(pick_buffer, MAXPICK);
	    initnames();
	    loadmatrix(state);
	    pushname(CONTROL);
	    pushname(PRECISION);
	    rectfi(-10, 60, 360, 76);
	retnumber = endpick(pick_buffer);
    popmatrix();
}
