#include "back2.h"

redraw_with()
/*---------------------------------------------------------------------------
 * Redraw the with window.
 *---------------------------------------------------------------------------
 */
{
    attach_to_with();
    reshapeviewport();
    draw_with();
}

redraw_without()
/*---------------------------------------------------------------------------
 * Redraw the without window.
 *---------------------------------------------------------------------------
 */
{
    attach_to_without();
    reshapeviewport();
    draw_without();
}

init_with()
/*---------------------------------------------------------------------------
 * Initiliaze the with backface window.
 *---------------------------------------------------------------------------
 */
{
    int res;

    prefposition(513, 1022, 1, 511);
    res = winopen("with");
    keepaspect(508, 510);
    wintitle("Backface -- WITH Backface Removal");
    winconstraints();
    setup_with_environ();

    return(res);
}

init_without()
/*---------------------------------------------------------------------------
 * Initialize the without window.
 *---------------------------------------------------------------------------
 */
{
    int res;

    prefposition(1, 511, 1, 511);
    res = winopen("without");
    keepaspect(510, 510);
    wintitle("Backface -- WITHOUT Backface Removal");
    winconstraints();
    setup_without_environ();

    return(res);
}

setup_with_environ()
/*---------------------------------------------------------------------------
 * Setup the drawing environment for the with window.
 *---------------------------------------------------------------------------
 */
{
    reshapeviewport();
    perspective(900, 1.0, 1.0, 4.0);
    translate(0.0, 0.0, -2.5);
}

setup_without_environ()
/*---------------------------------------------------------------------------
 * Setup the drawing environment for the without window.
 *---------------------------------------------------------------------------
 */
{
    reshapeviewport();
    perspective(900, 1.0, 1.0, 4.0);
    translate(0.0, 0.0, -2.5);
}

draw_cubes()
/*---------------------------------------------------------------------------
 * Draw the cubes in the lower half of the screen.
 *---------------------------------------------------------------------------
 */
{

    pushmatrix();
	loadmatrix(current);
        calculate_orientation_data();
    popmatrix();
	draw_with();
	draw_without();

}

draw_with()
/*---------------------------------------------------------------------------
 * Draw a cube in the lower right hand corner of the screen with backface
 * removal on.
 *---------------------------------------------------------------------------
 */
{
    attach_to_with();
    pushmatrix();
	color(BACKC);
	clear();

	multmatrix(current);

	if (arrows){
	    calculate_visibility();
	    draw_back_normals();
        }
	backface(TRUE);
	draw_cube();
	backface(FALSE);
	if (arrows)
	    draw_arrows();
    popmatrix();

}

draw_without()
/*---------------------------------------------------------------------------
 * Draw the cube without backface removal.
 *---------------------------------------------------------------------------
 */
{
    attach_to_without();
    pushmatrix();
	color(BACKC);
	clear();
	multmatrix(current);
	draw_cube();
    popmatrix();
}

calculate_intinsity()
/*---------------------------------------------------------------------------
 * Calculate the intinsity of each side of the cube. (Uses cosine shading.)
 *---------------------------------------------------------------------------
 */
{
    register int i;
    register float intins;

    for (i = 0 ; i < NUMSIDES ; i++){
	intins = dot_product(eye, new_normals[i]);
	intins = ABS(intins);
	mapcolor(CUBEBOT + i, (short) (CUBCR * intins), 
			      (short) (CUBCG * intins), 
			      (short) (CUBCB * intins));
    }
}

calculate_visibility()
/*---------------------------------------------------------------------------
 * Calculats the visibility of the faces of the cube with a dot product.
 *---------------------------------------------------------------------------
 */
{
    register int i;

    for (i = 0 ; i < NUMSIDES ; i++){
	visible[i] = (dot_product(eye, new_normals[i]) > 0.15);
    }
}

calculate_orientation_data()
/*---------------------------------------------------------------------------
 * Calculate all data (except visibility) pertaining to the orientation of the
 * cube in space.
 *---------------------------------------------------------------------------
 */
{
    calculate_normals();	    /* Get the rotated normals. */
    if (shading){
	calculate_intinsity();	    /* Do cosine shading.	*/
    }
}

calculate_normals()
/*---------------------------------------------------------------------------
 * Calculate the normals by sending the static data down the pipeline and
 * retreving the new locations.
 *---------------------------------------------------------------------------
 */
{
    long num;

    short buffer[NUMSIDES * 10];
    
    feedback(buffer, NUMSIDES * 10); /* Feedback mode, run away!! 	*/
	draw_normals_fb();	    /* Draw the normals with xfpt's	*/
    num = endfeedback(buffer);

    parse_feedback(buffer);
}

draw_normals_fb()
/*---------------------------------------------------------------------------
 * Draw the normals to the cube with xfpt's.
 *---------------------------------------------------------------------------
 */
{
    register int i;

    for (i = 0 ; i < NUMSIDES ; i++)
	xfpt(normals[i][X], normals[i][Y], normals[i][Z], 0.0);
}

parse_feedback(buffer)
/*---------------------------------------------------------------------------
 * Parse the feedback buffer buffer, containing n elements.
 *---------------------------------------------------------------------------
 */
short *buffer;
{
    register int i;

    for(i = 0 ; i < NUMSIDES ; i++)
	read_normal(buffer, i);
}

read_normal(buffer, n)
/*---------------------------------------------------------------------------
 * Read the i'th normal from the feedback buffer buffer.
 *---------------------------------------------------------------------------
 */
short *buffer;
int n;
{
    HVector hom;
    register int i;

    read_hvector(hom, buffer + (n * 9));

    vect_cpy(new_normals[n], hom);

    normalize(new_normals[n]);
}

read_hvector(vect, buff)
/*---------------------------------------------------------------------------
 * Read a homgenious vector hom out of a feedback buffer buff.
 *---------------------------------------------------------------------------
 */
HVector vect;
short *buff;
{
    register int i;
    float *fbuff;

    buff++;			    /* Skip the tolken.			*/
    fbuff = (float *)(buff);

    for (i = 0 ; i <= W ; i++)
	vect[i] = *(fbuff++);
}

draw_cube()
/*---------------------------------------------------------------------------
 * Draw the cube.
 *---------------------------------------------------------------------------
 */
{
    register int i;

    for (i = 0 ; i < NUMSIDES ; i++)
	draw_side(i);
}

draw_side(n)
/*---------------------------------------------------------------------------
 * Draw side n of the cube.
 *---------------------------------------------------------------------------
 */
int n;
{
    register int i;

    if (shading)
        color(CUBEBOT + n);
    else
	color(1 + n);

    pmv(points[verts[n][0]][X], 
	points[verts[n][0]][Y], 
	points[verts[n][0]][Z]);

    for (i = 1 ; i < NUMVERTS ; i++){
	pdr(points[verts[n][i]][X], 
	    points[verts[n][i]][Y], 
	    points[verts[n][i]][Z]);
    }

    pclos();
}

attach_to_with()
/*---------------------------------------------------------------------------
 * Attach to the section of the screen with the cube with backfacing.
 *---------------------------------------------------------------------------
 */
{
    winset(withw);
}

attach_to_without()
/*---------------------------------------------------------------------------
 * Attach to the window without backface removal.
 *---------------------------------------------------------------------------
 */
{
    winset(withow);
}

draw_arrows()
/*---------------------------------------------------------------------------
 * Draw the directional arrows on the visible faces of the cube.
 *---------------------------------------------------------------------------
 */
{
    register int i;

    linewidth(2);
    color(ARROWC);

    for (i = 0 ; i < NUMSIDES ; i++){
	if (visible[i]){
	    draw_arrow(i);
	    draw_normal(i);
	}
    }
}

draw_arrow(n)
/*---------------------------------------------------------------------------
 * Draw the arrows for side n.
 *---------------------------------------------------------------------------
 */
int n;
{
    register int i;

    for (i = 0 ; i < NUMVERTS-1 ; i++)
	if ((n == 0) || (n == 1)){
	arrow(points[verts[n][i]][X] * 3.0/5.0, 
	      points[verts[n][i]][Y] * 3.0/5.0, 
	      points[verts[n][i]][Z], 
	      points[verts[n][i+1]][X] * 3.0/5.0, 
	      points[verts[n][i+1]][Y] * 3.0/5.0, 
	      points[verts[n][i+1]][Z]);
	} else if ((n == 2) || (n == 3)){
	arrow(points[verts[n][i]][X] * 3.0/5.0, 
	      points[verts[n][i]][Y], 
	      points[verts[n][i]][Z] * 3.0/5.0, 
	      points[verts[n][i+1]][X] * 3.0/5.0, 
	      points[verts[n][i+1]][Y], 
	      points[verts[n][i+1]][Z] * 3.0/5.0);
	} else if ((n == 4) || (n == 5)){
	arrow(points[verts[n][i]][X], 
	      points[verts[n][i]][Y] * 3.0/5.0, 
	      points[verts[n][i]][Z] * 3.0/5.0, 
	      points[verts[n][i+1]][X], 
	      points[verts[n][i+1]][Y] * 3.0/5.0, 
	      points[verts[n][i+1]][Z] * 3.0/5.0);
	}

	if ((n == 0) || (n == 1)){
	arrow(points[verts[n][NUMVERTS-1]][X] * 3.0/5.0, 
	      points[verts[n][NUMVERTS-1]][Y] * 3.0/5.0, 
	      points[verts[n][NUMVERTS-1]][Z], 
	      points[verts[n][0]][X] * 3.0/5.0, 
	      points[verts[n][0]][Y] * 3.0/5.0, 
	      points[verts[n][0]][Z]);
	} else if ((n == 2) || (n == 3)){
	arrow(points[verts[n][NUMVERTS-1]][X] * 3.0/5.0, 
	      points[verts[n][NUMVERTS-1]][Y], 
	      points[verts[n][NUMVERTS-1]][Z] * 3.0/5.0, 
	      points[verts[n][0]][X] * 3.0/5.0, 
	      points[verts[n][0]][Y], 
	      points[verts[n][0]][Z] * 3.0/5.0);
	} else if ((n == 4) || (n == 5)){
	arrow(points[verts[n][NUMVERTS-1]][X], 
	      points[verts[n][NUMVERTS-1]][Y] * 3.0/5.0, 
	      points[verts[n][NUMVERTS-1]][Z] * 3.0/5.0, 
	      points[verts[n][0]][X], 
	      points[verts[n][0]][Y] * 3.0/5.0, 
	      points[verts[n][0]][Z] * 3.0/5.0);
	}
}

arrow(x1, y1, z1, x2, y2, z2)
/*---------------------------------------------------------------------------
 * Draw an arrow from here to there.
 *---------------------------------------------------------------------------
 */
Coord x1, y1, z1, x2, y2, z2;
{
    register float dx, dy, dz;

    dx = (x2-x1)/20.0;
    dy = (y2-y1)/20.0;
    dz = (z2-z1)/20.0;

    line(x1, y1, z1, x2, y2, z2);

    if (dx != 0.0){
	pmv(x2-dx, y2+dx, z2);
	pdr(x2-dx, y2-dx, z2);
	pdr(x2, y2, z2);
	pclos();
	pmv(x2, y2, z2);
	pdr(x2-dx, y2, z2+dx);
	pdr(x2-dx, y2, z2-dx);
	pclos();
    }

    if (dy != 0.0){
	pmv(x2, y2, z2);
	pdr(x2+dy, y2-dy, z2);
	pdr(x2-dy, y2-dy, z2);
	pclos();
	pmv(x2, y2, z2);
	pdr(x2, y2-dy, z2-dy);
	pdr(x2, y2-dy, z2+dy);
	pclos();
    }

    if (dz != 0.0){
	pmv(x2, y2, z2);
	pdr(x2-dz, y2, z2-dz);
	pdr(x2+dz, y2, z2-dz);
	pclos();
	pmv(x2, y2, z2);
	pdr(x2, y2-dz, z2-dz);
	pdr(x2, y2+dz, z2-dz);
	pclos();
    }
}

draw_normal(n)
/*---------------------------------------------------------------------------
 * Draw a normal for side n of the cube.
 *---------------------------------------------------------------------------
 */
int n;
{
    switch(n){
    case 0:
	arrow(0.0, 0.0, 0.5, 0.0, 0.0, 1.5);
	break;
    case 1:
	arrow(0.0, 0.0, -0.5, 0.0, 0.0, -1.5);
	break;
    case 2:
	arrow(0.0, 0.5, 0.0, 0.0, 1.5, 0.0);
	break;
    case 3:
	arrow(0.0, -0.5, 0.0, 0.0, -1.5, 0.0);
	break;
    case 4:
	arrow(0.5, 0.0, 0.0, 1.5, 0.0, 0.0);
	break;
    case 5:
	arrow(-0.5, 0.0, 0.0, -1.5, 0.0, 0.0);
	break;
    }
}

draw_back_normals()
/*---------------------------------------------------------------------------
 * Draw the normals for the cube.
 *---------------------------------------------------------------------------
 */
{
    register int i;

    linewidth(2);
    color(ARROWC);

    for (i = 0 ; i < NUMSIDES ; i++)
	if (!visible[i])
	    draw_normal(i);
}

