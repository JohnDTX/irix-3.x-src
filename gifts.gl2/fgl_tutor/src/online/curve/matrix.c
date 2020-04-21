#include "curve.h"

static Matrix beziermat = {
	{ -1, 3, -3, 1}, 
	{3, -6, 3, 0}, 
	{-3, 3, 0, 0}, 
	{1, 0, 0, 0}
    };

static Matrix cardmat = {
	{ -0.5, 1.5, -1.5, 0.5}, 
	{ 1.0, -2.5, 2.0, -0.5}, 
	{ -0.5, 0.0, 0.5, 0.0}, 
	{ 0, 1, 0, 0}
    };

static Matrix bsplinemat = {
	{-1.0/6.0, 3.0/6.0, -3.0/6.0, 1.0/6.0}, 
	{3.0/6.0, -1.0, 0.5, 0}, 
	{-0.5, 0.0, 0.5, 0.0}, 
	{1.0/6.0, 4.0/6.0, 1.0/6.0, 0}
    };

char *control_string();

char *control_string(n)
/*---------------------------------------------------------------------------
 * Returns the control string to print row n of the matrix.
 *---------------------------------------------------------------------------
 */
int n;
{
    static char contr[4][30] = 
       {" %5.2f, %5.2f, %5.2f, %5.2f", 
	" %5.2f, %5.2f, %5.2f, %5.2f", 
	" %5.2f, %5.2f, %5.2f, %5.2f", 
	" %5.2f, %5.2f, %5.2f, %5.2f"};

    return(contr[n]);
}

print_it()
/*---------------------------------------------------------------------------
 * Print the matrix.
 *---------------------------------------------------------------------------
 */
{
    int i;
    char buf[40];
    Matrix curmat;

    switch (curbasis){
    case BEZIER:
	matrix_cpy(curmat, beziermat);
	break;
    case CARDINAL:
	matrix_cpy(curmat, cardmat);
	break;
    case BSPLINE:
	matrix_cpy(curmat, bsplinemat);
	break;
    }

    color(UNPICKCOLOR);
    for(i = 0 ; i < 4 ; i++){
	cmov2(95.0, (float) (i*16+20));
	sprintf(buf, control_string(i), curmat[3-i][0], curmat[3-i][1], 
					curmat[3-i][2], curmat[3-i][3]);
	charstr(buf);
    }
    color(WHITE);
    line2(95.0, 18.0, 95.0, 80.0);
    line2(95.0, 18.0, 100.0, 18.0);
    line2(95.0, 80.0, 100.0, 80.0);

    line2(347.0, 18.0, 347.0, 80.0);
    line2(347.0, 18.0, 342.0, 18.0);
    line2(347.0, 80.0, 342.0, 80.0);
}

draw_it()
/*---------------------------------------------------------------------------
 * Draw the matrix box.
 *---------------------------------------------------------------------------
 */
{

    color(UNPICKCOLOR);
    cmov2i(0, 94);

    switch(curbasis){
#ifdef FORTRAN
    case BEZIER:
	charstr("CALL DEFBAS (BEZIER, beziermat)");
	break;
    case CARDINAL:
	charstr("CALL DEFBAS (CARDINAL, cardmat)");
	break;
    case BSPLINE:
	charstr("CALL DEFBAS (BSPLINE, bsplinemat)");
	break;
    }
#else
    case BEZIER:
	charstr("defbasis (BEZIER, beziermat);");
	break;
    case CARDINAL:
	charstr("defbasis (CARDINAL, cardmat);");
	break;
    case BSPLINE:
	charstr("defbasis (BSPLINE, bsplinemat);");
	break;
    }
#endif
}

draw_it2()
/*---------------------------------------------------------------------------
 * Draw the matrix box.
 *---------------------------------------------------------------------------
 */
{

    color(UNPICKCOLOR);
    cmov2i(0, 52);

    switch(curbasis){
    case BEZIER:
	charstr("beziermat");
	break;
    case CARDINAL:
	charstr("cardmat");
	break;
    case BSPLINE:
	charstr("bsplinemat");
	break;
    }

}
