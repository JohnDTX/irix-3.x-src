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

Coord geom1[4][3] = {
	{ 100.0, 100.0, 0.0},
	{ 200.0, 200.0, 0.0},
	{ 200.0, 0.0, 0.0},
	{ 300.0, 100.0, 0.0}
};	

main ()
{
	ginit();
	color(BLACK);
	clear();

	defbasis(BEZIER,beziermatrix); /* define a basis matrix called BEZIER */
	curvebasis(BEZIER); 		/* identify the BEZIER matrix
						as the current basis matrix */
	curveprecision(20); 	/* set the current precision to 20
		(the curve segment will be drawn using 20 line segments) */
	color(RED);
	crv(geom1); /* draw the curve based on four control points in geom1 */

	defbasis(CARDINAL,cardinalmatrix); /* a new basis is defined */
	curvebasis(CARDINAL); 		/* the current basis is reset */
			/* note that the curveprecision does not have to
				be restated unless it is to be changed */
	color(BLUE);
	crv(geom1); 		/* a new curve segment is drawn */

	defbasis(BSPLINE,bsplinematrix); /* a new basis is defined */
	curvebasis(BSPLINE); 	/* the current basis is reset */
	color(GREEN);
	crv(geom1); 		/* a new curve segment is drawn */

	gexit();
}
