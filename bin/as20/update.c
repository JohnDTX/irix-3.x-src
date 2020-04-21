#include <stdio.h>
#include "globals.h"
#include "sym.h"

#ifdef SDIPASS2
int obase_dot[];
#endif

update_statusers()
{
	/*  progress down the statusers_update chain
	    (whose head is in statuser_updatelist_head),
	    updating those symbols which statements 
	    reference.
	*/

	register struct loc_s *cur_update;
	struct loc_s *last_update, *cur_updatelist;
	register struct statement_s * stat;
	register symtabptr  targetsym;
	register long oldval,newval;
	register char *position;
	register int nstatusers;
	unsigned short wd,*wptr;
	int dontcondense;

	cur_updatelist = statuser_updatelist_head;

	while(cur_updatelist != (struct loc_s *)0)
	{
		/*  progress down the statusers of this symbol, updating each
		    one.
		*/
		/*  targetsym is the symbol to which the statuser chain is
		    attached.  It MUST have a value by now.
		*/
		stat = (struct statement_s *)cur_updatelist->stat;
		last_update = 0;
		cur_update = stat->sulist;
		nstatusers = 0;
#ifdef DEBUG
		if (debug)
		fprintf(stderr,"updating statuser chain of statement #%d: ",stat->line);
#endif
		linenum = stat->line;
		dontcondense = 0;
		if ((cur_update != (struct loc_s *)0) && 
		    (cur_update->next == (struct loc_s *)0)&& 
		    (!cur_update->s.canbecondensed))
			dontcondense++;
		while (cur_update != (struct loc_s *)0)
		{
			/* cur_update->stat is a symtabptr to the symbol
			   which this location relies upon.
			*/
			targetsym = (struct symtab_s *)cur_update->stat;
#ifdef DEBUG
	if (debug)
		fprintf(stderr,"symbol %s, ",targetsym->name);
#endif
			/* if the symbol that the line relies on is
			   not undefined, update the
			   binary data with the symbol's current value.
			*/
			if (!(targetsym->syminfo.isundefined))
			{
				/* SCR1780 */
				/* if this was a jsr which
			   	could NOT be changed to a bsr,
			   	it is not a displacement anymore.
				These two lines were duplicated from the
				isundefined case in response to scr1780.
				*/
				if ((cur_update->s.isdisp)&&(stat->sinfo.isjsr))
					cur_update->s.isdisp = 0;

				if (cur_update->s.isbitsize)
				{
					/* the symbol is to be placed
					   in a bit field in the instruction
					*/
					/* GB FIX */
					error(0,
		"non-constants cannot be encoded in instruction bitfields");
				}
				else {
					position = 
					  (char *)&(stat->bin.c[cur_update->offset]);

					if (((int)position & 1)&&
					    (cur_update->size != 1))
					    fatal(0,"misalignment in updating stat_user");
					else
					switch (cur_update->size)
					{
					case 1:
						oldval = *(char *)position;
						break;
					case 2:
						oldval = *(short *)position ;
						break;
					case 4:
						oldval = *(long *)position ;
						break;
					default:
						error(0,
					"illegal size in updating stat_user");
					}
				}
				if (cur_update->s.isdisp)
				{
					/* the quantity desired is the 
					   sum of the current value
					   and the difference (<lab> - .).
					   The quantity also must be adjusted
					   back to the start of the current
					   instruction by subtracting two.
					   The current statement is guaranteed
					   to have a label.
					*/
					if (stat->lab == (symtabptr)0)
						fatal(0,
	"statement %d has a displacement, but has no temporary label.",
							stat->line);
					newval = oldval - 2;
					if (cur_update->s.symisneg)
						newval -= targetsym->addr;
					else
						newval += targetsym->addr;
					newval -= stat->lab->addr;
				}
				else {
					if (cur_update->s.symisneg)
						newval = oldval - 
							targetsym->addr;
					else
						newval = oldval + 
							targetsym->addr;
				}
				
				if (cur_update->s.isbitsize)
				{
					/* the symbol is to be placed
					   in a bit field in the instruction
					*/
					/* GB FIX */
					error(0,
		"non-constants cannot be encoded in instruction bitfields");
				}
				else  {
					int badval=0;
					switch (cur_update->size)
					{
					case 1:
						if ((newval < (-128)) || (newval > 127))
							badval++;
						*(char *)position = newval;
						break;
					case 2:
						if ((newval < (-32768)) || (newval > 32767))
							badval++;
						*(short *)position = newval;
						break;
					case 4:
						*(long *)position = newval;
						break;
					default:
						error(0,
					"illegal size in updating stat_user");
					}
					if (badval) error(0,
			"value on statement %d can not be encoded in %d bits.",
				stat->line, 8*cur_update->size);

				}
			}
			else {
				targetsym->syminfo.isextern = 1;
#ifdef NEWJXX
				if (((cur_update->s.isdisp)&&
				     	(!stat->sinfo.isjsr))||
				    (cur_update->s.isnrexpr))
#else
				if ((cur_update->s.isdisp)||(cur_update->s.isnrexpr))
#endif
					error(0,
			"non-relocatable expression, statement %d. (relies on undefined symbol %s)",
					      stat->line,

#ifdef NOTDEF
			    /* if the targetsym was a user temp, show the
			       original $ symbol rather than the internal
			       # symbol
			    */
			    targetsym->syminfo.istemp?
				targetsym->def->lab->name:targetsym->name);
#endif
			    /* if the targetsym was a user temp, show the
			       original $ symbol rather than the internal
			       # symbol
			    */
			    targetsym->syminfo.istemp?
				"<user temporary>":targetsym->name);
			}
#ifdef NEWJXX
			/* if this was a jsr which
			   could NOT be changed to a bsr,
			   it is not a displacement anymore.
			*/
			if ((cur_update->s.isdisp)&&(stat->sinfo.isjsr))
				cur_update->s.isdisp = 0;
#endif
			if ((cur_update->s.isdisp)||
				  (cur_update->s.isnrexpr)||
				  (targetsym->syminfo.csect == ABS_CSECT)) {
				/*  the relocation has just been completed,
				    or the update is a stab entry.
				    We do not need the stat_user UNLESS the 
				    loader is going to do the sdi's, so re
				    move it.  (this small section of code
				    should be removed when the loader is
				    fixed).  Until then, this stat_user
				    will be ignored in generate_symbols.
				*/
				next_statuserno--;
				if (last_update == 0)  
					stat->sulist = cur_update->next;
				else { 
					last_update->next = cur_update->next;
					cur_update = last_update;
				}
			}
			else 
				last_update = cur_update;
			cur_update = cur_update->next;
		}
		/* now we have to run the statement and pack it.  

			* if the instruction has a displacement:
				if word 1 is deleted:
					check that it is zero.
					set byte 1 of word 0 to 0 
					if word 2 is deleted
						check that it is in range.
						set byte 1 of word 0 to the
							value.
				else if not 68020, complain
				reset the wordsdeleted mask.
			else
				check that all deleted words are 0 or 0xffff.
				pack the code sequence.
		*/

		wd = stat->sinfo.wordsdeleted;
		wptr = stat->bin.s;
		/* word zero better never be deleted. */
		if (wd & 1)
			fatal(0,"statement has word zero deleted");
		wd >>= 1;
		wptr++;
		if (stat->sinfo.issdi) {
			if (wd&1) {
				/* word one is deleted. */
				if (*wptr)
					if ((*wptr != 0xffff)||
					    (*(wptr+1)<=0x7fff))
					    fatal(0,
		  		       "illegal condensation on sdi, word 1");
				stat->bin.c[1] = 0;
				wd >>= 1;
				*wptr = (*(wptr+1));
				wptr++;
				if (wd & 1) {
					/* condensation to a byte.  check
					   the range.
					*/
					if ((!*wptr) || (((short)*wptr > 127)||
						 ((short)*wptr < (-128)))) {
					    	fatal(0,
		  		      "illegal condensation on sdi, word 2");
					}
					stat->bin.c[1] = stat->bin.s[1];
				}
			}
			else if ((!stat->sinfo.isjsr)&&(!is68020)&&(!dontcondense))
				error(0,"displacement overflow");

			stat->sinfo.wordsdeleted = 0;
		}
		else {
			/* check that all wordsdeleted are zero or (-1) */
			/* GB - FIX! if a word is > 0x7fff, it may still be
			   an overflow....
			*/
			unsigned short *wptr1, *wend;
			while (wd) {
				if (wd & 1) {
					if (*wptr)
						if ((*wptr != 0xffff)||
	    					    ((*(wptr+1)<=0x7fff)&&
					     (!stat->sinfo.isunsigneddata)))
					    fatal(0,
		  			"illegal condensation on sdi.");
				}
				wd >>= 1;
				wptr++;
			}
			wd = stat->sinfo.wordsdeleted;
			wptr = wptr1 = stat->bin.s;
			wend = wptr + (stat->len >> 1);
			while (wptr < wend) {
				if (wd & 1)
					*wptr1++;
				else
					*wptr++ = *wptr1++;
				wd >>= 1;
			}
		}
#ifdef DEBUG
	if(debug)
fprintf(stderr,"\n");
#endif
		cur_updatelist = cur_updatelist->next;
	}

}


update_symusers(lastchance)
{
	/*  progress down the symusers_update chain
	    (whose head is in symuser_updatelist_head),
	    updating those symbols whose equates were 
	    forward references.
	*/

	register struct loc_s *cur_update, *cur_updatelist, *last_update;
	register symtabptr sym,targetsym;
	int changed,undefineds;
	int errorpass = 0;

#ifdef DEBUG
if (debug)fprintf(stderr,"update_symusers (pass %d):\n",lastchance);
#endif

dopass:
	do
	{

	changed = undefineds = 0;
	cur_updatelist = symuser_updatelist_head;
	linenum = 0;
	while(cur_updatelist != (struct loc_s *)0)
	{
		/*  progress down the symusers of this symbol, updating each
		    one.
		*/
		/*  targetsym is the symbol to which the symuser chain is
		    attached.  It MUST have a value by now.
		*/
		targetsym = (symtabptr)cur_updatelist->stat;
		cur_update = targetsym->u.sym_users;
		last_update = (struct loc_s *)0;
#ifdef DEBUG
	if(debug)
		fprintf(stderr,"updating symuser chain of symbol %s: ",targetsym->name);
#endif
		while (cur_update != (struct loc_s *)0)
		{
			/* sym is the symbol whose definition relies on 
			   the targetsym */
			sym = (symtabptr)cur_update->stat;
#ifdef DEBUG
	if(debug)
		fprintf(stderr,"%s, ",sym->name);
#endif
			/*
			if ((targetsym->syminfo.isabs)||
			     (targetsym->syminfo.csect != UNDEF_CSECT))
			*/
			/*
			if (!(targetsym->syminfo.isundefined))
			*/
			if ((!targetsym->syminfo.isundefined)&&
			    ((lastchance)||
			       (targetsym->syminfo.csect == ABS_CSECT)))
			{
				/* add int the definition */
				if (!(targetsym->syminfo.csect == ABS_CSECT))
					warning(0,
"absolute symbol %s relies on relocatable symbol %s, and may not be correct",
						sym->name,targetsym->name);
				sym->syminfo.isundefined = 0;
				sym->addr += targetsym->addr;
				sym->syminfo.isword = 
				   ((sym->addr  < 32768) && 
				    (sym->addr >= (-32768)));
				sym->syminfo.isbyte = 
				   ((sym->addr  < 127) && 
				    (sym->addr >= (-128)));

				changed = 1;
				/* remove this symbol from the chain */
				if (last_update != (struct loc_s *)0)
					last_update->next = cur_update->next;
				else
					targetsym->u.sym_users = 
						cur_update->next;
			}
			else {
				if (!errorpass) undefineds = 1;
				else if (lastchance) 
					error(0, 
				"symbol %s relies on undefined symbol %s",
					sym->name,targetsym->name);
			}
			cur_update = cur_update->next;
		}
#ifdef DEBUG
	if(debug)
fprintf(stderr,"\n");
#endif
		cur_updatelist = cur_updatelist->next;
	}
	}
	while ((changed) && (undefineds));

	/* if there are no undefineds, return successfully */
	if (!undefineds) return(0);

	errorpass = 1;
	goto dopass;

}

int words_removed[] = {0};

#define MAX_WORDS 0x10
unsigned char possible_condensation[MAX_WORDS];
#define LONG_TO_WORD 1
#define WORD_TO_BYTE 2
#define LONG_TO_WORD_DISP 4
#define WORD_TO_BYTE_DISP 8
#define LONG_TO_BYTE_DISP (LONG_TO_WORD_DISP | WORD_TO_BYTE_DISP)

condense_statements()
{
	/*  run down the stat_users chain, fixing up the size
	    of the displacements, using the current dot info.
	*/

	register struct loc_s *cur_update, *cur_updatelist;
	struct loc_s *last_update;
	register symtabptr  targetsym;
	int isword;
	int isbyte;
	int seenrelocatablesym,i,type,csect;
	register long newval;
	register char *position;
	register struct statement_s * stat;
	static union gbinary tempbin;
	unsigned short *wptr;
	int checkval;

	cur_updatelist = statuser_updatelist_head;

	while(cur_updatelist != (struct loc_s *)0)
	{
	    /*  progress down the statusers of this symbol, updating each
		one.
	    */
	    /*  targetsym is the symbol to which the statuser chain is
		attached.  It MUST have a value by now.
	    */
	    stat = (struct statement_s *)cur_updatelist->stat;
	    linenum = stat->line;
	    seenrelocatablesym=0;
	    bzero(tempbin.chars,BUFMAX);
	    fastcopy(tempbin.chars,stat->bin.c,stat->len);
	    cur_update = stat->sulist;
	    last_update = 0;
#ifdef DEBUG
	    if(debug)
		fprintf(stderr,"condensation on statuser chain of statement #%d: ",
			stat->line);
#endif
	    linenum = stat->line;
	    if (cur_update != (struct loc_s *)0)
		bzero(possible_condensation,MAX_WORDS<<1);
	    while (cur_update != (struct loc_s *)0)
    	    {
		/* stat is the statement whose definition relies on 
	   	the targetsym */
		targetsym = (struct symtab_s *)cur_update->stat;
		if (targetsym->syminfo.csect != ABS_CSECT) {
			if ((seenrelocatablesym)&&(stat->sinfo.issdi)) {
				error(0,
			"span-dependent instruction may only have one\n\t\trelocatable symbol in target expression");
				if (last_update == (struct loc_s *)0)
					stat->sulist = cur_update->next;
				else
					last_update->next = cur_update->next;
				cur_update = cur_update->next;
				/* and decrement the count of statusers */
				next_statuserno--;
				continue;
			}
			seenrelocatablesym = 1;
		}

		/* get the current value */
		position = 
			  (char *)&(tempbin.chars[cur_update->offset]);

		if (((int)position & 1)&&(cur_update->size != 1)) {
			error(0,"misalignment in updating stat_user");
		}
		else {
			switch (cur_update->size)
			{
			case 1:
				newval = *(char *)position; 
				break;
			case 2:
				newval = *(short *)position ;
				break;
			case 4:
				newval = *(long *)position ;
				break;
			}
		}
#ifdef DEBUG
if(debug)
	fprintf(stderr,"symbol %s, ",targetsym->name);
#endif
		if (!(targetsym->syminfo.isundefined))
		{
		    /* add in the value of the symbol */
		    if (cur_update->s.symisneg)
		    	newval -= targetsym->addr ;
		    else
		    	newval += targetsym->addr ;
		    if (cur_update->s.isdisp)
		    {
			/* the quantity desired is the 
			   sum of the current value
			   and the difference (<lab> - .).
			   The quantity also must be 
			   adjusted back to the start of
			   the current instruction by
			   subtracting two.
			   The current statement is guaranteed
			   to have a label.
			*/
			if (stat->lab == (symtabptr)0)
				fatal(0,
"statement %d has a displacement, but has no temporary label.",
					stat->line);
			newval -= 2;
			newval -= stat->lab->addr;
			isword = ((newval <= 32767)&&
				(newval >= (-32768)));
			isbyte = ((newval <= 127)&&
				(newval >= (-128)));
			/* cant make bras out of bra . */
			/* NOTE!! This only works since only ONE
			   relocatable symbol is allowed in an SDI
			*/
#ifdef NOTDEF
			if (((newval == 4) && 
			    (cur_update->size == 4))) {
				isbyte = 0;
				/*newval = 0; */
			}
			if (newval == 2) {
				if (cur_update->size == 2) {
					isbyte = 0;
					/*newval = 0; */
				}
				else  
				  if ((cur_update->size == 4)&&
			       	      (stat->sinfo.wordsdeleted & 2)) {
			    		isbyte = 0;
					/*newval = 0; */
				  }
			}
#endif
		    }
		    else {
			isword = targetsym->syminfo.isword;
			isbyte = targetsym->syminfo.isbyte;
		    }
			
		    /* update the position */
		    switch (cur_update->size)
		    {
		    case 1:
			    *(char *)position = newval; 
			    break;
		    case 2:
			    *(short *)position = newval;
			    break;
		    case 4:
			    *(long *)position = newval;
			    break;
		    }
		    if (cur_update->s.isbitsize)
		    {
			/* the symbol is to be placed
			   in a bit field in the instruction
			*/
			/* GB FIX */
			error(0,
	"non-constants cannot be encoded in instruction bitfields");
		    }
		    else {
			/* if the symbol is indeed a word,
			   and the size of the relocation datum
			   is a long, and it CAN be condensed,
			   do it.
			*/
			if (cur_update->s.canbecondensed)
			{
			    if ((isword)&&(cur_update->s.isdisp)&&
				(cur_update->size == 4))
				possible_condensation[cur_update->offset>>1] = 
					  (isbyte)?LONG_TO_BYTE_DISP:
						   LONG_TO_WORD_DISP;
			    else if ((isbyte)&&(cur_update->s.isdisp)&&
				(cur_update->size == 2))
				possible_condensation[cur_update->offset>>1] = 
					  WORD_TO_BYTE_DISP;
			    else if ((isword)&&(cur_update->size == 4))
				possible_condensation[cur_update->offset>>1] = 
					LONG_TO_WORD;

#ifdef NOTDEF
		    	    if ((isword)&&(cur_update->s.isdisp)&&
		     	        (cur_update->size == 4))
			    {	
#ifdef NEWJXX
#endif
		    		condense(stat,
		    		    cur_update,
		    		    targetsym);
			    }
		    	    else if ((isword)&&
		    		(cur_update->size == 4))
			    {
		    		condense(stat,
		    		    cur_update,
		    		    targetsym);
			    }
		    	    if (!isword && 
		    		(stat->sinfo.issdi) && 
		    		 !is68020)
		    		    error(0,
		    		      "displacement overflow");
#endif
		    	}
		    }
		}
		else {
		    targetsym->syminfo.isextern = 1;
		    if (debug)
			warning(0,
		"\n(debug) statement %d relies on undefined symbol %s",
				stat->line,targetsym->name);
		}
		last_update = cur_update;
		cur_update = cur_update->next;
	    }
	    wptr = tempbin.shorts;
	    i = 0;
	    while (i < (stat->len>>1)) {
	        newval = *(long *)wptr;
		switch (possible_condensation[i]) {

			case  LONG_TO_BYTE_DISP:
					/* here, we must check that the
					   instruction is not really bra .
					   This is true if word 1 has NOT
					   been deleted, and the long val
					   is 4, or if word 1 HAS been deleted,
					   and the long val is 2.
					*/
					if (stat->sinfo.wordsdeleted & 2) 
						/* word one is gone, thus 
						   the next statement's address
						   would be .+2 if this 
						   condensation is done... */
						checkval = 2;
					else 
						/* word one is there, thus 
						   the next statement's address
						   would be .+4 if this 
						   condensation is done... */
						checkval = 4;

					if ((newval > 127)||(newval < (-128))||
					    (newval == checkval))
						 possible_condensation[i] = 
							LONG_TO_WORD_DISP;
					else {
						i++;wptr++;break;
					}
			case  LONG_TO_WORD_DISP:	
#ifdef NOTDEF
					if ((newval <= 127)&&
					    (newval >= (-128)) &&
					    (newval) {
						possible_condensation[i] = 
						   LONG_TO_BYTE_DISP;
						i++;wptr++;
						break;
					}
#endif
			case  LONG_TO_WORD:
					if ((newval > 32767)||
					    (newval < (-32768)))
						possible_condensation[i] = 0;
					break;
			case  WORD_TO_BYTE_DISP:	
					if ((*wptr == 2)||
					    (*wptr > 127)||(*wptr < (-128)))
						possible_condensation[i] = 0;
		}
		i++; wptr++;

	    }

	    /* now go through the updatelist again, doing the updates */
	    cur_update = stat->sulist;

	    while (cur_update != (struct loc_s *)0)
	    {
		if (cur_update->s.canbecondensed) {

			i = (cur_update->offset >> 1);
			if (type = possible_condensation[i]) {
				int w;
				int typesz = 2;
				w = 1<<i;
				/* always point at the word of data... 
				if (type != WORD_TO_BYTE_DISP)
					cur_update->offset += 2;
				*/	
				/*typesz = (type == LONG_TO_BYTE_DISP)?4:2;*/
				/*cur_update->size -= typesz; */
				if (!(stat->sinfo.wordsdeleted & w)) {
					/* this condensation has never 
					   before been done... */
#ifdef NEWJXX
					if ((stat->sinfo.isjsr)&&
					    (cur_update->s.isdisp)&&
				            (type & LONG_TO_WORD_DISP))
					{
				    		/* 
						   the jsr can be replaced 
						   with a bsr instruction.
				    		*/	
				    		stat->bin.s[0] = I_BSR;
				    		stat->sinfo.isjsr = 0;
					}
#endif
					stat->sinfo.wordsdeleted |= w;
					stat->len -= 2;
					*(condensation_info + stat->line) += 
						2;
					words_removed[csect = 
						stat->sinfo.csect]++; 

					if ((csect != TEXT_CSECT) && 
					    (csect != DATA_CSECT))
						fatal(0,
			 "condensation done on csect other than text or data!");
					if (type == LONG_TO_WORD) {
						/* clear the length bit 
						   in the instruction. */
						unsigned short w,*wptr;
						wptr = stat->bin.s + 
						 (cur_update->s.iswordbit >> 8);
						w = ~(1 << 
					((cur_update->s.iswordbit & 0xf) ));

						*wptr &= w;
					}
				}
				if ((type == LONG_TO_BYTE_DISP) &&
				    (! (stat->sinfo.wordsdeleted & (1<<(i+1))) )
				     ) {
					stat->sinfo.wordsdeleted |= (1<<(i+1));
					stat->len -= 2;
					*(condensation_info + stat->line) += 
						2;
					words_removed[csect = 
						stat->sinfo.csect]++; 
				}
			}
		}
		cur_update = cur_update->next;
	    }
#ifdef DEBUG
	if(debug)
fprintf(stderr,"\n");
#endif
	cur_updatelist = cur_updatelist->next;
	}
			
}

update_defined_labels(second_pass)
{
	/*  the linked list of defined labels must be run after
	    condensation has been performed.  At this point,
	    each statement number has the number of words which
	    were condensed from that statement, through the
	    condensation_info byte array.  As defined labels
	    are entered from first to last, the first entry
	    in the deflab_updatelist_head linked list is the 
	    first defined label in the file.  The entire 
	    condensation_info byte array must be summed, for
	    all entries in the text and data csect, as the deflab_updatelist
	    is run, and the number of words condensed subtracted
	    from the address of each label.  
	*/

	register struct loc_s *current_deflab = deflab_updatelist_head;
	register symtabptr sym;
	register struct statement_s *cur_stat;

	register long lineno=1;
	register long ntbytesdeleted = 0;
	register long ndbytesdeleted = 0;

#ifdef DEBUG
if (debug)
fprintf(stderr,"updating defined labels-> ");
#endif
	cur_stat = first_stat;
	while (current_deflab != (struct loc_s *)0)
	{
		sym = (symtabptr)current_deflab->stat;
		lineno = sym->u.line;

		while (cur_stat->line < lineno)
		{
			if (cur_stat->sinfo.csect == TEXT_CSECT)
				    ntbytesdeleted += 
					*(condensation_info+cur_stat->line);
			else if (cur_stat->sinfo.csect == DATA_CSECT)
				    ndbytesdeleted += 
					*(condensation_info+cur_stat->line);
#ifdef SDIPASS2
			*(condensation_info + cur_stat->line) = 0;
#endif
			/* check for consistency */
			if (cur_stat->next == (struct statement_s *)0)  
			{
				/* legal only if this is the last 
				   defined label.
				*/
				if ((cur_stat->line == lineno)&&
				    (current_deflab->next == 0)) break;
				fatal(0,
			"ran out of statements in update_defined_labels");
			}
			cur_stat = cur_stat->next;
		}
		if (sym->syminfo.csect == TEXT_CSECT)
			sym->addr -= ntbytesdeleted ;
		else if (sym->syminfo.csect == DATA_CSECT)
			sym->addr -= ndbytesdeleted ;

		/* add in the base of the correct csec */
#ifdef SDIPASS2
		if (!second_pass)
			sym->addr -= obase_dot[sym->syminfo.csect];
#endif
			sym->addr += base_dot[sym->syminfo.csect];

		/* and copy it to the location on the statement */
		sym->def->addr = sym->addr;
		sym->syminfo.isundefined = 0;
#ifdef DEBUG
if(debug)
fprintf(stderr,"%s: 0x%x, ",sym->name,sym->addr);
#endif
		current_deflab = current_deflab->next;
	}
#ifdef DEBUG
if(debug)
fprintf(stderr,"\n");
#endif
}

#ifdef NOTDEF /* old code */
update_defined_labels()
{
	/*  the linked list of defined labels must be run after
	    condensation has been performed.  At this point,
	    each statement number has the number of words which
	    were condensed from that statement, through the
	    condensation_info byte array.  As defined labels
	    are entered from first to last, the first entry
	    in the deflab_updatelist_head linked list is the 
	    first defined label in the file.  The entire 
	    condensation_info byte array must be summed, for
	    all entries in the text csect, as the deflab_updatelist
	    is run, and the number of words condensed subtracted
	    from the address of each label.
	*/

	register unsigned char *current_condensation = condensation_info;
	register long lineno=0;
	register long clineno=0;
	register struct loc_s *current_deflab = deflab_updatelist_head;
	register symtabptr sym;

	register long nbytesdeleted = 0;

#ifdef DEBUG
if (debug)
fprintf(stderr,"updating defined labels-> ");
#endif
	while (current_deflab != (struct loc_s *)0)
	{
		sym = (symtabptr)current_deflab->stat;
		if (sym->syminfo.csect == TEXT_CSECT)
		{
			lineno = sym->u.line;
			while (clineno < lineno)
			{
				nbytesdeleted += *current_condensation++;
				clineno++;
			}
			sym->addr -= nbytesdeleted ;
		}
		/* add in the base of the correct csec */
		sym->addr += base_dot[sym->syminfo.csect];
		/* and copy it to the location on the statement */
		sym->def->addr = sym->addr;
#ifdef DEBUG
if(debug)
fprintf(stderr,"%s: 0x%x, ",sym->name,sym->addr);
#endif
		current_deflab = current_deflab->next;
	}
}
#endif

condense(stat,cur_update,targetsym)
struct statement_s *stat;
register struct loc_s *cur_update;
symtabptr targetsym;
{
	/*  the location in the given instruction can be condensed
	    from a long to a word.  In cases in which the current size
	    IS a word, this is an indication that we are condensing
	    an sdi down so that the displacement can be encoded in 
	    the instruction itself.
	    This is somewhat of a tricky 
	    process.  There are several things needing to be done:

		*  if words have previously been condensed out of 
		   the instruction (seen by the wordsdeleted mask
		   in the sinfo section), and the condensed words
		   are BEFORE this occurrance, the offset and iswordbit
		   in the cur_update structure are incorrect, and 
		   must be updated before using.

		*  the correct bit in the wordsdeleted datum must be
		   set.  This corresponds to the ORIGINAL instruction.

		*  the length of the instruction must be altered.

		*  the longword indicated must be condensed to a word,
		   and the rest of the binary data abutted to it.

		*  the indicated bit in the instruction must be CLEARED
		   to indicate that the datum in question is now a
		   word, rather than the expected longword.

		*  the size in the cur_update structure must be set 
		   to word.

		*  update the condensation info for the statement in question.

	    (and you thought assemblers were simple....)

	*/

	unsigned char originaloffset = cur_update->offset;
	unsigned char realoffsetw;
	unsigned char originaloffsetw,nwds;
	short w,curword;
	short data;
	short condensed=0;
	register unsigned short * wptr;

	originaloffsetw = originaloffset>>1;
#ifdef DEBUG
	if(debug)
	fprintf(stderr,"condensation: stat #%d oldoff=%d, sz=%d, owd=0x%x;",
		stat->line, cur_update->offset, cur_update->size,
		stat->sinfo.wordsdeleted);
#endif
#ifdef NOTDEF

	if ((w = stat->sinfo.wordsdeleted)&&(cur_update->offset > 2))
	{
		/*  words have previously been deleted from this
		    statement.  Has a word been deleted PRIOR to 
		    the pending deletion? 
		*/

		unsigned char nd,nwd,bitno;
		register int temp;
		nd = 0;
		nwd = 0;
		temp = cur_update->s.iswordbit >> 8;
		bitno = 0;
		do
		{
			if (bitno > originaloffsetw) break;
			if (bitno < temp) nwd++;
			if (w & 1) nd++;
			w >>= 1;
			bitno++;

		} while (w);

		if (nd)
		{
			/* words were deleted PRIOR to the pending
			   deletion.  Alter the offset appropriately.
			*/
			cur_update->offset -= nd<<1;
		}

		if (nwd)
		{
			temp -= nwd;
			temp <<= 8;
			temp |= cur_update->s.iswordbit & 0xf;
			cur_update->s.iswordbit = temp;
		}
	}

	/* if the update is from a word to a byte for an sdi being
	   collapsed into an instruction, we have to retrieve the
	   original word data (constant offsets from labels) before
	   collapsing the code sequence.  (in long->word condensations,
	   the msw of the long is lost, which is ok).
	*/
	realoffsetw = cur_update->offset >> 1;
	if ((cur_update->size ==  2)&&(cur_update->s.isdisp))
	{
		data = *(stat->bin.s + realoffsetw);
	}
#endif
	/* now set the correct bit in the wordsdeleted mask */

	w = 1;
	w <<= (originaloffsetw );

	/* ok, has this word already been condensed?  (This can occur
	   if the word relies on more than one previously-undefined
	   symbols - it appears on each one's statuser chain.)
	*/
	if ((stat->sinfo.wordsdeleted & w))
	{
#ifdef DEBUG
		if (debug)
			fprintf(stderr,"..word already deleted\n");
		/* adjust the size of the update pending */
#endif
		if ((cur_update->size == 4)||(cur_update->s.isdisp)) 
			cur_update->size -= 2;
	}
	else {


#ifdef NOTDEF
		/* now do the actual condensation */
		wptr = (stat->bin.s + (realoffsetw) );
		nwds = stat->len >> 1;
		curword = realoffsetw;
		condensed++;

		while (curword < nwds)
		{
			*wptr = *(wptr+1);
			wptr++;
			curword++;
		}


#else
#endif
		/* adjust the size of the update pending */
		cur_update->size -= 2;
	}
			

#ifdef DEBUG
	if(debug)
	fprintf(stderr," -> newoff=%d, sz=%d, nwd=0x%x\n",
		cur_update->offset, cur_update->size,
		stat->sinfo.wordsdeleted);
#endif
}
		




