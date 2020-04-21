#include "stdio.h"
#include "mkentry.h"
#include "tokens.h"
#ifdef YYDEBUG
int yydebug;
#endif

#include "macros.h"
#include <ctype.h>
char *progname = "mkc2f";


typedef struct {
	char name[40];
	int class,type;
	/* offset from sp of the address of the argument */
	int stack_offset;
	/* len_offset is for char strings only */
	int len_offset; 
	int len;
#ifndef OLDWAY
	/* string pointer copy is for char strings (value parameters 
	   (char *)) only.  A copy of the pointer to the space
	   calloc'd is saved so that it can be free'd
	*/
	int strptrcopy_offset;
#endif
	} arg;

#define MAXPARMS 64
int f77_extname_max = 6;
arg arglist[MAXPARMS];

main(argc,argv) int argc; char **argv;
{
	char *sp,*cptr,*index(),*ctime();
	long ltime;

	int iarg = 1;

	while (iarg < argc) {
		
		if (argv[iarg][0] == '-') {

			switch (argv[iarg][1]) {


			case 'l':
				 f77_extname_max = 31;
				 break;

			default:
				error("unrecognized switch (%s) - ignored.\n",
				 &argv[iarg][1]);
			}
		}
		else {
			if (!ipfnm)
				ipfnm = argv[iarg];
			else if (!opfnm)
				opfnm = argv[iarg];
			else
				error("additional filename (%s) ignored.",
					argv[iarg]);
		}
		
		iarg++;
	}
	if (ipfnm) {
		if (freopen(ipfnm,"r",stdin) == NULL) {
			fprintf(stderr,"cant open input file %s\n",ipfnm);
			exit(-1);
		}
	}
	/* scr1250 - ipfnm must be null string rather than null pointer */
	else ipfnm = "";

	if (opfnm) {
		if (freopen(opfnm,"w",stdout) == NULL) {
			fprintf(stderr,"cant open output file\n");
			exit(-1);
		}
		COMMENT2("input file is ",ipfnm);
		COMMENT2("output file is ",opfnm);
		ltime = time(0);
		cptr = ctime(&ltime);
		if ((sp = index(cptr,'\n'))!=NULL) *sp = '\0';
		COMMENT2("generated on ",cptr);
	}
	else opfnm=(char *)0;

#ifdef YYDEBUG
	yydebug = 1;
#endif
	COMM("_$a4_save",4);
	COMM("_$a5_save",4);
	doparse();
	finishup();
	return(0);
}

eftn()
{
	int arg_offset,space_used;
	arg *curarg;
	int i,class;
#ifdef DEBUG
	fprintf(stderr,"eftn called\n");
	fprintf(stderr,"nargs = %d\n",nargs);
#endif
	if ((ftn.class & ~CLASS) == STATIC) return;
	/* eftn - the real work.  All of the parameters have been
	   declared and typed. We have to do the following work:

	   Beginning with the first argument and proceeding to the
	   last, each argument must be resolved and pushed on the
	   stack.  The first argument will be found at sp@(4).  This
	   resolution consists of the following:

		1. if the argument in C is a value, the address of the value
		   pushed on the stack must be loaded and pushed.  If the
		   value is NOT 32 bits, the address must be adjusted
		   to point to the correct byte or word.

		2. if the argument in C is an address and NOT a character
		   string, the address on the stack is copied.
		   NOTE: this is a problem for doubly indirect objects
		

		3. if the argument in C is a character string, we must:
			
			a. get the length of the C string. This length 
			   is pushed after the address of the string.

			b. push the address of the string.

			c. push the length of the string as a 16-bit integer.


	*/

	ftn.nargs = nargs;
	
	/* generate the entry code */
	TEXTCSEG;
	GLOBL(ftn.centry);
	GLOBL(ftn.fentry);
	STABF(ftn.fentry);
	COMMENT3(ftn.centry,"(C entry) for calling Fortran routine",ftn.fentry);
	LABEL(ftn.centry);
	/* push the current copies of a4/a5 on the stack so we can restore
	   the global ones Fortran wants */
	LINK(-16);
	SAVECREGS(-16);
	RESTOREFREGS;

	/* arg_offset is the offset of the current parameter from
	   the link register (a6) */
	arg_offset = FIRST_ARG_OFFSET;
	/* space used is the amount of stack space we have used for 
	   the Fortran stack frame */
	space_used = 0;

	for (i=0, curarg = &arglist[i]; i<nargs ; (i++),curarg++) {

		int type;

		class = curarg->class;
		switch (type = curarg->type) {
		
		    case PTR:	/* simplest case.  Just copy the address
				   from the stack pointer */
				COMMENT3("parameter '",curarg->name,
						    "' - pointer copy");
				PUSH_STK_VALUE(arg_offset,'l');
				arg_offset += 4;
				break;

		    case SHORT: /* */
				if (class == UNSIGNED) {
					COMMENT3("parameter '",curarg->name,
						    "' - unsigned short value");
				} else 
					COMMENT3("parameter '",curarg->name,
						    "' - signed short value");
				LOAD_STKADDR(arg_offset);
				/* offset to the word */
				INCR_ADDRESS(2);
				PUSH_ADDRESS;
				arg_offset += 4;
				break;

		    case CHAR:
				if (class == UNSIGNED) {
					COMMENT3("parameter '",curarg->name,
						    "' - unsigned char value");
				} else 
					COMMENT3("parameter '",curarg->name,
						    "' - signed char value");
				LOAD_STKADDR(arg_offset);
				/* offset to the byte */
				INCR_ADDRESS(3);
				PUSH_ADDRESS;
#ifdef OLDWAY
				PUSH_SHORT_CONSTANT(1);
#else
				PUSH_CONSTANT(1);
#endif
				arg_offset += 4;
				break;

		    case LFLOAT:
				COMMENT3("parameter '",curarg->name,
						    "' - double value");
				PUSH_STK_ADDR(arg_offset);
				arg_offset += 8;
				break;

		    case CHARARRAY:
				COMMENT3("parameter '",curarg->name,
						   "' - character array ");
				/* the hard case - character strings */
				/* get the length by passing the string
				   address to strlen */
				PUSH_STK_VALUE(arg_offset,'l');
#ifdef OLDWAY
				PUSH_SHORT_CONSTANT(curarg->len);
#else
				PUSH_CONSTANT(curarg->len);
#endif
				arg_offset += 4;
				break;

				
		    case STRING:
				COMMENT3("parameter '",curarg->name,
						   "' - character string. ");
				/* the hard case - character strings */
				/* get the length by passing the string
				   address to strlen */
				/* source address for strlen */
				COMMENT(
		"get the source address for strlen");
				PUSH_STK_VALUE(arg_offset,'l');
				CALL("_strlen");
				/* pop strlen's arg */
				UPDATE_SP(4);
				COMMENT("push the string address");
				PUSH_STK_VALUE(arg_offset,'l');
#ifdef OLDWAY
				PUSH_SHORT_VALUE;
#else
				PUSH_VALUE;
#endif
				arg_offset += 4;
				stringsused++;
				break;
		    
		    default:	/* case for INT, LONG, FLOAT */
				COMMENT3("parameter '",curarg->name,
						  "' - 32-bit value");
				PUSH_STK_ADDR(arg_offset);
				arg_offset += 4;
				break;

		}
	    }

	    /* args are pushed.  The stack is in good shape.  
	       do the jbsr.  */
	    CALL(ftn.fentry);

	    /* the saved copies of a4,a5 are in our stack frame */
	    RESTORECREGS(-16);
	    /* there should be no Fortran arguments left.  Unlink anyway */
	    UNLINK;
	    /* the return addres is at tos.  Just return. */
	    RTS;
	    /* C will pop the arguments it placed on the stack */
	    /* and return to the routine */


}




finishup()
{
#ifdef DEBUG
	fprintf(stderr,"finish up called\n");
#endif

	if (stringsused) {
		GLOBL("_strlen");
		GLOBL("_calloc");
		GLOBL("_strcpy");
	}

}





procarg()
{
	arg *curarg;
#ifdef DEBUG
	fprintf(stderr,"procarg called.  ");
	fprintf(stderr,"token is %s\n",lastname);
#endif
	if ((ftn.class & ~CLASS) == STATIC) return;
	/* procarg - the name of an argument has been spotted in 
	   a parameter list.  fill in the correct slot in arglist[]
	   Make the argument INT.
	*/


	if (nargs > MAXPARMS) 
		error(
"maximum number of parameters exceeded\nat parameter %s in function %s\n",
			lastname,ftn.centry);
	curarg = &arglist[nargs];

	/* copy the argument string to the slot */
	strcpy(curarg->name,lastname);
	curarg->class = 0;
	curarg->type = INT;
}






matcharg()
{
	int i;
	arg *curarg;
#ifdef DEBUG
	fprintf(stderr,"matcharg called.  curclass = 0x%x, curtype= 0x%x\n",
			 curclass,curtype);
	fprintf(stderr,"token is %s\n",lastname);
#endif
	if ((ftn.class & ~CLASS) == STATIC) return;
	/* matcharg - the declaration of an argument has been
	   spotted.  Match it with an argument in the parameter list
	   and fill in the class and type. */

	for (i=0,curarg=arglist;i<nargs;i++,curarg++) {
		if (!strcmp(lastname,curarg->name)) {
			/* match found */
#ifdef DEBUG
	fprintf(stderr,"match found arg #%d\n",i);
#endif
			if ((curtype & PTR)&&(curtype != STRING)) 
				curarg->type = PTR;
			else if ((curtype == FLOAT) && (curclass == LONG)){
				curarg->type = LFLOAT;
				curclass = 0;
			}
			else if (curtype == SU) {
					werror(
			"struct/union parameter assumed indirect. - simple pointer passed\n");
				curarg->type = PTR;
			}
			/* fix for scr1943 */
			else if (curtype == VOID) {
					werror(
			"void parameter assumed indirect. - simple pointer passed\n");
				curarg->type = PTR;
				curtype = PTR;
			}
			/* end of fix for scr1943 */
			else
				curarg->type = curtype;
			curarg->class = curclass;
			curarg->len = curnum;
			return;
		}
	}
	fprintf(stderr,
		"parameter %s in type declaration was not in parameter list\n",
		lastname);
	fprintf(stderr,"in declaration of function %s\n",&ftn.centry[1]);
	yyerror();
 
	
	
}


bftn()
{
	char *cptr,*fnameptr;
#ifdef DEBUG
	fprintf(stderr,"bftn called with function %s\n",lastname);
	fprintf(stderr,"class = %d, type = %d\n",curclass,curtype);
#endif
	/* the function name, type and class has been read. 
	   we have to do the following things:

		1. set up the function parameters in the ftn structure.
		2. generate the entry point, and the .globl.
		3. do the link.
		4. generate the special register saves (a4,a5)
	*/

	/* copy the C entry name */
	ftn.centry[0]='_';
	strcpy(&ftn.centry[1],lastname);
	/* and the Fortran entry - if OLDWAY, max 6 chars */
	for (cptr = lastname, fnameptr = ftn.fentry ; 
		(*cptr 
		&& fnameptr<&ftn.fentry[f77_extname_max]
		) ; cptr++) {
		if (isalnum(*cptr)) *fnameptr++ = 
#ifdef OLDWAY
			toupper
#else
			tolower
#endif
				(*cptr); 
		else werror(
		"removing illegal character %c (\\%3.3o) in FORTRAN entry for function %s\n",
			*cptr,*cptr,lastname);
	}
	*fnameptr = '\0';

			/* fix for scr1943 */
	if (curtype == VOID) curtype = INT;
			/* end of fix for scr1943 */
	ftn.type = curtype; ftn.class = curclass;
	if ((curclass & ~CLASS)== STATIC)
		werror(
	"static function (%s) ignored.\n",
		ftn.fentry);
	if (curtype == CHAR)
		werror(
	"function (%s) type of CHAR corresponds to FORTRAN type of INT*1",
		ftn.fentry);


}

