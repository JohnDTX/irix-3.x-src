/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sh:setbrk.c	1.8"
/*
 *	UNIX shell
 */

#include	"defs.h"

char 	*sbrk();

char*
setbrk(incr)
{

	register char *a = sbrk(incr);

	brkend = a + incr;
	return(a);
}
