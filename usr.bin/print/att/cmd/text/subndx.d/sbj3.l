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
/*The input to this module is the output of parts on a file of sentences.
  The output is a list of subjects.
  Subjects are defined as adjective-noun sequences.*/

#include <stdio.h>
#include "cnst.h"

#define ERROR1	      fputs("subject too long, is being truncated", stderr)

char	subj[MAXCHS];		/*buffer to store  a subject*/
int	next = 0;		/*next avail position in the buffer*/
int	terms = 0;		/*no. of subject terms found*/
int	subjno = 0;		/*current subject counter*/
char	prev;			/*type, adj or noun of prev subject term*/
%}
%START SK W
%%
noun[\t][ ]*			{prev = NOUN;
				 terms++;
				 BEGIN W;
				}
adj[\t][ ]*			{prev = ADJ;
				 terms++;
				 BEGIN W;
				}
<W>[0-9.$%,-]+/[\n]		{if (terms == 1)
					terms--;
				 else
					storeterm ();
				 BEGIN 0;
				}
<W>.+				{storeterm ();
				 BEGIN 0;
				}
^.+[\t]				{if (terms > 0 AND prev == NOUN)
					writesubj ();
				 reset_ptrs();
				 BEGIN SK;
				}
<SK>.*				{BEGIN 0;}
[\n]				{;}
.				{;}
%%
/*unless the current word is found on the list of words to be omitted from
subjects, storeterm stores the word in the subject buffer*/

storeterm()
{	if (omit(yytext) == FALSE)
		{if (next + yyleng >= MAXCHS)
			{writesubj();
			 reset_ptrs();
			 ERROR1;
			}
		 if (terms > 1)
		 	{subj[next] = ' ';
		 	next++;
			}
		 strcpy(&subj[next], yytext);
		 next = next + yyleng ;
		}
	else
		{terms--;			/*else decrement count*/
		 prev = UNK;			/*previous is now unknown*/
		}
}


writesubj ()

{
	int	i;

	subjno++;
	/*printf("%d	", subjno);*/
	{for (i = 0; i < next; i++)
		putchar(subj[i]);}
	putchar ('\n');
}


reset_ptrs ()
{
	terms = 0;
	next = 0;
}
