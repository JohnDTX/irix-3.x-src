/*	@(#)fgetc.c	1.2	*/
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include <stdio.h>

int
fgetc(fp)
FILE *fp;
{
	return(getc(fp));
}
