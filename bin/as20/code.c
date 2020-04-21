/*
 *
 *	code.c - code generation routines for the 68020 
 * 		 assmbler.
 *
 */

#include <stdio.h>
#include "globals.h"
#include "sym.h"
#include <a.out.h>

extern int sym_debug;
static struct exec header;

/* symbols, code and relocation information are written out in segments.
   The buffer size itself is BUFSZ.  These macros control the number
   of symbols/relocation info structs that will fit in the buffer
*/
#define BUFSZ 0x1100
static char buffer[BUFSZ];
#define NSYMS_IN_BUF (BUFSZ / sizeof(struct nlist))
#define SYMBUFSZ (NSYMS_IN_BUF * sizeof(struct nlist))
#define NRELOCS_IN_BUF (BUFSZ / sizeof(struct relocation_info))
#define RELOCBUFSZ (NRELOCS_IN_BUF * sizeof(struct relocation_info))

#ifdef SDIPASS2
int obase_dot[5];
#endif

generate_code()
{
	/* output the text and data control sections.  At this
	   point, all of the input has been read and assembled into
	   the preliminary code.  Several chores remain to be done:

		*  symbols which were equated to forward-defined
		   symbols must be updated (update_symusers).

		*  all offsets currently are the largest possible
		   size (ie almost all long).  Now that the final
		   value of symbols is determined,  these offsets
		   can be condensed. (condense_statements).  This
		   routine also takes care of sdi's.

		*  the final addresses of all defined labels
		   can be determined using their preliminary
		   addresses and the number of words condensed
		   from each statement in the condensation phase
		   (update defined_labels)

		*  Lastly, the statements which rely on a relocatable
		   expression must have their final value determined.
		   (update_statusers).

		*  when this is done, the text and data
		   csects can be written
	*/

	/* the buffer is only filled until 0x1000 */
	register char *bufptr,*bufend = &buffer[0x1000];

	register int nb,nb_in_partial_block;
	register struct statement_s * cur_stat ;
	register int *curdot;

	linenum = 0;

	allocate_condensation_info(last_statementno);
	update_symusers(0);
	condense_statements();

	/* set the base of each segment, using the 
	   number of words removed from each csect, 
	   as filled in by condense_statements
	*/
	base_dot[TEXT_CSECT] = 0;
	dot[TEXT_CSECT] = (dot[TEXT_CSECT] - 
		(words_removed[TEXT_CSECT]<<1) + 1)& (~1);
	base_dot[DATA_CSECT] = dot[TEXT_CSECT]; 
	dot[DATA_CSECT] = (dot[DATA_CSECT] - 
		(words_removed[DATA_CSECT]<<1) + 1)& (~1);
	base_dot[BSS_CSECT] = dot[DATA_CSECT] + base_dot[DATA_CSECT]; 
	update_defined_labels(1);
#ifdef SDIPASS2
	words_removed[DATA_CSECT] = 0;
	words_removed[TEXT_CSECT] = 0;
	condense_statements();
	obase_dot[TEXT_CSECT] = base_dot[TEXT_CSECT];
	obase_dot[DATA_CSECT] = base_dot[DATA_CSECT];
	obase_dot[BSS_CSECT] = base_dot[BSS_CSECT];
	dot[TEXT_CSECT] = (dot[TEXT_CSECT] - 
		(words_removed[TEXT_CSECT]<<1) + 1)& (~1);
	base_dot[DATA_CSECT] = dot[TEXT_CSECT]; 
	dot[DATA_CSECT] = (dot[DATA_CSECT] - 
		(words_removed[DATA_CSECT]<<1) + 1)& (~1);
	base_dot[BSS_CSECT] = dot[DATA_CSECT] + base_dot[DATA_CSECT]; 
	update_defined_labels(0);
#endif
	/* update those symbols which relied on a label */
	update_symusers(1);
	update_statusers();
	
	/* set the preliminary header information */
	header.a_magic = OMAGIC;
	header.a_syms = 0;
	header.a_trsize = 0;
	header.a_drsize = 0;
	header.a_syms = 0;
	header.a_syms = 0;
	header.a_text = dot[TEXT_CSECT];
	header.a_data = dot[DATA_CSECT];
	header.a_bss = dot[BSS_CSECT];

	/* position to the text csect */
	lseek(output,sizeof(struct exec),0);

	bufptr = buffer;

	cur_csect = TEXT_CSECT;

	/* copy the current csect.  The small binary data array
	   for each statement must be copied to the larger output
	   binary buffer.
	*/
copycsect:
	curdot = &dot[cur_csect];
	*curdot = base_dot[cur_csect];
	cur_stat = first_stat;
	nb_in_partial_block = 0;

	while(cur_stat != (struct statement_s *)0)
	{
		linenum = cur_stat->line;
		if (bufptr >= bufend)
		{
			/* the buffer is (over)full.  Write out
			   the full buffer
			*/
			write(output,buffer,0x1000);
			nb = bufptr - bufend;
			/* and copy the overflow to the front of the
			   buffer for writing in the next block
			*/
			if (nb)
				fastcopy(buffer,bufend,nb);
			bufptr = &buffer[nb];
		}
		nb = cur_stat->len;
		if ((nb)&&(cur_stat->sinfo.csect == cur_csect))
		{
			if (!nb_in_partial_block)
			{
				/* update the final address of the
				   statement, for use by sym_output() 
				*/
				cur_stat->addr = *curdot;
				*curdot += nb;
			}
			if (cur_stat->sinfo.mustpad)
			{
				/* the current statement needs a zero pad
				   byte output before it. (usually due
				   to a preceeding .align statement 
				*/
				*bufptr++ = 0;
				*curdot++;
			}
			if (cur_stat->sinfo.iszero)
			{
				/* the current statement is really a pseudo
				   op to output len bytes of zeroes.
				   ug - we may need to zero across a block
				   boundary.  Output successive blocks.
				   WARNING: this may not be tested for some
				   time.
				*/
				if ((char *)(nb + bufptr) > bufend)
				{
					/* split the data */
					nb_in_partial_block = bufend - bufptr;
					bzero(bufptr,nb_in_partial_block);
					cur_stat->len = nb - nb_in_partial_block;
					cur_stat->sinfo.mustpad = 0;
					bufptr += nb_in_partial_block;
				}
				else {
					bzero(bufptr,nb);
					bufptr += nb;
					nb_in_partial_block = 0;
				}
				goto next;
			}
			if (cur_stat->sinfo.isunsigneddata)
				pack_data(cur_stat);

			/* else, copy the statement information.  Note
			   that we may go past bufend, but never past
			   the end of the buffer (0x100 bytes to spare)
			*/
			fastcopy(bufptr,cur_stat->bin.c,nb);
			bufptr += nb;
#ifdef DEBUG
if (debug) dump_stat(cur_stat);
#endif
		}
		else {
			if ((cur_stat->sinfo.isstab)&&(cur_csect == TEXT_CSECT))
				cur_stat->addr = *curdot;
#ifdef DEBUG
			if ((cur_stat->sinfo.isstab)&&(debug))
				dump_stat(cur_stat);
#endif
		}
next:
		if (!nb_in_partial_block) cur_stat = cur_stat->next;
		/*
		nb_in_partial_block = 0;
		*/
	}
	/* if this was the text csect, we want to copy the data
	   csect next.
	*/
	if (cur_csect == TEXT_CSECT)
	{
		cur_csect = DATA_CSECT;
		goto copycsect;
	}
	/* this time, write the entire buffer */
	write(output,buffer,(bufptr - buffer));


}

generate_header()
{
	/* big deal... */
	lseek(output,0,0);
	write(output,&header,sizeof(struct exec));
}

/*  this structure is used to overlay loc_s structures and
    rename/reuse some of the fields 
*/
struct loc_s2
{
	struct loc_s2 *next;
	/* the loc_s chain can be used to link symbols which
	   need fixing up (in relation to other symbols)
	   or statements which need fixing up.
	*/
	struct symtab_s *sym;
	/* offset into binary data in the indicated statement of
	   this relocation datum, and whether the size is to
	   be interpreted in bits or bytes.
	*/
	unsigned char offset,size;
	unsigned char new_size,pad;
	/*  size of the binary datum this symbol is inhabiting,
	    and whether the datum is RELATIVE (a displacement)
	    or the actual address.  
	*/
	/*  the symbol number in the symbol table */
	struct
	{	unsigned 
			isdisp:1, isextundf:1, :1, isnrexpr:1, type:4,
			symno:24;
	} s;
};

/* this array is used to map internal csect numbers to symbol types
   in the a.out file
*/
int csect_to_type[5] = {N_UNDF,N_TEXT,N_DATA,N_BSS,N_ABS};

generate_symbols()
{
	/*  create the symbol table and the relocation info for the a.out
	    file.

	    The names of those symbols which must be placed in the a.out
	    file have been kept in separate buffers from other binary data.
	    The string table is in a series of buffers linked through
	    the chain indicated by strseglist_head. 
	    This forms the basis of the string table to be output to the 
	    a.out file.  At this point, the address of each statement
	    has been correctly computed (by generate_code()).  The steps
	    are as follows:

		*   Generate an array of base indices to each segment of
		    the string table.  

		*   Compute the offset of the symbol table from the 
		    start of the file.  This is done using the  number
		    of stat_users (next_statuserno) and the sizes for
		    the text and data csects.  Position the file to 
		    the beginning of the symbol table.  (Make sure
		    you are on an even boundary.  If not, add one to
		    the size of the data segment and write a null byte.)

		*   Walk though the symbol table. For each non-temporary
		    symbol:
			*  assign a number to the symbol.
			*  change the pointer to the symbol to an index
			       	into the future string table by using
			       	the number of the string segment indicated
			       	in the syminfo and the difference between
			       	the base of that segment and the symbol 
			       	itself.
			*  update the length of the symbol table.
			*  link together those stat_users which must become
				relocation data in the a.out file.
				When this is done, the symbol
				referenced is lost (as the only way
				previously to get at the stat_user was
				through the symbol table, and the end
				of the chain was the end of the stat_users
				relative to the current symbol).  Alleviate
				this problem by storing the symbol number
				in the loc_s2 structure, which is overlayed
				on top of the loc_s stat_user structure.
			*  assemble and write out the symbol.

		*   Write the size of the symbol table to the a.out header.

		*   Write out the string table length.

		*   Write out the string table.

		*   Reposition the file to the relocation information and
			run each of the (now linked) stat_users chains,
			assembling the relocation info. Keep a running
			size of each (data and text) relocation size.
		
		*   Update the header with the data and text relocation sizes.
	*/
	register int indx;
#ifdef NOTDEF
	register struct loc_s2 *su;
	struct loc_s2 *nextd_su,*nextt_su;
	struct loc_s2 *firstd_su,*firstt_su,*su1;
#endif
	symtabptr *symbptr;
	char **generate_strseg_array();
	char **strseg_base,**curstrsegptr;
	long symposition, relocinfo_size;
	struct nlist *symbuf;
	register struct nlist *symbufptr;
	long strtabsz = 0;
#define STR_EXTENT 0x1000 /* NOTE: This MUST match the definintion in alloc.c */

	header.a_syms = 0;

	/*  if there are no symbols, we have no work to do. */
	if (next_strsegno == 0) {
		/* we have to write the length of the string table as four */
		int junk = 4;
		symposition = header.a_text + 
			header.a_data + sizeof(struct exec);
		if (symposition & 1)
		{
			header.a_data++;
			symposition++;
		}
		lseek(output,symposition,0);
		write(output,&junk,4);
		return;
	}
	/*  generate the array of bases of the string table segments */
	strseg_base = generate_strseg_array();

	/* calculate the beginning of the symbol table */
	symposition = header.a_text + header.a_data + sizeof(struct exec);

	if (symposition & 1)
	{
		header.a_data++;
		symposition++;
	}
	relocinfo_size = (sizeof (struct relocation_info) * next_statuserno);
	symposition += relocinfo_size;

	/* position the file to the start of the symbols */
	if (lseek(output,symposition,0) != symposition)
		fatal(0,"cant position output to beginning of symbols at 0x%x",
			symposition);

	/* step through the symbol table */
	symbptr = sym_bucket;
	{
	register int nextsymno = 0;
	register symtabptr sym;
	struct nlist *symbuf;
	register struct nlist *symbufptr;
	register int nsymsinbuf;
	char type,isdisp;

	/* reuse the code buffer for symbol I/O */
	symbuf = symbufptr = (struct nlist *)buffer;
	bzero(buffer,BUFSZ);
	nsymsinbuf = 0;

	linenum = 0;
	for (indx = 0;indx <= NSYMBUCKETS;indx++)
	{
		sym = sym_bucket[indx];
		if ((sym == (symtabptr)0))
			continue;
		do {
			sym->symno = nextsymno;	
			/* form the index into the string table 
			*/
			symbufptr->n_un.n_strx = (int) (sym->name - 
				strseg_base[sym->syminfo.strseg]+sizeof(long)
				+ STR_EXTENT*(sym->syminfo.strseg ));

			/* assemble the nlist struct
			*/
			if (sym->syminfo.isundefined)
				type = N_UNDF;
			else
				type = csect_to_type[sym->syminfo.csect];

			/* or in the external bit if needed */
			if (sym->syminfo.isextern) type |= N_EXT;

			/* and place in the nlist structure */
			symbufptr->n_type = type;
			symbufptr->n_value = sym->addr;

			if (sym->syminfo.isstab)
			/* redundant.  we can tell since indx == NSYMBUCKETS */
			{
				makestab(sym,symbufptr);
			}
#ifdef NOTDEF
			/* get the start of the stat_user chain */
			else if ((su = (struct loc_s2 *)sym->stat_users) != 
			    (struct loc_s2 *)0)
			{
				/* the symbol has stat_users.  Tear the
				   chain apart, and link each stat_user
				   to either the data loc_s chain or the
				   text loc_s chain.  Copy
				   the symbol number into the loc_s2
				   structure for later.
				*/
				do
				{
					su->s.symno = sym->symno;
					/* careful! overlaying the 
					   isdisp bit here! */
					/* REMOVE THIS WHEN THE LOADER
					   DOES SDIS.  For now, the relocation
					   info for previously relocated 
					   displacements cannot be output,
					   as the loader will attempt
					   the munge it.
					*/
					if ((su->s.isdisp) || (su->s.isnrexpr))
					{
						su = su->next;
						continue;
					}

					if ((sym->syminfo.isextern)&&
					    (sym->syminfo.isundefined))
						su->s.isextundf = 1;

					/* if the symbol is undefined, it
					   must appear as a relocation datum
					*/
					switch (su->stat->sinfo.csect)
					{
					case TEXT_CSECT:
						if (firstt_su == 
						     (struct loc_s2 *)0)
							firstt_su = su;
						else
							nextt_su->next = su;
						nextt_su = su;
						break;
					case DATA_CSECT:
						if (firstd_su == 
						    (struct loc_s2 *)0)
							firstd_su = su;
						else
							nextd_su->next = su;
						nextd_su = su;
						break;
					default:
						fatal(0,
	"stat_user references statement which is in unknown csect");
					}

					/* put the type of symbol in the
					   loc_s2 struct */
					su->s.type = type;

					su = su->next;
				}
				while ((su != (struct loc_s2 *)0));
			}
#endif
			/* skip temporary (as20 generated) symbols */
			if (!(sym->syminfo.istemp))
			{
#ifdef DEBUG
		if (sym_debug) dump_sym(nextsymno,sym,symbufptr);
#endif
				nextsymno++;	
				nsymsinbuf++;
				if (nsymsinbuf < NSYMS_IN_BUF)
				{
					symbufptr++;
				}
				else
				{
					/* write out the full buffer */
					write(output,symbuf,SYMBUFSZ);
					bzero(buffer,BUFSZ);
					nsymsinbuf = 0;
					symbufptr = symbuf;
					header.a_syms += SYMBUFSZ;
				}
			}
			
		} while ((sym = sym->next) != (symtabptr)0);
	}
	if (nsymsinbuf)
	{	
		/* write out the partial buffer */
		nsymsinbuf *= sizeof (struct nlist);
		write(output,symbuf,nsymsinbuf );
		header.a_syms += nsymsinbuf;
	}

	}

#ifdef NOTDEF
	/*  make sure the csect chains are terminated! (ie, we
	    got into trouble with this once)  */
	if (firstt_su != (struct loc_s2 *)0)
		nextt_su->next = 
			(struct loc_s2 *)0;
	if (firstd_su != (struct loc_s2 *)0)
		nextd_su->next = 
			(struct loc_s2 *)0;
#endif

	/*  okay, the symbols are written.  Write the size of the
	    string table.
	*/
	strtabsz = STR_EXTENT*(next_strsegno - 1) + 
			nbinlaststrseg + sizeof(long);
		
	write(output,&strtabsz,4);

	/* now write out each string table segment contiguously */
	curstrsegptr = strseg_base;
	for (indx=0;indx < (next_strsegno-1); indx++)
	{
		write(output,*curstrsegptr++,STR_EXTENT);
	}
	write(output,*curstrsegptr,nbinlaststrseg);

	/* the last part to write is the relocation info.  Reposition
	   the file to the beginning of the relocation info.
	*/
	symposition = header.a_text + header.a_data + sizeof(struct exec);
	lseek(output,symposition,0);

	/* now run the statements twice, first writing out the text, then
	   the data relocation information.
	*/
	{
		struct relocation_info *relocbuf;
		register struct relocation_info *relocbufptr;
		register int nrelocsinbuf ;
		register struct statement_s *stat;
		register struct symtab_s *sym;
		register struct loc_s2 *su;
		int relocsegsz ;
		int csect,type;

		/* reuse the code buffer for symbol I/O */
		relocbuf = relocbufptr = (struct relocation_info *)buffer;
		bzero(buffer,BUFSZ);
		csect = TEXT_CSECT;
nextrelocseg:
		stat = first_stat;
		relocsegsz = 0;
		nrelocsinbuf = 0;

		while (stat != (struct statement_s *)0) {

			if ((stat->sinfo.csect == csect)&&
			    ((su = (struct loc_s2 *)stat->sulist)!=(struct loc_s2 *)0)) 

			    do {
				/*  assemble the relocation datum and write it out */
				/* REMOVE THIS WHEN THE LOADER
				   DOES SDIS.  For now, the relocation
				   info for previously relocated 
				   displacements cannot be output,
				   as the loader will attempt
				   the munge it.
				*/
				if ((su->s.isdisp) || (su->s.isnrexpr))
				{
#ifdef DEBUG
					fprintf(stderr,
				"isdisp/nrexpr stat_user found in generate_symbols\n");
#endif
					su->s.isdisp = su->s.isnrexpr = 0;
					su = su->next;
					continue;
				}

				relocbufptr->r_length = su->size >> 1;
				relocbufptr->r_pcrel = su->s.isdisp;
				/* yes, this next stuff is correct, but it is NOT
				   obvious from the minimal a.out documentation
				*/
				sym = su->sym;

				if ((sym->syminfo.isextern)&&
					  (sym->syminfo.isundefined))
						su->s.isextundf = 1;

				if (relocbufptr->r_extern = su->s.isextundf)
				{
					asm("grunt:");
					relocbufptr->r_symbolnum = sym->symno;
				} 
				else 
				{
					if (sym->syminfo.isundefined)
						type = N_UNDF;
					else
						type = csect_to_type[sym->syminfo.csect];

					/* or in the external bit if needed */
					if (sym->syminfo.isextern) type |= N_EXT;

					relocbufptr->r_symbolnum = su->s.type = type;
				}
				/* nor is this obvious from the a.out documentation */
				relocbufptr->r_address = stat->addr + 
					su->offset - base_dot[stat->sinfo.csect];

#ifdef DEBUG
				if (sym_debug) dump_reloc(relocbufptr,stat->line);
#endif
				su = su->next;
				relocsegsz += sizeof(struct relocation_info);
				nrelocsinbuf++;
				if (nrelocsinbuf < NRELOCS_IN_BUF)
					relocbufptr++;
				else {
					write(output,relocbuf,RELOCBUFSZ);
					bzero(buffer,BUFSZ);
					nrelocsinbuf = 0;
					relocbufptr = relocbuf;
				}
			} while (su != (struct loc_s2 *)0);
			stat = stat->next;
		}
		if (nrelocsinbuf)
		{
			write(output,relocbuf,
				(nrelocsinbuf * 
					  sizeof(struct relocation_info)));
			relocbufptr = relocbuf;
		}
		if (csect == TEXT_CSECT)
		{
			/* go do the data segment */
			csect = DATA_CSECT;
			header.a_trsize = relocsegsz;
			goto nextrelocseg;
		}
		else {
			header.a_drsize = relocsegsz;

			/* last, make a consistancy check */
			if ((header.a_drsize + header.a_trsize) != relocinfo_size)
				fatal(0,"mismatch of relocation segment sizes");
		}
	}
}

pack_data(stat)
struct statement_s *stat;
{
	/* the passed statement has unsigneddata which may need to be 
	   packed.  Pack it and check it.
	*/
	long *lptr,long_data;
	union binary target;
	unsigned char *finish;
	int nelems=0;

	lptr = (long *)stat->bin.l;
	target.c = stat->bin.c;
	finish = target.c + stat->len;

	while (target.c < finish) {
		long_data = *lptr;
		nelems++;
		if (stat->sinfo.isbytedata) {
			if ((long_data > 255)||(long_data < (-128))) 
				error(0,
		"element #%d in .byte overflows (value = 0x%x) - truncated.\n",
				nelems,(unsigned)*lptr);
			*target.c++ = (unsigned char)long_data;
		}
		else if (stat->sinfo.isworddata) {
			if ((long_data > 65535)||(long_data < (-32768))) 
				error(0,
		"element #%d in .word overflows (value = 0x%x) - truncated.\n",
				nelems,(unsigned)*lptr);
			*target.s++ = (unsigned short)long_data;
		}
		else
			*target.l++ = long_data;
		lptr++;
	}
}

		

#ifdef DEBUG
dump_sym(i,sym,symbufptr)
struct nlist *symbufptr;
symtabptr sym;
{
	fprintf(stderr,"%d - %s: ",i,sym->name);
	fprintf(stderr," type = 0x%x, value = 0x%x, number = %d\n",
		symbufptr->n_type,symbufptr->n_value,sym->symno);
	

}


dump_reloc(reloc,line)
struct relocation_info *reloc;
{
	fprintf(stderr,"RELOC @ 0x%x, file statement %d, using sym %d. ",
		reloc->r_address,line,
		reloc->r_symbolnum);
	if (reloc->r_pcrel) fprintf(stderr," PC ");
		else fprintf(stderr,"    ");
	if (reloc->r_length == 0) fprintf(stderr," B ");
		else if (reloc->r_length == 1) fprintf(stderr," W ");
		else if (reloc->r_length == 2) fprintf(stderr," L ");
		else fprintf(stderr," ? ");
	if (reloc->r_extern) fprintf(stderr," X \n");
		else fprintf(stderr,"   \n");

}
#endif
