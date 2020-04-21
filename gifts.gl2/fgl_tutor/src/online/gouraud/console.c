#include "console.h"

redraw_console()
/*---------------------------------------------------------------------------
 * Routine called from the main program to draw in the main console window.
 *---------------------------------------------------------------------------
 */
{
    attach_to_console();
    reshapeviewport();
    draw_console();
    getorigin(&ocx, &ocy);
    getsize(&scx, &scy);
}

int init_console()
/*---------------------------------------------------------------------------
 * Initialize the main console window.  This is the primary window and si
 * gotton with a getport.
 *---------------------------------------------------------------------------
 */
{
    int res;

/*    prefposition (100, 950, 600, 700);*/
    prefposition (100, 950, 530, 630);
    winopen("gouraud");		    /* Open it. */
    wintitle("Gouraud Shading -- CONSOLE");	    /* Title it. */
    keepaspect(17, 2);		    /* Make it the right shape. */
    winconstraints();
    res = winget();		    /* Get the ID. */
    winset(res);		    /* Make it current. */
    setup_cons_environ();	    /* Set the environment. */

    return(res);
}

setup_cons_environ()
/*---------------------------------------------------------------------------
 * Setup the viewport and viewing transformation for the console window.
 *---------------------------------------------------------------------------
 */
{
    reshapeviewport();
    ortho2(-SQRSIZE, SQRSIZE*33.0, -2.0*SQRSIZE, SQRSIZE*2.0);

}

attach_to_console()
/*---------------------------------------------------------------------------
 * Direct output to the console window.
 *---------------------------------------------------------------------------
 */
{
    winset(consid);
}

draw_console()
/*---------------------------------------------------------------------------
 * Draw the console.
 *---------------------------------------------------------------------------
 */
{
    color(CONSBACK);
    clear();
    draw_back_highlight();	    /* Draw the backgound boarder hlght.    */
    draw_color_ramp();		    /* Draw the color ramp to edit.	    */
    if (current_vertex != -1){
        draw_highlight(polyarray[current_vertex][I]);
	print_colorindex(polyarray[current_vertex][I]);
        print_name();
    }
}

draw_color_ramp()
/*---------------------------------------------------------------------------
 * Draw the color ramp in the console window.
 *---------------------------------------------------------------------------
 */
{
    register int i;

    for (i = 0 ; i < RAMPLEN ; i++)
	draw_box(i);

    linewidth(2);
    color(BLACK);
    rect(0.0, SQRSIZE, SQRSIZE*RAMPLEN, 0.0);

    for (i = 1 ; i < RAMPLEN ; i++){
	move((float) i * SQRSIZE, SQRSIZE);
	draw((float) i * SQRSIZE, 0.0);
    }
}

draw_box(n)
/*---------------------------------------------------------------------------
 * Draw the nth color ramp box in the console window. 
 *---------------------------------------------------------------------------
 */
int n;
{
    rampcolor(n);
    rectf(SQRSIZE*(float)n, SQRSIZE, SQRSIZE*(float)(n+1), 0.0);
}

draw_back_highlight()
/*---------------------------------------------------------------------------
 * Draw a rectangle that is slightly larger than the color ramp and highlight
 * an area representing the current color.
 *---------------------------------------------------------------------------
 */
{
    color(RAMPBACK);

    /* Draw the background. */
    rectf(-0.5*SQRSIZE, 1.5*SQRSIZE, 
	  SQRSIZE*((float) RAMPLEN + .5), -0.5 * SQRSIZE);
    linewidth(2);
    color(BLACK);
    rect(-0.5*SQRSIZE, 1.5*SQRSIZE, 
	  SQRSIZE*((float) RAMPLEN + .5), -0.5 * SQRSIZE);
}

draw_highlight(n)
/*---------------------------------------------------------------------------
 * Draw a hightlight at color square n on the color ramp.
 *---------------------------------------------------------------------------
 */
float n;
{
    color(RAMPHIGH);

    pmv(n*SQRSIZE, (1.5)*SQRSIZE);
    pdr((n+1.0)*SQRSIZE,(1.5)* SQRSIZE);
    pdr((n+.5)*SQRSIZE, SQRSIZE);
    pclos();

    pmv(n*SQRSIZE, (-.5)*SQRSIZE);
    pdr((n+.5)*SQRSIZE, 0.0);
    pdr((n+1.0)*SQRSIZE,(-.5)*SQRSIZE);
    pclos();

    linewidth(2);
    color(BLACK);
    move(n*SQRSIZE, (1.5)*SQRSIZE);
    draw((n+1.0)*SQRSIZE,(1.5)* SQRSIZE);
    draw((n+.5)*SQRSIZE, SQRSIZE);
    draw(n*SQRSIZE, (1.5)*SQRSIZE);

    move(n*SQRSIZE, (-.5)*SQRSIZE);
    draw((n+.5)*SQRSIZE, 0.0);
    draw((n+1.0)*SQRSIZE,(-.5)*SQRSIZE);
    draw(n*SQRSIZE, (-.5)*SQRSIZE);
}

load_vertex_name(st)
/*---------------------------------------------------------------------------
 * Load the name of the current vertex into the string st.
 *---------------------------------------------------------------------------
 */
char *st;
{
    sprintf(st, "V-%d  [%d, %d]", current_vertex, 
	    (int)polyarray[current_vertex][X], 
	    (int)polyarray[current_vertex][Y]);
}

print_name()
/*---------------------------------------------------------------------------
 * Print the name of the vertex at the bottom of the screen.
 *---------------------------------------------------------------------------
 */
{
    static char name[30];

    color(LABEL);
    load_vertex_name(name);
    set_digit_scale(6.0);

    pushmatrix();
	translate(12.0*SQRSIZE, -2.0*SQRSIZE, 0.0);
	draw_string(name);
    popmatrix();
}

erase_name()
/*---------------------------------------------------------------------------
 * Erase the name of the vertex on the console.
 *---------------------------------------------------------------------------
 */
{
    color(CONSBACK);
    rectf(12.0*SQRSIZE, -2.0*SQRSIZE, 22.0*SQRSIZE, -.8*SQRSIZE);
}

print_colorindex(n)
/*---------------------------------------------------------------------------
 * Print the colorindex number n in the square of colorindex n.
 *---------------------------------------------------------------------------
 */
float n;
{
    static char name[3];

    color(RAMPHIGH);

    sprintf(name, "%d", (int) n);
    set_digit_scale(4.0);
    pushmatrix();
	translate(SQRSIZE*n, 0.5, 0.0);
	draw_string(name);
    popmatrix();
}

pick_console()
/*---------------------------------------------------------------------------
 * Attach to and check the console for a picking hit.
 *---------------------------------------------------------------------------
 */
{
    register float cur;

    cur = polyarray[current_vertex][I];

    attach_to_console();
    pushmatrix();
        pick(pick_buffer, MAXPICK);
	    ortho2(-SQRSIZE, SQRSIZE*33.0, -2.0*SQRSIZE, SQRSIZE*2.0);
	    initnames();
	    pushname(CONSOLE);
	    rectf(SQRSIZE*(cur-.5),1.5*SQRSIZE,SQRSIZE*(cur+1.5),-.5*SQRSIZE);
	retnumber = endpick(pick_buffer);
    popmatrix();
}

respond_at_console()
/*---------------------------------------------------------------------------
 * Respond to the selector ont he console getting hit in picking.
 *---------------------------------------------------------------------------
 */
{
    selecting = TRUE;
    load_help("To edit the COLOR INDEX of the current VERTEX:", 
	      "  Keep the LEFT MOUSE button pressed and", 
	      "  DRAG the slider across the indecies.");
}

erase_highlight(n)
/*---------------------------------------------------------------------------
 * Erase the highlight at color index n on the color ramp console.
 *---------------------------------------------------------------------------
 */
float n;
{
    color(RAMPBACK);
    rectf(n*SQRSIZE, (1.5)*SQRSIZE, (n+1.0)*SQRSIZE, SQRSIZE);
    rectf(n*SQRSIZE, (-.5)*SQRSIZE, (n+1.0)*SQRSIZE, 0.0);
    draw_box((int) n);
    linewidth(2);
    color(BLACK);
    move(n*SQRSIZE, -.5*SQRSIZE);
    draw((n+1.0)*SQRSIZE, -.5*SQRSIZE);
    move(n*SQRSIZE, 1.5*SQRSIZE);
    draw((n+1.0)*SQRSIZE, 1.5*SQRSIZE);

    move(n*SQRSIZE, 0.0);
    draw((n+1.0)*SQRSIZE, 0.0);
    move(n*SQRSIZE, SQRSIZE);
    draw((n+1.0)*SQRSIZE, SQRSIZE);

    move(n*SQRSIZE, 0.0);
    draw(n*SQRSIZE, SQRSIZE);
    move((n+1.0)*SQRSIZE, 0.0);
    draw((n+1.0)*SQRSIZE, SQRSIZE);
}

move_highlight(from, to)
/*---------------------------------------------------------------------------
 * Move the hightlight from colorindex from to colorindex to.
 *---------------------------------------------------------------------------
 */
float from, to;
{
    erase_highlight(from);
    draw_highlight(to);
    print_colorindex(to);
}

do_selection()
/*---------------------------------------------------------------------------
 * This is called if the selection flag is true, it slides the slider bar
 * around according to the mouse and adjusts the colorindex of the current
 * vertex.
 *---------------------------------------------------------------------------
 */
{
    float pointer;

    attach_to_console();
    pointer = get_current_pointer();
    if (pointer != polyarray[current_vertex][I]){
	selected = TRUE;
    	move_highlight(polyarray[current_vertex][I], pointer);
	polyarray[current_vertex][I] = pointer;
	attach_to_grid();
	draw_h_current_vertex();
    }
	
}

float get_current_pointer()
/*---------------------------------------------------------------------------
 * Get the current color index the cursor is pointing at.
 *---------------------------------------------------------------------------
 */
{
    short mx, my;
    register float res;
    Coord wx, wy;

    grab_mice(&mx, &my);	    /* Get the valuator location. */

    mapw2(CONSTRAN, mx-ocx, my-ocy, &wx, &wy);

    res = (float) ((int) (wx/SQRSIZE));

    if (res < 0.0)
	res = 0.0;
    else if (res > (float) (RAMPLEN-1))
	res = (float) (RAMPLEN-1);
    
    return(res);
}

on_console(x, y)
/*---------------------------------------------------------------------------
 * Returns true if the cursor is on the console window.
 *---------------------------------------------------------------------------
 */
long x, y;
{
    return((x >= ocx) && (x <= ocx + scx) &&
	   (y >= ocy) && (y <= ocy + scy));
}
