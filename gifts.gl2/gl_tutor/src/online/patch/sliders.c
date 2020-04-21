#include "patch.h"

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
    wintitle("Patch -- CONTROL BARS");
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

draw_correct_sliders()
/*---------------------------------------------------------------------------
 * Draw the correct number of slider bars, depending on what's being edited.
 *---------------------------------------------------------------------------
 */
{
    if ((curcont > NONE) && (curcont < NUMPNTS))
	draw_coord_sliders();
    else if (curcont == PRECISION)
	draw_precision_sliders();
    else if (curcont == NUMBER)
	draw_number_sliders();
}

draw_coord_sliders()
/*---------------------------------------------------------------------------
 * Draw the slider bars for the coordinates of the current control point.
 *---------------------------------------------------------------------------
 */
{
    draw_loc_slider(TOP, CTOS(get_geomx(curcont)));
    draw_loc_slider(MIDDLE, CTOS(get_geomy(curcont)));
    draw_loc_slider(BOTTOM, CTOS(get_geomz(curcont)));
}

draw_number_sliders()
/*---------------------------------------------------------------------------
 * Draw a slider in the middle of the slider bar window with the current
 * precision.
 *---------------------------------------------------------------------------
 */
{
    draw_int_slider(0.0, 44.25, 33, 300, NTOS((float) curcursu), 1.0, 10.0);
    draw_int_slider(0.0, -44.25, 33, 300, NTOS((float) curcursv), 1.0, 10.0);
}
draw_precision_sliders()
/*---------------------------------------------------------------------------
 * Draw a slider in the middle of the slider bar window with the current
 * precision.
 *---------------------------------------------------------------------------
 */
{
    draw_int_slider(0.0, 44.25, 62, 12, PTOS((float) curprecu), 1.0, 25.0);
    draw_int_slider(0.0, -44.25, 62, 12, PTOS((float) curprecv), 1.0, 25.0);
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
    color(RED);
    pushmatrix();
        pick(pick_buffer, MAXPICK);
	    initnames();
	    ortho2(-200.0, 200.0, -88.0, 88.0);
	    pushname(SLIDERS);
	    if (curcont == PRECISION){
		pushname(TOP);
		draw_pick_slider(TOP);
		popname();
		pushname(BOTTOM);
		draw_pick_slider(BOTTOM);
		popname();
	    } else if (curcont == NUMBER){
		pushname(TOP);
		draw_pick_slider(TOP);
		popname();
		pushname(BOTTOM);
		draw_pick_slider(BOTTOM);
		popname();
	    } else if ((curcont > NONE) && (curcont < NUMPNTS)) {
		pushname(TOP);
		draw_pick_slider(TOP);
		popname();
		pushname(MIDDLE);
		draw_pick_slider(MIDDLE);
		popname();
		pushname(BOTTOM);
		draw_pick_slider(BOTTOM);
		popname();
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
	    rectfi(-150, -7, 149, 7);
	popmatrix();
	break;
    case MIDDLE:
	pushmatrix();
	    rectfi(-150, -7, 149, 7);
	popmatrix();
	break;
    case BOTTOM:
	pushmatrix();
	    translate(0.0, -44.25, 0.0);
	    rectfi(-150, -7, 149, 7);
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
	switch(curslider){
	case TOP:
	    curprecu = (short) (STOP(wx));
	    break;
	case BOTTOM:
	    curprecv = (short) (STOP(wx));

	}

	if (curprecu == 0)
	    curprecu = 1;
	if (curprecv == 0)
	    curprecv = 1;

    } else if (curcont == NUMBER) {
	switch(curslider){
	case TOP:
	    curcursu = (short) (STON(wx));
	    break;
	case BOTTOM:
	    curcursv = (short) (STON(wx));

	}
    } else {
	switch(curslider){
	case TOP:
	    geomx[ROW(curcont)][COL(curcont)] = STOC(wx);
	    break;
	case MIDDLE:
	    geomy[ROW(curcont)][COL(curcont)] = STOC(wx);
	    break;
	case BOTTOM:
	    geomz[ROW(curcont)][COL(curcont)] = STOC(wx);
	    break;
        }
    }
}
