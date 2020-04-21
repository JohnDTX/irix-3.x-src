/* Copyright (c) 1982 Regents of the University of California */

static char sccsid[] = "@(#)symbols.c 1.10 8/10/83";

static char rcsid[] = "$Header: /d2/3.7/src/usr.bin/fortran20/jlinker/RCS/symbols.c,v 1.1 89/03/27 18:01:40 root Exp $";

/*
 * Symbol management.
 */

#include "defs.h"
#include "symbols.h"
#include "languages.h"
#include "tree.h"
#include "operators.h"
#include "events.h"
#include "names.h"
#include "readj.h"

#ifndef public
typedef struct Symbol *Symbol;

#include "machine.h"
#include "names.h"
#include "languages.h"
#include "tree.h"

/*
 * Symbol classes
 */

typedef enum {
    BADUSE, CONST, TYPE, VAR, ARRAY, DYNARRAY, SUBARRAY, PTRFILE, RECORD, FIELD,
    PROC, FUNC, FVAR, REF, PTR, FILET, SET, RANGE, 
    LABEL, WITHPTR, SCAL, STR, PROG, IMPROPER, VARNT,
    FPROC, FFUNC, MODULE, TAG, COMMON, EXTREF, TYPEREF
} Symclass;

typedef enum { R_CONST, R_TEMP, R_ARG, R_ADJUST } Rangetype; 

#define INREG 0
#define STK 1
#define EXT 2

typedef unsigned integer Storage;

struct Symbol {
    Name name;
    long language;
    Symclass class : 8;
    Storage storage : 2;
    unsigned int level : 6;	/* for variables stored on stack only */
    Symbol type;
    Symbol chain;
    long	symnum;
    union {
	long constval;		/* value of constant symbol */
	int offset;		/* variable address */
	long iconval;		/* integer constant value */
	double fconval;		/* floating constant value */
	int ndims;		/* no. of dimensions for dynamic/sub-arrays */
	struct {		/* field offset and size (both in bits) */
	    int offset;
	    int length;
	} field;
	struct {		/* common offset and chain; used to relocate */
	    int offset;         /* vars in global BSS */
	    Symbol chain;
	} common;
	struct {		/* range bounds */
            Rangetype lowertype : 16; 
            Rangetype uppertype : 16;  
	    long lower;
	    long upper;
	} rangev;
	struct {
	    int offset : 16;	/* offset for of function value */
	    Boolean src : 1;	/* true if there is source line info */
	    Boolean inline : 1;	/* true if no separate act. rec. */
	    Boolean intern : 1; /* internal calling sequence */
	    Boolean ismain : 1;
	    int unused : 12;
	    Address beginaddr;	/* address of function code */
	} funcv;
	struct {		/* variant record info */
	    int size;
	    Symbol vtorec;
	    Symbol vtag;
	} varnt;
	String typeref;		/* type defined by "<module>:<type>" */
	Symbol extref;		/* indirect symbol for external reference */
    } symvalue;
    Symbol block;		/* symbol containing this symbol */
    Symbol next_sym;		/* hash chain */
};

/*
 * Basic types.
 */

Symbol t_boolean;
Symbol t_char;
Symbol t_int;
Symbol t_real;
Symbol t_dbl;
Symbol t_cmplx;
Symbol t_nil;
Symbol t_addr;

Symbol program;
Symbol curfunc;

boolean showaggrs;

#define symname(s) ident(s->name)
#define codeloc(f) ((f)->symvalue.funcv.beginaddr)
#define isblock(s) (Boolean) ( \
    s->class == FUNC or s->class == PROC or \
    s->class == MODULE or s->class == PROG \
)
#define isroutine(s) (Boolean) ( \
    s->class == FUNC or s->class == PROC \
)

#define nosource(f) (not (f)->symvalue.funcv.src)
#define isinline(f) ((f)->symvalue.funcv.inline)

#include "tree.h"

/*
 * Some macros to make finding a symbol with certain attributes.
 */

#define find(s, withname) \
{ \
    s = lookup(withname); \
    while (s != nil and not (s->name == (withname) and

#define where /* qualification */

#define endfind(s) )) { \
	s = s->next_sym; \
    } \
}

#endif

Symbol	typetable[MAXTYPES];
Symbol	btypetable[NUMSVSTYPS];
long	cursymnum = 1;
/*
 * Symbol table structure currently does not support deletions.
 */

#define HASHTABLESIZE 2003

private Symbol hashtab[HASHTABLESIZE];

#define hash(name) ((((unsigned) name) >> 2) mod HASHTABLESIZE)

/*
 * Allocate a new symbol.
 */
Symbol t_bool4;

#define SYMBLOCKSIZE 100

typedef struct Sympool {
    struct Symbol sym[SYMBLOCKSIZE];
    struct Sympool *prevpool;
} *Sympool;

private Sympool sympool = nil;
private Integer nleft = 0;

public Symbol symbol_alloc()
{
    register Sympool newpool;

    if (nleft <= 0) {
	newpool = new(Sympool);
	bzero(newpool, sizeof(newpool));
	newpool->prevpool = sympool;
	sympool = newpool;
	nleft = SYMBLOCKSIZE;
    }
    --nleft;
	(sympool->sym[nleft]).symnum = cursymnum++;
    return &(sympool->sym[nleft]);
}

public 
symbol_dump (func, ofilep)
Symbol func;
FILE	*ofilep;
{
	Sympool	poolp;
	Symbol	sym;
	int	i;

	    sym = insert(srcf.filename);
            sym->type = nil;
            sym->chain = nil;
	    sym->language = FORTRAN;
	    sym->class = MODULE;
	    sym->symvalue.funcv.beginaddr = 0;
            write_sym(sym, ofilep);
	for (poolp = sympool; poolp != nil; ) {
		for (i = SYMBLOCKSIZE-1;i >= (poolp == sympool?nleft:0); i--) {
			if ((poolp->sym[i]).class != MODULE) {
				write_sym(&(poolp->sym[i]), ofilep);
			}
		}
		poolp = poolp->prevpool;
	}
	symbol_free();
}

/*
 * Free all the symbols currently allocated.
 */

public 
symbol_free()
{
    Sympool s, t;
    register Integer i;

    s = sympool;
    while (s != nil) {
	t = s->prevpool;
	dispose(s);
	s = t;
    }
    for (i = 0; i < HASHTABLESIZE; i++) {
	hashtab[i] = nil;
    }
    sympool = nil;
    nleft = 0;
}
/*
 * Insert a symbol into the hash table.
 */

public Symbol insert(name)
Name name;
{
    register Symbol s;
    register unsigned int h;

    h = hash(name);
    s = symbol_alloc();
    s->name = name;
    s->next_sym = hashtab[h];
    hashtab[h] = s;
    return s;
}

/*
 * Symbol lookup.
 */

public Symbol lookup(name)
Name name;
{
    register Symbol s;
    register unsigned int h;

    h = hash(name);
    s = hashtab[h];
    while (s != nil and s->name != name) {
	s = s->next_sym;
    }
    return s;
}

/*
 * Delete a symbol from the symbol table.
 */

public delete (s)
Symbol s;
{
    register Symbol t;
    register unsigned int h;

    h = hash(s->name);
    t = hashtab[h];
    if (t == nil) {
	fprintf(stderr, "delete of non-symbol '%s'", symname(s));
    } else if (t == s) {
	hashtab[h] = s->next_sym;
    } else {
	while (t->next_sym != s) {
	    t = t->next_sym;
	    if (t == nil) {
		fprintf(stderr, "delete of non-symbol '%s'", symname(s));
	    }
	}
	t->next_sym = s->next_sym;
    }
}

/*
 * Create a builtin type.
 * Builtin types are circular in that btype->type->type = btype.
 */

private Symbol maketype(name, lower, upper)
String name;
long lower;
long upper;
{
    register Symbol s;
    Name n;

    if (name == nil) {
	n = nil;
    } else {
	n = identname(name, true);
    }
    s = insert(n);
    s->language = curlang;
    s->level = 0;
    s->class = TYPE;
    s->type = nil;
    s->chain = nil;
    s->type = newSymbol(nil, 0, RANGE, s, nil);
    s->type->symvalue.rangev.lower = lower;
    s->type->symvalue.rangev.upper = upper;
    return s;
}

/*
 * Create the builtin symbols.
 */

public symbols_init ()
{
    Symbol s;

    t_boolean = maketype("boolean", 0L, 1L);
    btypetable[-SVSLOG1] = t_boolean;
    t_bool4 = maketype("logical", 0L, 0x7fffffffL);
    btypetable[-SVSLOG4] = t_bool4;
    t_int = maketype("integer", 0x80000000L, 0x7fffffffL);
    btypetable[-SVSINT4] = t_int;
    t_char = maketype("char", 0L, 255L);
    btypetable[-SVSCHR1] = t_char;
    t_real = maketype("real", 4L, 0L);
    btypetable[-SVSFLT] = t_real;
    t_dbl = maketype("double", 8L, 0L);
    btypetable[-SVSDBL] = t_dbl;
    t_cmplx = maketype("complex", 8L, 0L);
    btypetable[-SVSCOMPX] = t_cmplx;
    t_nil = maketype("nil", 0L, 0L);
    t_addr = insert(identname("address", true));
    t_addr->language = FORTRAN;
    t_addr->level = 0;
    t_addr->class = TYPE;
    t_addr->type = newSymbol(nil, 1, PTR, t_int, nil);
    s = insert(identname("true", true));
    s->class = CONST;
    s->type = t_boolean;
    s->symvalue.constval = 1L;
    s = insert(identname("false", true));
    s->class = CONST;
    s->type = t_boolean;
    s->symvalue.constval = 0L;
}

/*
 * Initialize symbol information.
 */

private initsyms ()
{
    curblock = nil;
    curlevel = 0;
    nesting = 0;
    program = insert(identname("", true));
    program->class = PROG;
    program->language = primlang;
    program->symvalue.funcv.beginaddr = 0L;
    program->symvalue.funcv.inline = false;
    curmodule = program;
}

Symbol
gettype(typeno)
short	typeno;
{

	Symbol	makedimens();
	if (typeno >= 0) {
		return(typetable[typeno]);
	} else if (typeno < -100) {
		SVSFORDIM	chdim;
		chdim.nextdim = nil;
		chdim.lobound = 0;
		chdim.hibound = -(typeno) - 100;
		chdim.elemsize = 1;
		return(makedimens(&chdim, SVSCHR1));
	} else {
		return(btypetable[-typeno]);
	}
}

write_sym(s, ofilep)
Symbol	s;
FILE	*ofilep;
{

	if (s->type) {
		s->type = (Symbol) s->type->symnum;
	}
	if (s->chain) {
		s->chain = (Symbol) s->chain->symnum;
	}
	if (s->block) {
		s->block = (Symbol) s->block->symnum;
	}
	if (s->name) {
		s->name = (Name) stroff(s->name);
	} else {
		s->name = (Name) -1;
	}
			

	if (fwrite(s, sizeof(struct Symbol), 1, ofilep) != 1) {
		fprintf(stderr, "cannot write output file\n");
		exit (900);
	}
	dbxhead.nsyms++;
}
	
