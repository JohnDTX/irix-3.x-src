/*	@(#)libzer.c	1.2	*/
# include <stdio.h>

yyerror( s ) char *s; {
	fprintf(  stderr, "%s\n", s );
	}
