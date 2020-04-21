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

/*
%{
%}
*/

	/* Lex source for program chek.  Flags questionalbe uses & errors 
	 * in usage of PWB/mm macros and some tbl and eqn constructs. */


#include <stdio.h>
%a 3000
%e 1500
%p 4000
%k 2000
%o 4000
%n 1000
C [.']
SP [ ]
string (["]([^"\\\n]*[\\]*[^"\\\n]*[\\"]*)*[^"\\\n]*["])
num ([ 	]+[0-9]+)
end (([ 	]*)|([ 	]+[']?[\\]["].*))$
printables ([!"#$%&'()*+,\-./;:<=>?\\100\[\\\]^_`{}~]|[0-9A-Za-z])
	int  sg ;   /* flag for  SG macros */


	extern int errors;
	int ndx ;   /* for begin  macros */
	int begin_macros ; /* flag is set whwn a begin-macro is encountered */
	int null, text;/* flags to hold lineno of the last null or text line found*/
	int lno_bmac [12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } ;
	char *begin_mac[] = {
		"ND",
		"TL",
		"AF",
		"AU",
		"AT",
		"TM",
		"AS",
		"AE",
		"NS",
		"NE",
		"OK",
		"MT"
	};

	int lno_lmac [10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ;
	char *lettr_mac[] = {
		"ND",
		"LO",
		"WA",
		"WE",
		"IA",
		"IE",
		"LT",
		"SG",
		"NS",
		"NE"	} ;
	int letter, memo ; /* flag to indicate type of document */

%%	/*	lex rules start	*/
	sg = 0 ;
	null = text = begin_macros = letter = memo = 0 ;
^[ 	]*\n	if(!begin_macros)	null = yylineno -1 ;
^[^.'\n].*\n	if(!begin_macros)	text = yylineno - 1 ;
^{C}ND.*{end}	{
		ndx = 0;
		set_matrix(ndx, argument(1,1)) ;
		set_letter(ndx, 0) ;
	}

^{C}ND[ 	]*	{
		ndx = 0 ;
		set_matrix(ndx, 1) ;
		set_letter(ndx, 1) ;
	}
^{C}LO[ 	]+(CN|RN|AT|SA|SJ)([ 	]+{string})?{end}	{
		ndx = 1 ;
		set_letter(ndx, 0) ;

		}
^{C}LO[ 	]*	{
		ndx = 1 ;
		set_letter(ndx, 1) ;
		}
^{C}WA([ 	]+{string})([ 	]+{string})?{end}	{
		ndx = 2 ;
		set_letter(ndx, 0) ;
		}
^{C}WA[ 	]*	{
		ndx = 2 ;
		set_letter(ndx, 1) ;
		}
^{C}WE{end}	{
		ndx = 3 ;
		set_letter(ndx, 0) ;
		}
^{C}WE[ 	]*	{
		ndx = 3 ;
		set_letter(ndx, 1) ;
		}
^{C}I(A|E){end}	{
		if (yytext[2] == 'A')  ndx = 4;
		else			ndx = 5;
		set_letter(ndx, 0) ;
		}
^{C}I(A|E)[ 	]*	{
		if (yytext[2] == 'A')  ndx = 4;
		else			ndx = 5 ;
		set_letter(ndx, 1) ;
		}
^{C}LT([ 	]+(BL|SB|FB|SP))?{end}	{
		ndx = 6 ;
		set_letter(ndx, 0) ;
		}
^{C}LT[ 	]*	{
		ndx = 6 ;
		set_letter(ndx, 1) ;
		}
^{C}SG([ 	]+([A-Za-z]+|\"\"))?([ 	]+1)?{end}	{
		if (memo)	{
			if (sg) extra("SG", yylineno) ;
			else sg = 1 ;
			if (lno_bmac[3] == 0) missing("AU", "SG", yylineno) ;
		}
		else {
			ndx = 7 ;
			set_letter(ndx, 0) ;
			sg = 1 ;
		}
	}
^{C}SG[ 	]*	{
		if (memo) syntax("SG", yylineno) ;
		else	{
			ndx = 7 ;
			set_letter(ndx, 1) ;
			}
		}

^{C}.	{	if (letter&& !lno_lmac[6]) preceed("only ND, WA/WE or IA/IE may ", " LT", yylineno);
		REJECT;
	}
^{C}TL.*{end}	{
		ndx = 1 ;
		set_matrix(ndx, argument(2, 0)) ;
	}

^{C}TL[ 	]*	{
		ndx = 1 ;
		set_matrix(ndx, 1) ;
		}
^{C}AF.*{end}	{
		ndx = 2 ;
		set_matrix(ndx, argument(1,0)) ;
	}

^{C}AF[ 	]*	{
		ndx = 2 ;
		set_matrix(ndx, 1) ;
		}
^{C}AU.*{end}	{
		ndx = 3 ;
		set_matrix(ndx, argument(8,1)) ;
	}

^{C}AU[ 	]*	{
		ndx = 3 ;
		set_matrix(ndx, 1) ;
	}
^{C}AT.*{end}	{
		ndx = 4 ;
		set_matrix(ndx, argument(9,0)) ;
	}
^{C}AT[ 	]*	{
		ndx = 4 ;
		set_matrix(ndx, 1) ;
	}
^{C}TM.*{end}	{
		ndx = 5 ;
		set_matrix(ndx, argument(9,0)) ;
	}

^{C}TM[ 	]*	{
		ndx = 5 ;
		set_matrix(ndx, 1);
	}
^{C}AS([ 	]+[0-2])?{num}?{end}	{
		ndx = 6 ;
		set_matrix(ndx, 0) ;
	}
^{C}AS[ 	]*	{
		ndx = 6 ;
		set_matrix(ndx, 1) ;
	}

^{C}AE{end}	{
		ndx = 7 ;
		set_matrix(ndx, 0) ;
		if (!lno_bmac[6]) missing("AS", "AE", yylineno ) ;
	}
^{C}AE[ 	]*	{
		ndx = 7 ;
		set_matrix(ndx, 1) ;
		if(! lno_bmac[6])	missing("AS", "AE", yylineno) ;
	}

^{C}NS([ 	]+([0-9]|1[0-3]|{string}))?([ 	]+1)?{end}	{
		ndx = 8 ;
		if (memo) set_matrix(ndx, 0) ;
		else  if (letter) set_letter(ndx, 0) ;
			else	{
				set_matrix(ndx, 0) ;
				if (lno_lmac[8] == 0) lno_lmac[8] = yylineno ;
			}
		}

^{C}NE{end} 	{
		ndx = 9 ;
		if (memo) set_matrix(ndx, 0) ;
		else if (letter) set_letter(ndx, 0) ;
			else	{
				set_matrix(ndx, 0) ;
				if(lno_lmac[9] == 0) lno_lmac[9] = yylineno ;
				}
	}

^{C}N(E|S)[ 	]* 	{
		if (yytext[2] == 'S')   ndx = 8;
		else			ndx = 9;
		if (memo) set_matrix(ndx, 1) ;
		else if (letter) set_letter(ndx, 1) ;
			else	{
				set_matrix(ndx, 1) ;
				if(lno_lmac[ndx] ==0) lno_lmac[ndx] = yylineno ;
				}
	}
^{C}OK.*{end}	{
		ndx = 10 ;
		set_matrix(ndx, argument(9,0)) ;
	}
^{C}OK[ 	]*	{
		ndx = 10 ;
		set_matrix(ndx, 1) ;
	}

^{C}MT[ 	]+([0-3]|5|{string})?([ 	]+{string})?{end}	{
		ndx = 11 ;
		set_matrix(ndx, 0) ;
	}
^{C}MT[ 	]+4([ 	]+1)?{end}	{
		ndx = 11 ;
		set_matrix(ndx, 0) ;
	}
^{C}MT[ 	]*	{
		ndx = 11 ;
		set_matrix(ndx, 1) ;
	}




^{C}FC([ 	]+{string})?{end}	if (sg)	preceed("SG ", "s FC", yylineno) ;
^{C}FC[ 	]*	{
		syntax("FC", yylineno) ;
		if (sg) preceed("SG ", "s FC", yylineno) ;
	}
^{C}AV[ 	]+{string}([ 	]+1)?{end}	if(letter) ignore("AV", "letter") ;
^{C}AV[ 	]*	{
		syntax("AV", yylineno) ;
		if(letter) ignore("AV", "letter") ;
	}

^{C}CS{num}?{num}?{num}?{num}?{num}?{num}?{end}	if (letter)
	ignore("CS", "letter") ;

^{C}CS[ 	]*	{
		syntax("CS", yylineno) ;
		if (letter) ignore("CS", "letter") ;
	}
^{C}(1|2)C{end}		if (yytext[2] == '2' && letter) ignore("2C", "letter") ;
^{C}(1|2)C[ 	]*	{
	if (yytext[2] == '2')	
		if (letter) ignore("2C", "letter") ;
	else syntax("1C or 2C", yylineno) ;
	}
.	;
\n	;
%%	/*	lex rules end	*/
