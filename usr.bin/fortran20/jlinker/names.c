/* Copyright (c) 1982 Regents of the University of California */

static char sccsid[] = "@(#)names.c 1.3 2/16/83";

static char rcsid[] = "$Header: /d2/3.7/src/usr.bin/fortran20/jlinker/RCS/names.c,v 1.1 89/03/27 18:01:32 root Exp $";

/*
 * Name are the internal representation for identifiers.
 *
 * A hash table is used to map identifiers to names.
 */

#include "defs.h"
#include "names.h"
#include "symbols.h"
#include "readj.h"

#ifndef public
typedef struct Name *Name;

/*
 * Inline (for speed) function to return the identifier (string)
 * associated with a name.  Because C cannot support both inlines
 * and data hiding at the same time, the Name structure must be
 * publicly visible.  It is not used explicitly, however, outside of this file.
 */

struct Name {
	char	*identifier;
	Name	chain;
	long	tableoff;
};

#define ident(n) ((n == nil) ? "(noname)" : n->identifier)
#endif

#define HASHTABLESIZE 2997

private Name nametable[HASHTABLESIZE];

/*
 * Names are allocated in large chunks to avoid calls to malloc
 * and to cluster names in memory so that tracing hash chains
 * doesn't cause many a page fault.
 */

#define CHUNKSIZE 200

typedef struct Namepool {
    struct Name name[CHUNKSIZE];
    struct Namepool *prevpool;
    struct Namepool *nextpool;
} *Namepool;

private Namepool namepool = nil;
private Namepool firstpool = nil;
private Integer nleft = 0;

/*
 * Given an identifier, convert it to a name.
 * If it's not in the hash table, then put it there.
 *
 * The second argument specifies whether the string should be copied
 * into newly allocated space if not found.
 *
 * Pardon my use of goto's, but it seemed more efficient and less convoluted
 * than adding a collection of boolean variables.  This routine is time
 * critical when starting up the debugger on large programs.
 */

public Name identname(s, isallocated)
String s;
Boolean isallocated;
{
    register unsigned h;
    register Char *p, *q;
    register Name n;
    register Integer len;
    Namepool newpool;

    h = 0;
    for (p = s; *p != '\0'; p++) {
	h = (h << 1) ^ (*p);
    }
    h = h mod HASHTABLESIZE;
    len = p - s;
    n = nametable[h];
    while (n != nil) {
	p = s;
	q = n->identifier;
	for (;;) {
	    if (*p != *q) {
		goto nomatch;
	    } else if (*p == '\0') {
		goto match;
	    }
	    ++p;
	    ++q;
	}
    nomatch:
	n = n->chain;
    }

    /*
     * Now we know that name hasn't been found (otherwise we'd have jumped
     * down to match), so we allocate a name, store the identifier, and
     * enter it in the hash table.
     */
    if (nleft <= 0) {
	newpool = new(Namepool);
	bzero(newpool, sizeof(newpool));
	newpool->prevpool = namepool;
	if (namepool) {
		namepool->nextpool = newpool;
	} else {
		firstpool = newpool;
	}
	newpool->nextpool = nil;
	namepool = newpool;
	nleft = CHUNKSIZE;
    }
    --nleft;
    n = &(namepool->name[nleft]);
    if (isallocated) {
	n->identifier = s;
    } else {
	n->identifier = newarr(char, len + 1);
	p = s;
	q = n->identifier;
	while (*p != '\0') {
	    *q++ = *p++;
	}
	*q = '\0';
    }
    n->chain = nametable[h];
	n->tableoff = dbxhead.stringsize;
	dbxhead.stringsize += strlen(ident(n)) + 1;
    nametable[h] = n;

    /*
     * The two possibilities (name known versus unknown) rejoin.
     */
match:
    return n;
}


public	long
stroff(name)
Name	name;
{

	return(name->tableoff);
}

public
names_dump(ofilep)
FILE	*ofilep;
{
	Namepool	poolp;
	Name		namep;
	char		*namestr;
	int		i;

	for (poolp = firstpool; poolp != nil; poolp = poolp->nextpool) {
		for (i = CHUNKSIZE - 1; i >= (poolp == namepool?nleft:0); i--) {
			namep = &(poolp->name[i]);
			namestr = ident(namep);
			fwrite(namestr, strlen(namestr) + 1, 1, ofilep);
		}
	}
}

/*
 * Deallocate the name table.
 */

public names_free()
{
    Namepool n, m;
    register integer i;

    n = namepool;
    while (n != nil) {
	m = n->prevpool;
	dispose(n);
	n = m;
    }
    for (i = 0; i < HASHTABLESIZE; i++) {
	nametable[i] = nil;
    }
    namepool = nil;
    nleft = 0;
}
