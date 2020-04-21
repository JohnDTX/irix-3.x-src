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

/*   The input to this module is the output of parts on a file of headings.	*/
/* The output is a list of subjects.	*/
/* Headings are parsed to yield either simple subjects, e.g., "arrays", or	*/
/* complex subjects, e.g., "arrays, storage".  Headings or components of	*/
/* headings of the form X <prep> Y are turned into complex subjects.  X and Y	*/
/* are simple subjects, sequences of modifiers followed  by a noun. <prep> is	*/
/* a preposition, colon, or double dash.  Depending on the preposition, 	*/
/* X <prep> Y will result in one of three types of subjects:	*/
/* 	1) X, Y							*/
/* 	2) Y, X							*/
/* 	3) both 1 and 2						*/
%{
#include <stdio.h>

#include "cnst.h"

#define UPDATE_PTRS   {prev = curr; curr = (curr + 1) MOD 2; next[curr] = 0;}
#define PUTSUBJ(ptr)  for (i = 0; i < next[curr]; i++) putchar (part[ptr][i])
#define ERROR1	      fputs("subject too long, is being truncated", stderr)

char	part[2][250];		/*buffer to store two parts of a subject*/
int	next[2];		/*next avail position in each half of buffer*/
int	curr = 1;		/*ptrs to current and prev halves of buffer*/
int	prev = 0;
int	terms = 0;		/*no. of subject terms found*/
int	subjno = 0;		/*current subject ccounter*/
int	type = 0;		/*type of heading*/
char	first;			/*type, adj or noun of first subject term*/
int	ulcase ();		/*changes any uppercase letter in string to lower*/
%}
%START S W P UNKN
%%
noun[\t][ ]*			{first = NOUN;
				 terms++;
				 BEGIN W;
				}
adj[\t][ ]*			{first = ADJ;
				 terms++;
				 BEGIN W;
				}
<S>(noun|adj)[\t][ ]*		{terms++;
				 BEGIN W;
				}
<W>.+				{ulcase (yytext);
				 if (next[curr] + yyleng >= MAXCHS)
					{writesubj(type);
					 reset_ptrs();
					 ERROR1;
					}
				 if (terms > 1 &&  next[curr] > 0)
					{part[curr][next[curr]] = ' ';
					 next[curr] = next[curr] + 1;
					}
				 strcpy(&part[curr][next[curr]], yytext);
				 next[curr] = next[curr] + yyleng ;
				 BEGIN S;
				}
^[.][\t].*			{if (terms >1 OR first == NOUN)
					writesubj (type);
				 reset_ptrs();
				 BEGIN 0;
				}
<S>prep[\t]			{BEGIN P;}
<S>:[\t]			{BEGIN P;}
<S>(--[\t])|(-[\t])		{BEGIN P;}
<S>subcj[\t]			{BEGIN P;}
<P>to|of			{if (type > 0)
					{writesubj(type);
					 reset_ptrs();
					}
				 else
					UPDATE_PTRS;
				 type = 2;
				 BEGIN S;
				}
<P>in|for			{if (type > 0)
					{writesubj(type);
					 reset_ptrs();
					}
				 else
					UPDATE_PTRS;
				 type = 3;
				 BEGIN S;
				}
<P>(with)|([:])|(--)|(-)	{if (type > 0)
					{writesubj(type);
					 reset_ptrs();
					}
				 else
					UPDATE_PTRS;
				 type = 1;
				 BEGIN S;
				}
<P>.*				{if (type > 0)
					{writesubj(type);
					 reset_ptrs();
					}
				 else
					UPDATE_PTRS;
				 type = 1;
				 BEGIN S;
				}
<S>art.+                        {;}
<S>conj.*			{writesubj(type);
				 reset_ptrs();
				 BEGIN 0;
				}
<S>.				{BEGIN UNKN;}
<UNKN>.*				{BEGIN S;}
[\n]				{;}
.				{;}
%%
writesubj (type)

int type;
{
	int	i;

	if (subjno != 0)
		putchar('\n');
	subjno++;
	/*printf("%d	", subjno);*/
	if (type == 0)
		{for (i = 0; i < next[curr]; i++)
			putchar(part[curr][i]);}
	if (type == 1)
		{for (i = 0; i < next[prev]; i++)
			putchar(part[prev][i]);
		 fputs(", ", stdout);
		 for (i = 0; i < next[curr]; i++)
			putchar(part[curr][i]);
		}
	if (type == 2)
		{for (i = 0; i < next[curr]; i++)
			putchar(part[curr][i]);
		 fputs(", ", stdout);
		for (i = 0; i < next[prev]; i++)
			putchar(part[prev][i]);
		}
	if (type == 3)
		{for (i = 0; i < next[prev]; i++)
			putchar(part[prev][i]);
		 fputs(", ", stdout);
		 for (i = 0; i < next[curr]; i++)
			putchar(part[curr][i]);
		 subjno++;
		 putchar ('\n');
		 for (i = 0; i < next[curr]; i++)
		 	putchar(part[curr][i]);
		 fputs(", ", stdout);
		 for (i = 0; i < next[prev]; i++)
 			putchar(part[prev][i]);
		}
}


reset_ptrs ()
{
	terms = 0;
	prev = curr;
	curr = (curr +1) MOD 2;
	next[curr] = 0;
	type = 0;
}
