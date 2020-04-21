#include "curve.h"

init_cons()
/*---------------------------------------------------------------------------
 * Initialize the console window.
 *---------------------------------------------------------------------------
 */
{
    int res;

    prefposition(554, 954, 574, 750);
    res = winopen("cons");
    reshapeviewport();
    prefsize(400, 176);
    winconstraints();
    wintitle("Curve -- CONTROL BARS");
    ortho2(-200.0, 200.0, -88.0, 88.0);

    return(res);
}

redraw_cons()
/*---------------------------------------------------------------------------
 * Redraw the console window.
 *---------------------------------------------------------------------------
 */
{
    winset(consw);
    reshapeviewport();

    color(BLACK);
    clear();
    draw_correct_sliders();

    swapbuffers();

    color(BLACK);
    clear();
    draw_correct_sliders();

}

draw_cons()
/*---------------------------------------------------------------------------
 * Draw the slider bars in the left hand corner.
 *---------------------------------------------------------------------------
 */
{
    winset(consw);

    color(BLACK);
    clear();

    draw_correct_sliders();
}

draw_knob_no_ruling(pos)
/*---------------------------------------------------------------------------
 * Draw the knob at the slider bar at the position pos.
 *---------------------------------------------------------------------------
 */
float pos;
{
    color(CBAR2COLOR);
    rectf( -155.0 +  pos, -7.0, -145.0 + pos, 7.0);
}

draw_correct_sliders()
/*---------------------------------------------------------------------------
 * Draw the correct number of slider bars, depending on what's being edited.
 *---------------------------------------------------------------------------
 */
{
    if ((curcont > NONE) && (curcont < NUMPNTS))
	draw_coord_sliders();
    else if (curcont == PRECISION)
	draw_precision_slider();
}

draw_coord_sliders()
/*---------------------------------------------------------------------------
 * Draw the slider bars for the coordinates of the current control point.
 *---------------------------------------------------------------------------
 */
{
    int i;

    for (i = TOP ; i <= BOTTOM ; i++)
	draw_loc_slider(i, CTOS(curgeom[curcont][i]));
}

draw_precision_slider()
/*---------------------------------------------------------------------------
 * Draw a slider in the middle of the slider bar window with the current
 * precision.
 *---------------------------------------------------------------------------
 */
{
    draw_int_slider(0.0, 0.0, 60, 12, PTOS((float) curprec), 1.0, 25.0);
}

draw_loc_slider(loc, pos)
/*---------------------------------------------------------------------------
 * Draw a slider bar at either TOP, MIDDLE, or BOTTOM at position pos.
 *---------------------------------------------------------------------------
 */
int loc;
float pos;
{
    switch (loc){
    case TOP:
	draw_slider(0.0, 44.25, 75, 19, pos, -2.0, 2.0);
	break;
    case MIDDLE:
	draw_slider(0.0, 0.0, 75, 19, pos, -2.0, 2.0);
	break;
    case BOTTOM:
	draw_slider(0.0, -44.25, 75, 19, pos, -2.0, 2.0);
	break;
    }
}

handle_sliders(num)
/*---------------------------------------------------------------------------
 * Handle a pick hit on slider bar number n.
 *---------------------------------------------------------------------------
 */
int num;
{
    moving = TRUE;		    /* We are changing a value.		*/
    curslider = num;		    /* We are changing this value.	*/
}

pick_sliders()
/*---------------------------------------------------------------------------
 * Attach to and check the sliders for a picking hit.
 *---------------------------------------------------------------------------
 */
{
    int i;

    winset(consw);

    pushmatrix();
        pick(pick_buffer, MAXPICK);
    	    ortho2(-200.0, 200.0, -88.0, 88.0);
	    initnames();
	    pushname(SLIDERS);
	    if (curcont == PRECISION){
		pushname(MIDDLE);
		draw_pick_slider(MIDDLE);
		popname();
	    } else if ((curcont > NONE) && (curcont < NUMPNTS)) {
		for (i = TOP ; i <= BOTTOM ; i++){
		    pushname(i);
		    draw_pick_slider(i);
		    popname();
		}
	    }
	retnumber = endpick(pick_buffer);
    popmatrix();
}

draw_pick_slider(loc)
/*---------------------------------------------------------------------------
 * Draw a slider bar at either TOP, MIDDLE, or BOTTOM.
 *---------------------------------------------------------------------------
 */
int loc;
{
    switch (loc){
    case TOP:
	pushmatrix();
    	    translate(0.0, 44.25, 0.0);
	    rectfi(-150, -3, 149, 3);
	popmatrix();
	break;
    case MIDDLE:
	pushmatrix();
	    translate(0.0, 0.0, 0.0);
	    rectfi(-150, -3, 149, 3);
	popmatrix();
	break;
    case BOTTOM:
	pushmatrix();
	    translate(0.0, -44.25, 0.0);
	    rectfi(-150, -3, 149, 3);
	popmatrix();
	break;
    }
}

update_sliders()
/*---------------------------------------------------------------------------
 * Update the position of the slider according to mouse position.  Assumed
 * that  we are in movement mode when this routine is called.
 *---------------------------------------------------------------------------
 */
{
    short mx, my;
    Coord wx, wy;
    long ox, oy;

    winset(consw);
    grab_mice(&mx, &my);
    getorigin(&ox, &oy);

    mapw2(SLIDERTRANS, mx-ox, my-oy, &wx, &wy);

    wx += 150.0;

    if (wx > 300.0)
	wx = 300.0;
    else if (wx < 0.0)
	wx = 0.0;

    if (curcont == PRECISION){
	curprec = (short) (STOP(wx));
	if (curprec == 0)
	    curprec = 1;
    } else {
	curgeom[curcont][curslider] = STOC(wx);
    }
}
