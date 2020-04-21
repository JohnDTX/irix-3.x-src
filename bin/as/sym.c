#include "mical.h"
#include "a.out.h"
#include "scan.h"

/* Allocation increments for symbol buckets and character blocks */
#define	SYM_INCR	50
#define CBLOCK_INCR	512

struct sym_bkt *Last_symbol;			/* last symbol defined */
struct sym_bkt *sym_hash_tab[HASH_MAX];		/* Symbol hash table */
struct sym_bkt *sym_free = NULL;		/* head of free list */
char *cblock = NULL;				/* storage for symbol names */
int ccnt = 0;					/* number of chars left in c block */
extern FILE *List_file;				/* list file descriptor */

/* grab a new symbol bucket off of the free list; allocate space
 * for a new free list if necessary
 */
struct sym_bkt *gsbkt(string)
char *string;
  {	register struct sym_bkt	*sbp;
	register int i;

	if ((sbp = sym_free) != NULL) sym_free = sbp->next_s;
	else {
	  sbp = (struct sym_bkt *)calloc(SYM_INCR,sizeof(struct sym_bkt));
	  if (sbp == NULL)
	    Sys_Error("%s: Symbol bucket storage exceeded\n",string);
	  for (i = SYM_INCR-1; i--;) {
	    sbp->next_s = sym_free;
	    sym_free = sbp++;
	  }
	}

	return(sbp);
}

/* initialize hash table */
Sym_Init()
  {	register int i;

	for (i=0; i<HASH_MAX; i++) sym_hash_tab[i] = NULL;
}

char *sstring(string)
  register char *string;
  {	register char *p,*q;	/* working char string */
	register int i;

	i = strlen(string);	/* get length of string */

	if (++i > ccnt) {	/* if not enough room get more */
	  if ((cblock = (char *)calloc(CBLOCK_INCR,1)) == NULL)
	    Sys_Error("%s: Symbol storage exceeded\n",string);
	  ccnt = CBLOCK_INCR;
	}

	p = q = cblock;		/* copy string into permanent storage */
	while (*p++ = *string++);
	cblock = p;
	ccnt -= i;
	return(q);
}

/* lookup symbol in symbol table */
struct sym_bkt *Lookup(s)
  register char *s;
  {	register struct sym_bkt	*sbp;	/* general purpose ptr */
	register int Save;		/* save subscript in sym_hash_tab */
	register char *p;
	char local[50];			/* used for constructing local sym */

	if (*s>='0' && *s<='9') {	/* local symbol hackery */
	  p = local;
	  while (*p++ = *s++);		/* copy local symbol */
	  p--;
	  s = Last_symbol->name_s;	/* add last symbol defined as suffix */
	  while (*p++ = *s++);
	  s = local;			/* this becomes name to deal with */
	}

	/* if the symbol is already in here, return a ptr to it */
	for (sbp = sym_hash_tab[Save=Hash(s)]; sbp != NULL ; sbp = sbp->next_s)
	  if (sbp->name_s[0]==*s && strcmp(sbp->name_s,s) == 0) return(sbp);

	/* Since it's not, make a bucket for it, and put the bucket in the symbol table */
	sbp = gsbkt(s);				/* get the bucket */
	sbp->name_s = sstring(s);		/* Store it's name */
	sbp->value_s = sbp->id_s = sbp->attr_s = 0;
	sbp->csect_s = NULL;
	sbp->next_s = sym_hash_tab[Save];	/* and insert on top of list */
	if (s == local) sbp->attr_s |= S_LOCAL;
	return(sym_hash_tab[Save] = sbp);
}

/* Sym_Fix -	Assigns index numbers
		to the symbols.  Also performs relocation of
		the symbols assuming data segment follows text
		and bss follows the data.  If global flag,
		make all undefined symbols defined to be externals.
*/
Sym_Fix()
{
	register struct sym_bkt **sbp1, *sbp2;
	int i = 0;

	for (sbp1 = sym_hash_tab; sbp1 < &sym_hash_tab[HASH_MAX]; sbp1++)
	  for (sbp2 = *sbp1; sbp2; sbp2 = sbp2->next_s) {
	    if ((sbp2->attr_s & (S_DEC|S_DEF)) == 0) {
	      sbp2->attr_s |= S_EXT | S_DEC;
	      sbp2->csect_s = NULL;
	    }
	    sbp2->value_s += sdi_inc(sbp2->csect_s, sbp2->value_s);
	    if (sbp2->csect_s == Data_csect) sbp2->value_s += tsize;
	    else if (sbp2->csect_s == Bss_csect) sbp2->value_s += tsize + dsize;
	    if (sbp2 == Dot_bkt || sbp2->attr_s & (S_REG|S_MACRO|S_LOCAL|S_PERM))
	      sbp2->id_s = -1;
	    else sbp2->id_s = i++;
	  }
}

/*
 * Perm	Flags all currently defined symbols as permanent (and therefore
 *	ineligible for redefinition.  Also prevents them from being output
 *	in the object file).
 */
Perm()
  {	register struct sym_bkt **sbp1, *sbp2;

	for (sbp1 = sym_hash_tab; sbp1 < &sym_hash_tab[HASH_MAX]; sbp1++)
		for (sbp2 = *sbp1; sbp2; sbp2 = sbp2->next_s)
			sbp2->attr_s |= S_PERM;
}

#ifdef	SYS_3_AOUTH

/* symbol management */
struct sym {
	char	stype;		/* symbol type */
	char	sympad;		/* pad to long align */
	long	svalue;		/* value */
};

/* Sym_Write -	Write out the symbols to the specified
		file in b.out format, while computing size
		of the symbol segment in output file.
 */
long Sym_Write(file)
  FILE *file;
  {	register struct sym_bkt  **sbp1, *sbp2;
	register char *sp;
	long size = 0;
	int slength;
	struct sym s;
    struct stab_sym_bkt *t;

	s.sympad = (char) 0;
	for (sbp1 = sym_hash_tab; sbp1 < &sym_hash_tab[HASH_MAX]; sbp1++)
	  for (sbp2 = *sbp1; sbp2; sbp2 = sbp2->next_s)
	    if (sbp2->id_s != -1) {
	      if (!(sbp2->attr_s&S_DEF)) s.stype = UNDEF;
	      else if (sbp2->csect_s == Text_csect) s.stype = TEXT;
	      else if (sbp2->csect_s == Data_csect) s.stype = DATA;
	      else if (sbp2->csect_s == Bss_csect) s.stype = BSS;
	      else s.stype = ABS;
	      if (sbp2->attr_s & S_EXT) s.stype |= EXTERN;
	      s.svalue = sbp2->value_s;
	      sp = sbp2->name_s;
	      /** GB **/
	      s.sympad = strlen(sp);
	      if (fwrite(&s, sizeof s, 1, file) != 1)
		Sys_Error("Sym_Write:  fwrite failed\n", (char *) NULL);
	      slength = 0;
	      do { putc(*sp,file); slength++; } while (*sp++);
	      size += sizeof(s) + slength;
	    }
/** GB stabs **/
    /* This is to write out the .stabs and .stabd symbols onto the        */
    /* a.out file.  This is only being written after the regular          */
    /* symbols have been put out (the prior section of the function).     */
    t = stabkt_head;           /* obtain head of stabs/stabd symbol table */
    while (t != NULL)
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
          } /* end while */
	return(size);
}

#else

chkname(name)
struct sym_bkt *name;
{
#ifdef	notdef
	extern char O_Lflag;

	if (O_Lflag)
		return 1;
	if (name->name_s == NULL)
		return 1;
	if ((name->attr_s&S_DEF) == 0)
		return 1;		/* don't zap undef's */
	if (name->name_s[0] == '.')
		return 0;
#endif
	return 1;
}

redosyms()
{
	/* Go through the symbol table and get rid of "L" syms if 
		we are supposed to. */
	register long symnum = 0;
	register struct sym_bkt  **sbp1, *sbp2;


	for (sbp1 = sym_hash_tab; sbp1 < &sym_hash_tab[HASH_MAX]; sbp1++)
	    if (sbp2 = *sbp1)
		for (; sbp2; sbp2 = sbp2->next_s)
		{
		    if (sbp2->id_s != -1 && chkname(sbp2)) {
			    sbp2->final = symnum++;
		    }
		}
}

long Sym_Write(file)
  FILE *file;
  { register struct sym_bkt  **sbp1, *sbp2;
    register char *sp;
    long size = 0;
    struct nlist s;
    int strcount = 4;
    struct stab_sym_bkt *t;
	
    for (sbp1 = sym_hash_tab; sbp1 < &sym_hash_tab[HASH_MAX]; sbp1++)
        if (sbp2 = *sbp1) for (; sbp2; sbp2 = sbp2->next_s)
	   if (sbp2->id_s != -1 && chkname(sbp2)) {
		 /* Write out the symbol table using the 4.2 a.out format */
		 if ((sbp2->attr_s&S_DEF)== 0) {
		    s.n_type = N_UNDF;
		 } else if (sbp2->csect_s)
		    s.n_type = sbp2->csect_s->id_cs;
		 else 
		    s.n_type = N_ABS;
		if (sbp2->attr_s & S_EXT) s.n_type |= N_EXT;
		s.n_value = sbp2->value_s;
		/* For right now, just stuff these with 0 */
		s.n_other = s.n_desc = 0;
		s.n_un.n_strx = strcount;
		strcount += strlen(sbp2->name_s)+1;
		size += sizeof(s);
		fwrite(&s,sizeof(s),1,file);
	    } else if (!(sbp2->attr_s & S_DEF)){
		    Prog_Error( E_UNDEF_L );
	    }
        /* This is to write out the .stabs and .stabd symbols onto the        */
        /* a.out file.  This is only being written after the regular          */
        /* symbols have been put out (the prior section of the function).     */
        t = stabkt_head;           /* obtain head of stabs/stabd symbol table */
        while (t != NULL)
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
              } /* end while */
	return(size);
}

long Str_Write(file)
FILE *file;
{
	long size = 0;
	register struct sym_bkt  **sbp1, *sbp2;
	register char *sp;
	int slength;
	struct nlist s;
	int strcount = 4;
        struct stab_sym_bkt *t;


	fwrite(&strcount,sizeof(long),1,file);
	for (sbp1 = sym_hash_tab; sbp1 < &sym_hash_tab[HASH_MAX]; sbp1++)
		if (sbp2 = *sbp1) for (; sbp2; sbp2 = sbp2->next_s)
		if (chkname(sbp2))
		{
			if (sbp2->id_s != -1) {
				register i;
				strcount +=  i = strlen(sbp2->name_s)+1;
				fwrite(sbp2->name_s,i,1,file);
			}
		}
        /* This is to write out all the symbols (strings) generated by        */
        /* .stabs and .stabd directives.  They are being written onto the     */
        /* string table only after all the regular symbol tables have been    */
        /* written out (by the prior section of this function.                */
        t = stabkt_head;                     /* head of stabs/stabd link list */
        while (t != NULL)
              { register i;
                if (t->id)
                   { strcount += i = t->id + 1;
                     fwrite(t->ch,i,1,file); 
                   }
                t = t->next_stab;
              } /* end while */
	return(strcount);
}

#endif
