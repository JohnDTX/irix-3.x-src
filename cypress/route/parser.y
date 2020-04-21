
%{

#include "defs.h"
#include <strings.h>

char sbTemp[90];
char sbTemp2[90];
char sbMsg[90];
char *MakeString();
char *GetNetNumber();
extern struct Line LineTable[];
extern struct Connect ConnectTable[];
extern int cLineCurr;
extern int cConnectCurr;

/*
 * yacc.in - the definition that is input to yacc.  This routine parses
 *           the input files and builds the phone line, cypress connection
 *           and network reachability tables. 
 *
 * Programmed by:  Greg Smith
 * 
 * date -- 6-21-1985
 *
 */

%}

%union {
   int w;
   char *sb;
   float fl;
   struct Links *pLinks;
   }


%token DASH LPAREN NUMBER PROTOCOL RPAREN STRING SLASH

%type <sb> NUMBER
%type <sb> STRING
%type <sb> PROTOCOL
%type <sb> cypressname
%type <sb> linename
%type <pLinks> connections
%type <pLinks> link
%type <pLinks> links
%%

tableentry   : phone
	     | connect
	     ;

phone        : phoneentry
	     | phone phoneentry
	     ;

phoneentry   : linename NUMBER PROTOCOL who circuit
                   {
			/* enter the name of the leased line    */
			/* and the line speed into the phone    */
			/* line table			        */
			LineTable[cLineCurr].sbLineName = $1;
			LineTable[cLineCurr].speed = atoi($2);

			/* enter protocol into the line table */
			LineTable[cLineCurr].sbProtocol = $3;

			/* increment the count of table entries */
			/* so we're pointing to the next entry  */
                        cLineCurr += 1;
                   }

             | error NUMBER PROTOCOL who circuit
		   {
			yyerror("invalid line name");
		   }

             | linename error PROTOCOL who circuit
		   {
			yyerror("invalid line speed");
		   }

   	     | linename NUMBER error who circuit
		   { 
			yyerror("invalid protocol");
		   }

	     | linename NUMBER PROTOCOL error circuit
		   { 
			yyerror("invalid entry in bill to field");
		   }

	     | linename NUMBER PROTOCOL who error
		   {
			yyerror("invalid circuit number");
		   }
             ;

linename     : cypressname SLASH cypressname 
                   {
                        /* concatenate the fields together and enter */
                        /* them into the line table                  */
			sbTemp[0] = 0;
			strcat(sbTemp, $1);
			strcat(sbTemp, "/");
			strcat(sbTemp, $3);
			$$ = MakeString(sbTemp);
		   }

             | error SLASH cypressname
                   {
		      yyerror("invalid cypress name in first part of linename");
		   }

             | cypressname SLASH error
                   {
		      yyerror("invalid cypress name in second part of linename");
		   }
	     ;

who          : STRING
                   {
			/* enter into the line table */
			LineTable[cLineCurr].sbBillTo = $1;	
		   }
	     ;

circuit      : NUMBER STRING NUMBER
		   {
                        /* concatenate entries together and */
			/* enter into line table 	    */
			sbTemp[0] = 0;
			strcat(sbTemp, $1);
			strcat(sbTemp, $2);
			strcat(sbTemp, $3);
			LineTable[cLineCurr].sbCircuitNum = MakeString(sbTemp);
			sbTemp[0] = 0;
		   }
	     ;

connect      : connectentry
	     | connect connectentry
	     ;

connectentry : cypressname connections
		   {
			ConnectTable[cConnectCurr].sbNodeName = $1; 
 			ConnectTable[cConnectCurr].sbNetNumber = GetNetNumber($1); 
			ConnectTable[cConnectCurr].pConnections = $2;

			/* increment the count of entries in */
			/* the cypress connection table      */
			cConnectCurr++;
		   }

             | error connections
                   {
		      yyerror("invalid cypress name");
		   }
             ;


connections  : links
		   {
			$$ = $1;
		   }
	     ;

links        : link
		   {
 			$$ = $1;
		   }

	     | links link
		   {
			/* link the new node to the front of */
			/* the list			     */
			$2->pConnections = $1;
			$$ = $2;
		   }
             ;

link         : LPAREN linename NUMBER RPAREN
		   {
 			/* this connection either describes a line - */
			/* interface pair or a local net - # of hops */
			/* pair, allocate a structure to hold either */
			pLinks = New(struct Links);			
			pLinks->sb = MakeString($2);
			sbTemp[0] = 0;
			pLinks->w = atoi($3);
 			pLinks->pConnections = NULL;
			$$ = pLinks;
  		   }
	     ;

cypressname  : STRING
		   {
			/* concatenate the fields together to */
			/* create the node name 	      */ 
			$$ = $1;
                   }

             | STRING DASH STRING
                   {
		        sbTemp[0] = 0;
			strcat(sbTemp, $1);
			strcat(sbTemp, "-");
			strcat(sbTemp, $3);
			$$ = MakeString(sbTemp);
		     }
             ;

