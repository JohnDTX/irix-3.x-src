
#include "defs.h"
#include "symbols.h"
#include "languages.h"
#include "tree.h"
#include "operators.h"
#include "events.h"
#include "names.h"
#include "readj.h"



#define newSym(s, n) \
{ \
    s = insert(n); \
    s->level = curblock->level + 1; \
    s->language = curlang; \
    s->block = curblock; \
}

#define makeVariable(s, n, off) \
{ \
    newSym(s, n); \
    s->class = VAR; \
    s->symvalue.offset = off; \
    getType(s); \
}

#define makeParameter(s, n, cl, off) \
{ \
    newSym(s, n); \
    s->storage = STK; \
    s->class = cl; \
    s->symvalue.offset = off; \
    curparam->chain = s; \
    curparam = s; \
    getType(s); \
}


/*
 * Create a new symbol with the given attributes.
 */

public Symbol newSymbol(name, blevel, class, type, chain)
Name name;
Integer blevel;
Symclass class;
Symbol type;
Symbol chain;
{
	register Symbol s;

	s = symbol_alloc();
	s->name = name;
	s->language = curlang;
	s->storage = EXT;
	s->level = blevel;
	s->class = class;
	s->type = type;
	s->chain = chain;
	return s;
}

public Symbol
makesymbol(name, blevel, class, type, chain)
Name name;
Integer blevel;
Symclass class;
Symbol type;
Symbol chain;
{
	register Symbol s;

	s = insert(name);
	s->name = name;
	s->language = curlang;
	s->storage = EXT;
	s->level = blevel;
	s->class = class;
	s->type = type;
	s->chain = chain;
	return s;
}


public	Symbol
makedimens(svsdimptr, typeno)
SVSFORDIM	*svsdimptr;
short	typeno;
{

	Symbol	newdim;
	Symbol	gettype();
	
	while (svsdimptr) {
		newdim = newSymbol(nil, 0, ARRAY, nil, nil);
		newdim->chain = newSymbol(nil, 0, RANGE,
				gettype(typeno), nil);
		if (typeno == SVSCHR1) {
			svsdimptr->lobound++;
		}
		newdim->chain->symvalue.rangev.lower = svsdimptr->lobound;
		newdim->chain->symvalue.rangev.upper = svsdimptr->hibound;
		newdim->chain->block = nil;
		newdim->type = makedimens(svsdimptr->nextdim, typeno);
		newdim->block = nil;
		if (newdim->type == nil) {
			newdim->type = gettype(typeno);
		}
		return(newdim);
	}
	return(nil);
}

