#include <stdio.h>
#include "globals.h"
#include "sym.h"
#include "a.out.h"
#include "tokens.h"
#include "ps.h"

#define N_VALUE_OFFSET 8

struct stab_s 
{
	/* lab is a label attached to the stab so that its 
	   dot gets updated 
	*/
	struct symtab_s * lab;

	/* the stab string.  NULL if not a stabs */
	char  * stabstring;

	int i;

	/* the length of this binary data */
	int mustbezero;

	/* the current dot */
	unsigned long addr;

	struct 
	{
	    unsigned 
		:5,
		isstab:1,
		:4,
		:1,
		finalexpisdot:1,
		/* the csect this data belongs in */
		csect:4,
		/* the string segment that this stabs string is in */
		strseg:16;

	} sinfo;

	unsigned long pad; /* really the sulist */

	struct stab_s * next;

	struct nlist *stab_nlist;
};

static symtabptr last_stabsym = 0;

stab(t,hasstring,finalexpisdot)
token_t *t;
int hasstring,finalexpisdot;
{
	token_t *tokenptrsv;
	register char *cptr;
	long expval[4];
	struct stab_s *cur_stab = (struct stab_s *)cur_stat;
	symtabptr sym;
	int isminus=0;
	int nexprs;
	int i;

/*
 *
 *	Process stabs.  Stabs are created only by the f77
 *	and the C compiler with the -g flag set.
 *	We only look at the stab ONCE, during pass 1, and
 *	virtually remove the stab from the intermediate file
 *	so it isn't seen during pass2.  This makes for some
 *	hairy processing to handle labels occuring in
 *	stab entries, but since most expressions in the
 *	stab are integral we save lots of time in the second
 *	pass by not looking at the stabs.
 *	A stab that is tagged floating will be bumped during
 *	the jxxx resolution phase.  A stab tagged fixed will
 *	not be be bumped.
 *
 *	.stab:	Old fashioned stabs
 *	.stabn: For stabs without names
 *	.stabs:	For stabs with string names
 *	.stabd: For stabs for line numbers or bracketing,
 *		without a string name, without
 *		a final expression.  The value of the
 *		final expression is taken to be  the current
 *		location counter, and is patched by the 2nd pass
 *
 *	.stab{<expr>,}*NCPName,<expr>, <expr>, <expr>, <expr>
 *	.stabn		 <expr>, <expr>, <expr>, <expr>
 *	.stabs   STRING, <expr>, <expr>, <expr>, <expr>
 *	.stabd		 <expr>, <expr>, <expr> # . 
 */

	/* first, we have to parse the stab directive */
	tokenptrsv = t;
	if (hasstring)
	{
		if (((*t) == (token_t)0) || ((*t)->tokennum != T_STRING))
		{
			error(*t,".stabs directive must have string argument");
			errors_in_statement++;
			return(0);
		}
		cptr = (*t)->u.cptr;
		t++;
		if ((*t)->tokennum != T_CM)
		{
			error(*t,"ill-formed .stabs directive");
			errors_in_statement++;
			return(0);
		}
		t++;

	}

	nexprs = (finalexpisdot)?2:3;
	{
		
		for (i=0;i<=nexprs;i++)
		{
			expval[i] = 0;
#ifdef NOTDEF
			isminus = 0;

			while (((*t) != (token_t)0) &&
			       ((*t)->tokennum != T_CM))
			{
				isminus=0;
				if ((*t)->tokennum == T_MINUS)
				{
					isminus++;
					t==;
				}
				if ((*t)->tokennum == T_NUMBER)
				{
					num = buildnum((*t)->u.cptr);
					expval[i] += 
				}
				
			}
#endif
			if ((*t) == (token_t)0)
			{
				error((*(t-1)),"mal-formed stab directive");
				errors_in_statement++;
				return(0);
			}
			isminus=0;
			if ((*t)->tokennum == T_MINUS)
			{
				isminus++;
				t++;
				if ((*t) == (token_t)0)
				{
					error((*(t-1)),
					    "mal-formed stab directive");
					errors_in_statement++;
					return(0);
				}
			}
			if ((*t)->tokennum == T_NUMBER)
			{
				expval[i] = buildnum((*t)->u.cptr);
				if (isminus) expval[i] = -expval[i];
				t++;
			}
			else if ((*t)->tokennum == T_CM)
			{
				expval[i] = 0;
			}
			else if (((*t)->tokennum == T_ALPHA)&&(i == nexprs)&&
				 (!finalexpisdot))
			{
				/* last expression is label.  Value has
				   to be updated as a stat_user.
				*/
				symtabptr sym;
				int newsymbol;
				struct op_immabs opseg;
				sym = getsym(*t,&newsymbol);
				opseg.sym = 0;
				add_statuser(sym,&opseg,0);
				fix_statusers(opseg.sym,cur_stab,
					N_VALUE_OFFSET,4,ISNREXPR);
				t++;
			}
			if ((i != nexprs)&&
				(((*t)== (token_t)0)||((*t)->tokennum != T_CM)))
			{
				error((*t),"mal-formed stab directive");
				errors_in_statement++;
				return(0);
			}
			t++;
		}

	}

	t = tokenptrsv;
	if (hasstring)
	{
		register char * stabstring; 
		register char * end; 
		register char c;
		/* this may be too much string space if the stab string
	   	has escapes in it.  Since this hardly ever happens,
	   	we will ignore it and pay the cost of some extra junk
	   	in the string table.
		*/
		stabstring = (char *)allocate_strspace((*t)->length);
		cur_stab->stabstring = stabstring;
	
		end = cptr + (*t)->length;
		while (c = *cptr)
		{
			if (c == '\\')
			{
				cptr = doesc(cptr,end);
				if (escval != (-1)) *stabstring++ = escval;
			}
			else *stabstring++ = *cptr++;
		}
	
		*stabstring++ = 0;
	}
	else cur_stab->stabstring = 0;

	/* okay, the stab is correct. allocate the nlist structure for it
	*/
	stabsinsource = 1;
	cur_stab->sinfo.isstab = 1;
	cur_stab->sinfo.csect = TEXT_CSECT;
	cur_stab->stab_nlist = 
	    (struct nlist *)allocate_binary(sizeof(struct nlist));

	if (finalexpisdot)
	{
		cur_stab->sinfo.finalexpisdot = 1;
	}
	
	/* okay, now copy the expression's values into the stab_nlist
	   structure
	*/
    	cur_stab->stab_nlist->n_type = expval[0];
    	cur_stab->stab_nlist->n_other = expval[1];
    	cur_stab->stab_nlist->n_desc = expval[2];
	cur_stab->stab_nlist->n_value = expval[3];

	/* last, allocate the symbol for the stab, and link it into the
	   special stab bucket at the end of the symbol table.
	*/
	sym = allocate_symbol(0);
	sym->name = cur_stab->stabstring;
	sym->def = (struct statement_s *)cur_stab;
	sym->syminfo.isstab = 1;
	if (hasstring) 
		sym->syminfo.strseg = next_strsegno - 1;
	sym->next = 0;
	if (last_stabsym != 0) 
		last_stabsym->next = sym;
	else
		sym_bucket[NSYMBUCKETS] = sym;
	last_stabsym = sym;

}

makestab(sym,mynlist)
symtabptr sym;
struct nlist *mynlist;
{
	/*  copy the nlist info from the statement which defined
	    sym to mynlist (the buffer from which the symbols are
	    being written
	*/
	register struct stab_s *mystab = (struct stab_s *)sym->def;

	mynlist->n_type = mystab->stab_nlist->n_type;
	mynlist->n_other = mystab->stab_nlist->n_other;
	mynlist->n_desc = mystab->stab_nlist->n_desc;
	if (mystab->sinfo.finalexpisdot)
		mynlist->n_value = mystab->addr;
	else
		mynlist->n_value = mystab->stab_nlist->n_value;
	if (sym->name == 0)
		mynlist->n_un.n_strx = 0;


}

#ifdef NOTDEF
    if (opcode==i_stabs){
	/* have a string operand -- parse it a la Ascii(), and save it off */
	register char *cp;
	register nchars;
	char *sp;
	if (numops != 5 ){
	    SProg_Error( E_NUMOPS); return;
	}
	if (operands[0].type_o != t_string ) {
	    DEBUG("first stabs operand not a string\n");
	    SProg_Error( E_OPERAND); return;
	}
	nchars = 0;
	cp = (char *)operands[0].value_o;
	while (*cp++)++nchars;
	cp = (char *)operands[0].value_o;
	s->ch = sp = (char *) calloc( nchars +1, sizeof(char));
	if (sp==NULL) Sys_Error( "Out of string space\n", 0);
	...
	s->id = nchars;
}
    /* stuff the next three numbers without too much thought */

    if (opcode == i_stabd) {
	/* value is Dot */
	s->tag = STABFIXED;
	s->value = Dot; /* this looks like a bug if you consider sdi's */
    } else {
	/* value is either a label or its a number */
		if ((operands[opno].type_o != t_normal) && 
			(operands[opno].type_o != t_abss)){
	    DEBUG("stab last operand isn't `normal'\n");
	    SProg_Error(E_OPERAND); return;
	}
	switch (s->type) {
	case N_FUN:
	case N_STSYM:
	case N_LCSYM:
	case N_SLINE:
	case N_SO:
	case N_SOL:
	case N_ENTRY:
	case N_LBRAC:
	case N_RBRAC:
	case N_ECOML:
		/* it a label */
		if (!operands[opno].sym_o
		|| operands[opno].value_o != operands[opno].sym_o->value_s) {
		    DEBUG("stab last operand isn't just a label\n");
		    SProg_Error(E_OPERAND); return;
		}
		s->label = operands[opno].sym_o;
		s->tag = STABFLOATING;
		break;
	default:
		/* its a number */
		if ( operands[opno].sym_o) {
		    DEBUG("stab last operand isn't just a number\n");
		    SProg_Error(E_OPERAND); return;
		}
		s->value = operands[opno].value_o;
		s->tag = STABFIXED;;
		break;
	}
    }


          { s.n_type   = t->type;
            s.n_other  = t->other;
            s.n_desc   = t->desc;       
            s.n_value  = t->value;                /* zero for testing now */
            if (t->id)
               { s.n_un.n_strx = strcount;      /* assign string offset   */
                 strcount += t->id + 1;	    /* increment str location */
               }                                /* in string table.       */
            else s.n_un.n_strx = 0;             /* else if no string is   */
                                                /* present, assign 0.     */
            size += sizeof(s);
            fwrite(&s, sizeof(s), 1, file);
            t = t->next_stab;
#endif
