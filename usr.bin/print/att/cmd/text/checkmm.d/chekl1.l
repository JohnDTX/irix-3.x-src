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


%a 6000
%e 3000
%p 6000
%k 3000
%o 10000
%n 1000
C [.']
SP [ ]
string (["]([^"\\\n]*[\\]*[^"\\\n]*[\\"]*)*[^"\\\n]*["])
num ([ 	]+[0-9]+)
end (([ 	]*)|([ 	]+[']?[\\]["].*))$
hmargs ([ 	]+(0001|[AaIi1]))
printables ([!"#$%&'()*+,\-./;:<=>?\\100\[\\\]^_`{}~]|[0-9A-Za-z])
	extern int errors;
	int i, max, jl, il;   /* for list macros */
	char listm[20][20] ;  /* storage for list macro args */
	int ds, fs, ts, tsh, eq, sg, as ; /* use ds, as, ... as indicators of .DS, .AS, ... */
	int delset, ldelset, notwarned; /* used for EQN checks */
	char ldel, rdel;      /* used for EQN checks */
	int tl, ok, bs, as, ns;
	int tm, nd, ofl, rs ;



%%	/*	lex rules start	*/
	as = ns = ds = fs = ts = tsh = eq = sg = as = tl = ok = 0 ;
	rs = tm = nd  = ofl = bs = 0 ;
	il = -1 ; /* list index, indexes listm 0-19 */
	delset = ldelset = notwarned = 0;

^{C}..	{
		if (tl) if (yytext[1] != 'F' && yytext[2] != 'S') tl = 0;
		REJECT;
	}
^{C}AS as = 1;
^{C}AE	as = 0;
^{C}NS	ns = 1;
^{C}NE	ns = 0;
^{C}TL	tl = 1;
^{C}RS.*{end}	{
		if (argument(1,0)) syntax("RS", yylineno);
		if (rs) within ("RS", "RS", yylineno ) ;
		rs = 1 ;
	}
^{C}RE{end} if (rs) {
		printf("Line %5u: expecting RF\n", yylineno); errors = 1 ; 
 	}
^{C}RF{end} {
	if(rs == 0) extra ("RF", yylineno) ;
	rs = 0 ;
	}

^{C}RP([ 	]+[0-1])?([ 	]+[0-3])?{end}	;

^{C}R(E|F|P)[ 	]*	{
	syntax(" RF or RP", yylineno) ;
	}

^{C}LI([ 	]+{printables}+)?([ 	]+1)?{end}	if (il == -1 )
		missing("list begin", "LI", yylineno ) ;
^{C}(BL|DL|RL){num}?([ 	]+1)?{end}	lists(yylineno) ;

^{C}AL([ 	]+[1AaIi])?{num}?([ 	]+1)?{end}	lists(yylineno) ;

^{C}ML([ 	]+{printables}+)?{num}?{end}	lists(yylineno) ;

^{C}VL{num}{num}?([ 	]+1)?{end}	lists(yylineno) ;
^{C}LB{num}{num}{num}{num}([ 	]+{printables}+)?{num}?{num}?{end}	lists(yylineno) ;

^{C}LE([ 	]+1)?{end}	 if(--il < -1 )  {
	extra("LE", yylineno) ;
	il = -1 ;
	}
^{C}(AL|BL|DL|ML|RL|VL|LB)[ 	]*	{
		syntax("list-initialization macro", yylineno) ;
		lists(yylineno) ;
		}


^{C}LC([ 	]+[0-5]){end}	{
	for( i = 4 ; yytext[i] < '0' || yytext[i] > '6' ; i++) ;
	i = yytext[i] - '0' ;

	if(i>il)	{
	printf("Line %5u: LC %d with %d lists active\n", yylineno, i,  il + 1)  ;
	errors = 1 ;
	}
	else
	il = i - 1 ;
	}

^{C}LC[ 	]*	syntax("LC", yylineno) ;

^{C}EQ([ 	]+([^ 	"\n])+)?{end} {if (eq == 1)
	 extra("EQ", yylineno) ;	
	else eq = 1;
	if (ldelset)
	{ printf("Line %5u: EQ in %c%c, line %d\n", yylineno,  ldel, rdel) ; errors = 1; }
	if (!ds)	within ("EQ", "DS/DE", yylineno ) ;
	}

^{C}EQ[ 	]*	syntax("EQ", yylineno) ;
^{C}EN{end} {if (eq<1)
	 extra("EN", yylineno) ;	
	else eq = -1;

	if (ldelset) { printf("Line %5u: EN in %c%c\n", yylineno, ldel, rdel) ; errors = 1; }
	}

^{C}EN[ 	]*	syntax("EN", yylineno) ;
^{SP}*define {
	if(ds && eq) within ( "define", "DS", yylineno) ;
	}

^{SP}*delim{SP}*.. {
	if(eq && ds)  within ("delim", "DS", yylineno ) ;

	if(eq && ds==0){
	if (ldelset) {
	 printf("Line %5u: missing right delimiter %c, at delimiter change\n", yylineno,  rdel); errors = 1; }
	if (yytext[yyleng-2] == 'o' && yytext[yyleng-1] == 'f') {
	    if (delset == 0) extra ("delim off", yylineno ) ;
	    else
	    delset = 0 ;
	    ldelset = 0;
	    }
	else {
	    ldel = yytext[yyleng-2];
	    rdel = yytext[yyleng-1];
	    delset = 1;
	    ldelset = 0;
	    }
	}

	if(eq==0) REJECT;
	}

^{C}D(S|F)([ 	]+(\"\"|[0-3]|[LIC]|CB))?([ 	]+(\"\"|[0-1]|[NF]))?([ 	]+[\-+]?[0-9]+)?{end}  {
		if (ds) within("DS", "DS", yylineno ) ;

	if(as)  within ("DS or DF", "AS", yylineno) ;

	if(fs) within ("DS or DF", "FS", yylineno ) ;
	ds = 1 ;
	}

^{C}DE{end} {
	if(ds == 0) extra("DE", yylineno) ;

	if(fs) within("DE", "FS", yylineno) ;

	if(as) within("DE", "AS", yylineno) ;
	ds = 0 ;
	if (eq ==1) 	{
		missing("EN" ,"DE", yylineno) ;
		eq = 0;
		}
	}
^{C}D(S|F|E)[ 	]*	syntax(" DS or DF or DE" , yylineno);

^{C}FS([ 	]+{printables}+)?{end}	{
		if (fs) within ("FS", "FS", yylineno ) ;

/* 		if(ds) within("FS", "DS", yylineno) ; */
		fs = 1 ;

		if (as || tl || tsh )	{
			int label ;
			label = 1;
			if (yyleng < 5) label = 0;
			else	{
				i = 4;
				while (i <= yyleng)	{
					if (yytext[i] != ' ' && yytext[i] != '	') break;
					i++;
				}
				if ( i == yyleng ) label = 0;
			}
			if (!label) printf("Line %5u: only labeled footnotes appear properly in title, abstract or a table\n", yylineno) ;
			if (tl) tl = 0 ;
		}
	}


^{C}FE{end} {if(fs == 0) extra ("FE", yylineno) ;

/* 	if(as) within ("FE", "AS", yylineno) ; */
/* 	if(ds) within ("FE", "DS", yylineno ) ; */
	fs = 0 ; 
	}

^{C}FD([ 	]+([0-9]|1[0-1]|\"\"))?([ 	]+1)?{end}	;

^{C}F(S|E|D)[ 	]*	syntax(" FS or FE or FD " , yylineno) ;

^{C}SG([ 	]+([A-Za-z]+|\"\"))?([ 	]+1)?{end} 	{ 
		if (fs) within ("SG", "FS", yylineno ) ;
		if (ds) within ("SG", "DS", yylineno) ;
	}

^{C}(EC|FG|TB|EX)([ 	]+{string})?([ 	]+0)?([ 	]+1|2)?{end}	if (!ds) 
		within("EC or FG or TB or EX not", "DS/DE", yylineno) ;
^{C}H([ 	]+[1-7])([ 	]+({printables}+|{string}))?([ 	]+{printables}+)?{end} {
	if (ts) missing("TE", "H", yylineno) ;

	if(tsh) missing("TH", "H", yylineno) ;

	if (ds) missing("DE", "H", yylineno) ;

	if (fs) missing( "FE", "H", yylineno) ;

	if (ns) missing("NE", "H", yylineno) ;

	if (eq >0) missing("EN", "H", yylineno) ;

	if(as) {
		while ( il > -1 ) 
			printf("Line %5u: missing LE before H for %s\n", yylineno, listm[il--]) ; 
			ds = fs = ts = eq = tsh = 0 ;
			il = -1 ;
		}
	}
^{C}HC([ 	]+{printables})?{end}	;

^{C}HM{hmargs}?{hmargs}?{hmargs}?{hmargs}?{hmargs}?{hmargs}?{hmargs}?{end}	;

^{C}HU.*{end}	if (argument(1,1)) syntax("HU", yylineno);

^{C}H(C|M)?[ 	]*	syntax("H or HC or HM", yylineno) ;




^{C}TS{end}	{
		if (ts) within ("TS", "TS", yylineno ) ;
		ts = 1 ; 
	}

^{C}TS[ 	]+H  {
	if (ts) within ("TS", "TS", yylineno) ;

	if (ds) within ("TS H", "DS", yylineno ) ;
	tsh = 1 ;
	ts = 1 ;
	}

^{C}TE{end} {
	if (ts == 0) extra ("TE", yylineno ) ;
	ts = 0;

	if(tsh) {
		missing("TH", "TE", yylineno) ;
		ts = tsh = 0 ; 
		}
	}

^{C}T&{end} if (ts == 0)	missing("TS", "T&", yylineno) ;

^{C}TH{end} {
	if(tsh == 0)		missing("TS H", "TH", yylineno) ;
	tsh = 0 ; 
	}

^{C}TC([ 	]+[1-7])?{num}?([ 	]+[1-7])?{num}?{end}	;
^{C}TC[ 	]*	syntax("TC", yylineno) ;
^{C}[PEO][HF][ 	]+{string}{end}	{
	char delimtr ;
	int del_cnt ;
	int i;
	i = 4 ;
	while ( i <= yyleng ) 	{
		if (yytext[i] == '"') i++ ;
		else break ;
	}
	if (i < yyleng ) delimtr = yytext[i] ;

	del_cnt = 0;
	while( i <= yyleng && yytext[i] != '"' )	{
		if (yytext[i] == delimtr)	del_cnt++ ;
		if (yytext[i] == '\\' && yytext[i + 1] == '"' ) i++ ;
		i++ ;
	}
	if (del_cnt !=4 ) syntax( "Header of Footer Macro", yylineno) ;
	}
^{C}(PH|EH|OH|PF|EF|OF)[ 	]*	syntax("Header of Footer Macro", yylineno) ;
^{C}OP{end}	;
^{C}OP[ 	]*	syntax("OP", yylineno) ;
^{C}P([ 	]+1|2)?{end}	;
^{C}P[ 	]*	syntax("P", yylineno) ;
^{C}PM([ 	]+(P|N|BP|BPP|BR|BPN|ILL|CI-II|CA|CP|PM[1-7]))?{end}	;
^{C}PM[ 	]*	syntax("PM", yylineno) ;
^{C}S([ 	]+(\"\"|P|C|D|[\-+]?[0-9]+))?([ 	]+(\"\"|P|C|D|[\-+]?[0-9]+))?{end}	;
^{C}S[ 	]*	syntax("S", yylineno) ;
^{C}SA([ 	]+(0|1))?{end}	;
^{C}SA[ 	]*	syntax("SA", yylineno) ;
^{C}(SK|SP){num}?{end}	;
^{C}(SK|SP)[ 	]*	syntax("SK or SP", yylineno) ;
^{C}VM{num}?{num}?{end}	;
^{C}VM[ 	]*	syntax("VM", yylineno) ;

^{C}BS{end}	{
	if (bs) within ("BS", "BS", yylineno) ;

	if (ds) within ("BS", "DS", yylineno ) ;

	if(as) within ("BS", "AS", yylineno ) ;

	if (fs) within ("BS", "FS", yylineno ) ;

	bs = 1 ;
	}

^{C}BE{end}	{
	if(!bs)	missing("BS", "BE", yylineno) ;
	if (bs || fs) within ("BE", "DS or FS", yylineno ) ;

	if(as) within ("BE", "AS", yylineno ) ;

	bs = 0 ;
	}

^{C}B(S|E)[ 	]*	syntax ("BS or BE", yylineno ) ;

.	{if(delset){
	if(ldelset){
	 if (yytext[0] == rdel) {
  	    if (notwarned == 0) 
		  { printf("Line %5u: right delimiter %c only\n", yylineno,  rdel);
			errors = 1;
		}
	    ldelset = 0;
	 }
	}
	else {
	if (yytext[0] == ldel){
	    if (eq) { printf("Line %5u:  left delimiter %c in EQ\n", yylineno,  ldel ) ;
		errors = 1; 
		}
	    else { ldelset = 1; notwarned = 1; } 
	}
	}
	}
	}

\n	{if (ldelset && notwarned)	{
		printf("Line %5u: left delimiter %c only\n", yylineno - 1,  ldel);
		errors = 1; 
		}
	notwarned = 0;
	}
%%
