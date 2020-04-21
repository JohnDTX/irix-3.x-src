#include "grid.h"

/*---------------------------------------------------------------------------
 * All programs pertaining to the large polygon grid window.
 *---------------------------------------------------------------------------
 */

redraw_grid()
/*---------------------------------------------------------------------------
 * Routine called by the main program to display the grid and all of the data
 * pertaining to it.
 *---------------------------------------------------------------------------
 */
{
    attach_to_grid();			    /* Attach to the grid window. */
    reshapeviewport();
    color(GRIDBACK);
    clear();
    getorigin(&ogx, &ogy);
    getsize(&sgx, &sgy);
    draw_grid_at(GRIDCOLOR, GRIDX, GRIDY);  /* Draw the grid. */
    draw_big_poly(GRIDX, GRIDY);	    /* Draw the polygon. */
}

draw_grid_at(grid_color, gridx, gridy)
/*---------------------------------------------------------------------------
 * Draw the large polygon grid at with lower left corner coordinates (gridx,
 * gridy).
 *---------------------------------------------------------------------------
 */
Colorindex grid_color;
Screencoord gridx, gridy;
{
    pushmatrix();	    /* Save the stack, as always. */
	translate((Coord) gridx, (Coord) gridy, 0.0);
	draw_grid(grid_color);
    popmatrix();
}

draw_grid(grid_color)
/*---------------------------------------------------------------------------
 * Draw the grid in color grid_color.
 *---------------------------------------------------------------------------
 */
Colorindex grid_color;
{
    register Gridcoord i;

    color(grid_color);
    for(i = 0 ; i < GRIDSIZE ; i++){
	move2i(screenc(i), 0);
	draw2i(screenc(i), screenc(GRIDSIZE-1));
	move2i(0, screenc(i));
	draw2i(screenc(GRIDSIZE-1), screenc(i));
    }
}

Screencoord screenc(i)
/*---------------------------------------------------------------------------
 * Return the screen coordinate of the grid coordinate i.
 *---------------------------------------------------------------------------
 */
Gridcoord i;
{
    return(i * UNITSIZE);
}

int init_grid()
/*---------------------------------------------------------------------------
 * Initialize the grid window and return the window id to the calling routine.
 *---------------------------------------------------------------------------
 */
{
    int res;

    prefposition (275, 725, 50, 500);
    res = winopen("grid");	    /* Open it. */
    wintitle("Gouraud Shading -- GRID");	    /* Title it. */
    winset(res);		    /* Make it current. */
    keepaspect(1, 1);		    /* Make is square. */
    winconstraints ();
    setup_grid_environ();    	    /* Setup the environment. */

    return(res);
}

setup_grid_environ()
/*---------------------------------------------------------------------------
 * Setup the viewing transformation and viewport for the grid environment.
 *---------------------------------------------------------------------------
 */
{
    reshapeviewport();
	ortho2(-((float)UNITSIZE),
	(float)(UNITSIZE*GRIDSIZE), 
	-((float)UNITSIZE),
	(float)(UNITSIZE*GRIDSIZE));
}



float get_dxdy(bot, top)
/*---------------------------------------------------------------------------
 * Returns the change in x over the change in y of the edge defined by bot and
 * top.
 *---------------------------------------------------------------------------
 */
int bot, top;
{
    float dy, dx;

    dy = polyarray[top][Y] - polyarray[bot][Y];
    dx = polyarray[top][X] - polyarray[bot][X];

    if (dy != 0.0)
        return(dx/dy);
    else
	return(0.0);
}

float get_didy(bot, top)
/*---------------------------------------------------------------------------
 * Returns the change in i over the change in y of the edge defined by bot and
 * top.
 *---------------------------------------------------------------------------
 */
int bot, top;
{
    float dy, di;

    dy = polyarray[top][Y] - polyarray[bot][Y];
    di = polyarray[top][I] - polyarray[bot][I];

    if (dy != 0.0)
	return(di/dy);
    else
	return(0.0);
}

get_bot_vertecies(minleft, minright)
/*---------------------------------------------------------------------------
 * Search the vertex list for the left and right most vertex with a minimum y
 * value.
 *---------------------------------------------------------------------------
 */
int *minleft, *minright;
{
    register int i, minid;
    register float miny;

    miny = (float) GRIDSIZE;

    /* Find the left most minimum. */
    for (i = 0 ; i < NUMVERTS ; i++)
	if (polyarray[i][Y] < miny){
	    minid = i;
	    miny = polyarray[i][Y];
	} else if (polyarray[i][Y] == miny){
	    if (polyarray[i][X] < polyarray[minid][X]){
		minid = i;
	    }
	}
    *minleft = minid;


    /* Find the right most minimum. */
    for (i = 0 ; i < NUMVERTS ; i++)
	if ((polyarray[i][Y] == miny) && (i != minid))
	    if (polyarray[i][X] > polyarray[minid][X])
		minid = i;
    *minright = minid;
}

get_left(cur, left)
/*---------------------------------------------------------------------------
 * Find the left most nieghbore of the vertex cur.
 *---------------------------------------------------------------------------
 */
int cur, *left;
{
    register int less, more;

    less = cycle(cur - 1);
    more = cycle(cur + 1);

    if (polyarray[less][X] < polyarray[more][X])
	*left = less;
    else
	*left = more;
}

get_right(cur, right)
/*---------------------------------------------------------------------------
 * Find the right most nieghbore of the vertex cur.
 *---------------------------------------------------------------------------
 */
int cur, *right;
{
    register int less, more;

    less = cycle(cur - 1);
    more = cycle(cur + 1);

    if (polyarray[less][X] > polyarray[more][X])
	*right = less;
    else
	*right = more;
}

draw_scanline(lx, li, rx, ri, line)
/*---------------------------------------------------------------------------
 * Draw the scanline starting from lx and goind to ly, interpolating the color
 * indecis along the way.
 *---------------------------------------------------------------------------
 */
float lx, li, rx, ri, line;
{
    float di;
    register int i;

    if (rx != lx)
        di = (ri - li)/(rx - lx);
    else
	di = 0.0;


    for (i = (int) (lx + 0.5) ; i <= (int) (rx + 0.5) ; i++){
	draw_pixel(i, (int) line, (Colorindex) (li));
	li += di;
	if (di < 0.0){
	    if (li < ri)
		li = ri;
	} else {
	    if (li > ri)
		li = ri;
	}
    }
}

cycle(n)
/*---------------------------------------------------------------------------
 * Returned the cycled value of the number n, within the range of 0 to
 * NUMVERTS.
 *---------------------------------------------------------------------------
 */
int n;
{
    if (n < 0)
	return(NUMVERTS+n);

    if (n >= NUMVERTS)
	return(n - NUMVERTS);

    return(n);
}

draw_pixel(x, y, i)
/*---------------------------------------------------------------------------
 * Draw the pixel at (x, y) with the color index RAMPBOT+i.
 *---------------------------------------------------------------------------
 */
int x, y;
Colorindex i;
{
	rampcolor(i);
	pushmatrix();
	    translate((Coord) screenc(x), (Coord) screenc(y), 0.0);
	    callobj(PIXEL);
	    callobj(HIGHLIGHT);
	popmatrix();
}

draw_h_pixel(x, y, i)
/*---------------------------------------------------------------------------
 * Draw a highlighted pixel at (x, y) with the color index RAMPBOT+i.
 *---------------------------------------------------------------------------
 */
int x, y;
Colorindex i;
{
	rampcolor(i);
	pushmatrix();
	    translate((Coord) screenc(x), (Coord) screenc(y), 0.0);
	    callobj(HPIXEL);
	popmatrix();
}

attach_to_grid()
/*---------------------------------------------------------------------------
 * Attach all graphics output to this window.
 *---------------------------------------------------------------------------
 */
{
    winset(gridid);
}

pick_grid()
/*---------------------------------------------------------------------------
 * Attach to and check the grid for a picking hit.
 *---------------------------------------------------------------------------
 */
{
    register int i;
    attach_to_grid();
    pushmatrix();
        pick(pick_buffer, MAXPICK);
	    ortho2(-((float)UNITSIZE),
	    (float)(UNITSIZE*GRIDSIZE), 
	    -((float)UNITSIZE),
	    (float)(UNITSIZE*GRIDSIZE));
	    initnames();
	    pushname(GRID);
	    for (i = 0 ; i < NUMVERTS ; i++){
		pushname(i);
		draw_pixel((int)polyarray[i][X], (int)polyarray[i][Y], WHITE);
		popname();
	    }
	retnumber = endpick(pick_buffer);
    popmatrix();
}

draw_current_vertex()
/*---------------------------------------------------------------------------
 * Draw an un-highlighted current vertex. 
 *---------------------------------------------------------------------------
 */
{
    if (current_vertex != -1){
    draw_pixel((int)polyarray[current_vertex][X], 
		(int)polyarray[current_vertex][Y], 
		(int)polyarray[current_vertex][I]);

    pushmatrix();
	translate((Coord)UNITSIZE*polyarray[current_vertex][X], 
		  (Coord)UNITSIZE*polyarray[current_vertex][Y], 0.0);
	color(YELLOW);
	callobj(HIGHLIGHT);
    popmatrix();
    }
}

draw_h_current_vertex()
/*---------------------------------------------------------------------------
 * Draw an un-highlighted current vertex. 
 *---------------------------------------------------------------------------
 */
{
    char lab[2];

    if (current_vertex != -1)
    draw_h_pixel((int)polyarray[current_vertex][X], 
		(int)polyarray[current_vertex][Y], 
		(int)polyarray[current_vertex][I]);
}

respond_at_grid(vert)
/*---------------------------------------------------------------------------
 * Respond at the grid with a hit on the vertex vert.
 *---------------------------------------------------------------------------
 */
int vert;
{
    float old;
    
    old = polyarray[current_vertex][I];

    load_help("To move the current VERTEX:", 
	      "  Keep the LEFT MOUSE button down and", 
	      "  DRAG it around the grid.");

    if (vert == current_vertex){
	moving = TRUE;
    } else {
	moving = TRUE;
	attach_to_grid();
	draw_current_vertex();
	current_vertex = vert;
	draw_h_current_vertex();
	attach_to_console();
	move_highlight(old, polyarray[vert][I]);
	erase_name();
	print_name();
    }

}

do_moving()
/*---------------------------------------------------------------------------
 * If the moving flag is true, then do movement of the current polygon vertex.
 *---------------------------------------------------------------------------
 */
{
    float vx, vy;
    float ox, oy;

    attach_to_grid();
    get_cursor_coord(&vx, &vy);
    if ((vx != polyarray[current_vertex][X]) ||
	(vy != polyarray[current_vertex][Y])){
	moved = TRUE;
	ox = polyarray[current_vertex][X];
	oy = polyarray[current_vertex][Y];
	polyarray[current_vertex][X] = vx;
	polyarray[current_vertex][Y] = vy;
	if (!convex()){
	    polyarray[current_vertex][X] = ox;
	    polyarray[current_vertex][Y] = oy;
	} else {
	    color(GRIDBACK);
	    clear();
	    draw_grid_at(GRIDCOLOR, GRIDX, GRIDY);
	    draw_wire_poly(GRIDX, GRIDY);
	    attach_to_console();
	    erase_name();
	    print_name();
	}
    }
}

get_cursor_coord(cx, cy)
/*---------------------------------------------------------------------------
 * Load the coordinate of the cursor in grid space into the variables cx and
 * cy.
 *---------------------------------------------------------------------------
 */
float *cx, *cy;
{
    short mx, my;
    Coord wx, wy;

    grab_mice(&mx, &my);	/* Get valuator location. */
    mapw2(GRIDTRAN, mx-ogx, my-ogy, &wx, &wy);	/* Make world coordinates. */
    *cx = gridc(wx);
    *cy = gridc(wy);
}

gridc(wc)
/*---------------------------------------------------------------------------
 * Convert the world coordinate wc into grid coordinates.
 *---------------------------------------------------------------------------
 */
Coord wc;
{
    register float res;

    res = (float) ( (int) ( (wc/ (Coord) UNITSIZE ) + 0.5 ) );
    if (res < 0.0)
	res = 0.0;
    else if (res > (float) (GRIDSIZE-1))
	res = (float) (GRIDSIZE-1);

    return(res);
}

draw_wire_poly()
/*---------------------------------------------------------------------------
 * Draw a wire model of the large polygon on the large polygon grid.
 *---------------------------------------------------------------------------
 */
{
    connect_vertecies();	    /* Connect all vertecies. */
    draw_all_vertecies();	    /* Draw the vertecies. */
}

connect_vertecies()
/*---------------------------------------------------------------------------
 * Draw a wire frame of the large polygon.
 *---------------------------------------------------------------------------
 */
{
    register int i;

    color(EDGECOLOR);

    move2i(screenc((int)polyarray[0][X]), screenc((int)polyarray[0][Y]));

    for (i = 1 ; i < NUMVERTS ; i++)
	draw2i(screenc((int)polyarray[i][X]), screenc((int)polyarray[i][Y]));

    draw2i(screenc((int)polyarray[0][X]), screenc((int)polyarray[0][Y]));
}

draw_all_vertecies()
/*---------------------------------------------------------------------------
 * Draw all of the vertecies with there respective color values.
 *---------------------------------------------------------------------------
 */
{
    register int i;
    char lab[2];

    for(i = 0 ; i < NUMVERTS ; i++){
	draw_pixel( (int) polyarray[i][X], 
		    (int) polyarray[i][Y], 
		    (Colorindex) polyarray[i][I]);
	pushmatrix();
	    translate((Coord)UNITSIZE*polyarray[i][X], 
		      (Coord)UNITSIZE*polyarray[i][Y], 0.0);
	    color(YELLOW);
	    callobj(HIGHLIGHT);
	popmatrix();
    }

    draw_h_current_vertex();
}

get_max_y(top)
/*---------------------------------------------------------------------------
 * Get the vertex with the greatest y value.
 *---------------------------------------------------------------------------
 */
int *top;
{
    register int i, curi;
    register float maxy;

    maxy = -1.0;

    for (i = 0 ; i < NUMVERTS ; i++)
	if (polyarray[i][Y] > maxy){
	    maxy = polyarray[i][Y];
	    *top = i;
	}	
}

convex()
/*---------------------------------------------------------------------------
 * Check the current polygon, return true if it is convex.
 *---------------------------------------------------------------------------
 */
{
    Vector v1, v2, res, base, ve1, ve2;
    int cursgn;
    register int i;

    vect_cpy(base, polyarray[i]);
    vect_cpy(ve1, polyarray[cycle(i-1)]);
    vect_cpy(ve2, polyarray[cycle(i+1)]);

    base[I] = 0.0;
    ve1[I] = 0.0;
    ve2[I] = 0.0;

    vect_mult(base, -1.0);
    vect_add(ve1, base, v1);
    vect_add(ve2, base, v2);

    cross_product(v1, v2, res);
    cursgn = sgnf(res[Z]);

    for (i = 1 ; i < NUMVERTS ; i++){
	vect_cpy(base, polyarray[i]);
	vect_cpy(ve1, polyarray[cycle(i-1)]);
	vect_cpy(ve2, polyarray[cycle(i+1)]);

	base[I] = 0.0;
	ve1[I] = 0.0;
	ve2[I] = 0.0;

	vect_mult(base, -1.0);
	vect_add(ve1, base, v1);
	vect_add(ve2, base, v2);

	cross_product(v1, v2, res);

	if ((cursgn != sgnf(res[Z])) && (sgnf(res[Z]) != 0.0))
	    return(FALSE);

    }

    return(TRUE);
}

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

on_grid(x, y)
/*---------------------------------------------------------------------------
 * Returns true if the cursor is on the grid window.
 *---------------------------------------------------------------------------
 */
long x, y;
{
    return((x >= ogx) && (x <= ogx + sgx) &&
	   (y >= ogy) && (y <= ogy + sgy));
}
