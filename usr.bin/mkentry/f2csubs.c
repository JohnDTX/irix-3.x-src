#include "stdio.h"
#include "mkentry.h"
#include "tokens.h"
#ifdef YYDEBUG
int yydebug;
#endif

#include "macros.h"
#include <ctype.h>
char *progname = "mkf2c";


typedef struct {
	char name[40];
	int class,type;
	/* offset from sp of the address of the argument */
	int stack_offset;
	/* len_offset is for char strings only */
	int len_offset; 
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
				 /* allow fortran long external names */
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
			fprintf(stderr,"cant open output file %s\n",opfnm);
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
	exit(0);
}

eftn()
{
	int arg_offset,space_used;
	arg *curarg;
	int i,class;
	int type;

#ifdef DEBUG
	fprintf(stderr,"eftn called\n");
	fprintf(stderr,"nargs = %d\n",nargs);
#endif
	if ((ftn.class & ~CLASS) == STATIC) return;
	/* eftn - the real work.  All of the parameters have been
	   declared and typed. We have to do the following work:

	   Beginning with the last argument and proceeding to the
	   first, each argument must be resolved and pushed on the
	   stack.  The last argument will be found at sp@(4).  This
	   resolution consists of the following:

		1. if the argument in C is a value, the value must
		   be loaded (from its address) and pushed.  If the
		   value is NOT 32 bits, it must be extended.

		2. if the argument in C is an address and NOT a character
		   string, the address on the stack is copied.

		3. if the argument in C is a character string, we must:
			
			a. take the length of the Fortran string off
			   the stack (32-bits now, was 16 bits before in 2.4
			   pre-release)

			b. use the length to calloc space for a copy of 
			   the string.

			c. copy the string to the calloc'd area.

			d. push the address of the string copy.

		4. on return, the string probably should be free'd.

	*/

	ftn.nargs = nargs;
	
	/* generate the entry code */
	TEXTCSEG;
	GLOBL(ftn.centry);
	GLOBL(ftn.fentry);
	STABC(ftn.centry);
	COMMENT3(ftn.fentry,"(FORTRAN entry) for calling C routine",ftn.centry);
	LABEL(ftn.fentry);
#ifdef OLDWAY
	LINK(0);
#else
	/* save some space on the stack for copies of
	   the strings to free
	*/
	LINK(localspace);
#endif
	SAVEFREGS;

	/* arg_offset is the offset of the current parameter from
	   the link register (a6) */
	arg_offset = FIRST_ARG_OFFSET;
	/* space_used is the amount of stack space we have used in
	   the C stack frame */
	space_used = 0;

	for (i=(nargs-1),curarg= &arglist[i]; i>=0 ; (i--),curarg--) {
	/* push the lengths of the FORTRAN strings in reverse order
	   for C functions to use if they want them */
		switch (type = curarg->type) {

			default: 
				arg_offset += 4;
				break;

			case CHARARRAY:
				COMMENT3("character array '",curarg->name,
					 "' length");
#ifdef OLDWAY
				LOAD_VALUE(arg_offset,'w');
				EXTEND('l');
#else
				LOAD_VALUE(arg_offset,'l');
#endif
				PUSH_VALUE;
				space_used += 4;
			case STRING:
			case CHAR:
#ifdef OLDWAY
				arg_offset += 6;
#else
				arg_offset += 8;
#endif
				break;
		}
	}
	arg_offset = FIRST_ARG_OFFSET;
		
	for (i=(nargs-1),curarg= &arglist[i]; i>=0 ; (i--),curarg--) {


		class = curarg->class;
		switch (type = curarg->type) {
		
		    case PTR:	/* simplest case.  Just copy the address
				   from the stack pointer */
				COMMENT3("parameter '",curarg->name,
						    "' - pointer copy");
				PUSH_STK_VALUE(arg_offset,'l');
				arg_offset += 4;
				space_used += 4;
				break;

		    case CHARARRAY:	/* simplest case.  Just copy the address
				   from the stack pointer */
				COMMENT3("parameter '",curarg->name,
						    "' - pointer to char string");
#ifdef OLDWAY
				arg_offset += 2;
#else
				arg_offset += 4;
#endif
				PUSH_STK_VALUE(arg_offset,'l');
				arg_offset += 4;
				space_used += 4;
				break;

		    case SHORT: /* */
				if (class == UNSIGNED) {
					COMMENT3("parameter '",curarg->name,
						    "' - unsigned short value");
					LOAD_ZERO;
				} else 
					COMMENT3("parameter '",curarg->name,
						    "' - signed short value");
				LOAD_IND_STKVAL(arg_offset,'w');
				if (class != UNSIGNED) EXTEND('l');
				PUSH_VALUE;
				space_used += 4;
				arg_offset += 4;
				break;

		    case CHAR:
				if (class == UNSIGNED) {
					COMMENT3("parameter '",curarg->name,
						    "' - unsigned char value");
					LOAD_ZERO;
				} else 
					COMMENT3("parameter '",curarg->name,
						    "' - signed char value");
#ifdef OLDWAY
				LOAD_IND_STKVAL(arg_offset+2,'b');
#else
				LOAD_IND_STKVAL(arg_offset+4,'b');
#endif
				if (class != UNSIGNED) {
					EXTEND('w');
					EXTEND('l');
				}
				PUSH_VALUE;
				space_used += 4;
#ifdef OLDWAY
				arg_offset += 6;
#else
				arg_offset += 8;
#endif
				break;

		    case LFLOAT:
				COMMENT3("parameter '",curarg->name,
						    "' - double value");
				LOAD_STKVAL_A(arg_offset);
				PUSH_INDIRECT(4);
				PUSH_INDIRECT(0);
				arg_offset += 4;
				space_used += 8;
				break;

		    case STRING:
				COMMENT3("parameter '",curarg->name,
						   "' - character string copy");
				/* the hard case - character strings */
				/* length for strncpy */
				COMMENT("first, get length of Fortran string.");
#ifdef OLDWAY
				LOAD_ZERO;
				LOAD_VALUE(arg_offset,'w');
#else
				LOAD_VALUE(arg_offset,'l');
#endif
				/* length for strncpy */
				COMMENT("push it for strncpy later");
				PUSH_VALUE;
				/* source address for strncpy */
				COMMENT("push address of Fortran string for strncpy");
#ifdef OLDWAY
				PUSH_STK_VALUE(arg_offset+2,'l');
#else
				PUSH_STK_VALUE(arg_offset+4,'l');
#endif
				/* add one to the length (for null terminator)*/
				COMMENT("add one to length for null, and push size of area to calloc");
				INCR_VALUE(1);
				PUSH_VALUE;
				/* one element of the string */
				COMMENT("only need one element");
				PUSH_CONSTANT(1);
				/* we use calloc, because it inits the area
				   to nulls */
				CALL("_calloc");
				/* pop calloc's args */
				UPDATE_SP(8);
				/* we want to save the value returned by calloc
				   to push later for the target routine.  We do
				   this by employing the fact that strncpy 
				   returns this pointer */
				COMMENT("push the address of the new string as the destination for strncpy");
				PUSH_VALUE;
				CALL("_strncpy");
				UPDATE_SP(12);
				COMMENT("strncpy returns the address of the destination");
#ifndef OLDWAY
				/* at this point, we want to squirrel away
				   a copy of the string pointer, so that 
				   eftn can free it
				*/
				SAVE_VALUE(curarg->strptrcopy_offset);
#endif
				PUSH_VALUE;
				space_used += 4;
#ifdef OLDWAY
				arg_offset += 6;
#else
				arg_offset += 8;
#endif
				stringsused++;
				break;

				
		    
		    default:	/* case for INT, LONG, FLOAT */
				COMMENT3("parameter '",curarg->name,
						  "' - 32-bit value");
				LOAD_STKVAL_A(arg_offset);
				PUSH_INDIRECT(0);
				arg_offset +=4;
				break;

		}
	    }

	    /* args are pushed.  The stack is in good shape.  
	       do the jbsr.  */
	    CALL(ftn.centry);
		/* if the routine is to return a short or char,
		   we have to extend the result register */
		if (ftn.type == CHAR) {
			if (ftn.class == UNSIGNED) {
				COMMENT("result is unsigned char");
				printf("\tmovl\td0,d1\n");
				printf("\tclrl\td0\n");
				printf("\tmovb\td1,d0\n");
			} else {
				COMMENT("result is signed char");
				EXTEND('w');
				EXTEND('l');
			}
		} else if (ftn.type == SHORT) {
			if (ftn.class == UNSIGNED){
				COMMENT("result is unsigned short");
				printf("\tmovl\td0,d1\n");
				printf("\tclrl\td0\n");
				printf("\tmovw\td1,d0\n");
			} else {
				COMMENT("result is signed short");
				EXTEND('l');
			}
		}
#ifndef OLDWAY
	    if (stringsused)
	    {
		/* save the return result on the stack */
		PUSH_VALUE;
		if (ftn.type == LFLOAT) {
			printf("\tmovl\td1,sp@-\n");
		}

	    	/* call free() for each string copy calloc'd */

	    	for (i=(nargs-1),curarg= &arglist[i]; i>=0 ; (i--),curarg--) {

			if (curarg->type == STRING) {
				/* retrieve the address of the string copy */
				LOAD_VALUE(curarg->strptrcopy_offset,'l');
				/* push it for free() */
				PUSH_VALUE;
				CALL("_free");
				UPDATE_SP(4);
			}

		}
		if (ftn.type == LFLOAT) {
			printf("\tmovl\tsp@+,d1\n");
		}
		printf("\tmovl\tsp@+,d0\n");
	    }
#endif

	    /* get rid of the C arguments by doing an unlink */
	    UNLINK;
	    /* the return addres is at tos.  Save it. */
	    PULL_ADDRESS;
#ifdef OLDWAY
	    /* peel off the Fortran parameters */
	    if (nargs) UPDATE_SP(arg_offset - FIRST_ARG_OFFSET);
#endif
	    /* and return to the routine */
	    JUMP_INDIRECT;


}




finishup()
{
#ifdef DEBUG
	fprintf(stderr,"finish up called\n");
#endif

	if (stringsused) {
		GLOBL("_strncpy");
		GLOBL("_calloc");
	}
}





procarg()
{
	arg *curarg;
#ifdef DEBUG
	fprintf(stderr,"procarg called.  ");
	fprintf(stderr,"token is %s\n",lastname);
#endif
	/* procarg - the name of an argument has been spotted in 
	   a parameter list.  fill in the correct slot in arglist[]
	   Make the argument INT.
	*/
	if ((ftn.class & ~CLASS) == STATIC) return;

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
#ifndef OLDWAY
			if (curtype == STRING)
			{
				localspace -= 4;
				curarg->strptrcopy_offset = localspace;
			}
#endif
			curarg->class = curclass;
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
		2. generate the entry point, the .globl and the
		   special register saves (a4,a5)

	*/

	/* copy the C entry name */
	ftn.centry[0]='_';
	strcpy(&ftn.centry[1],lastname);
	/* and the Fortran entry - if OLDWAY, there is a max of 6 chars */
	for (cptr = lastname, fnameptr = ftn.fentry ; 
		(*cptr 
		&& fnameptr<&ftn.fentry[f77_extname_max]
		) ; cptr++) {
		if (isalnum(*cptr)) 
			*fnameptr++ = 
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
	"function (%s) type of CHAR corresponds to FORTRAN type of INT*1\n",
		ftn.centry);

#ifndef OLDWAY
	localspace = 0;
#endif


}

