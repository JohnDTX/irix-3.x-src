/*---------------------------------------------------------------------------
 * Type definitions etc for the math library. 
 *---------------------------------------------------------------------------
 */
#define	    X   0
#define	    Y	1
#define	    Z	2
#define	    W	3

#define ABS(A)	(A < 0.0 ? -A:A)

typedef float	Vector[3], 
		HVector[4];

float dot_product();
float norm();
