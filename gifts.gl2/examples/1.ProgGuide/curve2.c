#include "gl.h"

Matrix beziermatrix = {
	{ -1, 3, -3, 1 },
	{ 3, -6, 3, 0 },
	{ -3, 3, 0, 0 },
	{ 1, 0, 0, 0 }
};	

Matrix cardinalmatrix = {
	{ -0.5, 1.5, -1.5, 0.5 },
	{ 1.0, -2.5, 2.0, -0.5 },
	{ -0.5, 0, 0.5, 0 },
	{ 0, 1, 0, 0 }
};	

Matrix bsplinematrix = {
	{ -1.0/6.0, 3.0/6.0, -3.0/6.0, 1.0/6.0 },
	{ 3.0/6.0, -6.0/6.0, 3.0/6.0, 0 },
	{ -3.0/6.0, 0, 3.0/6.0, 0 },
	{ 1.0/6.0, 4.0/6.0, 1.0/6.0, 0 }
};	

#define BEZIER 1
#define CARDINAL 2
#define BSPLINE 3

Coord geom2[6][3] = {
	{ 150.0, 400.0, 0.0},
	{ 350.0, 100.0, 0.0},
	{ 200.0, 350.0, 0.0},
	{ 50.0, 0.0, 0.0},
	{ 0.0, 200.0, 0.0},
	{ 100.0, 300.0, 0.0},
};

main ()
{

	ginit();

	color(BLACK);
	clear();

	defbasis(BEZIER,beziermatrix); /* define a basis matrix
				called BEZIER */
	defbasis(CARDINAL,cardinalmatrix); /* a new basis is defined */
	defbasis(BSPLINE,bsplinematrix); /* a new basis is defined */

	curvebasis(BEZIER); /* the Bezier matrix becomes the current basis */
	curveprecision(20); /* the precision is set to 20 */
	color(RED);
	crvn(6, geom2); /* the curvs command called with a Bezier basis
			causes three separate curve segments to be drawn */

	curvebasis(CARDINAL); /* the Cardinal basis becomes the current basis */
	color(GREEN);
	crvn(6, geom2); /* the curvs command called with a Cardinal spline basis
			causes a smooth curve to be drawn */

	curvebasis(BSPLINE); /* the B-spline basis becomes the current basis */
	color(BLUE);
	crvn(6, geom2);  /* the curvs command called with a B-spline basis
			causes the smoothest curve to be drawn */
	sleep(5);
	gexit();
}

