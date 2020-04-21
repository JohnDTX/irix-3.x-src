/*	these two routines are the only members of the pascal runtimes
	that are necessary to satisfy the single precision floating point
	routines written in Pascal, with our compiler */

#include <stdio.h>

caseerr(mode, loc)
int loc,mode;

{
#ifndef STANDALONE
	fprintf(stderr,"fatal error in single precision float routines.\n");
	exit(-1);
#else
	printf("fatal error in single precision float routines.\n");
	asm("	trap  #14");
#endif
}

/* Round real number */
round (r)
float r;
{
    if ( *(int*)&r & 0x80000000)
        return ((int)(r - 0.5));
    else return ((int)(r + 0.5));
}
