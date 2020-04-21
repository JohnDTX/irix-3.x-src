/*	@(#)xlocal.c	1.3	*/
#include <stdio.h>
#include "mfile1"
#include "lmanifest"
/*
 * this file contains the functions local to CXREF
 * they put their output to outfp
 * the others contain lint's 1st pass with output thrown away
 * cgram.c has calls to these functions whenever a NAME is seen
 */

char infile[120];
FILE *outfp;

ref( i, line)
	int i, line;
{
	fprintf(outfp, "R%.8s\t%05d\n",stab[i].sname,line);
}

def( i, line)
	int i, line;
{
	if (stab[i].sclass == EXTERN)
		ref(i, line);
	else
	fprintf(outfp,"D%.8s\t%05d\n",stab[i].sname,line);
}


newf(i, line)
	int i, line;
{
	fprintf(outfp,"F%.8s\t%05d\n",stab[i].sname, line);
}
