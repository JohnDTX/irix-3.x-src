/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

	/* This program produces symbol use statistics. It reads the
	 * alphabetically sorted cross-reference file and writes its
	 * output stdout			*/

	/* The format of the input line is:
	 * [ [ (NMname)] Mname-] tpye lnum [#]   where
	 * (NMname) are the maros within in which Mname is defined
	 * Mname is the name of the macro within which reference occurs
	 * type may be request, macro, diversion, string, number register
	 *      or a parameter
	 * line number of reference
	 * # value of symbol modified at this reference			*/
#include <stdio.h>

char osym[100], oline[100];
char sym[100], nsym[100] ;
int strcnt, nrcnt, maccnt, reqcnt, parcnt, divcnt, modcnt;
 
macrstat(infile)
FILE *infile ;
{
	FILE *inf ;
	char  type[2], line[100];
	char first[100], last[100], junk[100] ;

	start:
		inf = infile;
		sym[0] = osym[0] = oline[0] = '\0' ;
		strcnt = nrcnt = maccnt = reqcnt = 0 ;
		parcnt = divcnt = modcnt = 0;
		printf("Symbol Use Statistics\n") ;
		printf("---------------------\n") ;
	m11 :
		while (fgets(line, 100, inf) != NULL) { 
			if (match(line, nsym, "	", last)) /* horizontal tab */
				assign(sizeof(line), line, last) ;
			if (line[0] ) {
				type[0] = line[0] ;
				type[1] = '\0' ;
			}
			/* type may follow a '-', if symbol within a macro */
			if (match(line, first, "-", last)) {
				while ( last[0] == '-' ) { /* ignore any '-' included in macro name */
					match (last, first, "-", junk) ;
					assign (sizeof(last), last, junk ) ;
				}
				if (last[0]  ) {
					type [0] = last [0] ;
					type[1] = '\0' ;
				}
			}
			if ( ! match(sym, first, nsym, last)) { /* different symbol than being processed */
				out() ;
				goto m12 ;
			}
			if ( ! match(nsym, first, sym ,last)) out() ;
		m12:
			switch( type[0] ) {
				case 'p':	/* parameter */
					parcnt++ ;
					break ;
				case 'm':	/* macro */
					maccnt++ ;
					break ;
				case 's':	/* string */
					strcnt++ ;
					break ;
				case 'n':	/* number register */
					nrcnt++ ;
					break ;
				case 'r':	/* request */
					reqcnt++ ;
					break ;
				case 'd':	/* diversion */
					divcnt++ ;
					break ;
				default:	/* bad case */
					printf("? type error: [%s]\n", type) ;
					return(0) ;
			}
		m13:
			if (  match(line, first, "#", last)) modcnt++ ;
		}
	lastl:
		out() ;
		printf("\n") ;
}
out() 

	/* This subroutine appends the statistics gathered to the symbol
	 * by invoking the subroutine plural. It also initializes the
	 * counters for the next symbol.				*/
{
	char first[100], last[100] ;
		if ( ! match("", first, sym, last)) { /* else goto out1 */
			plural (maccnt, "macro ref") ;
			plural(reqcnt, "request call") ;
			plural(strcnt, "string ref") ;
			plural (nrcnt, "number reg ref") ;
			plural (divcnt, "diversion ref") ;
			plural (parcnt, "parameter ref") ;
			plural (modcnt, "modification") ;
			printf("%s\n", oline) ;
		}
	out1:
		assign(sizeof(sym), sym ,nsym) ;
		assign(sizeof(osym), osym, sym) ;
		strcnt = nrcnt = maccnt = reqcnt = 0 ;
		parcnt = divcnt = modcnt = 0 ;
}

	/* this subroutine converts the interger, num, into a right justified
	 *  space filled string. It also adjusts the
	 * tense of the pharse if required. It adds a ',' to the output  
	 * line to take account of modifications made to the symbol, if any */
plural (num, string )
int num ;
char string[];
{
	char first[100], last[100], plur ;
	char numb[8] ;
	int lcount, n = 0 ;
		if (num == 0) return(1) ;
		lcount = num ;
		n = 7 ; numb[n--] = '\0' ;
		while ( lcount > 9 ) {
			numb[n--] = lcount % 10 + '0' ;
			lcount = lcount / 10 ;
		}
		numb[n--] = lcount + '0' ;
		while (n >= 0) numb[n--] = ' ' ;
		plur = '1' ;
		if (match("", first, osym, last)) { /* string is 'modification' */
			concat(oline, ", ", oline) ;
			n = 0 ;
		}
		else {
			concat(osym, "	", oline) ; /* hor. tab */
			osym[0] = '\0' ;
		}
		if (num == 1) plur = '\0' ; /* singular tense */
		concat(oline, numb, oline) ;
		/* squeeze out more than one white spaces following comma */
		while (match(oline,first,",  ", last)) {
			concat(first, ", ", oline) ;
			concat(oline, last, oline) ;
		}
		concat(oline, " ", oline) ; /* white space */
		concat(oline, string, oline) ;
		if (plur) concat(oline, "s", oline) ;
		return(1) ;
}
