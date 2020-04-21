#include <stdio.h>
#include "globals.h"
#include "sym.h"
#include "tokens.h"


struct statement_s * stat_head;
symtabptr sym_bucket[] = {0};

init_sym()
{
	cur_csect = TEXT_CSECT;
	dot[TEXT_CSECT] = 0;
	dot[DATA_CSECT] = 0;
	dot[BSS_CSECT] = 0;
	last_stat = stat_head = allocate_statement();
}


enter(element,nelem,list)
generic_t *element,*list[];
{
	/*  element is to be entered on hash chain chain */
	int bucket = hash(nelem,element->name);

	generic_t *temp = list[bucket];
	list[bucket] = element;
	element->next = temp;
}

generic_t *
lookup(cptr,nelem,list)
char *cptr;
generic_t *list[];
{
	/*  look up the character string cptr to see if it is
 	    on one of the chains pointed at by list[]
	*/
	int bucket = hash(nelem,cptr);
	register generic_t *temp = list[bucket];
	register char *src,*dest;

	while (temp != (generic_t *)0)
	{
		src = cptr;
		dest = temp->name;
		while ((*src == *dest))
		{
			if (!*src++) 
				return(temp);
			dest++;
		}
		temp = temp->next;
	}
	return((generic_t *)0);
}

static struct loc_s *last_deflab;

symtabptr 
define_label(token,stat,addr)
tokentype *token;
struct statement_s *stat;
unsigned long addr;
{
	/*  define a label for the current statement 
	    using the given token. A pointer to the
	    entry in the symbol table is returned.
	    the passed address is attached to the label.
	    the label is entered at the end of the deflab_updatelist.
	*/
	register symtabptr sym;
	int newsymbol;

	sym = getsym(token,&newsymbol);
	if (!newsymbol)
	{
		/* okay, the symbol previously existed.  If it has 
		   an address, it is in error.  
		*/
		if (!(sym->syminfo.isundefined) )
		{
#ifdef NOTDEF
			if (sym->syminfo.istemp)
			{
				/* the symbol may have been a usertemp. check */
				symtabptr tsym;
				if ((tsym = 
					(symtabptr)lookup(token->u.cptr,
						NSYMBUCKETS,sym_bucket)) == 
						(symtabptr)0)
					fatal(token,
					   "second lookup of temp sym failed");
				/* check the real temporary symbol */
				if (tsym->syminfo.isusertemp) 
				{
					sym = tsym;
					goto usertemp;
				}
			}
#endif
			error(token,"label %s is previously defined",
				token->u.cptr);
			return((symtabptr)0);
		}

	}
usertemp:


	sym->syminfo.isundefined = 0;
	sym->u.line = stat->line;
	sym->syminfo.csect = stat->sinfo.csect;
	sym->addr = addr;

	sym->def = stat;

	last_deflab = deflab_updatelist_tail;

		/*  add an entry onto the deflab_updatelist.  */
		/*  if there is no head, allocate one, and set the tail to it */
		if (deflab_updatelist_head == (struct loc_s *)0)
			deflab_updatelist_head = deflab_updatelist_tail = 
				allocate_locs();
		else {
			deflab_updatelist_tail->next = allocate_locs();
			deflab_updatelist_tail = deflab_updatelist_tail->next;
		}
		/* and now update the tail to point to this symbol */
		deflab_updatelist_tail->stat = (struct generic_s *)sym;
		
		/* and, lastly, delete the active user temporary labels, 
		   as we have entered a new local symbol block
		*/
		if (!sym->syminfo.istemp ) delete_usertemps();

	return(sym);
}

undefine_last_label()
{
	symtabptr sym;
	/* undefine the last label.  It was bogus. */
	if   (deflab_updatelist_tail == (struct loc_s *)0)
		fatal(0,"attempt to undefine label when there isn't one");

	if (last_deflab == (struct loc_s *)0)  {
	    if (deflab_updatelist_tail != deflab_updatelist_head)
		fatal(0,"last label inconsistent");
	}
	else if (last_deflab->next != deflab_updatelist_tail) {
		fatal(0,"last label inconsistent");
	}


	sym = (symtabptr)deflab_updatelist_tail->stat; 

	sym->syminfo.isundefined = 1;
	sym->u.line = 0;
	sym->syminfo.csect = UNDEF_CSECT;
	sym->addr = 0;

	sym->def = (struct statement_s *)0;

	if (last_deflab == (struct loc_s *)0) {
		deflab_updatelist_head = deflab_updatelist_tail = 
			(struct loc_s *)0;
	}
	else {
		deflab_updatelist_tail = last_deflab;
		deflab_updatelist_tail->next = (struct loc_s *)0;

		/* ok, find the new last label... */
		last_deflab = deflab_updatelist_head;
		if (last_deflab->next == (struct loc_s *)0) {
			/* head == tail.  No last label */
			last_deflab = (struct loc_s *)0;
		}
		else {
			/* head != tail.  Need to follow chain */
			while (last_deflab->next != deflab_updatelist_tail) {
				last_deflab = last_deflab->next;
				if (last_deflab == (struct loc_s *)0)  
					fatal(0,"last label inconsistent");
			}
		}
	}

}

static struct loc_s *usertemp_head = 0,*usertemp_tail= 0;

add_usertemp(sym)
register symtabptr sym;
{
	/* the user has defined a temporary label ( of the form
	   <num>$ ).  This has had an internal temporary label
	   (of the form #T<num2>) assigned to it, and linked through
	   a fudge of the sym_users field in the symtab entry of 
	   symbol <num>$.  We must keep track of these user temporary 
	   labels so that the links between them and the real labels
	   can be removed when we enter a new local symbol block
	   (this is done in delete_usertemps()).
	*/

	register struct loc_s *temp = allocate_locs();
	if (usertemp_head == (struct loc_s *)0)
		usertemp_head = usertemp_tail = temp;
	else {
		usertemp_tail->next = temp;
		usertemp_tail = temp;
	}
	temp->next = (struct loc_s *)0;
	temp->stat = (struct generic_s *)sym;
}


delete_usertemps()
{
	register struct loc_s *temp = usertemp_head;
	register symtabptr sym;

	while (temp != (struct loc_s *)0)
	{
		sym = (symtabptr)temp->stat;
		sym->u.sym_users = 0;
		temp = temp->next;
	}
	/* so long, memory.... */
	usertemp_head = 0;
}




hash(nelem,cptr)
register unsigned char *cptr;
{
	/*  hash the string and return a hash index mod'd by nelem
	    (number of elements in chain  ) 
	*/
	register unsigned long l=0;

	while (*cptr) 
	{
		l <<= 1 ;
		l += *cptr++;
	}
	return (l %= nelem) ;
}

sym_equates(t)
token_t *t; 
{
	/* beginning at tokenlist[tokenindx], a list of equates
	   appears on the current line.  Enter them into the 
	   symbol table.  Currently, only absolute symbols may
	   be equated.
	*/


	token_t *t1 = &tokenlist[ntokens-1];
	token_t *tsave = t;
	int opnext,isminus;
	int newsymbol=0;
	int num;
	tokentype *token = *t;
	tokentype *symtoken;
	tokentype **lasteq;
	register symtabptr sym,targetsym;
	long targetaddr;
	char symchar;
	short s;

	if (ntokens <3) errors_in_statement++;
	else
	    do {
		if ((*t)->tokennum == T_ALPHA)
		{
			/* equate found */
			t++;
			if (*t == (tokentype *)0)
				/* line is okay */
				break;
			if ((*t)->tokennum == T_EQ)
				/* another equate... */
			{	lasteq = t++; continue; }
			/* ok.. this must be the last equate. check it */
			while (*t != (token_t)0)
			{
				if ((*t)->tokennum == T_EQ)
					errors_in_statement++;
				t++;
			}
			break;
		}
		else if (((*t)->tokennum == T_DOT))
		{
			t++;
			/* this must be the last equate */
			while (*t != (token_t)0)
			{
				if ((*t)->tokennum == T_EQ)
					errors_in_statement++;
				t++;
			}
			break;

		}
		else if (((*t)->tokennum == T_NUMBER)||
			 ((*t)->tokennum == T_MINUS)||
		         (*(t+1) == (tokentype *)0))
		{
			/*  equate is good.. last element was number */
			break;
		}
		else errors_in_statement++;
	    } while ((!errors_in_statement) && (*t != (tokentype *)0));

	token = *t;
	if (token == (token_t)0) token = *(t-1);
	if (errors_in_statement)
	{
badsyntax:
		errors_in_statement = 1;
		error(token,
		  "badly formed direct assignment statement");
		return(0);
	}

	/*  ok, the line seems to be in the proper format.  Munch it */
	/*  the target expression begins at lasteq + 1 */
	t1 = lasteq - 1;
	lasteq++;

	targetaddr = 0;
	isminus = 0;
#define NEITHER 3
	opnext = NEITHER;
	while ((token = (*lasteq++)) != (token_t)0)
	{
		if (token->tokennum == T_PLUS) 
		{
			if (opnext != TRUE) goto badsyntax;
			opnext = FALSE;
			continue;
		}
		if (token->tokennum == T_MINUS) 
		{
			isminus++; 
			if (opnext != TRUE) goto badsyntax;
			opnext = FALSE;
			continue;
		}
		if (opnext == TRUE) goto badsyntax;
		opnext = TRUE;
		if (token->tokennum == T_NUMBER)
		{
			num = buildnum(token->u.cptr);
			if (isminus) num = (-num);
			targetaddr += num;
			isminus = 0;
			continue;
		}
		else if (token->tokennum == T_DOT)
		{
			/*  define a temp label to attach to the current 
		    	statement as a stat_user.  If the current csect
			is TEXT, this label is the
			single allowed undefined.  If the current csect
			is DATA, DOT is correct.
			*/

			if (cur_csect == DATA_CSECT)
			{
				num = dot[cur_csect];
				if (isminus) num = (-num);
				targetaddr += num;
				isminus = 0;
				continue;
			}
			if (targetsym != (symtabptr)0)
			{
				error(token,
"symbol cannot be equated to expression with more than one relocatable/undefined");
				errors_in_statement++;
				return(0);
			}
			else if (cur_csect != DATA_CSECT)
				warning(token,
"current location counter (\".\") in csect other than DATA may be incorrect");
			if (cur_stat->lab == (symtabptr)0)
			{
				cur_stat->lab = 
		    			define_label(allocate_templab(),
						cur_stat,curstat_labaddr);
				cur_stat->lab->syminfo.istemp = 1;
			}
			targetsym = cur_stat->lab;
		} 
		else if (token->tokennum == T_ALPHA)
		{
			sym = getsym(token,&newsymbol);
			if (!newsymbol)
			{
				if (sym->syminfo.csect == UNDEF_CSECT)
					sym->syminfo.csect = ABS_CSECT;
				if (sym->syminfo.isundefined)
				{
					if (sym->syminfo.csect == ABS_CSECT)
					{
						/* if we have our single undefined
					   	already, this is an error.  Otherwise
					   	link the single sym user in 
						*/
						if (targetsym == (symtabptr)0)
							targetsym = sym;
						else
						{
							error(token,
"symbol cannot be equated to expression with more than one forward-referenced absolute symbol");
							errors_in_statement++;
							return(0);
						}
					}
					else {
						error(token,
"symbol cannot be equated to expression involving relocatable symbols");
						errors_in_statement++;
						return(0);
					}

				}
				else {
					if (sym->syminfo.csect != ABS_CSECT)
					{
#ifdef NOTDEF
						if (targetsym == (symtabptr)0)
							targetsym = sym;
						else
						{
							error(token,
"symbol cannot be equated to expression with more than one relocatable/undefined");
							errors_in_statement++;
							return(0);
						}
#endif
						error(token,
"symbol cannot be equated to expression involving relocatable symbols");
						errors_in_statement++;
						return(0);
					}
					num = sym->addr;
					if (isminus) num = (-num);
					targetaddr += num;
					continue;
				}
			} 
			else {
				sym->syminfo.csect = ABS_CSECT;
				sym->syminfo.isundefined = 1;

				/* add in the new single sym_user if
				   there is not one, else give an error 
				*/
				if (targetsym == (symtabptr)0)
					targetsym = sym;
				else
				{
					error(token,
"symbol cannot be equated to expression with more than one forward reference");
					errors_in_statement++;
					return(0);
				}
			}
		}
		else goto badsyntax;
	}

#ifdef NOTDEF
	/*  the token we are defining TO is the last one */
	symtoken = tokenlist[ntokens-1];

	/* is it a number? */
	symchar = symtoken->u.cptr[0];
	if ((symchar >= '0')&&(symchar <= '9'))
	{
		short i;
		targetaddr = buildnum(symtoken->u.cptr)
		newsymbol = 0;
		targetsym = (symtabptr)0;
	}
	else 
	{
		/* look for it */
		targetsym = getsym(symtoken,&newsymbol);
		if (!newsymbol) 
		{
			if (targetsym->syminfo.csect != ABS_CSECT)
			{
				if (targetsym->syminfo.csect != UNDEF_CSECT)
				{
					error(symtoken,
			  		"equated symbol must be absolute");
					return(0);
				}
			}
			targetaddr = targetsym->addr;
		}
		else 
			/* UNDEFINED BUG */
			targetsym->syminfo.isundefined = 1;

		targetsym->syminfo.csect = ABS_CSECT;
		cur_stat->smulti.equatesym = targetsym;

	}
#endif
			
	/* at this point, if there is a targetaddr, use it, else
	   build the symuser chain
	*/
	while (t1 >= tsave)
	{
		/* equate the next symbol to the target */
		token = *t1;
		sym = getsym(token,&newsymbol);
		if (!newsymbol)
		{
			/*
			if ((!sym->syminfo.isabs)&& 
			    (sym->syminfo.csect != UNDEF_CSECT))
			*/
			if (sym->syminfo.csect != ABS_CSECT)
			{
				if (sym->syminfo.csect == UNDEF_CSECT)
					sym->syminfo.csect = ABS_CSECT;
				else {
					error(token,
					    "equated symbol must be absolute");
					goto bump;
				}
			}
		}
		else sym->syminfo.csect = ABS_CSECT;
		/*sym->syminfo.isabs = 1;*/
		/* fold in any constants found */
		sym->addr = targetaddr;
		if (targetsym == (symtabptr)0) 
		{
			/* the target has been defined.  copy the definition */
			sym->syminfo.isundefined = 0;
			if ((sym->addr >= (-32768)) && (sym->addr <= 32767))
				sym->syminfo.isword = 1;
		}
		else {
			/* the target has not been defined.  Must link
			   the equated symbols through the sym_users chain
			*/
			add_symuser(targetsym,sym);
			/* the symbol is not yet defined */
			sym->syminfo.isundefined = 1;
		}
bump:
		t1 -= 2;
	}
}

#define SYMUSER_UPDATE 1
#define STATUSER_UPDATE 2

add_symuser(symbol,element)
register symtabptr symbol;
generic_t *element;
{
	/*  add a user of the passed symbol.
	    The element is a pointer to the user
	*/
	
	struct loc_s *loc;

	loc = allocate_locs();

	loc->next = symbol->u.sym_users;
	if (symbol->u.sym_users == (struct loc_s *)0)
		add_updatelist(SYMUSER_UPDATE,symbol);
	symbol->u.sym_users = loc;
	symbol->syminfo.isundefined = 1;
	loc->stat = element;
	return(0);
}


int next_statuserno=0;

fix_statusers(suchain,stat,offset,size,
/* isbitsz,isdisp, canbecondensed, symisneg, */ flags,
		wordbit,wordword)
/* VARARGS */
struct loc_s * suchain;
struct statement_s *stat; /* really a statement_s * */
int offset,size,/*isbitsz,isdisp,canbecondensed,*/ wordbit,wordword;
register int flags;
{
	/*  add the passed information to the stat user
	    chain passed as sym.  This chain was attached to
	    an operand segment earlier (by add_statuser), 
	    and now must be reworked.  Each element of the
	    chain currently has the symbol it references in
	    its stat element.  This loc_s structure must be 
	    attached to the stat_user chain of that symbol.
	    Each of the elements must be updated with the offset,
	    stat, size, and flags passed.
	*/
	register struct loc_s *cursu = suchain;
	register struct loc_s *temp;

	if (stat->sulist == (struct loc_s *)0)
		add_updatelist(STATUSER_UPDATE,stat);
	do 
	{
		next_statuserno++;
		temp = stat->sulist;
		stat->sulist = cursu;
		cursu->offset = offset;
		cursu->size = size;
		if (flags & ISBITSZ) cursu->s.isbitsize = 1;
		if (flags & ISDISP) cursu->s.isdisp = 1;
		if (flags & ISNREXPR) cursu->s.isnrexpr = 1;
		if (flags & CANBECONDENSED)
		{
			cursu->s.canbecondensed = 1;
			if (!(flags & ISDISP))
				cursu->s.iswordbit = (wordword<<8)|wordbit;
		}
		else 
			cursu->s.canbecondensed = 0;
		if (flags & CANBEBYTEDISP)
		{
			cursu->s.canbebytedisp = 1;
		}
		else
			cursu->s.canbebytedisp = 0;

		if (flags & SYMISNEG) cursu->s.symisneg = 1;

		cursu = cursu->next;
		stat->sulist->next = temp;
	} while (cursu != (struct loc_s *) 0);
	

	return(0);
}



add_updatelist(type,symbol)
int type;
symtabptr symbol;
{
	/* a side list is kept of those symbols which statements 
	   and symbols reference.
	   Prior to code generation, these symbols must be run through
	   and the references updated.
	*/
	struct loc_s **update_head;
	struct loc_s * su;

	if (type == SYMUSER_UPDATE) update_head = &symuser_updatelist_head;
		else update_head = &statuser_updatelist_head;
	su = allocate_locs();
	su->next = (*update_head);
	su->stat = (struct generic_s *)symbol;
	(*update_head) = su;
}

#ifdef DEBUG
dump_symtab()
{
	/*  dump the symbol table.  Simply run through each of the
	    symbol buckets and dump each symbol 
	*/

	int indx;
	int nlines=0;
	symtabptr sym,*symbptr;
#define LINESPERPAGE 20
	symbptr = sym_bucket;
	for (indx = 0;indx < NSYMBUCKETS;indx++)
	{
		sym = sym_bucket[indx];
		if (sym == (symtabptr)0)
			continue;

		do {
		
			if (!(nlines % LINESPERPAGE))
			{
				putc('\n',stderr);
				header();
			}
			dump_symbol(sym);
			nlines++;

		} while ((sym = sym->next) != (symtabptr)0);
	}
	{
		struct loc_s *temp = statuser_updatelist_head;
		fprintf(stderr,"\nSTATEMENTS with a STATUSER updatelist:\n");
		while (temp != (struct loc_s *)0)
		{
			fprintf(stderr,"#%d -> ",temp->stat->num);
			temp = temp->next;
		}
		temp = symuser_updatelist_head;
		fprintf(stderr,"\nSYMBOLS with a SYMUSER updatelist:\n");
		while (temp != (struct loc_s *)0)
		{
			fprintf(stderr,"%s -> ",temp->stat->name);
			temp = temp->next;
		}
	}
}

header()
{
	fprintf(stderr,
       /*.........*.........*.........*.........*.........*.........*/	
	"\nname              addr      defline   (flags)\n");
}

char csect_char[6] = {'U','T','D','B','A','z'};

dump_symbol(sym)
symtabptr sym;
{
	/*  dump the symbol sym.   */
	struct loc_s * s = sym->u.sym_users;

	fprintf(stderr,"%-15.15s  0x%8.8x  %6.6d  %c%c%c",sym->name,
		sym->addr,(sym->def == (struct statement_s *)0)?0:(sym->u.line),
		(sym->syminfo.isextern)?'E':' ',
		csect_char[sym->syminfo.csect],
		(sym->syminfo.isundefined)?'U':' ');

	if (sym->def == (struct statement_s *)0)
	{
		/*  progress down the sym_users chain */
		if (s!=(struct loc_s *)0)
		{
			fprintf(stderr,"\n  USERS: ");
			while (s != (struct loc_s *)0)
			{
				fprintf(stderr,"%s,",s->stat->name);
				s = s->next;
			}
		}
	}
#ifdef NOTDEF
	if ((s = sym->stat_users) != (struct loc_s *)0)
	{
		/* progress down the stat_users chain */
		while (s != (struct loc_s *)0)
		{
			fprintf(stderr,
		  "statement #%d(o=0x%x,s=%d,bit=%c,disp=%c,cond=%c,bit=0x%x),",
				s->stat->num,
				s->offset,s->size,(s->s.isbitsize)?'T':'F',
				(s->s.isdisp)?'T':'F',(s->s.canbecondensed)?'T':'F',
				s->s.iswordbit);
			s = s->next;
		}
	}
#endif
	putc('\n',stderr);
}

#endif
symtabptr 
getsym(token,newsymbol)
tokentype *token;
int *newsymbol;
{
	register symtabptr sym;

	*newsymbol = 0;
	if ((sym = 
		(symtabptr)lookup(token->u.cptr,
			NSYMBUCKETS,sym_bucket)) ==
			(symtabptr)0)
	{
		int usertempsym =
			(token->u.cptr[token->length-1] == '$');
		int tempsym = ((*(token->u.cptr) == '#')||(usertempsym));
		/* the newly equated symbol did not previously exist */
		sym = allocate_symbol(tempsym);
		if (tempsym)
		{
			sym->name = (char *)allocate_binary(token->length+1);
			sym->syminfo.istemp = 1;
			if (usertempsym) sym->syminfo.isusertemp = 1;
		}
		else
		{
			sym->name = (char *)allocate_strspace(token->length+1);
			sym->syminfo.strseg = next_strsegno - 1;
		}
		fastcopy(sym->name,token->u.cptr,token->length);
		enter(sym,NSYMBUCKETS,sym_bucket);
		sym->syminfo.isundefined = 1;
		*newsymbol = 1;
	}
	{
		/* If it is a user temporary label,
		   return the real symbol.
		*/
		symtabptr temp;
		if (sym->syminfo.isusertemp) {
			temp = (symtabptr)sym->u.sym_users;
			if (temp == (symtabptr)0)
			{
				/* label is a user temporary.  Define an
		   		internal temporary for it and overwrite
		   		the sym's sym_users entry with the real
		   		label pointer.  Return this real label pointer.
				*/
				tokentype *allocate_templab(),*temptoken;
				symtabptr tempsym;
				int newsym;

				sym->syminfo.isusertemp = 1;
				sym->syminfo.istemp = 1;
				temptoken = allocate_templab();
				tempsym = getsym(temptoken,&newsym);
#ifdef NOTDEF
/* it'd be nice if this worked, but the 
   def field of sym hasn't been filled in yet
*/
				/* make the #T<num> symbol point back to the
				   statement which has the original <num1>$ 
				   symbol for error reporting. */
				tempsym->def = sym->def;
#endif
				sym->u.sym_users = (struct loc_s *)tempsym;
				if (!newsym)
					fatal(token,
				   "reuse of templab %s",temptoken->u.cptr);
				/* add to the loc_s chain of the user temporary 
		   		labels active in this block */
				add_usertemp(sym);
		
				/* the real symbol is the internal temp */
				sym = (symtabptr)sym->u.sym_users;
				*newsymbol = 1;
			}
			else
				sym = temp;
		}
	}
	return(sym);
	
}
