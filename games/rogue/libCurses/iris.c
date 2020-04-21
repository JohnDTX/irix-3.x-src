static	char	*Iris_c	= "@(#)iris.c	1.2";

#ifdef	iris

#include "gl.h"

/*
 * Interface to Iris where names clash
 */

/*
 * clear the screen via gl
 */
glclear()
{
	clear();
	icursor(0,0);
}

#endif
