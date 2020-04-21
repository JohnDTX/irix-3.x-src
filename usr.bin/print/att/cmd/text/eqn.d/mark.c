/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#include "e.h"

mark(p1)
	int p1;
{
	markline = 1;
	printf(".ds %d \\k(09\\*(%d\n", p1, p1);
	yyval = p1;
	dprintf(".\tmark %d\n", p1);
}

lineup(p1)
{
	markline = 2;
	if (p1 == 0) {
		yyval = salloc();
		printf(".ds %d \\h'|\\n(09u'\n", yyval);
	}
	dprintf(".\tlineup %d\n", p1);
}
