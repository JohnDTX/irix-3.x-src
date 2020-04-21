char _Version_[] = "(C) Copyright 1983 UniSoft Corp. Version V.1.0";
char _Origin_[] = "System V";

/*	@(#)setbrk.c	1.2	*/
#
/*
 *	UNIX shell
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */

#include	"defs.h"

setbrk(incr)
{
	REG BYTPTR	a=(STRING)(sbrk(incr));
 	if ((int)a == -1) {
 		prs("sh: Out of Memory\n");
 		abort();
 	}
	brkend=a+incr;
	return((INT)(a));
}
