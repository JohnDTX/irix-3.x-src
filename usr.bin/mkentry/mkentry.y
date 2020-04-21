
%start ftn_list
%token NAME 256
%token SU 512
%token CLASS 384
%token TYPE  320
%token INDIRECT	2
%token LP	3
%token RP	4
%token LBRACKET	5
%token RBRACKET	6
%token LBRACE	7
%token RBRACE	8
%token CM	10
%token SM	11
%token NUMBER	12

%{
#include <stdio.h>
#include <errno.h>
#include "mkentry.h"
#include "tokens.h"
#define ENDFILE 0xFFFF8000
%}

%%

ftn_list	:	ftn
		|	ftn_list ftn
		;

ftn		:	type ftn_decl
				{	/* function body seen.
					   all arguments are parsed.
					   patch any that have not been
					   typed as INT and generate the
					   call
					*/
					eftn();
				}
		;

type		:	/* empty type */	
				{	curclass = 0;
					curtype = INT; }
		|	CLASS
				{	curclass = $1;
					curtype = INT; }
		|	TYPE
				{	curclass = 0;
					curtype = $1;
				}
		|	CLASS TYPE 
				{	curclass = $1;
					curtype = $2; }	
		|	CLASS CLASS TYPE
				{	curclass = $1 | $2;
					curtype = $3;
				}
		|	CLASS CLASS
				{	curclass = $1 | $2;
					curtype = INT;	}
		|	CLASS CLASS CLASS
				{	curclass = $1 | $2 | $3;
					curtype = INT;	}
		|	CLASS CLASS CLASS TYPE
				{	curclass = $1 | $2 | $3;
					curtype = $4;	}
		;

ftn_decl	:	NAME {	/* curclass/curtype have the class and
				   type of the function.  save it */
				   nfuncs++;
				   bftn(); 
				   curclass = 0;
				   curtype = basetype = INT;}
			  arg_list arg_decls program
		|	NAME NAME
				{	curclass = 0;
					curtype = PTR|INT;
				   bftn(); 
				   curclass = 0;
				   curtype = basetype = INT;
				}
		|	ind_list NAME
				{	curclass = 0;
					curtype = PTR|INT;
				   bftn(); 
				   curclass = 0;
				   curtype = basetype = INT;
				}
			  arg_list arg_decls program
/*		|	error
				={	resynch();
					YYACCEPT;
				}
*/
		;

arg_list	:	LP args RP 
		;

args		:	/* empty arg list */
				{	nargs = 0; }
		|	NAME
				{	nargs = 0;
					/* the argument name is in tokenbuf.
					   Enter it onto the list and process */
					procarg();
					nargs++;
				}
		|	args CM NAME
				{	procarg();
					nargs++;
				}
		;

arg_decls	:	/* no arg declarations - all are int */
		|	arg_decl_list
		;

arg_decl_list	:	arg_decl
				{	curclass = 0;
					curtype = basetype = INT;
				}
		|	arg_decl_list 
				{	curclass = 0;
					curtype = basetype = INT;
				}
			    arg_decl
		;

arg_decl	:	arg_namelist SM 
		|	arg_type arg_namelist SM
		;

arg_namelist	:	arg_name
		|	arg_namelist CM arg_name
		;

arg_type	:	TYPE
				{	curclass = 0;
					curtype = basetype = $1;
				}
		|	CLASS
				{	curclass = $1;
					curtype = basetype = INT; }
		|	CLASS TYPE 
				{	curclass = $1;
					curtype = basetype = $2; }	
		|	CLASS CLASS TYPE 
				{	curclass = $1 | $2;
					curtype = basetype = $3; }	
		|	CLASS CLASS
				{	curclass = $1 | $2;
					curtype = basetype = INT;}
		|	CLASS CLASS CLASS
				{	curclass = $1 | $2 | $3;
					curtype = basetype = INT;	}
		|	CLASS CLASS CLASS TYPE
				{	curclass = $1 | $2 | $3;
					curtype = basetype = $4;	}
		|	NAME
				{	
					curtype = basetype = PTR|INT;
					if (!((ftn.class & ~CLASS) == STATIC))
						werror(
			"unknown type name. simple pointer assumed\n");
				}
		|	SU NAME
				{	curtype = basetype = SU;
				}
		;


arg_name	:	name 
				{	/* tokenbuf has the name of 
					   the arg being declared, the class
					   and type are in curtype/curclass.
					   Match it with one of the args */
					   matcharg();	
					   curtype = basetype;}
		|	name dimension
				{	if (curtype ==  CHAR) {
					if (!((ftn.class & ~CLASS) == STATIC))
						werror(
				"char array, simple pointer passed\n");
						curtype = CHAR|ARRAY;
					}
					else
						curtype = PTR|INT;
					matcharg();	
					curtype = basetype;
				}
		;

dimension	:	dim
			|	dimension dim
			;

dim			:	LBRACKET RBRACKET {
					curnum =1;
				}
			|	LBRACKET NUMBER RBRACKET
				{
					curnum = $2;
				}
			;

name		:	NAME
		|	ind_list NAME
				{	if (curtype & PTR) {
					if (!((ftn.class & ~CLASS) == STATIC))
						werror(
			"multiply indirect type, simple pointer passed\n");
						curtype = INT|PTR;
					}
					else
						curtype |= PTR;
				}
		;

ind_list	:	INDIRECT
		|	ind_list 
				{	curtype |= PTR;
				}
			    INDIRECT 
		;

program		:	lb linear_segs RBRACE;
		|	lb RBRACE;
		;

lb		:	LBRACE {	skiptobrace();}
		;

rb		:	RBRACE {	skiptobrace();}
		;

/*
prog_segments	: linear_segs
		|	lb prog_segments rb
		;
*/

linear_segs	:	segment
		|	linear_segs segment
		;

segment	:	lb rb
		|	lb linear_segs rb
		;
%%

int nfuncs = 0;

error(s,a,b,c) /* VARARGS */
{
	fprintf(stderr,"%s, fatal error: line %d - ",ipfnm,_par_lineno);
	fprintf(stderr,s,a,b,c);
	fclose(stdout);
	if (opfnm) {
		if (unlink(opfnm)) 
			fprintf(stderr,"cant unlink %s. errno = 0x%x\n",opfnm,errno);
	}
	exit(-1);
}

werror(s,a,b,c) /* VARARGS */
{
	fprintf(stderr,"%s, warning: line %d - ",ipfnm,_par_lineno);
	fprintf(stderr,s,a,b,c);
}

yyerror()
{
	/* GB SCR802 10/8/85 - on null input simply exit with warning.  */
	char *cptr = 0;
	if (yylval != ENDFILE) 
		cptr = "%s: %s, syntax error at line %d.\n";
	else if (nfuncs)
		cptr = "%s: %s, unexpected end-of-file hit at line %d.\n";
	if (cptr)
		fprintf(stderr,cptr,progname,ipfnm,_par_lineno);
	else
		fprintf(stderr,"%s: %s, no input given.\n",progname,ipfnm);
	/*resynch();*/
	fclose(stdout);
	if (opfnm) {
		if (unlink(opfnm)) 
			fprintf(stderr,"%s: cant unlink %s. errno = 0x%x\n",
				progname,opfnm,errno);
	}
	if (cptr)
		exit(-1);
	else
		exit(0);
}

resynch() {
	int tval;
	if ((retval == SM)/*|| (retval==RBRACE)*/) return;
	/* try to resynchronize */
	while (((tval = yylex())!= SM) && (tval != ENDFILE)
	       /*&& (tval != RBRACE)*/) ;
}
