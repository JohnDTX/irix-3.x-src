#include <gl.h>
#include <math.h>
#include  "mymath.h"

/*---------------------------------------------------------------------------
 * My math library with all my silly little math functions that I use all the
 * time.  Mostly vector routines and personalized hacks etc.
 *---------------------------------------------------------------------------
 */

sgni(x)
/*---------------------------------------------------------------------------
 * Returns the sign of the integer x.
 *---------------------------------------------------------------------------
 */
int x;
{
    if (x < 0)
	return(-1);
    else if (x > 0)
	return(1);
    else
	return(0);
}

sgnf(x)
/*---------------------------------------------------------------------------
 * Returns the sign of the floating point number x.
 *---------------------------------------------------------------------------
 */
float x;
{
    if (x < 0.0)
	return(-1);
    else if (x > 0.0)
	return(1);
    else
	return(0);
}

float dot_product(v1, v2)
/*---------------------------------------------------------------------------
 * Returns the dot product of the two vectors v1 and v2.
 *---------------------------------------------------------------------------
 */
Vector v1, v2;
{
    register float res;
    register int i;

    res = 0.0;

    for (i = X ; i < W ; i++)
	res += v1[i]*v2[i];

    return(res);
}

vect_cpy(v1, v2)
/*---------------------------------------------------------------------------
 * Copy vector v2 into v1.
 *---------------------------------------------------------------------------
 */
Vector v1, v2;
{
    register int i;

    for (i = X ; i <= Z ; i++)
	v1[i] = v2[i];
}

vect_add(v1, v2, res)
/*---------------------------------------------------------------------------
 * Add vector v1 to vector v2 and put the result in res.
 *---------------------------------------------------------------------------
 */
Vector v1, v2, res;
{
    register int i;

    for (i = X ; i <= Z ; i++)
	res[i] = v1[i] + v2[i];
}

vect_mult(v1, n)
/*---------------------------------------------------------------------------
 * Multiply all of the elements of the vector v1 by n and put the result back
 * in v1.
 *---------------------------------------------------------------------------
 */
Vector v1;
float n;
{
    register int i;

    for (i = X ; i <= Z ; i++)
	v1[i] *= n;
}

cross_product(v1, v2, res)
/*---------------------------------------------------------------------------
 * Loads the cross product of v1 with v2 into the vector res.
 *---------------------------------------------------------------------------
 */
Vector v1, v2, res;
{
    res[X] = v1[Y]*v2[Z] - v1[Z]*v2[Y];
    res[Y] = v1[Z]*v2[X] - v1[X]*v2[Z];
    res[Z] = v1[X]*v2[Y] - v1[Y]*v2[X];
}

float norm(v1)
/*---------------------------------------------------------------------------
 * Returns the norm (ie length) of the vector v1.
 *---------------------------------------------------------------------------
 */
Vector v1;
{
    return(sqrt(dot_product(v1, v1)));
}

normalize(v1)
/*---------------------------------------------------------------------------
 * Normalize the vector v1.
 *---------------------------------------------------------------------------
 */
Vector v1;
{
    register float n;

    n = norm(v1);
    vect_mult(v1, 1.0/n);
}

vector_print(vec)
/*---------------------------------------------------------------------------
 * Print the 3 diminsional vector vec.
 *---------------------------------------------------------------------------
 */
Vector vec;
{
    register int i;

    printf("[");
    for (i = 0 ; i <= Z ; i++)
	printf("%4.6f ", vec[i]);
    printf("]\n");
}

hvector_print(vec)
/*---------------------------------------------------------------------------
 * Print the 4 diminsional vector vec.
 *---------------------------------------------------------------------------
 */
Vector vec;
{
    register int i;

    printf("[");
    for (i = 0 ; i <= W ; i++)
	printf("%4.6f ", vec[i]);
    printf("]\n");
}

matrix_print(mat)
/*---------------------------------------------------------------------------
 * Print the 4x4 matrix mat.
 *---------------------------------------------------------------------------
 */
Matrix mat;
{
    register int i, j;

    for (i = 0 ; i < 4 ; i++){
        for (j = 0 ; j < 4 ; j++)
	    printf("%4.6f ", mat[i][j]);
	printf("\n");
    }
}

matrix_cpy(mat1, mat2)
/*---------------------------------------------------------------------------
 * Copy matrix mat2 to matrix mat1.
 *---------------------------------------------------------------------------
 */
Matrix mat1, mat2;
{
    register int i, j;

    for (i = 0 ; i < 4 ; i++)
	for (j = 0 ; j < 4 ; j++)
	    mat1[i][j] = mat2[i][j];
}
