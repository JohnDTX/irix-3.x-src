/*	@(#)move.c	1.1	*/
# include "e.h"
# include "e.def"

move(dir, amt, p) int dir, amt, p; {
	int a;

	yyval = p;
	a = VERT( (EFFPS(ps) * 6 * amt) / 100);
	printf(".ds %d ", yyval);
	if( dir == FWD || dir == BACK )	/* fwd, back */
		printf("\\h'%s%du'\\*(%d\n", (dir==BACK) ? "-" : "", a, p);
	else if (dir == UP)
		printf("\\v'-%du'\\*(%d\\v'%du'\n", a, p, a);
	else if (dir == DOWN)
		printf("\\v'%du'\\*(%d\\v'-%du'\n", a, p, a);
	if(dbg)printf(".\tmove %d dir %d amt %d; h=%d b=%d\n", 
		p, dir, a, eht[yyval], ebase[yyval]);
}
