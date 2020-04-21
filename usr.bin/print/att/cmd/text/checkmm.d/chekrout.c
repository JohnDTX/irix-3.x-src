/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#include <stdio.h>
extern FILE *fout;
extern int errors;
extern int lno_bmac[12] ;
extern char *begin_mac[12];
extern char *lettr_mac[10] ;
extern int lno_lmac[10], memo, letter ;
extern int yylineno, yyleng;
extern char yytext[];
set_matrix(ndx, error)
int ndx, error ;
{
	static char *prevmac = { "  " };
	int i ;
 	extern int begin_macros, null, text;
	errors = error ;
	if ( ndx == 99 )	{	/* resets prevmac to "  " for the */
		prevmac = "  " ;	/* next input file to be processed */
		for (i =0; i <12; i++ ) lno_bmac[i] = 0 ;
		for (i =0; i< 10; i++)  lno_lmac[i] = 0 ;
		return(0) ;
	}
	begin_macros = 1;
	if (null)	{
		preceed("null or blank line(s) ", " MM macros",  null) ;
		null = 0 ;
	}
	if (text)	{
		preceed("text line(s) ", " MM macros",  text ) ;
		text = 0 ;
	}
	if (ndx >0 && ndx < 8 || ndx > 9 && ndx < 12 && !letter) memo = 1 ; /* the document is a memo */
	if (letter)
		if (ndx > 0 && ndx < 8 || ndx >9 )	{
			ignore(begin_mac[ndx], "letter");
			return(1);
		}
	if (error == 1)
		fprintf(fout, "Line %5u: syntax error in %s\n", yylineno,  begin_mac[ndx]) ;
	if (lno_bmac[ndx] == 0) lno_bmac[ndx] = yylineno;

	if (ndx != 8 && ndx != 9 && lno_bmac[ndx] != yylineno) /* not AU/AT and multiple occurence */
	{
		fprintf(fout, "Line %5u: extra %s; first referenced at line %d\n",yylineno, begin_mac[ndx], lno_bmac[ndx]);
		errors = 1 ;
	}

	if (ndx < 8 || ndx > 9 )	{
		for (i = ndx + 1; i < 12; i++)	{
			if (lno_bmac[i] > 0 && lno_bmac[i] < yylineno)	{
				fprintf(fout, "Line %5u: %s is after %s at line %d\n", yylineno,  begin_mac[ndx], begin_mac[i], lno_bmac[i]);
				errors = 1;
				break ;
			}
		}
	}
	if (ndx == 4 && prevmac != begin_mac[ndx -1])
			fprintf(fout, "Line %5u: AT  must follow AU\n", yylineno) ;
	prevmac = begin_mac[ndx] ;
}

set_letter(ndx, error)
int ndx, error ;
{
	int i ;
	static int lo, lo_cn, lo_rn, lo_at, lo_sa, lo_sj;
	lo = lo_cn = lo_rn = lo_at = lo_sa = lo_sj = 0;
	errors = error ;
	if(ndx > 0 && ndx < 7 && !memo) letter = 1; /* the document is a letter */
	if(memo)
		if(ndx > 0 && ndx < 7) {
			ignore(lettr_mac[ndx], "memorandum") ;
			return(1);
		}
	if (error == 1)
		fprintf(fout, "Line %5u: syntax error in %s\n", yylineno, lettr_mac[ndx]) ;
	if (lno_lmac[ndx] == 0) lno_lmac[ndx] = yylineno;

	if ( ndx != 1 && lno_lmac[ndx] != yylineno) /*not LO and multiple occurence*/
	{
		fprintf(fout, "Line %5u: extra %s; first referenced at line %d\n", yylineno, lettr_mac[ndx], lno_lmac[ndx]);
		errors = 1 ;
	}
	if (ndx == 3 || ndx == 5)	{	/*WE or IE */
		if (lno_lmac[ndx-1] == 0)	{
			fprintf(fout, "Line %5u: unmatched %s\n", yylineno, lettr_mac[ndx]);
			errors = 1;
		}
	}
	if (ndx == 1)	{ /* LO */
		if (lno_lmac[6] && yylineno > lno_lmac[6] )
			preceed("LO must ", " LT", yylineno);
		if (yylineno > lno_lmac[2] && yylineno < lno_lmac[3] || yylineno >lno_lmac[4] && yylineno < lno_lmac[5])
			fprintf(fout,"Line %5u: LO must not be between WA\/WE or IA\/IE\n", yylineno);
		lo++;
		if (lo > 5) fprintf(fout, "Line %5u: more than 5 LOs\n", yylineno) ;
		i = 4;
		while (yytext[i] == ' ' || yytext[i] == '	' && i <= yyleng)i++;
		if (yytext[i] =='C')	{
			if (lo_cn) extra ("LO CN", yylineno);
			else lo_cn = 1;
		}
		if (yytext[i] == 'R')	{
			if (lo_rn) extra ("LO RN", yylineno);
			else lo_rn = 1;
		}
		if (yytext[i] == 'A')	{
			if (lo_at) extra ("LO AT", yylineno);
			else lo_at = 1;
		}
		if (yytext[i+1] == 'A')	{
			if (lo_sa) extra ("LO SA", yylineno);
			else lo_sa = 1;
		}
		if (yytext[i + 1] == 'J')	{
			if (lo_sj)	extra("LO SJ", yylineno);
			else lo_sj = 1;
		}
	}
	else	{
		for (i = ndx + 1; i < 10; i++)	{
			if (lno_bmac[i] > 0 && lno_bmac[i] < yylineno)	{
				fprintf(fout, "Line %5u: %s is after %s at line %d\n", yylineno,  lettr_mac[ndx], lettr_mac[i], lno_lmac[i]);
				errors = 1;
				break ;
			}
	}
		}
}
within(mac1, mac2, lineno) 
int lineno ;
char *mac1, *mac2 ;
{
	errors = 1 ;
	printf ("Line %5u: %s within %s\n", lineno, mac1, mac2 ) ;
}

missing(mac1, mac2, lineno)
int lineno;
char *mac1, *mac2;
{
	fprintf(fout, "Line %5u: missing %s before %s\n", lineno,  mac1, mac2 ) ;
	errors = 1 ;
}
ignore(string1, string2)
char *string1, *string2 ;
{
	fprintf(fout, "Line %5u: %s will be ignored in a %s\n", yylineno,  string1, string2);
}
extra(macro, lineno)
int lineno;
char *macro;

{
	errors = 1;
	fprintf(fout, "Line %5u: extra %s\n", macro, lineno);
}

syntax(macro, lineno)
int lineno;
char *macro;

{
	errors = 1;
	fprintf(fout,  "Line %5u: syntax error in %s\n", lineno,  macro) ;
}
preceed(str1, str2, lineno)
char *str1, *str2;
int lineno;
{
	fprintf(fout, "Line %5u: %spreceed%s\n", lineno, str1, str2) ;
	errors = 1;
}
int i, j;
argument(numb, musthave)
int numb, musthave;
{
	i = 3; j = 0;
	while(i < yyleng)	{
		if(yytext[i] != ' ' && yytext[i]!= '	') return(1);
		while(i < yyleng && yytext[i] == ' ' || yytext[i] == '	') i++;
		if (i >= yyleng) break;
		if (yytext[i] == '"')	{
			if ( string() ) return (1) ;
			else continue;
		}
		while (i < yyleng && yytext[i] != ' ' && yytext[i] != '	') i++;
		j++;
	}
	if (j >numb) return (1);
	if ( j < musthave) return (1);
	return (0) ;
}
extern int i, j;
string()
{
	i++;
	if (i >= yyleng) return (1);
	while(i < yyleng)	{
		while ( i < yyleng && yytext[i] != '\\' && yytext[i] != '"') i++;
		if (i >= yyleng) return(1);
		if (yytext[i] == '\\')	{
			i++;
			if (i >=yyleng) return(1);
			if (yytext[i] == '"')	{
				i++;
				continue;
			}
		}
		break;
	}
	j++;
	i++;
	return(0);
}
yywrap(){

	if(lno_bmac[8] && !lno_bmac[9] || lno_lmac[8] && !lno_lmac[9]) {
	 fprintf(fout, "Unfinished NS\n") ; errors = 1;  }

	if (memo)	{
		if(lno_bmac[6] && !lno_bmac[7] ) {
	 	fprintf(fout, "Unfinished AS\n") ; errors = 1;  }

		if (!lno_lmac[11])	{

			fprintf(fout, "Missing MT ! \n") ;
			errors =1 ;
		}
		set_matrix(99, 0 ) ; /* reset prevmac to blanks */
	}
	if (letter)	{
		if (!lno_lmac[6])	{
			fprintf(fout, "Missing LT ! \n");
			errors = 1;
		}
		if(lno_lmac[2] && !lno_lmac[3])	{
			fprintf(fout, "Unfinished WA \n") ; errors = 1 ; }
		if(lno_lmac[4] && ! lno_lmac[5])	{
			fprintf(fout, "Unfinished IA \n"); errors = 1; }
	}
	return(1) ;
}
