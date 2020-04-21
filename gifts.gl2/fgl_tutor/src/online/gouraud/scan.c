#include "grid.h"

draw_big_poly(x, y)
/*---------------------------------------------------------------------------
 * Draw the big polygon on the polygon grid.
 *---------------------------------------------------------------------------
 */
int x, y;
{
    float cury,				/* Current scanline.		    */
	   curxl, curxr,		/* Current x values.		    */
	   curil, curir;		/* Current index values.	    */
    int	top, bot;			/* Top vertex on the polygon.	    */

    make_edge_list();
    pushmatrix();
	translate((Coord) x,  (Coord) y, 0.0);
	get_max_y(&top);
	get_min_y(&bot);

	for (cury  = polyarray[bot][Y] ; cury <= polyarray[top][Y] ; 
	    cury += 1.0){
	    get_edges(&curxl, &curil, &curxr, &curir, cury);
	    draw_scanline(curxl, curil, curxr, curir, cury);
	}

	draw_all_vertecies();
    popmatrix();
}

make_edge_list()
/*---------------------------------------------------------------------------
 * Create an edge list out of the current polygon.
 *---------------------------------------------------------------------------
 */
{
    register int i;

    ned = 0;

    for ( i = 0 ; i < NUMVERTS ; i++ ){
	make_edge(i);
	if (!horiz(ned)){
	    ned++;
	}
    }
}

make_edge(i)
/*---------------------------------------------------------------------------
 * Make the edge i out of the current polygon.
 *---------------------------------------------------------------------------
 */
int i;
{
    register int top, bot;

    top = i;
    bot = cycle(i+1);

    if (polyarray[top][Y] < polyarray[bot][Y]){
	top = bot;
	bot = i;
    }

    ed[ned][0] = top;
    ed[ned][1] = bot;
}

horiz(i)
/*---------------------------------------------------------------------------
 * Checks to see if the current edge i is horizontal, if so return TRUE.
 *---------------------------------------------------------------------------
 */
int i;
{
    return(polyarray[ed[i][0]][Y] == polyarray[ed[i][1]][Y]);
}



get_active_edges(i)
/*---------------------------------------------------------------------------
 * Get the active edges for scanline i.
 *---------------------------------------------------------------------------
 */
int i;
{
    register int cnt;
    register int j;

    cnt = 0;

    for (j = 0 ; j < ned ; j++){
	if (active_edge(j, i)){
	    ae[cnt][0] = ed[j][0];
	    ae[cnt][1] = ed[j][1];
	    cnt++;
	}
	if (cnt >= 2)
	    break;
    }
    sort_active_edges(i);
}

active_edge(j, i)
/*---------------------------------------------------------------------------
 * Returns TRUE if the scanline i passes through the edge j.
 *---------------------------------------------------------------------------
 */
int j, i;
{
    return((i >= (int) polyarray[ed[j][1]][Y]) && 
	   (i < (int) polyarray[ed[j][0]][Y]));}

sort_active_edges(i)
/*---------------------------------------------------------------------------
 * Sort the active edges in order of increasing x value.
 *---------------------------------------------------------------------------
 */
int i;
{
    float leftint, rightint;
    float dxdyl, dxdyr;

    dxdyl = get_dxdy(ae[0][1], ae[0][0]);
    dxdyr = get_dxdy(ae[1][1], ae[1][0]);

    leftint = polyarray[ae[0][1]][X] + 
		((float)i - polyarray[ae[0][1]][Y]) * dxdyl;
    rightint = polyarray[ae[1][1]][X] + 
		((float)i - polyarray[ae[1][1]][Y]) * dxdyr;

    if (leftint > rightint)
	swap_ae();
}

swap_ae()
/*---------------------------------------------------------------------------
 * Swap the two edges in the active edge list.
 *---------------------------------------------------------------------------
 */
{
    register int top, bot;

    top = ae[0][0];
    bot = ae[0][1];

    ae[0][0] = ae[1][0];
    ae[0][1] = ae[1][1];

    ae[1][0] = top;
    ae[1][1] = bot;
}

get_min_y(bot)
/*---------------------------------------------------------------------------
 * Get the vertex with the minimum y value.
 *---------------------------------------------------------------------------
 */
int *bot;
{
    register int i;
    register float miny;

    miny = (float) GRIDSIZE;

    for (i = 0 ; i < NUMVERTS ; i++)
	if (polyarray[i][Y] < miny){
	    miny = polyarray[i][Y];
	    *bot = i;
	}	
}

get_edges(xl, il, xr, ir, i)
/*---------------------------------------------------------------------------
 * Get the current value of x and i for the scanline i.
 *---------------------------------------------------------------------------
 */
float *xl, *xr, *il, *ir, i;
{
    float dxdyl, dxdyr, didyl, didyr;

    get_active_edges((int) i);

    dxdyl = get_dxdy(ae[LEFT][BOT], ae[LEFT][TOP]);
    didyl = get_didy(ae[LEFT][BOT], ae[LEFT][TOP]);

    dxdyr = get_dxdy(ae[RIGHT][BOT], ae[RIGHT][TOP]);
    didyr = get_didy(ae[RIGHT][BOT], ae[RIGHT][TOP]);

    *xl = polyarray[ae[LEFT][BOT]][X] +
	(i - polyarray[ae[LEFT][BOT]][Y]) * dxdyl;
    *il = polyarray[ae[LEFT][BOT]][I] +
	(i - polyarray[ae[LEFT][BOT]][Y]) * didyl;


    *xr = polyarray[ae[RIGHT][BOT]][X] +
	(i - polyarray[ae[RIGHT][BOT]][Y]) * dxdyr;
    *ir = polyarray[ae[RIGHT][BOT]][I] +
	(i - polyarray[ae[RIGHT][BOT]][Y]) * didyr;
}
