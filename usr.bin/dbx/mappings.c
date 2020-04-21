/* Copyright (c) 1982 Regents of the University of California */

static char sccsid[] = "@(#)mappings.c 1.4 8/10/83";

static char rcsid[] = "$Header: /d2/3.7/src/usr.bin/dbx/RCS/mappings.c,v 1.1 89/03/27 17:44:38 root Exp $";

/*
 * Source-to-object and vice versa mappings.
 */

#include "defs.h"
#include "mappings.h"
#include "symbols.h"
#include "source.h"
#include "object.h"
#include "machine.h"
#include "main.h"

#ifndef public
#include "machine.h"
#include "source.h"
#include "symbols.h"

typedef struct {
    Address addr;
    String filename;
    Lineno lineindex;		/* index to first linetab entry */
	long	nlines;
} Filetab;

typedef struct {
    Lineno line;
    Address addr;
} Linetab;

Filetab *filetab;
Linetab *linetab;

#define NOADDR ((Address) -1)	/* no address for line or procedure */

#endif

/*
 * Get the source file name associated with a given address.
 */

public String srcfilename(addr)
Address addr;
{
    register Address i, j, k;
    Address a;
    Filetab *ftp;
    String s;

    s = nil;
    if (nlhdr.nfiles != 0 and addr >= filetab[0].addr) {
	i = 0;
	j = nlhdr.nfiles - 1;
	while (i < j) {
	    k = (i + j) / 2;
	    ftp = &filetab[k];
	    a = ftp->addr;
	    if (a == addr) {
		s = ftp->filename;
		break;
	    } else if (addr > a) {
		i = k + 1;
	    } else {
		j = k - 1;
	    }
	}
	if (s == nil) {
	    if (addr >= filetab[i].addr) {
		s = filetab[i].filename;
	    } else {
		s = filetab[i-1].filename;
	    }
	}
    }
    return s;
}

/*
 * Find the line associated with the given address.
 * If the second parameter is true, then the address must match
 * a source line exactly.  Otherwise the nearest source line
 * below the given address is returned.
 *
 * Return the index of the line table entry or -1 if none suitable.
 */

#ifdef NOTDEF
private integer findline (addr, exact)
Address addr;
Boolean exact;
{
    register Address i, j, k;
    register Lineno r;
    register Address a;

    if (nlhdr.nlines == 0 or addr < linetab[0].addr) {
	r = -1;
    } else {
	i = 0;
	j = nlhdr.nlines - 1;
	if (addr == linetab[i].addr) {
	    r = i;
	} else if (addr == linetab[j].addr) {
	    r = j;
	} else if (addr > linetab[j].addr) {
	    r = exact ? -1 : j;
	} else {
	    do {
		k = (i + j) div 2;
		a = linetab[k].addr;
	    if (a == addr) break;
		if (addr > a) {
		    i = k + 1;
		} else {
		    j = k - 1;
		}
	    } while (i <= j);
	    if (a == addr) {
		r = k;
	    } else if (exact) {
		r = -1;
	    } else if (addr > linetab[i].addr) {
		r = i;
	    } else {
		r = i - 1;
	    }
	}
    }
    return r;
}
#endif

private integer findline (addr, exact)
Address addr;
Boolean exact;
{
    register Address i, j, k;
    register Lineno r;
    register Address a;
    Filetab	*ftp;
	int	firstline;
    Linetab *tlinetab;

    if (nlhdr.nlines == 0) { 
	r = -1;
    } else if (addr < filetab[0].addr) {
	if (exact == true) {
		return(-1);
	} else {
		return(filetab[0].lineindex);
	}
    } else {
	i = 0;
	j = nlhdr.nlines - 1;
	tlinetab = linetab;
	firstline = -1;
    	for (ftp = &filetab[0]; ftp < &filetab[nlhdr.nfiles]; ftp++) {
/*
fprintf(stderr, "file:%s first:%x %d %d last:%x %d %d\n", ftp->filename, linetab[ftp->lineindex].addr, linetab[ftp->lineindex].line, ftp->lineindex, linetab[ftp->lineindex + ftp->nlines -1].addr, linetab[ftp->lineindex + ftp->nlines - 1].line, ftp->lineindex+ftp->nlines - 1);
*/
	    if (ftp->nlines == 0) {
		continue;
	    }
	    if ((ftp != &(filetab[0])) && (linetab[ftp->lineindex].addr > addr) 
		&& (linetab[(ftp-1)->lineindex].addr < addr)) {
		if (exact == true) {
			return(-1);
		} else {
			return(ftp->lineindex);
		}
	    }
            if ((linetab[ftp->lineindex].addr <= addr) 
                    && ((linetab[ftp->lineindex + ftp->nlines - 1].addr) >= addr)) {
                firstline = ftp->lineindex;
		tlinetab = (&linetab[firstline]);
		i = 0;
		j = ftp->lineindex + ftp->nlines - firstline - 1;
		break;
            }
        }
        if (firstline == -1) {
		r = -1;
	} else if (addr == tlinetab[i].addr) {
	    r = i;
	} else if (addr == tlinetab[j].addr) {
	    r = j;
	} else if (addr > tlinetab[j].addr) {
	    r = exact ? -1 : j;
	} else {
	    do {
		k = ((i + j)) div 2;
		a = tlinetab[k].addr;
	    if (a == addr) break;
		if (addr > a) {
		    i = k + 1;
		} else {
		    j = k - 1;
		}
	    } while (i <= j);
	    if (a == addr) {
		r = k;
	    } else if (exact) {
		r = -1;
	    } else if (addr > tlinetab[i].addr) {
		r = i;
	    } else {
		r = i - 1;
	    }
	}
    }
    if (r == -1) {
	return(-1);
    } else {
    	return r + firstline;
    }
}

/*
 * Lookup the source line number nearest (from below) to an address.
 *
 * It is possible (unfortunately) that the compiler will generate
 * code before line number for a procedure.  Therefore we check
 * to see that the found line is in the same procedure as the given address.
 * If it isn't, then we walk forward until the first suitable line is found.
 */

public Lineno srcline (addr)
Address addr;
{
    Lineno i, r;
    Symbol f1, f2;
    Address a;

    i = findline(addr, false);
    if (i == -1) {
	f1 = whatblock(addr);
	if (f1 == nil or nosource(f1)) {
	    r = 0;
	} else {
	    a = codeloc(f1);
	    for (;;) {
		r = linelookup(a);
		if (r != 0 or a >= codestart + objsize) {
		    break;
		}
		++a;
	    }
	}
    } else {
	r = linetab[i].line;
	if (linetab[i].addr != addr) {
	    f1 = whatblock(addr);
	    if (nosource(f1)) {
		r = 0;
	    } else {
		f2 = whatblock(linetab[i].addr + 1);
		if (f1 != f2) {
		    do {
			++i;
		    } while (linetab[i].addr < addr and i < nlhdr.nlines);
		    r = linetab[i].line;
		}
	    }
	}
    }
    return r;
}

/*
 * Look for a line exactly corresponding to the given address.
 */

public Lineno linelookup(addr)
Address addr;
{
    integer i;
    Lineno r;

    i = findline(addr, true);
    if (i == -1) {
	r = 0;
    } else {
	r = linetab[i].line;
    }
    return r;
}

/*
 * Lookup the object address of a given line from the named file.
 *
 * Potentially all files in the file table need to be checked
 * until the line is found since a particular file name may appear
 * more than once in the file table (caused by includes).
 */

public Address objaddr(line, name)
Lineno line;
String name;
{
    register Filetab *ftp;
    register Lineno i, j;
    Boolean foundfile;

    if (nlhdr.nlines == 0) {
	return NOADDR;
    }
    if (name == nil) {
	name = cursource;
    }
    foundfile = false;
    for (ftp = &filetab[0]; ftp < &filetab[nlhdr.nfiles]; ftp++) {
	if (streq(ftp->filename, name)) {
	    foundfile = true;
	    i = ftp->lineindex;
	    if (ftp == &filetab[nlhdr.nfiles-1]) {
		j = nlhdr.nlines;
	    } else {
		j = (ftp + 1)->lineindex;
	    }
		j = ftp->lineindex + ftp->nlines;
	    while (i < j) {
		if (linetab[i].line == line) {
		    return linetab[i].addr;
		}
		i++;
	    }
	}
    }
    if (not foundfile) {
	error("source file \"%s\" not compiled with -g", name);
    }
    return NOADDR;
}

/*
 * Table for going from object addresses to the functions in which they belong.
 */

#define NFUNCS 4000	/* initial size of function table */

typedef struct {
    Symbol func;
    Address addr;
} AddrOfFunc;

private AddrOfFunc *functab;
private int nfuncs = 0;
private int functablesize = 0;

/*
 * Insert a new function into the table.
 */

public newfunc(f, addr)
Symbol f;
Address addr;
{
    register AddrOfFunc *af;
    register int i;
    AddrOfFunc *newfunctab;

    if (nfuncs >= functablesize) {
	if (functablesize == 0) {
	    functab = newarr(AddrOfFunc, NFUNCS);
	    functablesize = NFUNCS;
	} else {
	    functablesize *= 2;
	    newfunctab = newarr(AddrOfFunc, functablesize);
	    bcopy(functab, newfunctab, nfuncs * sizeof(AddrOfFunc));
	    dispose(functab);
	    functab = newfunctab;
	}
    }
    af = &functab[nfuncs];
    af->func = f;
    af->addr = addr;
    ++nfuncs;
}

/*
 * Return the function that begins at the given address.
 */

public Symbol whatblock(addr)
Address addr;
{
    register int i, j, k;
    Address a;

    i = 0;
    j = nfuncs - 1;

    if ((!functab) || (addr < functab[i].addr)) {
	return program;
    } else if (addr == functab[i].addr) {
	return functab[i].func;
    } else if (addr >= functab[j].addr) {
	return functab[j].func;
    }
    while (i <= j) {
	k = (i + j) / 2;
	a = functab[k].addr;
	if (a == addr) {
	    return functab[k].func;
	} else if (addr > a) {
	    i = k+1;
	} else {
	    j = k-1;
	}
    }
    if (addr > functab[i].addr) {
	return functab[i].func;
    } else {
	return functab[i-1].func;
    }
    /* NOTREACHED */
}

/*
 * Order the functab.
 */

private int cmpfunc(f1, f2)
AddrOfFunc *f1, *f2;
{
    register Address a1, a2;

    a1 = (*f1).addr;
    a2 = (*f2).addr;
    return ( (a1 < a2) ? -1 : ( (a1 == a2) ? 0 : 1 ) );
}

public ordfunctab()
{
    qsort(functab, nfuncs-1, sizeof(AddrOfFunc), cmpfunc);
}

int
cmpfile(file1, file2)
Filetab	*file1;
Filetab	*file2;
{

	register Address a1, a2;

	a1 = (*file1).addr;
	a2 = (*file2).addr;
	return ( (a1 < a2) ? -1 : ( (a1 == a2) ? 0 : 1 ) );
}

public ordfiletab()
{
	qsort(filetab, nlhdr.nfiles, sizeof(Filetab), cmpfile);
}

/*
 * Clear out the functab, used when re-reading the object information.
 */

public clrfunctab()
{
    nfuncs = 0;
}

public dumpfunctab()
{
    int i;

    for (i = 0; i < nfuncs; i++) { 
	fprintf(stderr, "functab[i].addr=%x \n", functab[i].addr);
    }
}

Address
endaddr(line, name)
int	line;
String	name;
{
    register Filetab *ftp;
    register Lineno i, j;
    Boolean foundfile;

    if (nlhdr.nlines == 0) {
	return NOADDR;
    }
    if (name == nil) {
	name = cursource;
    }
    foundfile = false;
    for (ftp = &filetab[0]; ftp < &filetab[nlhdr.nfiles]; ftp++) {
	if (streq(ftp->filename, name)) {
	    foundfile = true;
	    i = ftp->lineindex;
	    if (ftp == &filetab[nlhdr.nfiles-1]) {
		j = nlhdr.nlines;
	    } else {
		j = (ftp + 1)->lineindex;
	    }
		j = ftp->lineindex + ftp->nlines;
	    while (i < j) {
		if (linetab[i].line == line) {
		    return linetab[i+1].addr;
		}
		i++;
	    }
	}
    }
    if (not foundfile) {
	error("source file \"%s\" not compiled with -g", name);
    }
    return NOADDR;
}


disass(lp1, lp2)
Node	lp1;
Node	lp2;
{
	int	i;
	int	lineno1;
	int	lineno2;
	Address	startaddr;
	Address	end_addr;

	lineno1 = lp1->value.lcon;
	if (lp2 == nil) {
		lineno2 = lineno1;
	} else {
		lineno2 = lp2->value.lcon;
	}
	for (i = lineno1; i <= lineno2; i++) {

		printf("%d:\n", i);
		startaddr = objaddr(i, cursource);
		end_addr = endaddr(i, cursource);
		printinst1(startaddr, end_addr);
	}
}

#ifdef DBMEX
#ifdef notdef
dumpfiles() {

	int	i;
	char	p[512];
	char	*pptr;
	char	*s;

	shmptr->lineno = nlhdr.nfiles;
	pptr = p;
	for (i = 0; i < nlhdr.nfiles; i++) {
		s = findsource(filetab[i].filename);
		if (s == nil) {
			strcpy(pptr, filetab[i].filename);
			pptr += strlen(filetab[i].filename);
		} else {
			if (strncmp(s, "./", 2) == 0) {
				s += 2;
			}
			strcpy(pptr, s);
			pptr += strlen(s);
		}
		*pptr = '\0';
		pptr++;
	}
	shm_write('F', p, nlhdr.nfiles, pptr - p);
}
#endif

dumpfiles() {

	int	i;
	char	p[512];
	char	*pptr;
	char	*s;
	int	filecount;
	int	j = 0;
	Boolean	first_one = true;

	pptr = p;
	filecount = 0;
	for (i = 0; i < nlhdr.nfiles; i++) {
		s = findsource(filetab[i].filename);
		if (s == nil) {
			continue;
		} else {
			if (strncmp(s, "./", 2) == 0) {
				s += 2;
			}
			if (((int) (pptr - p) + strlen(s)) > 512) {
				if (first_one == true) {
					shm_write('F', p, j, 
						(int) (pptr - p), "1");
					first_one = false;
				} else {
					shm_write('F', p, j, 
						(int) (pptr - p), "M");
				}
				pptr = p;
				j = 0;
			}
			strcpy(pptr, s);
			pptr += strlen(s);
		}
		j++;
		*pptr = '\0';
		pptr++;
	}
	if (first_one) {
		shm_write('F', p, j, (int) (pptr - p), "1");
	} else {
		shm_write('F', p, j, (int) (pptr - p), "M");
	}
	shm_write('F', p, j, (int) (pptr - p), "D");
}
#endif
