/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

# include "e.h"
# include "e.def"

funny(n)
	int n;
{
	char *f;

	yyval = salloc();
	switch(n) {
	case SUM:
		f = "\\(*S"; break;
	case UNION:
		f = "\\(cu"; break;
	case INTER:	/* intersection */
		f = "\\(ca"; break;
	case PROD:
		f = "\\(*P"; break;
	default:
		error(FATAL, "funny type %d in funny", n);
	}
	printf(".ds %d \\v'.3m'\\s+5%s\\s-5\\v'-.3m'\n", yyval, f);
	eht[yyval] = EM(1.0, ps+5) - EM(0.2, ps);
	ebase[yyval] = EM(0.3, ps);
	eps[yyval] = ps;
	dprintf(".\tS%d <- %s; h=%g b=%g\n", 
		yyval, f, eht[yyval], ebase[yyval]);
	lfont[yyval] = rfont[yyval] = ROM;
}
