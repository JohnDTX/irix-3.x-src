#ifdef NOTDEF
struct nlist {
	union {
		char *n_name; /* for use when in-core */
		long n_strx; /* index into file string table */
	} n_un;
	unsigned char n_type; /* type flag, i.e. N_TEXT etc; see below */
	char n_other; /* unused */
	short n_desc; /* see <stab.h> */
	unsigned long n_value; /* value of this symbol (or sdb offset) */
};
#endif
#include "defs.h"
/****************************************************************************

 DEBUGGER - symbol table management

****************************************************************************/
MSG BADFIL;
unsigned maxoff;
long int mainsval;
long int symnum;
long int symbas;
int hexf;
int pid;
struct symb symtab[SYMTABSIZE];
SYMPTR symbol;
SYMPTR symtabb;
SYMPTR symnxt;
SYMPTR symend;
/* space for the 4.2 string table */
char * strtab;

char * symfil;
char * errflg;
unsigned findsym();
BHDR bhdr;

setupsym(ssize)
long ssize /* size of symbols */;
{
	int j;
	long symval,strtabsz;
	SYMPTR symptr;
	char *nameptr;
	extern char *malloc();
	FILE *bout;

	if ((bout = fopen(symfil,"r")) == NULL)
		printf("\n\tcannot fopen %s", symfil);

	/* space for symbol table */
	symtabb = symtab;

#ifdef DEBUG
	printf("seeking to string table at byte 0%o\n",N_STROFF(bhdr));
#endif
	/*seek to string table */
	fseek(bout,N_STROFF(bhdr),0L);
	if (fread(&strtabsz,1,sizeof strtabsz,bout) != sizeof strtabsz)
		printf("\n\tcant read string table size\n");
#ifdef DEBUG
	printf("size of string table is %x\n",strtabsz);
#endif

	/* space for string table /**GB**/
	strtab = malloc(strtabsz+4);

	/* read string table */
	fread(strtab+4,1,strtabsz,bout);

	fseek(bout, symbas, 0); /* seek start of symbols */
#ifdef DEBUG
	printf("seeking to start of symbols at %o, size %o\n",symbas,ssize);
#endif
	symnum = 0;
	symptr = symtabb;

	/* build symbol table */
	while ((ssize > 0) && (symptr < &symtab[SYMTABSIZE]))
	{
		struct nlist temp;
		/* read one symbol from the a.out file */
		if (fread(&temp,1,sizeof(struct nlist),bout) != sizeof(struct nlist))
			break;

		ssize -= sizeof(struct nlist);
		if ((symptr->smtp = SYMTYPE(symptr->symf = temp.n_type)) == NOT_SYM)
			continue;

		symptr->symc = temp.n_un.n_strx + strtab;

		/* Mark the filename symbols. For some obscure reason,
 ld marks them as LOCAL TEXT */
		if ((symptr->symf == N_TEXT)&&(isfnsym(symptr->symc)))
			symptr->symf = N_FN;

		symptr->vals = temp.n_value;

#ifdef DEBUG
		printf("entered %s with val %x, type %x\n",symptr->symc,symptr->vals,
		    symptr->symf);
#endif

		/* if (ssize == 0) break; */
		symnum++;
		symptr++;
	}
	symend = symptr; /* end of symbols */
	symend->smtp = ESYM;
	if (ssize != 0) printf("\n\tsymbol table problem - proceeding anyway");
	if (symptr >= &symtab[SYMTABSIZE])
		printf("\n\tbeware: symbol table overflow");
	fclose(bout);
	if (symptr = lookupsym("_main")) mainsval = symptr->vals;
}

longseek(f, adr)
FILE *f;
long adr;
{
	return(lseek(f, adr, 0) != -1);
}

valpr(v, idsp)
long v;
int idsp;
{
	unsigned w = findsym(v, idsp);
	if (w < maxoff)
	{
		printf("%s", symbol->symc);
		if (w && hexf) printf("+%x", w);
		else if (w) printf("+%d", w);
	}
}

psymoff(v, type, s)
long v;
int type;
char *s;
{
	unsigned w = findsym(v, type);
	if (w >= maxoff) printf("%X", v);
	else
	{
		printf("%s", symbol->symc);
		if (w && hexf) printf("+%x", w);
		else if (w) printf("+%d", w);
	}
	printf(s);
}

isfnsym(cptr)
char *cptr;
{
	/* examine the indicated symbol to determine if it is a filename symbol */

	cptr += strlen(cptr) - 2;
	return ((*cptr++ == '.')&&(*cptr == 'o'));
}

unsigned
findsym(svalue, type)
long svalue;
int type;
{
	long diff = 0377777L, symval;
	SYMPTR symptr;
	SYMPTR symsav = NULL;
	/* NOTE - with 4.2 symbols, we want to find the location
 relative to a symbol OTHER THAN the filename symbols.
 */

	if ((type != NSYM) && (symptr = symtabb))
	{
		while (diff && (symptr->smtp != ESYM))
		{
			if (symptr->symf != N_FN)
			{
				symval = symptr->vals;
				if (((svalue - symval) < diff)
				    && (svalue >= symval))
				{
					diff = svalue - symval;
					symsav = symptr;
				}
			}
			symptr++;
		}
		if (symsav) symbol = symsav;
	}
	return(shorten(diff));
}

nextsym()
{
	if (++symbol == symend)
	{
		symbol--;
		return(FALSE);
	}
	else return(TRUE);
}

symset()
{
	symnxt = symtabb;
}

SYMPTR
symget()
{
	SYMPTR ptr;

	if (symnxt >= symend) return(NULL);
	ptr = symnxt;
	symnxt++;
	return(ptr);
}

