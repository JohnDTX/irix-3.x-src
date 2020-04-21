#include "patch.h"

static Matrix beziermat = {
	{ -1, 3, -3, 1}, 
	{3, -6, 3, 0}, 
	{-3, 3, 0, 0}, 
	{1, 0, 0, 0}
    };

static Matrix cardinalmat = {
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

draw_matrix()
/*---------------------------------------------------------------------------
 * Draw the basis matrix.  Called from the main program.
 *---------------------------------------------------------------------------
 */
{
    attach_to_matrix();		    /* Direct the output.	*/

    color(YELLOW);
    print_it(curbasisu, 2);
    color(RED);
    print_it(curbasisv, 8);
}

attach_to_matrix()
/*---------------------------------------------------------------------------
 * Direct the graphics output to the matrix window.
 *---------------------------------------------------------------------------
 */
{
    viewport(513, 767, 539, 741);
    ortho2(0.0, 28.0, 12.0, 0.0);
}

char *control_string(n)
/*---------------------------------------------------------------------------
 * Returns the control string to print row n of the matrix.
 *---------------------------------------------------------------------------
 */
int n;
{
    static char contr[4][30] = 
       {"{%5.2f, %5.2f, %5.2f, %5.2f", 
	" %5.2f, %5.2f, %5.2f, %5.2f", 
	" %5.2f, %5.2f, %5.2f, %5.2f", 
	" %5.2f, %5.2f, %5.2f, %5.2f}"};

    return(contr[n]);
}

print_it(basis, n)
/*---------------------------------------------------------------------------
 * Print the matrix for basis basis at line n.
 *---------------------------------------------------------------------------
 */
int basis, n;
{
    int i;
    char buf[40];
    Matrix curmat;

    switch (basis){
    case BEZIER:
	matrix_cpy(curmat, beziermat);
	break;
    case CARDINAL:
	matrix_cpy(curmat, cardinalmat);
	break;
    case BSPLINE:
	matrix_cpy(curmat, bsplinemat);
	break;
    }

    for(i = 0 ; i < 4 ; i++){
	cmov2(.2, (float) (i+n));
	sprintf(buf, control_string(i), curmat[i][0], curmat[i][1], 
					curmat[i][2], curmat[i][3]);
	charstr(buf);
    }
}
