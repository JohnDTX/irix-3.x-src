/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
extern int errno;
extern char *sys_errlist[];

main(argc, argv) 
char *argv[] ; 
int argc ;
{
char TOC = '\0' ; /* '-t' option (table of contents) default: disabled */
char SUM = '\0' ; /* '-s' option (summary of symbol use) default: disabled */
char MAIN = '\0' ; /* '-n' option (no formatting of cross-reference listing) default: enabled */
char temp[120], sort[120], toc[120], *buf;
extern char  *malloc(), *mktemp() ;
 
FILE *fin, *fout, *fopen() ;
struct stat stbuf ;
char line[100] ; /* to read lines of sorted cross-reference file */
char *pntr1;
char enable = '\0' ; /* flag used to check if any command line option has been set */
char end_options = '\0' ; /* end of options flag */

/********************* command line options parser *************************/

	if (argc == 1) {
			usage(); /* display the usage message and exit */
	}
/* while macref is called with one or more arguments, start looking */
/* beyond the option delimeter '-' 				    */
	while ( --argc > 0 && (*++argv)[0] == '-') {
		if (end_options) usage(0); /*no options after '--' */
		/* options with no arguments  may be grouped behind one '-' */
		for (pntr1 = argv[0] + 1; *pntr1 != '\0'; pntr1++) {
			switch (*pntr1) {
			case '-':
				end_options = '1' ;
				continue;
			case 'n':
				if (end_options) usage();
				enable = MAIN = '1' ;
				continue ;
			case 's':
				if (end_options) usage();
				enable = SUM = '1' ;
				continue ;
			case 't':
				if (end_options) usage();
				enable = TOC = '1' ;
				continue ;
			default:
				fprintf (stderr, "illegal option:  %c\n", *pntr1) ;
				usage() ;
			}
		}
		if ( !end_options && ! enable ) {
			printf("illegal option: -\n") ;
			usage() ;
		}
		enable = '\0' ;
	}

/********************* process the input file(s)  ***********************/
	argv-- ;


	if (! argc) usage();
/* at this point get the input file name(s) from the command line and
   process them one by one 						*/
	while (argc-- > 0 ) {
		argv++ ;
		if ( stat(*argv, &stbuf) == -1  )  {/* get file info */
			fprintf (stderr, "can't find: %s\n", *argv) ; 
			continue ;
		}
		if ((stbuf.st_mode & S_IFMT) != S_IFREG ) { /* not a regular file*/
			fprintf(stderr, "%s is not a regular file\n", *argv) ;
			continue ;
		}
		if ((fin = fopen(*argv, "r")) == NULL)  {
			fprintf(stderr, "Can't open %s\n", *argv) ;
			continue ;
		}

/*  make three temporary file names in the arrays temp, sort and toc */
		strcpy(temp, "/tmp/macref.tXXXXXX");
		strcpy(sort, "/tmp/macref.sXXXXXX");
		strcpy(toc,  "/tmp/macref.cXXXXXX");
		(void) mktemp(temp);
		(void) mktemp(sort);
		(void) mktemp(toc) ;

		printf("File:-%s\n\n", *argv);
		fout = fopen(temp, "w") ;
		macrefmain(fin, fout) ; /* main parser for macref */
		fclose(fin) ;
		fclose(fout) ;
		if (stat(temp, &stbuf) == -1 ) {
			fprintf(stderr, "can't find %s \n", temp) ;
			continue ;
		}
		if ( stbuf.st_size == 0 ) { /* no references */
			fprintf(stderr, "Empty cross-reference file\n") ;
			continue ;
		}
		sprintf((buf = malloc(100)), "sort %s >%s\n", temp, sort) ;
		if ( (system(buf)) < 0 )	{
			(void) perror(*sys_errlist[errno] );
			continue ;
		}
		if (TOC) { /* macro table of contents */
			if ((fin = fopen(sort, "r")) != NULL) {
				fout = fopen(temp, "w") ;
				macrtoc(fin, fout) ; 
				fclose(fin) ;
				fclose(fout) ;
				printf("	Macro Table of Contents\n");
				printf("	-----------------------\n");
				sprintf(buf, "pr -4 -t %s >%s\n", temp, toc );
				if ( (system(buf)) < 0 )	{
					(void) perror(*sys_errlist[errno] );
					continue ;
				}
				fin = fopen (toc, "r") ;
				while (fgets(line, 100, fin) != NULL )
					printf("%s", line) ;
				fclose(fin) ;
			}
		}
		if (SUM) { /* Symbol use statistics */
			if ((fin = fopen(sort, "r")) != NULL) {
				macrstat(fin) ;
				fclose(fin) ;
			}
		}
		printf("Cross-reference listing\n") ;
		printf("-----------------------\n") ;
		if ((fin = fopen(sort, "r")) != NULL) {
			if ( ! MAIN ) macrform (fin ) ; /* multi-column output of references */
			else	{ /* display the sorted cross-reference file */
				while (fgets(line, 100, fin) != NULL )
					printf("%s", line) ;
			}
			fclose(fin) ;
		}
		sprintf(buf, "rm -f %s %s %s\n", temp, sort, toc);
		if ( (system(buf)) < 0 )	{
			(void) perror(*sys_errlist[errno] );
			continue;
		}
	}
}

usage()
{
	fprintf (stderr, "Usage: macref [-n] [-s] [-t] [--] file ...\n") ;
	exit(1) ;
}
