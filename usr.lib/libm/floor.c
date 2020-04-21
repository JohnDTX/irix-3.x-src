/*	@(#)floor.c	1.5	*/
/*LINTLIBRARY*/

/*
 *	floor(x) returns the largest integer (as a double-precision number)
 *	not greater than x.
 *	ceil(x) returns the smallest integer not less than x.
 */

extern float modf();

float
floor(x) 
float x;
{
	float y;

	return (modf(x, &y) < 0.0 ? y - 1.0 : y);
}


float
ceil(x)
float x;
{
	float y;

	return (modf(x, &y) > 0.0 ? y + 1.0 : y);
}

/* the following are the double precision versions */

extern long float _lmodf();

long float
_lfloor(x) 
long float x;
{
	long float y;

	return (_lmodf(x, &y) < 0.0 ? y - 1.0 : y);
}


long float
_lceil(x)
long float x;
{
	long float y;

	return (_lmodf(x, &y) > 0.0 ? y + 1.0 : y);
}
