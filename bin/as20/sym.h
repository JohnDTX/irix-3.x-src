#define UNDEF_CSECT 0
#define TEXT_CSECT 1
#define DATA_CSECT 2
#define BSS_CSECT 3
#define ABS_CSECT 4

/* the current CSECT */
int cur_csect;	

/* the current location in each CSECT */
int dot[5];
int base_dot[5];
int words_removed[5];

union  binary
{
	unsigned long *l;
	unsigned short *s;
	unsigned char *c;
} ;
	
/** NOTE!! the stab_s structure in stab.c must overlay the statement_s
    structure..... Make sure that they agree...
*/

struct statement_s 
{
	/* lab is a label defined by this statement. */
	struct symtab_s * lab;
	/* jumplab is the jump target for sdis, or the symbol
	   table entry for the equates
	*/
	union
	{
		/*  symbol table entry for symbols on a comm or globl line */
		struct symtab_s * auxsym;
		/*  the jump target for sdis */
		struct symtab_s * displab;
		/*  an error entry for listings 
		struct error_s * errorptr;
		*/
		/*  a symbol table pointer to one of the equated symbols */
		struct symtab_s * equatesym;
	} smulti;
	/* the line number of this statement */
	int line;
	/* the length of this binary data */
	int len;
	/* the first-estimate addr of the beginning of this statement */
	unsigned long addr;
	struct 
	{
	    unsigned 
		/* set if this statement does NOT have to be word aligned */
		isbytealigned:1,
		/* set if this instruction is a span-dependent instr */
		issdi:1,
		/* set if this statement is zeroed data of the given length */
		iszero:1,
		/* set if a zero byte must be output PRIOR to the data
		   in this statement 
		*/
		mustpad:1,
		/* set if this statement is a symbol table equates set only
		*/
		isequates:1,
		/* set if this statement is really a stab.  If so, the
		   statement_s structure is really a stab_s structure.
		*/
		isstab:1,
		isjsr:1,
		isunsigneddata:1,
		isbytedata:1,
		isworddata:1,
		:1,
		/* this bit is used by the stab_s structure which overlays
		   this one.... see stab.c
		*/
		finalexpisdot:1,
		/* the csect this data belongs in */
		csect:4,
		/* a bit is set in this field when the corresponding word
		of the original instruction has been deleted */
		wordsdeleted:16;
	} sinfo;
	struct loc_s *sulist;
	struct statement_s * next;
	union binary bin;
};

struct statement_s *first_stat;

struct loc_s
{
	struct loc_s *next;
	/* the loc_s chain can be used to link symbols which
	   need fixing up (in relation to other symbols)
	   or statements which need fixing up.
	*/
	struct generic_s *stat;
	/* offset into binary data in the indicated statement of
	   this relocation datum, and whether the size is to
	   be interpreted in bits or bytes.
	*/
	unsigned char offset,size;
	unsigned char new_size,pad;
	struct 
	{ unsigned
		/*  size of the binary datum this symbol is inhabiting,
	    	and whether the datum is RELATIVE (a displacement)
	    	or the actual address.  
		*/
		isdisp:1,
		isbitsize:1,

		/*  whether this datum can be condensed */ 
		canbecondensed:1,

		/*  the symbol indicated should be negated before
		    adding to the location in questions 
		*/
		isnrexpr:1,
		canbebytedisp:1,
		symisneg:1,

		:10,
		/* the bit to twiddle in the instruction when the condensation
	    	   is performed.  (condensation: long -> word)
		*/
		iswordbit:16;
	}s;
};

/*
   	the use chain elements point to locations in the 
	output which reference this symbol.
*/
struct symtab_s 
{
	char *name;
	struct symtab_s * next;
	/* address of this symbol in the indicated csect. Multiplexed
	   as the length of comm directives, if the csect indicates
	   undefined. */
	long addr;
	/* the number of this symbol when writing out symbols */
	long symno;
	/* pointer to the statement which defines (has as 
	   a label) this symbol 
	*/
	struct statement_s * def;
	struct
	{
		/* in the case of symbols which are labels,
		   the line number this symbol was defined on 
		*/
		int line;
		/* in the case of equated (absolute) symbols, the 
		   chain of symbols which are equated to this one, 
		   and need patching */
		/* in the case of user-defined temporary labels,
		   this points to the real (static) temp label
		   assigned for this symbol, and is really a symtabptr.
		*/
		struct loc_s * sym_users;
	}u;
	struct 
	{
		unsigned 
			/* set if externally defined */
			isextern:1,
			/* set if a temporary label */
			istemp:1,
			/* set if undefined */
			isundefined:1,
			/* set if word value, else long */
			isword:1,
			isbyte:1,
			isusertemp:1,
			isstab:1,
			:1,
			/* the csect */
			csect:8,
			/* the number of the string segment this symbol is in,
			   if the symbol is to be in the .o file 
			*/
			strseg:16;
	} syminfo;

		
};
typedef struct symtab_s symtab_t, *symtabptr;

symtabptr labsym;

/*  all of the hash tables use a generic structure to which elements
    are cast so that a generic set of handling routines can be used.
    All of the structures that are to be placed in these chains 
    have a common base, as described in the generic_t structure 
*/
struct generic_s { char *name; struct generic_s *next; unsigned long num;} ;
typedef struct generic_s generic_t ;

generic_t *lookup();
#define NSYMBUCKETS 0x100
/* the last symbol bucket is a dummy one for stab symbols */
symtabptr sym_bucket[NSYMBUCKETS+1];

symtabptr define_label();
struct statement_s * allocate_statement();
symtabptr allocate_symbol(), getsym();
unsigned char *allocate_binary();
struct loc_s * allocate_locs();

struct loc_s *statuser_updatelist_head;
struct loc_s *symuser_updatelist_head;
struct loc_s *deflab_updatelist_head;
struct loc_s *deflab_updatelist_tail;

struct statement_s * cur_stat, * last_stat;

int next_strsegno,next_statuserno;
int nbinlaststrseg;

#define NOADDR (-1)

#define ISBITSZ 1
#define ISDISP 2
#define SYMISNEG 4
#define CANBECONDENSED 8
#define CANBEBYTEDISP 0x10
#define ISNREXPR 0x20

#define I_BSR 0x61ff

