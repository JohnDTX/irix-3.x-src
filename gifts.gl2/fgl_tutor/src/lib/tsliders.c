
#include "tutor.h"
#include <gl.h>

/*---------------------------------------------------------------------------
 * Draw a slider bar with the center at (x, y).  Puts the major rulings every
 * maj step and the minor rulings every min steps.
 *---------------------------------------------------------------------------
 */
draw_slider(x, y, maj, min, pos, left, right)
Coord x, y;
int maj, min;
float pos;
float left, right;
{


    static char buffer[10];
    pushmatrix();
	linewidth(1);
	translate(x, y, 0.0);
        color(CBARCOLOR);
	rectfi(-150, -3, 149, 3);

	color(UNPICKCOLOR);
	sprintf(buffer, "%7.2f", left);
    	cmov2i( (-150+2-9*7) + (int) (4.5 * (float) len(buffer)), -25);
	charstr(buffer);

	sprintf(buffer, "%7.2f", right);
	cmov2i( (149+4-9*7) + (int) (4.5 * (float) len(buffer)), -25);
	charstr(buffer);

	draw_knob(pos, maj, min);
	linewidth(2);
    popmatrix();
}

/*---------------------------------------------------------------------------
 * Draw a slider bar with the center at (x, y).  Puts the major rulings every
 * maj step and the minor rulings every min steps.
 *---------------------------------------------------------------------------
 */
draw_int_slider(x, y, maj, min, pos, left, right)

Coord x, y;
int maj, min;
float pos;
float left, right;
{

    static char buffer[10];
    pushmatrix();
	linewidth(1);
	translate(x, y, 0.0);
        color(CBARCOLOR);
	rectfi(-150, -3, 149, 3);

	color(UNPICKCOLOR);

	sprintf(buffer, "  %5d", (int) right);
	cmov2i( (149+4-9*7) + (int) (4.5 * (float) len(buffer)), -25);
	charstr(buffer);

	sprintf(buffer, "%5d", (int) left);
    	cmov2i( (-150 + 2 - (9 * 5))+ (int)(4.5 * (float) len(buffer)), -25);
	charstr(buffer);

	draw_knob(pos, maj, min);
	linewidth(2);
    popmatrix();
}


/*---------------------------------------------------------------------------
 * Draw the knob at the slider bar at the position pos.
 *---------------------------------------------------------------------------
 */
draw_knob(pos, maj, min)

float pos;
int maj, min;
{
    color(CBAR2COLOR);

    rectf( -155.0 +  pos, -7.0, -145.0 + pos, 7.0);
    draw_ruling(maj, min);

    color(YELLOW);
    rect( -155.0 + pos, -7.0, -145.0 + pos, 7.0);
    pnt2(-150.0 + pos, -5.0); 
    pnt2(-150.0 + pos, 5.0);
    pnt2(-150.0 + pos, -6.0); 
    pnt2(-150.0 + pos, 6.0); 
}

/*---------------------------------------------------------------------------
 * Draw the major rulings on the slider bar every n steps and all the minor
 * rulings every m steps.
 *---------------------------------------------------------------------------
 */
draw_ruling(n, m)

int n, m;
{
    int i;

    color(WHITE);

    for (i = 0 ; i < 300 ; i += n){
        move2i(-150 + i, 3);
	draw2i(-150 + i, -3);
    }
    
    for (i = m ; i < 300 ; i += m){
        move2i(-150 + i, 1);
	draw2i(-150 + i, -1);
    }
}

int len(string)
char string[];
{
    int i=0, j=0;

    while(string[i] != '\0' )
	if (string[i++] != ' ' ) j++;
    return(j);
}

/*---------------------------------------------------------------------------
 * Draw a slider bar with the center at (x, y).  Puts the major rulings every
 * maj step and the minor rulings every min steps.
 *---------------------------------------------------------------------------
 */
draw_blank_slider(x, y, maj, min)

Coord x, y;
int maj, min;
{
    static char buffer[10];
    pushmatrix();
	linewidth(1);
	translate(x, y, 0.0);
        color(CBARCOLOR);
	rectfi(-150, -3, 149, 3);

	draw_ruling(maj, min);

    popmatrix();
}
