/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

char *xxxvers = "@(#)checkmm:chekmain1.c	1.4";

	/* This is the second pass through the file */
	/* This is required because a single lex file  generates
	 * a file too big for the 'as' on SVR2 to create the '.o'
	 * file. 						
	 * This is executed from within 'checkmm'		*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

int errors = 0;

main(argc, argv) 

char **argv ; 
int argc ;

{
extern int yylineno ;
extern FILE *yyin ;
FILE *fin ;

struct stat stbuf ;
	if (argc == 1)
		{ yyin = stdin;
		  yylineno = 1 ;
		  yylex() ;
		}
	else
	while (--argc > 0) {
		stat(*++argv, &stbuf) ;

		if ( stbuf.st_mode &  S_IFDIR ) {
			continue ;
		}

		if ((fin = fopen(*argv, "r")) == NULL) {
			printf("Can't open %s\n", *argv) ;
			continue ; 
		}
		yyin = fin ;
		yylineno = 1 ;
		yylex() ;
		fclose(fin) ;
	}

	exit(errors);
}
