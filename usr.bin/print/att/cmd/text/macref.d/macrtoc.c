/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

	/* This program produces the Macro table of contents. It reads the
	 * alphabetically sorted cross-reference file and writes its
	 * output to the file pointed to by outfile						*/

	/* The format of the input line is:
	 * [ [ (NMname)] Mname-] tpye lnum [#]   where
	 * (NMname) are the maros within in which Mname is defined
	 * Mname is the name of the macro within which reference occurs
	 * type may be request, macro, diversion, string, number register
	 *      or a parameter
	 * line number of reference
	 * # value of symbol modified at this reference			*/

#include <stdio.h>
macrtoc(infile, outfile)
FILE *infile, *outfile ;
{
	
	FILE *inf, *outf ;
	char toc[100], line[100], name[100], ref[100], num[10];
	char ttt[100], first[100], last[100] ;
	int n = 0 ;

	start:
		toc[0] = '\0' ;
		inf = infile ;
		outf = outfile ;
	readf:
	while (fgets(line, 100, inf) != NULL ) {
		if ( ! match(line, name, "	", last)) continue; /* hor. tab */
		assign(sizeof(line), line, last) ;
		concat(name, "-m", ttt ) ;
		if ( ! match (line, ref, ttt, last ) ) continue ;
		if ( ! match (last, num ,"#", first ) ) continue ;
		concat(ref, name, ttt) ;
		if ( ! match (toc, first, ttt, last)) goto read1;
		if ( match (ttt, first, toc, last)) continue ;
	  read1:
		while ( match( num, ttt, " ", last)) /* white space */
			concat (ttt, last, num) ;
		concat (ref, name, toc) ;
		concat (ref, name, name) ;
		concat (name, "........", name) ;
		/* name is 9 characters long */
		name[9] = '\0' ;
		concat ("....", num, num) ;
		concat(num, "	", num) ; /* hor. tab */
		if (match(num, first, "	", last)) /* hor. tab */
			if ( first[0] && first[1] && first[2] && first[3] ) split(first, num ,4) ;
		fprintf (outf, "%s%s%s\n", name, "..", num) ;
	}
	if (  match ("", first, toc, last)) 
		fprintf (outf, "\n%s\n", "No Macros Defined.") ;
	fprintf (outf, "\n") ;
}
