#ifndef symbols_h
#define symbols_h
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

typedef unsigned int Storage;

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

Symbol symbol_alloc(/*  */);
symbol_dump (/* func */);
symbol_free(/*  */);
Symbol newSymbol(/* name, blevel, class, type, chain */);
Symbol insert(/* name */);
Symbol lookup(/* name */);
delete (/* s */);
dumpvars(/* f, frame */);
symbols_init (/*  */);
Symbol rtype(/* type */);
resolveRef (/* t */);
integer regnum (/* s */);
Symbol container(/* s */);
Node constval(/* s */);
Address address (/* s, frame */);
defregname (/* n, r */);
findtype(/* s */);
findbounds (/* u, lower, upper */);
integer size(/* sym */);
integer psize (/* s */);
Boolean isparam(/* s */);
boolean isopenarray (/* type */);
Boolean isvarparam(/* s */);
Boolean isvariable(/* s */);
Boolean isconst(/* s */);
Boolean ismodule(/* s */);
markInternal (/* s */);
boolean isinternal (/* s */);
Boolean isbitfield(/* s */);
Boolean compatible(/* t1, t2 */);
Boolean istypename(/* type, name */);
boolean passaddr (/* p, exprtype */);
Boolean isambiguous(/* s */);
assigntypes (/* p */);
Node dot(/* record, fieldname */);
Node subscript(/* a, slist */);
int evalindex(/* s, base, i */);
chkboolean(/* p */);
unmkstring(/* s */);
Symbol which (/* n */);
Symbol findfield (/* fieldname, record */);
Boolean getbound(/* s,off,type,valp */);
#endif


extern Symbol	curblock;
extern	int	curlevel;
