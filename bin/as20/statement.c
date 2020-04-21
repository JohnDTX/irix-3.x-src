#include <stdio.h>
#include "globals.h"
#include "sym.h"
#include "scan.h"
#include "tokens.h"
#include "ps.h"


init_statement()
{
	first_stat = allocate_statement();
	cur_stat = first_stat;
}


do_statement()
{

	/* process the next statement.  returns FALSE only if there was
	   no next statement.
	*/
	register linetype_t linetype;
	int labelpending;
	register tokenindx;
	register token_t *tokenptrptr;
	int nb;
	int hadlab;
	pstab_t * pselem;
	int (*func)();
	
	/*  if there is no current statement, allocate the next one */
	if (cur_stat == (struct statement_s *)0)
		cur_stat = allocate_statement();
	nbinary = errors_in_statement = 0;
	linenum++;
	/* parse_line tokenizes the line and returns the linetype and
	   a list of tokens */
	if ((linetype = parse_line()) == L_EOF) return(0);
#ifdef DEBUG
	if(debug)
	dump_tokens(linetype);
#endif

	if (linetype ==  L_ERROR)
	{
		/*  illegal character on input line */
		error(0,"illegal character in statement");
		errors_in_statement++;
	}
	if (linetype ==  L_TOOMANYTOKENS)
	{
		/* too many tokens on input line */
		error(tokenlist[MAX_TOKEN-1],
		      "too many tokens in statement - ignored");
		errors_in_statement++;
	}
	if ((int)linetype <= (int)L_IGNORE) 
	{
		return(1);
	}
	/* check for a label, and if it exists, enter it. */
	labelpending = 0;
	curstat_labaddr = dot[cur_csect];
	if ((ntokens >1) && (tokenlist[1]->tokennum == T_COLON))
	{
		/*  label on the line.  Enter it. */
		labelpending++;
		tokenindx = 2;
	} 
	else {
		tokenindx = 0;
	}

	tokenptrptr = &tokenlist[tokenindx];

	/*  NOTE: if a label is the ONLY item on the line,
	    the linetype has been deduced as L_STATEMENT 
	*/

	cur_stat->sinfo.csect = cur_csect;
	cur_stat->line = linenum;
	cur_stat->addr = curstat_labaddr;
	if (labelpending) {
		cur_stat->lab = define_label(tokenlist[0],cur_stat,curstat_labaddr);
		hadlab++;
	}
	else
		hadlab = 0;

	switch (linetype)
	{
	case L_STATEMENT: 
		if ((*tokenptrptr) != (token_t)0)
			/*  dont call inst for simple labels
			*/
			inst(tokenptrptr);
		break;
	
	case L_EQUATES:	  
		sym_equates(tokenptrptr);
		break;

	case L_PSEUDO:	  
		/*  find the pseudo-op in the table */
		if ((pselem = (pstab_t *)lookup((*tokenptrptr)->u.cptr,
		     NPSBUCKETS,ps_bucket)) == (pstab_t *)0)
		{
			error((*tokenptrptr),"unrecognizable pseudo-op");
			if (!labelpending) return(1);
		}
		else 
			(*(pselem->psfunc))
				(++tokenptrptr,pselem->arg0,pselem->arg1);
		break;
	}
	if (errors_in_statement)  {
		nbinary=0;
		if (!hadlab) return(1);
	}
	/*  fill in the current statement info */
	/*  the statement info should previously have
	    been filled in with the exception of the binary data,
	    the length, and, possibly, the csect
	*/
	{
	register unsigned char *src, *dest;
	cur_stat->len = nbinary;
	if ((nbinary)&&(!cur_stat->sinfo.iszero))
	{
		int nb = nbinary;
		if (cur_stat->sinfo.isunsigneddata) {
			/* really want nelements*4 bytes of space.. */
			if (cur_stat->sinfo.isbytedata)
				nb <<= 2;
			else if (cur_stat->sinfo.isworddata)
				nb <<= 2;
		}
		cur_stat->bin.c = dest = allocate_binary(nb);
		if (cur_stat->sinfo.mustpad) 
		{
			*dest++ = 0;
			nbinary--;
			nb--;
		}
		src = (psstr == (char *)0)?binary.chars:(unsigned char *)psstr;
		fastcopy(dest,src,nb);
	}
	}
	psstr = (char *)0;
	last_stat->next = cur_stat;
	last_stat = cur_stat;
	cur_stat = (struct statement_s *)0;
#ifdef DEBUG
	if(debug)
	dump_stat(last_stat);
#endif
	return(1);
}


#ifdef NOTDEF
{
	/* lab is a label defined by this statement. */
	struct symtab_s * lab;
	/* jumplab is the jump target for sdis, or the symbol
	   table entry for the equates
	*/
	union
	{
		/*  aux sym slot (for attaching .comm, .globl symbols) */
		struct symtab_s *auxsym;
		/*  the jump target for sdis */
		struct symtab_s * jumplab;
		/*  an error entry for listings 
		struct error_s * errorptr;
		*/
		/*  a symbol table pointer to one of the equated symbols */
		struct symtab_s * equatesym;
	} smulti;
};
#endif

#ifdef DEBUG
dump_stat(stat)
register struct statement_s *stat;
{
	int nb=stat->len;
	int ba=stat->sinfo.isbytealigned;
	int multiplier=1;
	int indx;
	struct loc_s *s;

	if (stat->sinfo.isunsigneddata) {
		if (stat->sinfo.isworddata) multiplier=2;
		else if (stat->sinfo.isbytedata) multiplier=4;
		nb *= multiplier;
	}
	
	fprintf(stderr,
	  "\nLINE = %d %s %s %s %s %s %s, csect = %d, address = 0x%x",
		stat->line,(stat->sinfo.issdi)?"ISSDI":"",
		(stat->sinfo.iszero)?"ZERO":"",
		(ba)?"BA":"",
		(stat->sinfo.isequates)?"EQ":"",
		(stat->sinfo.issdi)?"SDI":"",
		(stat->sinfo.isunsigneddata)?
		  ((multiplier == 2)?"(worddata)":
		      (multiplier == 4)?"(bytedata)":"(longdata)"):"",
		(stat->sinfo.csect),stat->addr);
	if (stat->lab != (symtabptr)0)
	{
		fprintf(stderr,"\nLABEL = %s, addr = 0x%x",stat->lab->name,
			stat->lab->addr);
	}
	if (stat->smulti.auxsym != (symtabptr)0)
	{
		if (stat->sinfo.isstab)
			fprintf(stderr,"\nSTABSTRING = %s",(char *)stat->smulti.auxsym);
		else
		fprintf(stderr,"\nAUX = %s addr = 0x%x",
		    stat->smulti.auxsym->name,stat->smulti.auxsym->addr);
	}

	if (!stat->sinfo.iszero)
	{
		if (stat->sinfo.isstab) nb = 12;
		if (ba) 
		{
			fprintf(stderr,"\n%d bytes binary %s= [",
				stat->len,
				(multiplier == 1)?"":
				    (multiplier == 2)?"(/2) ":
					"(/4) ");
			for (indx=0;indx<nb;indx++)
				fprintf(stderr," 0x%x,",stat->bin.c[indx]);
		}
		else {
			nb >>= 1;
			fprintf(stderr,"\n%d words binary %s= [",
				(stat->len>>1),
				(multiplier == 1)?"":
				    (multiplier == 2)?"(/2) ":
					"(/4) ");
			for (indx=0;indx<nb;indx++ )
				fprintf(stderr," 0x%x,",stat->bin.s[indx]);
		}
		fprintf(stderr," ]\n");
	}
	else fprintf(stderr," nbytes = 0x%x\n",stat->len);
	if ((s = stat->sulist) != (struct loc_s *)0)
	{
		/* progress down the stat_users chain */
		while (s != (struct loc_s *)0)
		{
			fprintf(stderr,
		  "symbol %s(o=0x%x,s=%d,bit=%c,disp=%c,cond=%c,bit=0x%x),",
				((symtabptr)s->stat)->name,
				s->offset,s->size,(s->s.isbitsize)?'T':'F',
				(s->s.isdisp)?'T':'F',(s->s.canbecondensed)?'T':'F',
				s->s.iswordbit);
			s = s->next;
		}
	}
	putc('\n',stderr);
}
#endif
