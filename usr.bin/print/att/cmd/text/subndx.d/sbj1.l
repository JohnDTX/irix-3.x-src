%{
/*	Copyright (c) 1984 AT&T	*/
%}
%{
/*	  All Rights Reserved  	*/
%}

%{
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
%}
%{
/*	The copyright notice above does not evidence any   	*/
%}
%{
/*	actual or intended publication of such source code.	*/
%}

%{
%}

%{
#include <stdio.h>
#include "cnst.h"

#define LEOF 0
#define RMQUOTES(s,l)  if (*s == '\"')  {int i; for(i=l-1; *(s+i)=='\"' && i>0;\
  i--)  *(s+i) = ' '; *s = ' ';}

FILE	*hfile, *sfile, *cfile;
int	preced_blnk = FALSE;
char	state = 'O';
int	Eqn_f = FALSE;
char	chr;
%}
%p 2000
%e 700
%n 300
%a 3000
%o 5000
%START H P TX SB A C CMD H2
%%


<C>^[.]((TS)|(DF)|(DS)|(NS)|(RL))	{
					yyless(0); 
					 preced_blnk = FALSE;
					 BEGIN TX;
					}
^[.]EQ.*[\n]			{ skip ('E');}
^[.]TS.*[\n]			{skip ('T');}
^[.]PS.*[\n]			{skip ('P');}
^[.]G1.*[\n]			{skip ('G');}
^[.]FS.*[\n]			{skip ('F');}
^[.]D(F|S).*[\n]		{skip ('D');}
^[.]NS.*[\n]			{skip ('N');}
^[.]RL.*[\n]			{skip ('L');}
<TX,SB,P>^[.']((HU)|(H[ ][0-9]))[ \t]+  { 
					BEGIN H;}
<H>.*				{BEGIN P;
				 RMQUOTES(yytext,yyleng);
				 fprintf (hfile, "%s is.\n",yytext);
				}
<TX,SB,P>^[.'][SN]H.*[\n]	{
				BEGIN H2;}
<H2>^[.]			{
				unput(*yytext);
				BEGIN P;
				}
<H2>^.*				{
                                 RMQUOTES(yytext,yyleng);
				fprintf(hfile, "%s is.\n", yytext);
				}
<H2>\n				;
<TX,SB>^[.']P.*[\n]		{BEGIN P;}
<P,H2,TX,SB>^[.'][ILP]P.*[\n]	{BEGIN P;}
<TX,SB>^[.']A[SB].*[\n]		{BEGIN A;}

<A>^[.]AE.*[\n]                 {BEGIN SB;}
<A,P>^[.][IB].*[\n]		{
				if (yyleng > 3)
					{italic_arg();
					 fprintf (sfile, "%s", yytext);
					}
				}
<A,P>^[.]UL.*[\n]		{
				if (yyleng > 4)
					{italic_arg();
					 fprintf (sfile, "%s", yytext);
					}
				}
<P>^[.]LI.*[\n]			{
				fputs (".\n", sfile);
				 BEGIN SB;
				}
<A>^[.]				{state = 'A'; BEGIN CMD;}
<P>^[.]				{state = 'P'; BEGIN CMD;}
<A,P>[$]			{
				if (Eqn_f)
					{
					 while ((chr=input()) != LEOF AND
						 chr != '$')
					     ;
					}
				 else
					putc ('$', sfile);
				}
<A,P>[^.!?\n$]*			{fprintf (sfile, "%s", yytext);}
<P>[.!?]["]*[ \t]*[\n]		{
				BEGIN SB;
				 fprintf (sfile, "%s", yytext);
				}
<A,P>.|[\n]			{fprintf (sfile, "%s", yytext);}
<C>[IVX]			{
				 if (preced_blnk)
					{putc (' ', cfile);
					 preced_blnk = FALSE;
					}
				 fprintf (cfile, "%s", yytext);
				}
^[.]LI.*[\n]			{BEGIN SB;}
<SB>^[.']			{
				BEGIN CMD;
				 state = 'B';
				}
<SB>[ \t\n]			{;}
<SB>[A-Z][^ \t]*		{BEGIN TX;}
<TX>[A-Z][.][ \t]*[\n]		{
				BEGIN SB;
				 preced_blnk = FALSE;
				}
<C>[A-Z][.][ \t]*[\n]		{
				BEGIN SB;
				 while (*(yytext + --yyleng) != '.');
				 *(yytext + yyleng) = '\0';
				 if (preced_blnk)
					{putc (' ', cfile);
                                         preced_blnk = FALSE;
                                        }
				 fprintf (cfile, "%s", yytext);
				 putc ('\n', cfile);
				 preced_blnk = FALSE;
				}
<TX,C>[0-9]*[A-Z]+[&][0-9]*[A-Z]+	{
				BEGIN C;
				 if (preced_blnk)
					{putc (' ', cfile);
                                         preced_blnk = FALSE;
                                        }
				 fprintf (cfile, "%s", yytext);
				}
<C>[,]*[ \t\n]*((Inc)|(INC))[.]*	{
					if (preced_blnk)
					{putc (' ', cfile);
					 preced_blnk = FALSE;
					}
				fprintf (cfile, "%s", yytext);
				BEGIN TX;
				}
<TX,C>([0-9]*[A-Z][a-zA-Z0-9'/-]+)|([A-Z][.])[ \t]*	{
				BEGIN C;
				 if (preced_blnk)
					{putc (' ', cfile);
                                         preced_blnk = FALSE;
                                        }
				 fprintf (cfile, "%s", yytext);
				}
<C>((of)|[&])[ \t\n]+/[A-Z]		{
				 if (preced_blnk)
					{putc (' ', cfile);
                                         preced_blnk = FALSE;
                                        }
				 fprintf (cfile, "%s", yytext);
				}
<C>[0-9]			{
				 if (preced_blnk)
					{putc (' ', cfile);
                                         preced_blnk = FALSE;
                                        }
				 fprintf (cfile, "%s", yytext);
				}
<TX,C>((du)|(de)|(di)|(y)|(de[ ]la)|(van[ ]der)|(van)|(von))[ \t\n]  {
				BEGIN C;
				 if (preced_blnk)
					{putc (' ', cfile);
                                         preced_blnk = FALSE;
                                        }
				
				 fprintf (cfile, "%s", yytext);
				}
<C>[a-z][a-zA-Z0-9'/-]*		{
				BEGIN TX;
				 preced_blnk = FALSE;
				 putc ('\n', cfile);
				}
<C>[.!?][ \t]*[\n]		{
				BEGIN SB;
				 preced_blnk = FALSE;
                                 putc ('\n', cfile);
				}
<TX>[.!?][ \t)"\n]+		{BEGIN SB;}
<C>[,;:]			{
				BEGIN TX;
				 preced_blnk = FALSE;
                                 putc ('\n', cfile);
				}
<C>^[.']			{
				BEGIN CMD;
				 preced_blnk = FALSE;
                                 putc ('\n', cfile);
				}
<C>([ ][ ][ ]+)|[\t]+|[\n][\n][\n]*	{
				BEGIN TX;
				 preced_blnk = FALSE;
				 putc ('\n', cfile);
				}
<C>[ \n]			{preced_blnk = TRUE;}
<C>.				{
				BEGIN TX;
				 preced_blnk = FALSE;
                                 putc ('\n', cfile);
				}
^[.']				{BEGIN CMD;}
<CMD>.*[\n]			{
				 if (state == 'A') BEGIN A;
				 else if (state == 'P') BEGIN P;
				 else if (state == 'B') BEGIN SB;
				 else BEGIN TX;
				 state = 'O';
				}
<TX>[a-z][a-zA-Z0-9'/-]*	{;}
[\n]				{;}
.				{;}
%%

/*skip skips all text between matching macro calls, such as .FS to .FE*/
/*It must be called when the last character read was a newline        */

skip (type)

char	type;			/*type of macro call being skipped*/
{
	char	ch;
	char	ch2 = 'E';	/*second character of closing macro*/
	int	end = FALSE;

	if (type == 'E')
		ch2 = 'N';		/*chg second char of closing macro*/
	else if (type == 'G')
		ch2 = '2';
	while ((ch = input()) != LEOF AND end == FALSE )
		{if (ch == '.')
			if ((ch = input()) == type)		/*check for macro end*/
				if ((ch = input ()) == ch2)
					end = TRUE;
		 while (ch  != LEOF AND ch != '\n')	/*skip line*/
			ch = input ();
		}
	unput(ch);					/*return last character*/
}

/*italic_arg removes special characters and leaves the cleaned argument in*/
/*yytext. Yytext is expected to contain a                                 */
/*	.I argument							  */
/* or									  */
/*	.B argument							  */
/*command line. The first 3 macro call characters are removed, double	  */
/*quotes are removed and blanks that were not within quoted text are 	  */
/*removed.  For example, 						  */
/*	.I "a b" c d "f g"						  */
/*would return in yytext						  */
/*	a bcdf g							  */
/*The resulting text simulates the mm output in regular font.		  */
/*Note, the max. argument to .I (or .B) is expected to be less than 80	  */
/*characters.								  */

italic_arg()
{
	int	end = FALSE;
	int	openquote = FALSE;
	char	*copy;		/*pointer to char in string*/
	char	*arg;		/*pointer to char in yytext*/
	char	ch;
	char	string[80];	/*receives laundered argument*/

if (yyleng - 3 < 80)
	{copy = string;
	 arg = yytext + 3;		/*1st 3 chars are cmd chars*/
	 while (!end)
		{
		while (!openquote AND !end)
			{
			while ((ch = *arg++) != '\"' AND ch != ' ' AND ch != '\0')
				{
				*copy++ = ch;	/*copy all but blanks and d_quotes*/
				}
			if (ch == '\"')
				openquote = TRUE;
			if (ch == '\0')
				end = TRUE;
			}
		while (openquote AND !end)
			{
			while ((ch = *arg++) != '\"' AND ch != '\0')
				{
				*copy++ = ch;	/*copy all but d_quotes*/
				}
			if (ch == '\"')
				openquote = FALSE;
			if (ch == '\0')
				end = TRUE;
			}
		}
	*copy = '\0';
	strcpy (yytext, string);
	}
}



char	*filename = "-";

main (argc, argv)

int	argc;
char	*argv[];

{
	int	rc = 0;
	BEGIN TX;
	if (argc < 5)
		rc++;
	else
		{
		if (freopen (argv[1], "r", stdin) == NULL)
			{fprintf (stderr, "%s: cannot open %s\n",argv[0],argv[1]);
			 rc++;
			}
		else
			{filename = argv[1];
			 hfile = fopen (argv[2], "a");		/*headings file*/
			 sfile = fopen (argv[3], "a");		/*sentence file*/
			 cfile = fopen (argv[4], "a");		/*cap. seq. file*/
			 if (argc > 5 AND *argv[5] == 'd')
				{
					Eqn_f = TRUE;
				}
			 yylex();
			}
		}
return (rc);
}
