
%{
#include "tn.h"
#undef input
#undef unput

extern int input();
extern int unput();
extern int cLineNum;
extern int cError;
char *MakeString();

/*
 * scanner.l - defines tokens that are input to the parser.
 *
 * Programmed by:  Greg Smith
 *
 */

%}

%%

"-"                        return(DASH);

"("			   return(LPAREN);

")"			   return(RPAREN);

"/"			   return(SLASH);

CYPRESS			   {
				yylval.sb = MakeString(yytext);
				return(PROTOCOL);
			   }

[a-zA-Z][a-zA-Z0-9.]*      {
                                yylval.sb = MakeString(yytext);
                                return(STRING);
                           }

[0-9]+                     {
                                yylval.sb = MakeString(yytext);
                                return(NUMBER);
                           }

[\n]                       cLineNum += 1;

[ \t]+                     ;

"#".*\n                    cLineNum += 1;

.                          { 
                               /* if you get here the character is unknown */
                               yyerror("unknown character ");
                               fprintf(stderr, "\\%o\n", yytext[0]);
                           }

