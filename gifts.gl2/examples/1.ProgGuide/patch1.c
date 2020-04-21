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
	{ -0.5, 0.0, 0.5, 0.0 },
	{ 0.0, 1.0, 0.0, 0.0 }
};	

Matrix bsplinematrix = {
	{ -1.0/6.0, 3.0/6.0, -3.0/6.0, 1.0/6.0 },
	{ 3.0/6.0, -6.0/6.0, 3.0/6.0, 0.0 },
	{ -3.0/6.0, 0.0, 3.0/6.0, 0.0 },
	{ 1.0/6.0, 4.0/6.0, 1.0/6.0, 0.0 }
};	

#define BEZIER 1
#define CARDINAL 2
#define BSPLINE 3

Coord geomx[4][4] = {
	{ 0.0, 100.0, 200.0, 300.0},
	{ 0.0, 100.0, 200.0, 300.0},
	{ 700.0, 600.0, 500.0, 400.0},
	{ 700.0, 600.0, 500.0, 400.0}
};	

Coord geomy[4][4] = {
	{ 400.0, 500.0, 600.0, 700.0},
	{ 0.0, 100.0, 200.0, 300.0},
	{ 0.0, 100.0, 200.0, 300.0},
	{ 400.0, 500.0, 600.0, 700.0}
};	

Coord geomz[4][4] = {
	{ 100.0, 200.0, 300.0, 400.0 },
	{ 100.0, 200.0, 300.0, 400.0 },
	{ 100.0, 200.0, 300.0, 400.0 },
	{ 100.0, 200.0, 300.0, 400.0 }
};	

main ()
{

	ginit();
	color(BLACK);
	clear();

	ortho(0.0, (float)XMAXSCREEN, 0.0, (float)YMAXSCREEN, 
	    (float)XMAXSCREEN, -(float)XMAXSCREEN);

	defbasis(BEZIER,beziermatrix); /* define a basis matrix
				called BEZIER */
	defbasis(CARDINAL,cardinalmatrix); /* define a basis matrix
				called CARDINAL */
	defbasis(BSPLINE,bsplinematrix); /* define a basis matrix
				called BSPLINE */

	patchbasis(BEZIER,BEZIER); /* a Bezier basis will be used for both
					directions in the first patch */
	patchcurves(4,7); /* seven curve segments will be drawn in the
				u direction and four in the v direction */
	patchprecision(20,20); /* the curve segments in u direction will consist
				of 20 line segments (the lowest multiple of
				vcurves greater than usegments) and the curve
				segments in the v direction will consist of 21
				line segments (the lowest multiple of ucurves
					greater than vsegments) */
	color(RED);
	patch(geomx,geomy,geomz); /* the patch is drawn based on the sixteen
					specified control points */

	patchbasis(CARDINAL,CARDINAL); /* the bases for both directions are reset */
	color(GREEN);
	patch(geomx,geomy,geomz);  /* another patch is drawn using the same
					control points but a different basis */

	patchbasis(BSPLINE,BSPLINE);  /* the bases for both directions are
					reset again */
	color(BLUE);
	patch(geomx,geomy,geomz);  /* a third patch is drawn */

	sleep(10);
	gexit();
}
