/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

	/* This program formats the alphabetically  sorted cross-reference 
	 * file into a list of symbols, each accompanied to its right, by a
	 * multi-column list of all references to that symbol. It writes its
	 * output on stdout						*/

	/* The format of the input line is:
	 * [ [ (NMname)] Mname-] tpye lnum [#]   where
	 * (NMname) are the maros within in which Mname is defined
	 * Mname is the name of the macro within which reference occurs
	 * type may be request, macro, diversion, string, number register
	 *      or a parameter
	 * line number of reference
	 * # value of symbol modified at this reference			*/


#include <stdio.h>
macrform(infile)
FILE *infile ;
{
	FILE *inf ;
	char sym[100] ; /* symbol beng processed */
	char nsym[100] ; /* symbol read */
	char osym[100] ; /* symbol appended with appropriate number of white spaces for output */
	char olline[100], hline[100], lline[100] ;
	char ttt[100], first[100], last[100] ;
	int n;

	start:
		inf = infile ;
		sym[0] = nsym[0] = olline[0] = '\0' ;
		while (fgets(lline, 100, inf) != NULL) {
			hline[0] = '\0' ;
			if (match (lline, first, "\n", last))
				concat (first, last, lline) ;
			if (! match(lline, nsym, "	", last)) continue ; /* hor. tab */
			assign(sizeof(lline), lline, last) ;
		/* symbol in nsym; input line beyond 'ht' in lline */
		rd0:
			if (match(lline, ttt, ")", last)) {
				assign(sizeof(lline), lline, last) ;
				goto rd4 ;
			}
		rd1:
		/* squeeze out any white spaces(s) between type and line-no */
			while (match(lline, ttt, " ", last)) /* white space */
				concat(ttt, last, lline) ;
			concat (hline, lline, lline) ;
			if ( match(sym, first, nsym, last)) 
				if ( match(nsym, first, sym, last)) goto rd5 ;
		/* different symbol encountered */
		rd2:
			if ( ! match("", first, olline, last)) 
				printf("%s\n", olline) ;
		rd3:
			assign (sizeof(sym), sym, nsym) ;
			concat(sym, "      ", osym) ; /* six white spaces */
			osym[6] = '\0' ;
			assign(sizeof(olline), olline, osym) ;
			goto rd6 ;
		rd4:
			concat(hline, ttt, hline) ;
			concat( hline, ")", hline) ;
			goto rd0 ;
		rd5:
			if ( match("", first, olline, last)) 
				assign(sizeof(olline), olline, "      ") ; /* 6 white spaces */
		rd6:
			if ( ! match(lline, first, "#", last)) 
		/* adjust the length of lline by appending a white space in lieu of # */
				concat(lline, " ", lline) ; /* white space */
		rd7:
			/* 17 character positions per column */
			concat("                 ", lline, lline) ; /* 17 white spaces */
			concat(lline, "	", lline) ; /* horizontal tab */
			if (match(lline,first,"	", last)) /* horizontal tab */
				split(first, lline, 17) ; /* cut off 17 characters from right end of lline */
			concat(olline, lline, olline) ; /* append lline to olline */
			n = 0 ;
			while (olline[n++]) ;
			if (n < 68 ) continue ;
			/* cannot accomodate another reference on this output line */
			printf("%s\n", olline) ;
			olline[0] = '\0' ;
			continue ;
		}
		if ( ! match ("", first, olline, last ))
			printf("%s\n", olline) ;
}
