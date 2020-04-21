#include "curve.h"

init_state()
/*---------------------------------------------------------------------------
 * Open the window for the status window.
 *---------------------------------------------------------------------------
 */
{
    int res;
    
    prefposition(569, 939, 377+20, 528+20);
    res = winopen("state");
    reshapeviewport();
    wintitle("Curve -- STATUS");
    prefsize(370,  151);
    winconstraints();
    ortho2(-10.0, 360.0, -10.0, 141.0);

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
    print_precision();
    print_it();
    draw_it();
    draw_it2();

    swapbuffers();
    
    color(BLACK);
    clear();
    print_basis();
    print_precision();
    print_it();
    draw_it();
    draw_it2();

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
    print_precision();
    print_it();
    draw_it();
    draw_it2();
}

print_basis()
/*---------------------------------------------------------------------------
 * Display the current basis in the state window.
 *---------------------------------------------------------------------------
 */
{
    char buf[50];

#ifdef FORTRAN
    sprintf(buf, "CALL CURVEB (%s)", basis_names[curbasis-1]);
#else
    sprintf(buf, "curvebasis (%s);", basis_names[curbasis-1]);
#endif
    color(UNPICKCOLOR);
    cmov2(0.0, 0.0);
    charstr(buf);
}

print_precision()
/*---------------------------------------------------------------------------
 * Print the current curve precision in the state window.
 *---------------------------------------------------------------------------
 */
{
    char buf[50];

#ifdef FORTRAN
    sprintf(buf, "CALL CURVEP (%d)", curprec);
#else
    sprintf(buf, "curveprecision (%d);", curprec);
#endif
    if (curcont == PRECISION)
	color(HIGHCOLOR);
    else
	color(NORMCOLOR);
    cmov2i(0, 121);
    charstr(buf);

    color(WHITE);
    linewidth(2);
    line2(-10.0, 111.0, 360.0, 111.0);

}

pick_state()
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
   	    ortho2(-10.0, 360.0, -10.0, 141.0);
	    pushname(CONTROL);
	    pushname(PRECISION);
	    rectfi(-10, 111, 360, 141);
	retnumber = endpick(pick_buffer);
    popmatrix();
}
