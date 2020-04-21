/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


extern int errors;

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
	printf("Line %5u: missing %s before %s\n", lineno,  mac1, mac2) ;
	errors = 1 ;
}
lists(lineno)
int lineno;
{
	extern int il;
	extern char listm[20][20];
	extern char yytext[];
	extern int yyleng;
	int jl, max;
	il++ ;

	if(il > 5 )	{
		printf("Line %5u: lists nested %d deep\n", lineno, il+1) ;
		errors = 1;
	}

	max = (yyleng < 20 ? yyleng : 19 ) ;
	for( jl = 0 ; jl < max ; jl++ )
	listm[il][jl] = yytext[jl] ;
	listm[il][jl] = '\0' ;
}
extra(macro, lineno)
int lineno;
char *macro;

{
	errors = 1;
	printf("Line %5u: extra %s\n", lineno,  macro);
}

syntax(macro, lineno)
int lineno;
char *macro;

{
	errors = 1;
	printf( "Line %5u: syntax error in %s\n", lineno, macro) ;
}
extern int i;
int j;
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
extern int ds, bs, eq, ldelset, rdel, rs, fs, ts, tsh, yylineno; 

	if(ds) {
	{ printf("Unfinished DS\n") ; errors = 1; } }

	if(bs)
	{ printf("Unfinished BS\n") ; errors = 1; }

	if (eq > 0) {
	 printf("Unfinished EQ\n") ; errors = 1;  }

	if (ldelset) {
	 printf("Missing right delimiter %c\n", rdel); errors = 1;  }

	if(fs) {
	 printf("Unfinished FS\n") ; errors = 1;  }

	if(rs) {
	 printf("Unfinished RS\n") ; errors = 1;  }

	if(ts) {
	 printf("Unfinished TS\n") ; errors = 1;  }


	if(tsh ) {
	 printf("Unfinished TS H\n" ) ; errors = 1;  }
	while ( il > -1 ) {
	 printf("Unfinished \'%s\'\n", listm[il--]) ; errors = 1;  }
	printf("~~~~~   %d lines done  ~~~~~\n\n",yylineno- 1);
	return(1) ;
}
