/*	
 *
 * 	ps.c -	process pseudo-ops.
 *
 *		A routine in ps.c is called by do_statement
 *		(statement.c) when a pseudo op has been found
 *		which is in the table pslist (initialized at
 *		the end of this file and included in ps.h).
 *		The structure is documented in ps.h, and includes
 *		arguments to pass to routines which process
 *		more than one pseudo-op.  These routines get
 *		most of their information from the rest of the
 *		tokens, indicated by the passed tokenlist pointer.
 *		
 *
*/

#include <stdio.h>
#include "globals.h"
#include "tokens.h"
#include "sym.h"
#include "ps.h"

#ifdef NOTDEF
	this typedef is in ps.h and is replicated here for
	easy reference:

typedef struct pstab_s {
		/* the ascii string naming the pseudo-op..
		   for entry into a hash table
		*/
		char *name;
		/* for linking within a bucket in the
		   hash table
		*/
		struct pstab_s *next;
		/* pointer to function to 
		   process the pseudo-op 
		*/
		int (*psfunc)();
		/* two arguments to pass to the function */
		int arg0,arg1;
		} pstab_t ;
#endif


#define bzero(a,b) 



ascii(t,isnullterm) 
int isnullterm;
token_t *t;
{
	/*  an asci[iz] directive has been found.  Enter the 
	    bytes into the current control section.

	    an asciz directive differs from an ascii directive only
	    in the addition of a terminal null byte.
	*/


	register char * cptr; 
	char *end;
	register char c;
	register unsigned char *bin = binary.chars;
	int nbytes;


	/* check for string argument in the token list */
	if (((*t) == (token_t)0) || ((*t)->tokennum != T_STRING))
	{
		error(*t,".ascii[ z] directive must have string argument");
		errors_in_statement++;
		return(0);
	}

	/* delimit the string argument */
	cptr = (*t)->u.cptr;
	end = cptr + (*t)->length;

	/* we simply store back into the input buffer as we
	   translate the string, and copy the resultant string
	   into the codespace at the end of the statement. For
	   this we use psstr, which is used as the source by
	   do_statement if it is non-null
	*/
	psstr = cptr;

	/* copy the string data into the binary buffer */
	while (cptr < end)
	{
		c = *cptr;
		if (c == '\\')
		{
			cptr = doesc(cptr,end);
			if (escval != (-1)) *psstr++ = escval;
		}
		else *psstr++ = *cptr++;
	}

	/* null terminate if asciz */
	if (isnullterm) *psstr++ = 0;

	/* calculate the number of bytes in the string */
	nbytes = psstr - (*t)->u.cptr;

	/* update the current location counter */
	update_dot(nbytes);

	/* and restore the pointer to the beginning of the string */
	psstr = (*t)->u.cptr;

	/* this statement is byte-aligned. */
	cur_stat->sinfo.isbytealigned = 1;
}
#ifdef NOTDEF	/* old code */
ascii(t,isnullterm) 
int isnullterm;
token_t *t;
{
	/*  an asci[iz] directive has been found.  Enter the 
	    bytes into the current control section.
	*/


	register char * cptr; 
	char *end;
	register char c;
	register unsigned char *bin = binary.chars;
	int nbytes;


	if (((*t) == (token_t)0) || ((*t)->tokennum != T_STRING))
	{
		error(*t,".ascii[ z] directive must have string argument");
		errors_in_statement++;
		return(0);
	}

	cptr = (*t)->u.cptr;
	end = cptr + (*t)->length;

	while (cptr < end)
	{
		c = *cptr;
		if (c == '\\')
		{
			cptr = doesc(cptr,end);
			if (escval != (-1)) *bin++ = escval;
		}
		else *bin++ = *cptr++;
	}

	if (isnullterm) *bin++ = 0;
	nbytes = bin - binary.chars;
	update_dot(nbytes);
	cur_stat->sinfo.isbytealigned = 1;
}
#endif

/* NOTE -- this must be the same as the instance in inst.h **/
struct op_immabs
#ifdef NOTDEF
{
	short isnrexpr,isunsigned;
	/*
	symtabptr sym;
	*/
	struct loc_s * sym;
	/* if the sym structure is not null, the
		  address is an offset
	*/
	long addr;
} 
#endif
dummyop;

#ifdef EXPERIMENTAL

static struct tokentype_s dummycomm_s[3];
static token_t dummycomm[4] = { 
		&dummycomm_s[0],
		&dummycomm_s[1],
		&dummycomm_s[2],
		(token_t)0
};
#endif


byteword(tokenptr,elemsz,initialized)
token_t *tokenptr;
{
	/*  a pseudo op for initialized or non-initialized data
	    has been encountered, i.e. .space, .long, .byte,
	    .word.  There are 2>>elemsize 
	    bytes per element of the datum.  As there is
	    a maximum of MAX_TOKEN tokens and MAX_TOKEN>>1 longs
	    in the binary array, code can be put directly into
	    the binary array.  Uninitialized space is set to zero.
	*/
	union binary u;
	long num,num1,newsymbol;
	int nb;
	symtabptr sym;
	short s;
	int nelems=0;


	dummyop.isnrexpr = 0;
	dummyop.addr = 0;
	u.l = binary.longs;

	if (elemsz == 1) cur_stat->sinfo.isbytealigned = 1;

	if (!initialized)
	{
		/* .space directive */
		/* the only allowable form here is elemsz = 1 */
		if (elemsz != 1)
			fatal((*tokenptr),
			  "byteword called to zero non-bytes");

		/* get the number of bytes to zero */
		if ((*tokenptr)->tokennum != T_NUMBER)
		{
			error((*tokenptr),
			  "length of zeroed data must be numeric");
			nb = 0;
		}
		else if ((*(tokenptr + 1)) != (token_t)0) {
			error((*(tokenptr+1)),
"illegal size specification for .space directive. - simple number expected.");
			nb = 0;
		}
		else
			nb = buildnum((*tokenptr)->u.cptr);
		cur_stat->sinfo.iszero = 1;
	}

	else {
#ifdef EXPERIMENTAL
		/* check that we are NOT in the bss segment */
		if (cur_csect == BSS_CSECT) {
			warning(*tokenptr,
		    "cannot initialize bss.  .comm directive substituted.");
		}
#else
		/* check that we are NOT in the bss segment */
		if (cur_csect == BSS_CSECT) {
			warning(*tokenptr,
		    "cannot initialize bss. - initialization ignored.");
		}
#endif
		cur_stat->sinfo.isunsigneddata = 1;
		if (elemsz == 1)
			cur_stat->sinfo.isbytedata = 1;
		else if (elemsz == 2)
			cur_stat->sinfo.isworddata = 1;

		/*  check for an odd address */
		if (elemsz != 1)
		{
			if (!align(0,2))
			{
				error(*tokenptr,
				  "odd address");
				cur_stat->sinfo.mustpad = 1;
			}
		}
		/*  get successive data items to place in the buffer */
		while ((*tokenptr) != (token_t)0)
		{
			num = 0;
			dummyop.sym = (struct loc_s *)0;
			dummyop.addr = 0;

			switch ((*tokenptr)->tokennum)
			{
			case T_NUMBER:
			case T_MINUS:
			case T_DOT:
			case T_ALPHA:
				dummyop.isunsigned = 1;
				dotokennum(&tokenptr,&dummyop);
				if (dummyop.sym != (struct loc_s *)0)
					/* fix the stat_user chain with 
					   the size, offset, and whether
					   the update is non-relocatable
					   (it is relocatable ONLY if 
					   the value of the symbol must be
					   ADDED in: if the value has to
					   be complemented, or negated, it
					   is non-relocatable)
					*/
					fix_statusers(dummyop.sym,cur_stat,
						nbinary,4,
						(dummyop.isnrexpr)?ISNREXPR:0);
				num = dummyop.addr;
				break;
			default:
				error(*tokenptr,
			"initialization data must be symbolic or numeric");
				/* skip this initialization */
				while ((*tokenptr != (token_t)0) &&
					((*tokenptr)->tokennum != T_CM))
					tokenptr++;
				if ((*tokenptr)->tokennum == T_CM) tokenptr++;
			}

			/* ok, put the current data value in the code.
			   The code section may still need to be 
			   updated as a relocatable.
			*/
#ifdef NOTDEF
			switch (elemsz)
			{
				case 1:	*u.c++ = num;
					break;
				case 2: *u.s++ = num;
					break;
				case 4: *u.l++ = num;
					break;
				default:
					fatal((*tokenptr),
					  "illegal elemsz to byteword");
			}
#else
			*u.l++ = num;
			nelems++;
#endif
			/* past the comma */
			if (((*tokenptr) != (token_t)0) &&
			    ((*tokenptr)->tokennum == T_CM))
				tokenptr++;
		}

		/* get the amount of space used */
#ifdef NOTDEF
		nb = u.c - binary.chars;
#else
		nb = nelems * elemsz;
#endif
	}
	/* and update the location counter correctly */
#ifdef EXPERIMENTAL
	if (cur_csect == BSS_CSECT) {
		u.c = binary.chars;

		/* ok, we need to fake up a 
			.comm  <sym>,nb 
		   directive for the current statement.
		*/
		if (cur_stat->lab == (struct symtab_s *)0) {
			error(0,
      "cannot fake .comm directive for initialized bss if there's no label.");
			errors_in_statement++;
			return;
		}
		/* fake up a .comm directive */
		dummycomm_s[0].u.cptr = cur_stat->lab->name;
		sprintf(dummycomm_s[2].u.cptr,"%d",nb);
		undefine_last_label();
		comm_globl(dummycomm,TRUE);
	}
	else
#endif
		update_dot(nb);


}

align(tokenptr,alignsz)
{
	/* simply force the current csect to be aligned 
	   on a boundary of the passed number of bytes.
	   returns 1 if the section was already aligned

	   called with alignsz=2 when the .even pseudo-op
	   is encountered, and with alignsz=4 when the .quad 
	   pseudo-op is encountered.
	*/

	int nb;
	if (alignsz == 2) 
	{
		if (dot[cur_csect] & 1) 
		{
			binary.chars[0] = 0;
			update_dot(1);
			cur_stat->sinfo.isbytealigned = 1;
			return(0);
		}
	}
	else {
		nb = dot[cur_csect]%alignsz;
		if (nb)
		{
			bzero(&binary,nb);
			update_dot(nb);
			cur_stat->sinfo.isbytealigned = 1;
			return(0);
		}
	}
	return(1);
}


change_csect(tokenindx,new_csect)
{
	/* simply change the current control section.
	   called in response to .text, .data, .bss
	*/

#ifdef NOTDEF
	/* if we are changing csects, the user temporary
	   symbols need to be invalidated.
	*/
	if (cur_csect != new_csect) delete_usertemps();
#endif
	cur_csect = new_csect;
}

comm_globl(tokenptr,iscomm)
token_t *tokenptr;
{
	/* declare the string found in the indicated token as
	   a symbol with the given attributes.

	   iscomm = 1 if .comm, else .globl
	*/
	tokentype *token;
	register symtabptr sym;
	long len;
	int newsymbol;
	short s;

	token = *tokenptr;

	/*  consistency check.  Must be string in the next token. */
	if ((token == (token_t)0) || 
	   (token->tokennum != 0)||(token->u.cptr == (char *)0))
	{
		error(token,"illegal argument to .{comm,globl}");
		errors_in_statement++;
	}

	if (iscomm)
	{
		/*  check that a length exists on the .comm directive
		*/
		tokenptr++;
		if (((*tokenptr) == (tokentype *)0) ||
		    ((*tokenptr)->tokennum != T_CM))
		{
			error((*tokenptr),
				"no length for .comm");
			errors_in_statement++;
			return(0);
		}
		tokenptr++;
		if (((*tokenptr) == (tokentype *)0) || 
			((*tokenptr)->tokennum != T_NUMBER))
		{
			error((*tokenptr),"no length for .comm");
			errors_in_statement++;
			return(0);
		}
		len = buildnum((*tokenptr)->u.cptr);
	}
	do {
		/* get the symbol */
		sym = getsym(token,&newsymbol);
		if ((!newsymbol)&&(iscomm)&&(sym->syminfo.csect != BSS_CSECT)&&
			(sym->syminfo.csect != UNDEF_CSECT))
			/* redeclaration doesnt match.  use the new one */
		        warning((*tokenptr),
				 ".comm symbol previously declared.");

		/* both comm and globl are externals */
		sym->syminfo.isextern = 1;
		if (iscomm)
		{
			/* .comm */
			sym->syminfo.csect = BSS_CSECT;
			sym->addr = len;
			/* it is undefined */
			sym->syminfo.isundefined = 1;
		}
		else if (sym->syminfo.isundefined) {
			/* .globl which has not been defined */
			sym->syminfo.csect = UNDEF_CSECT;
			sym->addr = 0;
		}

		token = *(++tokenptr);
		if ((token == (token_t ) 0) || (token->tokennum != T_CM))
			break;
		else token = *(++tokenptr);

		/*  consistency check */
		if ((token == (token_t)0) || 
	   		(token->tokennum != 0)||(token->u.cptr == (char *)0))
		{
			error(token,"illegal argument to .{comm,globl}");
			errors_in_statement++;
			break;
		}
	} 
	/* only loop if .globl */
	while (!iscomm);
}

dobreak()
{
	/* .break is a pseudo-op which can be used for debuggine
	   the assembler.  This routine is called when the pseudo-op
	   is parsed, and a breakpoint can be set here.  Just put
	   the pseudo-op in at the appropriate spot in the .s file
	*/
}


/* external pseudo-op handlers.  */
int stab();

/* abort() handles .abort.  This is put out by the compiler if
   it is piping its output to the assembler and wants to abort the
   process. as20 exits with an error.
*/
int abort();

static pstab_t pslist[] = {
      /* the ascii[ z] directive */
	{".ascii",(pstab_t *)0,ascii,0,0,},
	{".asciz",0,ascii,1,0},

      /* data value in code directives */
	{".byte",0,byteword,1,1},
	{".word",0,byteword,2,1},
	{".long",0,byteword,4,1},
	{".space",0,byteword,1,0},

      /* csect directives */
	{".text",0,change_csect,TEXT_CSECT,0},
	{".data",0,change_csect,DATA_CSECT,0},
	{".bss",0,change_csect,BSS_CSECT,0},

      /* arg is for iscomm */
	{".globl",0,comm_globl,0,0},
	{".comm",0,comm_globl,1,0},

      /* really .align 1 in disguise */
	{".even",0,align,2,0},
	{".walign",0,align,2,0},
	{".lalign",0,align,4,0},

      /* debugging pseudo-op */
	{".break",0,dobreak,2,0},

      /* this pseudo-op is issued by the compiler if
	 it has to abort.  The assembler just exits with an error.
      */
	{".abort",0,abort,0,0},

      /* stabs pseudo-ops */
	{".stabn",0,stab,0,0},
	{".stabs",0,stab,1,0},
	{".stabd",0,stab,0,1},
	{0} };



/* maximum number of pseudo-ops */
#define NPSEUDO 20

init_ps() 
{
	/* initialize the pseudo-op hash table */
	register int indx;
	register pstab_t *ps;
	for (indx=0;indx<NPSBUCKETS;indx++)
		ps_bucket[indx] = (pstab_t *)0;

	/* ensure that the ps string is null (for ascii[ z] 
	   directives) at the start
	*/
	psstr = 0;

	ps = pslist;
	while (ps->name != (char *)0)
	{
		enter(ps,NPSBUCKETS,ps_bucket);
		ps++;
	}
	
#ifdef EXPERIMENTAL
	dummycomm_s[0].tokennum = T_ALPHA;
	dummycomm_s[1].tokennum = T_CM;
	dummycomm_s[2].tokennum = T_NUMBER;
	dummycomm_s[2].u.cptr = "                ";
#endif
}
