/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

char *xxxvers = "@(#)macref:main.c	1.3";
/***************************************************************************
	* This is the main parser for macref.
	* It is a transalation of macref.sno, the parser for macref in SNOBOL,
	* into the C language. 
	* It is invoked by main() with 2 file pointers, to the input and the
	* output files, as its arguments.
	* It invokes the following subroutines:
	*	initialize:  various data items, each time an input line
	*		     is read.
	*   match(src, prec, pat, fol): searches for the string 'pat' in 
	*	the string 'src'. Returns 'false' on failure, else returns 
	*	'true' copies the string precpeeding 'pat' in 'prec' and  
	*	and that following 'pat' in 'fol'.
	*   concat(first, second, result): concatenates the two strings, 
	*	'first' and 'second' in that order and copies the result 
	*	into 'result'.
	*   assign(size, dest, source): copies the string 'source' into the
	*	string 'dest', truncating to 'size' of 'dest', if required.
	*   split(source, dest, number): truncates 'number' characters off 
	*	the end of the string 'source' and copies them to the 
	*	string 'dest'.
	*****************************************************************/

#include <stdio.h>
#define len 1000 /* maximum input line size */

char aline[80], rpt[80], rqname[2],  lcnt[100];
char pline[len], line[len];
int lines ;

macrefmain(textfile, outfile)
FILE *textfile, *outfile ;
{
	FILE *tf, *outf ;
	int n;
	char name[80], name1[80], namef[80];
	char ttt[1000], manr[2], cmd[2];
	char rrr[80], uuu[80], last[1000], junk[1000], tline[1000];
	char out[80], first[1000], markst[100];
	char pat[2];

	start:
		tf = textfile; outf = outfile;
		name1[0] = namef[0] = '	' ; /* hor. tab */
		name1[1] = namef[1] = name[0] = aline[0] = '\0' ;
		lines = 0;
	next1:
		initialize() ;
		next13() ;
		while (fgets(line, len, tf) != NULL) {
			if ( match (line, first, "\n", last ))
				concat ( first, last, line ) ;
			pat[0] = '\134'; pat[1] = '\042'; pat[2] = '\0';
			if (match(line, ttt, pat, last) ) /* \" */
				assign (sizeof(line), line, ttt) ;
			pat[1] = '\134' ;
			while (match (line, ttt, pat, last) ) { /* \\ */
				concat (ttt, "\\", line) ;
				concat (line, last, line) ;
			}
			pat[1] = '!';
			while (match(line, ttt, pat, last)) /* \! */
				concat (ttt, last, line) ;
			pat[1] = '{' ;
			while (match(line, ttt, pat, last)) /* \{ */
				concat(ttt, last, line) ;
			pat[1] = '}' ;
			while (match(line, ttt, pat, last)) /* \} */
				concat(ttt, last, line) ;
			pat[1] = '.' ;
			while (match(line, ttt, pat, last)) { /* \. */
				concat(ttt, ".", line) ;
				concat (line, last, line) ;
			}
			while (match(line, ttt, "	", last)) { /* hor. tab */
				concat(ttt, " ", line) ; /* a white space */
				concat (line, last, line) ;
			}
			while (match(line, ttt, "  ", last)) { /* 2 white spaces */
				concat(ttt, " ", line) ; /* a white space */
				concat (line, last, line) ;
			}
		nlopx:
			if (match(line, ttt, ".", last)) {
				if ( ! ttt[0] ) { /* '.' is the first character on the line */
					assign (sizeof(line), line, last) ;
					goto cline ;
				}
			}
			if (match (line, ttt ,"'", last)) {
				if ( ! ttt[0] ) { /* "'" is the first character on the line */
					assign (sizeof(line), line, last) ;
					goto cline ;
				}
			}
		text:
			if (match(line, pline, "\\n+(", last)) {
				if ( last[0] && last[1] ) 	{
					manr[0] = last[0]; manr[1] = last[1];
					manr[2] = '\0';
					match(last, first, manr, junk) ;
					assign (sizeof(line), line, junk) ;
					goto tsetnr ;
				}
			}
			if (match(line, pline, "\\n+", last)) {
				if ( last[0] ) {
					manr[0] = last[0]; manr[1] = '\0' ;
					match(last, first, manr, junk) ;
					assign (sizeof(line), line, junk) ;
					goto tsetnr ;
				}
			}
			if (match(line, pline, "\\n(", last)) {
				if ( last [0] && last [1] ) {
					manr[0] = last[0]; manr[1] = last[1];
					manr[2] = '\0' ;
					match(last, first, manr, junk) ;
					assign (sizeof(line), line, junk) ;
					goto trefnr ;
				}
			}
			if (match(line, pline, "\\n", last)) {
				if (last [0] ) {
					manr[0] = last[0]; manr[1] = '\0' ;
					match(last, first, manr, junk) ;
					assign (sizeof(line), line, junk) ;
					goto trefnr ;
				}
			}
			if (match(line, pline, "\\*(", last)) {
				if ( last [0] && last [1] ) {
					manr[0] = last[0]; manr[1] = last[1];
					manr[2] = '\0' ;
					match(last, first, manr, junk);
					assign (sizeof(line), line, junk) ;
					goto trefstr ;
				}
			}
			if (match(line, pline, "\\*", last)) {
				if ( last [0] ) {
					manr[0] = last[0]; manr[1] = '\0' ;
					match(last, first, manr, junk);
					assign (sizeof(line), line, junk) ;
					goto trefstr ;
				}
			}
			if (match(line, pline, "\\$", last)) {
				if (last[0]  ) {
					manr[0] = last[0]; manr[1] = '\0' ;
					match(last, first, manr, junk) ;
					assign (sizeof(line), line, junk) ;
					goto trefpar ;
				}
			}
			if ( ! rqname[0] ) {
				if ( ! aline[0] ) { /* aline is null */
					initialize() ;
					continue ; /* read next line */
				}
				else {
					next13() ;
					goto text ;
				}
			}
			concat (rqname, "	", line) ; /* hor. tab */
			concat(line, name, line) ;
			concat(line, "r", line) ;
			concat(line, lcnt, line) ;
			concat(line, " ", line) ; /* white space */
			fprintf (outf, "%s\n", line) ;
			rqname[0] = '\0' ;
			if ( ! aline[0] ) { /* aline is null */
				initialize() ;
				continue ; /* read next input line */
			}
			else {
				next13() ;
				goto text ;
			}
		cline:
			if (match(line, first, " ", last)) /* white space */
				if ( ! first[0] ) assign(sizeof(line), line, last );
			cmd[0] = '\0' ;
			if ( ! line[0]  || ! line[1] ) /* size of line < 2 */
				goto cline2;
			cmd[0] = line[0]; cmd[1] = line[1]; cmd[2] = '\0';
			if  ( match(line, first, cmd, last))  
				assign (sizeof(line), line, last);
			if (cmd[1] == ' ') { /* white space */
				cmd[1] = '\0';
				goto cline1 ;
			}
			if (match(line, first, " ", last)) /* white space */
				if ( ! first[0] ) assign(sizeof(line), line, last );
			assign (sizeof(rqname), rqname, cmd) ;
			if (cmd[0] == 'n' && cmd[1] == 'r') goto setnr ;
			if (cmd[0] == 'a' && cmd[1] == 'm') goto casede ;
			if (cmd[0] == 'd' && cmd[1] == 'e') goto casede ;
			if (cmd[0] == 'r' && cmd[1] == 'm') goto caserm ;
			if (cmd[0] == 'r' && cmd[1] == 'r') goto caserr ;
			if (cmd[0] == 'd' && cmd[1] == 's') goto setstr ;
			if (cmd[0] == 'i' && cmd[1] == 'g') goto refmac ;
			if (cmd[0] == 'm' && cmd[1] == 'k') goto setnr ;
			if (cmd[0] == 'a' && cmd[1] == 's') goto setstr ;
			if (cmd[0] == 'r' && cmd[1] == 'n') goto caserr ;
			if (cmd[0] == 'd' && cmd[1] == 'i') goto setdiv ;
			if (cmd[0] == 'd' && cmd[1] == 'a') goto setdiv ;
			if (cmd[0] == 'w' && cmd[1] == 'h') goto casedt ;
			if (cmd[0] == 'c' && cmd[1] == 'h') goto setnr ;
			if (cmd[0] == 'd' && cmd[1] == 't') goto casedt ;
			if (cmd[0] == 'i' && cmd[1] == 't') goto casedt ;
			if (cmd[0] == 'e' && cmd[1] == 'm') goto refmac ;
			if (cmd[0] == 'a' && cmd[1] == 'f') goto refnr ;
			if (cmd[0] == 'i' && cmd[1] == 'f') goto caseif ;
			if (cmd[0] == 'i' && cmd[1] == 'e') goto caseif ;
			if (cmd[0] == 'e' && cmd[1] == 'l') goto caseel ;
			if (cmd[0] == 'p' && cmd[1] == 's') goto text ;
			if (cmd[0] == 's' && cmd[1] == 's') goto text ;
			if (cmd[0] == 'c' && cmd[1] == 's') goto text ;
			if (cmd[0] == 'b' && cmd[1] == 'd') goto text ;
			if (cmd[0] == 'f' && cmd[1] == 't') goto text ;
			if (cmd[0] == 'f' && cmd[1] == 'p') goto text ;
			if (cmd[0] == 'p' && cmd[1] == 'l') goto text ;
			if (cmd[0] == 'b' && cmd[1] == 'p') goto text ;
			if (cmd[0] == 'p' && cmd[1] == 'n') goto text ;
			if (cmd[0] == 'p' && cmd[1] == 'o') goto text ;
			if (cmd[0] == 'n' && cmd[1] == 'e') goto text ;
			if (cmd[0] == 'r' && cmd[1] == 't') goto text ;
			if (cmd[0] == 'b' && cmd[1] == 'r') goto text ;
			if (cmd[0] == 'f' && cmd[1] == 'i') goto text ;
			if (cmd[0] == 'n' && cmd[1] == 'f') goto text ;
			if (cmd[0] == 'a' && cmd[1] == 'd') goto text ;
			if (cmd[0] == 'n' && cmd[1] == 'a') goto text ;
			if (cmd[0] == 'c' && cmd[1] == 'e') goto text ;
			if (cmd[0] == 'v' && cmd[1] == 's') goto text ;
			if (cmd[0] == 'l' && cmd[1] == 's') goto text ;
			if (cmd[0] == 's' && cmd[1] == 'p') goto text ;
			if (cmd[0] == 's' && cmd[1] == 'v') goto text ;
			if (cmd[0] == 'o' && cmd[1] == 's') goto text ;
			if (cmd[0] == 'n' && cmd[1] == 's') goto text ;
			if (cmd[0] == 'r' && cmd[1] == 's') goto text ;
			if (cmd[0] == 'l' && cmd[1] == 'l') goto text ;
			if (cmd[0] == 'i' && cmd[1] == 'n') goto text ;
			if (cmd[0] == 't' && cmd[1] == 'i') goto text ;
			if (cmd[0] == 't' && cmd[1] == 'a') goto text ;
			if (cmd[0] == 't' && cmd[1] == 'c') goto text ;
			if (cmd[0] == 'l' && cmd[1] == 'c') goto text ;
			if (cmd[0] == 'f' && cmd[1] == 'c') goto text ;
			if (cmd[0] == 'e' && cmd[1] == 'c') goto text ;
			if (cmd[0] == 'e' && cmd[1] == 'o') goto text ;
			if (cmd[0] == 'l' && cmd[1] == 'g') goto text ;
			if (cmd[0] == 'u' && cmd[1] == 'l') goto text ;
			if (cmd[0] == 'c' && cmd[1] == 'u') goto text ;
			if (cmd[0] == 'u' && cmd[1] == 'f') goto text ;
			if (cmd[0] == 'c' && cmd[1] == 'c') goto text ;
			if (cmd[0] == 'c' && cmd[1] == '2') goto text ;
			if (cmd[0] == 't' && cmd[1] == 'r') goto text ;
			if (cmd[0] == 'n' && cmd[1] == 'h') goto text ;
			if (cmd[0] == 'h' && cmd[1] == 'y') goto text ;
			if (cmd[0] == 'h' && cmd[1] == 'c') goto text ;
			if (cmd[0] == 'h' && cmd[1] == 'w') goto text ;
			if (cmd[0] == 't' && cmd[1] == 'l') goto text ;
			if (cmd[0] == 'p' && cmd[1] == 'c') goto text ;
			if (cmd[0] == 'l' && cmd[1] == 't') goto text ;
			if (cmd[0] == 'n' && cmd[1] == 'm') goto text ;
			if (cmd[0] == 'n' && cmd[1] == 'n') goto text ;
			if (cmd[0] == 'e' && cmd[1] == 'v') goto text ;
			if (cmd[0] == 'r' && cmd[1] == 'd') goto text ;
			if (cmd[0] == 'e' && cmd[1] == 'x') goto text ;
			if (cmd[0] == 's' && cmd[1] == 'o') goto text ;
			if (cmd[0] == 'n' && cmd[1] == 'x') goto text ;
			if (cmd[0] == 'p' && cmd[1] == 'i') goto text ;
			if (cmd[0] == 'm' && cmd[1] == 'c') goto text ;
			if (cmd[0] == 't' && cmd[1] == 'm') goto text ;
			if (cmd[0] == 'p' && cmd[1] == 'm') goto text ;
			if (cmd[0] == 'f' && cmd[1] == 'l') goto text ;
		cline1:
			rqname[0] = '\0' ;
			concat("	", cmd, junk) ; /* hor. tab */
			concat(junk, "	", junk) ; /* hor. tab */
			if (match(namef, out, junk, last)) {
				concat("	", last, namef) ; /* hor tab */
				goto cline3 ;
			}
			out[0] = '\0' ;
			assign(sizeof(manr), manr, cmd) ;
			assign (sizeof(markst), markst, name) ;
			concat( markst, "m", markst) ;
			concat(markst, lcnt, markst) ;
			concat(markst, " ", markst) ; /* white space */
			goto mark2 ;
		cline2:
			if ( ! line[0] ) { /* line is null */
				if ( ! aline[0] ) { /* aline is null */
					initialize() ;
					continue ; /* read next input line */
				}
				else {
					next13() ;
					goto text ;
				}
			}
			assign ( sizeof(cmd), cmd, line) ;
			goto cline1 ;
		cline3:
			if (! match(out, first, "	", last))  /* hor. tab */
				goto cline4 ;
			if ( first [0] ) goto cline4 ; /* tab is not at position one */
			concat(first, last, out) ;
			concat(out, "	", out) ; /* hor. tab */
		cline4:
			if (match(name1, name, "	", last)) { /* hor. tab */
				if ( name [0] && name [1] ) {
					split (name, ttt, 2) ;
					concat(name, "	", name1) ; /* hor. tab */
					concat(name1, last, name1) ;
				}
			}
			if (match(name1, ttt, " 	", last)) { /* whitespace-hor. tab */
				concat(ttt, "	", name1) ; /* hor. tab */
				concat(name1, last, name1) ;
			}
			if (match("	", first, name1, last)) goto cline6; /* hor. tab */
			if (match(out, first, "	", last)) { /* hor. tab */
				assign (sizeof(out), out, last) ;
				goto cline4 ;
			}
			if (match(name1, first, " ", last)) { /* white space */
				if ( ! first [0] ) {
					if (match(last, first, "	", junk)) { /* hor. tab */
						if ( first[0] && first[1] ) {
							split(first, ttt, 2) ;
							assign (sizeof(name), name, first) ;
						}
					}
				}
			}
			if (match(ttt, first, " ", last )) /* white space */
				if ( ! first [0] ) assign( sizeof(ttt), ttt, last ) ;
			concat (name, "	", name) ; /* hor. tab */
			if (match(name, uuu, " 	", last)) /* whitespace-hor.tab */
				concat (uuu, last, name) ;
			if (match(name, uuu, "	", last)) /* hor. tab */
				concat (uuu, last, name) ;
			if (match("", first, name, last)) goto cline5 ;
			concat("(", name, name) ;
			concat(name, ")", name) ;
			concat(name, ttt, name) ;
			concat(name, "-", name) ;
			goto text ;
		cline5:
			concat (ttt, "-", name) ;
			goto text ;
		cline6:
			name[0] = '\0' ;
			goto text ;
		tsetnr:
			concat(name, "n", markst) ;
			concat(markst, lcnt, markst) ;
			concat(markst, "#", markst) ;
			goto mark2 ;
		trefnr:
			concat (name, "n", markst) ;
			concat(markst, lcnt, markst) ;
			goto mark2 ;
		trefstr:
			concat( name, "s", markst) ;
			concat(markst, lcnt, markst) ;
			goto mark2 ;
		trefpar:
			concat (name, "p", markst) ;
			concat(markst, lcnt, markst) ;
			goto mark2 ;
		setnr:
			concat(name, "n", markst) ;
			concat(markst, lcnt, markst) ;
			concat(markst, "#", markst) ;
			goto mark ;
		setmac:
			concat(name, "m", markst) ;
			concat(markst, lcnt, markst) ;
			concat(markst, "#", markst) ;
			goto mark ;
		setstr:
			concat(name, "s", markst) ;
			concat(markst, lcnt, markst) ;
			concat(markst, "#", markst) ;
			goto mark ;
		setdiv:
			concat(name, "d", markst) ;
			concat(markst, lcnt, markst) ;
			concat(markst,"#", markst) ;
			goto mark ;
		refmac:
			concat(name, "m", markst) ;
			concat(markst, lcnt, markst) ;
			goto mark ;
		refnr:
			concat(name, "n", markst) ;
			concat(markst, lcnt, markst) ;
		mark:
			if ( line [0] && line [1] ) {
				manr [0] = line [0] ;
				manr [1] = line [1] ;
				manr [2] = '\0' ;
				match ( line, first, manr, last ) ;
				concat (first, last, line ) ;
				goto mark1 ;
			}
			if (match("", first, line, last)) goto text ;
			assign (sizeof(manr), manr, line) ;
			line[0] = '\0' ;
		mark1:
			if (manr[0]  && manr[1] <= ' ')
				manr[1] = '\0' ;
			if (match(line, first, " ", last)) /* white space */
				if (! first [0] ) assign(sizeof(line), line, last ) ;
		mark2:
			pat[1] = '\0' ;
			if (! match(manr, first, pat, last)) goto mark3; /* "\" */
			if ( markst [0] ) {
				ttt[0] = markst[0];
				ttt[1] = '\0' ;
			}
			if (match(markst, first, "-", last)) {
				ttt[0] = last[0]; ttt[1] = '\0';
			}
			rrr[0] = '\0';
			if ( ! match(markst, first, "#", last)) goto mark25 ;
			rrr[0] = '#' ;
			rrr[1] = '\0' ;
		mark25:
			concat("~gen	", name, first) ; /* hor. tab */
			concat(first, ttt, first) ;
			concat(first, lcnt, first) ;
			concat(first, rrr, first) ;
			fprintf (outf, "%s\n", first) ;
			concat(manr, line, line) ;
			if (match("", first, rpt, last)) goto mark4 ;
			goto mark5 ;
		mark3:
			concat(manr, "	", first) ; /* hor. tab */
			concat(first, markst, first) ;
			fprintf (outf, "%s\n", first) ;
		mark4:
 			concat (pline, line, line) ; 
			if (match("", first, rpt, last)) goto text ;
			goto mark ;
		mark5:
			if (! match(line, ttt, " ", last)) goto mark6; /* white space */
			assign(sizeof(line), line, last) ;
			concat(aline, ttt, aline) ;
			goto mark4 ;
		mark6:
			rpt[0] = '\0';
			goto text ;
		casedt:
			if (match(line, ttt, " ", uuu)) { /* white space */
				concat(uuu, " ", line) ; /* white space */
				concat(line, ttt, line) ;
				goto refmac;
			}
			goto text ;
		caseif:
			concat("if	", name, first) ; /* hor. tab */
			concat(first, "r", first);
			concat(first, lcnt, first) ;
			concat(first, " ", first); /* white space */
			fprintf (outf, "%s\n", first) ;
			rqname[0] = '\0' ;
			if (match(line, ttt, " ", rrr))  /* white space */
				assign (sizeof(line), line, rrr) ;
			concat(aline, ttt, aline) ;
			goto nlopx;
		caserr:
			assign (sizeof(rpt), rpt, "yes") ;
			goto setnr ;
		caserm:
			assign (sizeof(rpt), rpt, "yes") ;
			goto setmac;
		casede:
			assign (sizeof(tline), tline, line) ;
			if (tline[0]  && tline[1]  ) {
				name[0] = tline[0] ;
				name[1] = tline[1] ;
				name[2] = '\0' ;
				match (tline, first, name, last) ;
				assign( sizeof(tline), tline, last) ;
				goto casede0 ;
			}
			name[0] = '\0' ;
			if (match("", first, tline, last)) goto text;
			assign (sizeof(name), name, tline) ;
			tline[0] = '\0' ;
		casede0:
			if (name[0]  && name[1] == ' ') name[1] = '\0' ; /* white space */
			if (match(name1, ttt, "	", last)) { /* hor. tab */
				concat(ttt, " ", name1) ; /* white space */
				concat(name1, name, name1) ;
				concat(name1, "	", name1) ; /* hor. tab */
				concat(name1, last, name1) ;
			}
			if ( ! match ("	", first, namef, last)) /* hor. tab */
				goto casede4 ;
		casede1:
			concat(name, "-", name) ;
			if (match(tline, first, " ", last))  /* white space */
				if ( ! first [0] ) assign (sizeof(tline), tline, last) ;
			if (tline[0]  && tline[1] ) {
				manr[0] = tline[0]; manr[1] = tline[1] ;
				manr[2] = '\0' ;
				match(tline, first, manr, last) ;
				concat(first, last, tline) ;
				goto casede2 ;
			}
			if ( match("", first, tline, last) ) goto casede3 ;
			assign(sizeof(manr), manr, tline) ;
/*			assign (sizeof(manr), manr, tline) ;*/
		casede2:
			if (manr[1] == ' ' ) manr[1] = '\0' ; /* white space */
			concat("	", manr, junk) ; /* hor. tab */
			concat(junk, namef, namef) ;
			goto setmac ;
		casede3:
			concat("	.", namef, namef) ; /* hor. tab */
			goto setmac ;
		casede4:
			concat(" ", name, junk ) ; /* white space */
			concat(junk, "	", junk) ; /* hor. tab */
			if (match(name1, first, junk, last ))
				if ( first [0] == ' ' ) /* white space */
					match (first, junk, " ", ttt ) ; /* white space */
			concat("(", ttt, ttt) ;
			concat(ttt, ")", ttt) ;
			concat(ttt, name, name) ;
			goto casede1 ;
		caseel:
			concat("el	", name, first) ; /* hor. tab */
			concat(first, "r", first) ;
			concat(first, lcnt, first) ;
			concat(first, " ", first) ; /* white space */
			fprintf (outf, "%s\n", first) ;
			rqname[0] = '\0' ;
			goto nlopx ;
		}
}
