/* Copyright (c) 1982 Regents of the University of California */

static char sccsid[] = "@(#)object.c 1.14 10/22/83";

static char rcsid[] = "$Header: /d2/3.7/src/usr.bin/dbx/RCS/object.c,v 1.1 89/03/27 17:44:43 root Exp $";

/*
 * Object code interface, mainly for extraction of symbolic information.
 */

#include "defs.h"
#include "object.h"
#include "stabstring.h"
#include "main.h"
#include "symbols.h"
#include "names.h"
#include "languages.h"
#include "mappings.h"
#include "lists.h"
#include "c.h"
#include "fortran.h"
/*
#include <sys/types.h>
#include <sys/stat.h>
*/
#include <a.out.h>
#include <stab.h>
#include <ctype.h>
#include "newobj.h"
#include "process.h"

#ifndef public

struct {
    unsigned int stringsize;	/* size of the dumped string table */
    unsigned int nsyms;		/* number of symbols */
    unsigned int nfiles;	/* number of files */
    unsigned int nlines;	/* number of lines */
} nlhdr;

#include "languages.h"
#include "symbols.h"

#endif

#ifndef N_MOD2
#    define N_MOD2 0x50
#endif

public String objname = "a.out";
public integer objsize;
public unsigned long codestart;

public Language curlang;
public Symbol curmodule;
public Symbol curparam;
public Symbol curcomm;
public Symbol commchain;
private havenewsymt;

private char *stringtab;
private struct nlist *curnp;
private Boolean warned;
private Boolean strip_ = false;

private Filetab *filep;
private Linetab *linep, *prevlinep;
Filetab *setfileaddr();
Filetab *elfilep;
boolean stripped_;

public String curfilename ()
{
    return ((filep-1)->filename);
}

/*
 * Blocks are figured out on the fly while reading the symbol table.
 */

#define MAXBLKDEPTH 25

public Symbol curblock;

private Symbol blkstack[MAXBLKDEPTH];
integer curlevel;
private integer bnum, nesting;
private Address addrstk[MAXBLKDEPTH];

public pushBlock (b)
Symbol b;
{
    if (curlevel >= MAXBLKDEPTH) {
	fatal("nesting depth too large (%d)", curlevel);
    }
    blkstack[curlevel] = curblock;
    ++curlevel;
    curblock = b;
    if (traceblocks) {
	printf("entering block %s\n", symname(b));
    }
}

/*
 * Change the current block with saving the previous one,
 * since it is assumed that the symbol for the current one is to be deleted.
 */

public changeBlock (b)
Symbol b;
{
    curblock = b;
}

public enterblock (b)
Symbol b;
{
    if (curblock == nil) {
	b->level = 1;
    } else {
	b->level = curblock->level + 1;
    }
    b->block = curblock;
    pushBlock(b);
}

public exitblock ()
{
    if (curblock->class == FUNC or curblock->class == PROC) {
	if (prevlinep != linep) {
	    curblock->symvalue.funcv.src = true;
	}
    }
    if (curlevel <= 0) {
	panic("nesting depth underflow (%d)", curlevel);
    }
    --curlevel;
    if (traceblocks) {
	printf("exiting block %s\n", symname(curblock));
    }
    curblock = blkstack[curlevel];
}

/*
 * Enter a source line or file name reference into the appropriate table.
 * Expanded inline to reduce procedure calls.
 *
 * private enterline (linenumber, address)
 * Lineno linenumber;
 * Address address;
 *  ...
 */

#define enterline(linenumber, address) \
{ \
    register Linetab *lp; \
 \
    lp = linep - 1; \
    if (linenumber != lp->line) { \
	if (address != lp->addr) { \
	    elfilep->nlines++;\
	    ++lp; \
	} \
	lp->line = linenumber; \
	lp->addr = address; \
	linep = lp + 1; \
    } \
}

/*
 * Read in the namelist from the obj file.
 *
 * Reads and seeks are used instead of fread's and fseek's
 * for efficiency sake; there's a lot of data being read here.
 */

public readobj (file)
String file;
{
    Fileid f;
    struct exec hdr;
    struct nlist nlist;
#ifdef SVS && sgi
	long	pad;
	struct	stat	statbuf;
#endif

    f = open(file, 0);
    if (f < 0) {
	fatal("can't open %s", file);
    }
    read(f, &hdr, sizeof(hdr));
    if (N_BADMAG(hdr)) {
	objsize = 0;
	nlhdr.nsyms = 0;
	nlhdr.nfiles = 0;
	nlhdr.nlines = 0;
	codestart = CODESTART;
    } else {
	objsize = hdr.a_text;
	nlhdr.nsyms = hdr.a_syms / sizeof(nlist);
	nlhdr.nfiles = nlhdr.nsyms;
	nlhdr.nlines = nlhdr.nsyms;
#ifdef sgi
	codestart = hdr.a_entry;
#else
	codestart = CODESTART;
#endif
    }
    if (nlhdr.nsyms > 0) {
	long dbxfiloff;
	long	stringsz;
	lseek(f, (long) N_STROFF(hdr), 0);
	read(f, &(nlhdr.stringsize), sizeof(nlhdr.stringsize));
	stringsz = nlhdr.stringsize;
	nlhdr.stringsize -= 4;
	stringtab = newarr(char, nlhdr.stringsize);
	read(f, stringtab, nlhdr.stringsize);
#ifdef SVS && sgi
	fstat(f, &statbuf);
	modtime = statbuf.st_mtime;
	pad = BLKSIZE - ((N_STROFF(hdr) + stringsz) % BLKSIZE);
	dbxfiloff = N_STROFF(hdr) + stringsz + (pad ) ;
	if (dbxfiloff < statbuf.st_size) {
		int	nlines;
		lseek(f, dbxfiloff, 0);
		if (read(f, &dbxhead, sizeof(DBXHEADER)) == -1) {
			panic("cannot read dbxheader\n");
		}
		lseek(f, dbxfiloff, 0);
		nlines = statbuf.st_size - (dbxfiloff + sizeof(DBXHEADER) 
			+ (dbxhead.nsyms * sizeof(struct Symbol)) +
			dbxhead.stringsize);
		nlines /= sizeof(LINEADDR);
		nlhdr.nlines += nlines;
		allocmaps(nlhdr.nfiles, nlhdr.nlines);
		rdbinst(f, dbxfiloff);
		havenewsymt = 1;
	} else {
		allocmaps(nlhdr.nfiles, nlhdr.nlines);
	}
#else
	allocmaps(nlhdr.nfiles, nlhdr.nlines);
#endif
	lseek(f, (long) N_SYMOFF(hdr), 0);
	readsyms(f);
	ordfunctab();
#ifdef SVS
	if (havenewsymt) {
		rdbinlt(f, N_STROFF(hdr) + stringsz + pad);
	}
#endif
	setnlines();
	setnfiles();
	ordfiletab();
#ifdef DBMEX
	if (winmode == true) {
		dumpfiles();
	}
#endif
    } else {
	fatal("Program has been stripped, use adb");
	initsyms();
    }
    close(f);
}

/*
 * Found the beginning of the externals in the object file
 * (signified by the "-lg" or find an external), close the
 * block for the last procedure.
 */

private foundglobals ()
{
    if (curblock->class != PROG) {
	exitblock();
	if (curblock->class != PROG) {
	    exitblock();
	}
    }
    if (linep > linetab) {

    	enterline(0, (linep-1)->addr + 1);
    }
}

/*
 * Read in symbols from object file.
 */
int readcount;
private readsyms (f)
Fileid f;
{
    struct nlist *namelist;
    register struct nlist *np, *ub;
    register String name;
    boolean afterlg, foundstab;
    integer index;
    char *lastchar;

	
#ifdef SVS
	if (!havenewsymt) {
#endif
    		initsyms();
#ifdef SVS
	}
#endif
    namelist = newarr(struct nlist, nlhdr.nsyms);
    read(f, namelist, nlhdr.nsyms * sizeof(struct nlist));
    afterlg = false;
    foundstab = false;
    ub = &namelist[nlhdr.nsyms];
    curnp = &namelist[0];
    np = curnp;
    while (np < ub) {
	index = np->n_un.n_strx;
	if (index != 0) {
	    name = &stringtab[index - 4];
	    /*
             *  If the program contains any .f files a trailing _ is stripped
       	     *  from the name on the assumption it was added by the compiler.
	     *  This only affects names that follow the sdb N_SO entry with
             *  the .f name. 
             */
            if (strip_ and name[0] != '\0' ) {
		lastchar = &name[strlen(name) - 1];
		if (*lastchar == '_') {
		    *lastchar = '\0';
		}
            }
	} else {
	    name = nil;
	} 

	/*
	 * Assumptions:
	 *	not an N_STAB	==> name != nil
	 *	name[0] == '-'	==> name == "-lg"
	 *	name[0] != '_'	==> filename or invisible
	 *
	 * The "-lg" signals the beginning of global loader symbols.
         *
	 */
	if ((np->n_type&N_STAB) != 0) {
	    foundstab = true;
	    enter_nl(name, np);
	} else if (name[0] == '-') {
	    afterlg = true;
	    foundglobals();
	} else if (afterlg) {
	    check_global(name, np);
	} else if ((np->n_type&N_EXT) == N_EXT) {
	    afterlg = true;
	    foundglobals();
	    check_global(name, np);
	} else if (name[0] == '_') {
	    check_local(&name[1], np);
	} else if ((np->n_type&N_TEXT) == N_TEXT) {
	    check_filename(name);
	}
	++curnp;
	np = curnp;
    }
    if (not foundstab and not havenewsymt) {
	warning("no source compiled with -g (or program not linked using 'cc -g')");
    }
    dispose(namelist);
}

/*
 * Get a continuation entry from the name list.
 * Return the beginning of the name.
 */

public String getcont ()
{
    register integer index;
    register String name;

    ++curnp;
    index = curnp->n_un.n_strx;
    if (index == 0) {
	panic("continuation followed by empty stab");
    }
    name = &stringtab[index - 4];
    return name;
}

/*
 * Initialize symbol information.
 */

initsyms ()
{
    curblock = nil;
    curlevel = 0;
    nesting = 0;
    program = insert(identname("", true));
    program->class = PROG;
    program->language = primlang;
    program->symvalue.funcv.beginaddr = codestart;
    program->symvalue.funcv.inline = false;
    newfunc(program, codeloc(program));
    findbeginning(program);
    enterblock(program);
    curmodule = program;
}

/*
 * Free all the object file information that's being stored.
 */

public objfree ()
{
    symbol_free();
    /* keywords_free(); */
    /* names_free(); */
    /* dispose(stringtab); */
    clrfunctab();
}

/*
 * Enter a namelist entry.
 */

private enter_nl (name, np)
String name;
register struct nlist *np;
{
    register Symbol s;
    register Name n;

    s = nil;
    switch (np->n_type) {
	/*
	 * Build a symbol for the FORTRAN common area.  All GSYMS that follow
	 * will be chained in a list with the head kept in common.offset, and
	 * the tail in common.chain.
	 */
	case N_BCOMM:
 	    if (curcomm) {
		curcomm->symvalue.common.chain = commchain;
	    }
	    n = identname(name, true);
	    curcomm = lookup(n);
	    if (curcomm == nil) {
		curcomm = insert(n);
		curcomm->class = COMMON;
		curcomm->block = curblock;
		curcomm->level = program->level;
		curcomm->symvalue.common.chain = nil;
	    }
	    commchain = curcomm->symvalue.common.chain;
	    break;

	case N_ECOMM:
	    if (curcomm) {
		curcomm->symvalue.common.chain = commchain;
		curcomm = nil;
	    }
	    break;

	case N_LBRAC:
	    ++nesting;
	    addrstk[nesting] = (linep - 1)->addr;
	    break;

	case N_RBRAC:
	    --nesting;
	    if (addrstk[nesting] == NOADDR) {
		exitblock();
		newfunc(curblock, (linep - 1)->addr);
		addrstk[nesting] = (linep - 1)->addr;
	    }
	    break;

	case N_SLINE:
	    enterline((Lineno) np->n_desc, (Address) np->n_value);
	    break;

	/*
	 * Source files.
	 */
	case N_SO:
	    n = identname(name, true);
	    enterSourceModule(n, (Address) np->n_value);
	    break;

	/*
	 * Textually included files.
	 */
	case N_SOL:
	    enterfile(name, (Address) np->n_value);
	    break;

	/*
	 * These symbols are assumed to have non-nil names.
	 */
	case N_GSYM:
	case N_FUN:
	case N_STSYM:
	case N_LCSYM:
	case N_RSYM:
	case N_PSYM:
	case N_LSYM:
	case N_SSYM:
	case N_LENG:
	    if (index(name, ':') == nil) {
		if (not warned) {
		    warned = true;
		    printf("warning: old style symbol information ");
		    printf("found in \"%s\"\n", curfilename());
		}
	    } else {
		entersym(name, np);
	    }
	    break;

	case N_ISWRP:
	    {
		Symbol	t;
		boolean isextref = false;
		Symbol	findsym();
		Symbol	deffunc();

		if (np->n_desc == FORTRAN) {
    			t = findsym(identname(name), &isextref);
			if (t) {
				t->symvalue.funcv.haswrapper = true;
			} else {
				t = deffunc(identname(name));
    				t->language = findlanguage(".f");
			}
		} else {
    			t = findsym(identname(&(name[1])), &isextref);
			if (t) {
				t->symvalue.funcv.haswrapper = true;
			} else {
				t = deffunc(identname(&(name[1])));
    				t->language = findlanguage(".c");
			}
		}
		t->symvalue.funcv.haswrapper = true;
    		t->symvalue.funcv.src = true;
             }
			
		
		

	case N_PC:
	case N_MOD2:
	    break;

	default:
	    printf("warning:  stab entry unrecognized: ");
	    if (name != nil) {
		printf("name %s,", name);
	    }
	    printf("ntype %2x, desc %x, value %x'\n",
		np->n_type, np->n_desc, np->n_value);
	    break;
    }
}

/*
 * Try to find the symbol that is referred to by the given name.  Since it's
 * an external, we need to follow a level or two of indirection.
 */

private Symbol findsym (n, var_isextref)
Name n;
boolean *var_isextref;
{
    register Symbol r, s;

    *var_isextref = false;
    find(s, n) where
	(
	    s->level == program->level and (
		s->class == EXTREF or s->class == VAR or
		s->class == PROC or s->class == FUNC
	    )
	) or (
	    s->block == program and s->class == MODULE
	)
    endfind(s);
    if (s == nil) {
	r = nil;
    } else if (s->class == EXTREF) {
	*var_isextref = true;
	r = s->symvalue.extref;
	delete(s);

	/*
	 * Now check for another level of indirection that could come from
	 * a forward reference in procedure nesting information.  In this case
	 * the symbol has already been deleted.
	 */
	if (r != nil and r->class == EXTREF) {
	    r = r->symvalue.extref;
	}
/*
    } else if (s->class == MODULE) {
	s->class = FUNC;
	s->level = program->level;
	r = s;
 */
    } else {
	r = s;
    }
    return r;
}

/*
 * Create a symbol for a text symbol with no source information.
 * We treat it as an assembly language function.
 */

private Symbol deffunc (n)
Name n;
{
    Symbol f;

    f = insert(n);
    f->language = findlanguage(".s");
    f->class = FUNC;
    f->type = t_int;
    f->block = curblock;
    f->level = program->level;
    f->symvalue.funcv.src = false;
    f->symvalue.funcv.inline = false;
    return f;
}

/*
 * Create a symbol for a data or bss symbol with no source information.
 * We treat it as an assembly language variable.
 */

private Symbol defvar (n)
Name n;
{
    Symbol v;

    v = insert(n);
    v->language = findlanguage(".s");
    v->storage = EXT;
    v->class = VAR;
    v->type = t_int;
    v->level = program->level;
    v->block = curblock;
    return v;
}

/*
 * Update a symbol entry with a text address.
 */

private updateTextSym (s, name, addr, np)
Symbol s;
char *name;
Address addr;
struct	nlist	*np;
{
	char	buf[1024];

	if (s->class == VAR) {
		s->symvalue.offset = addr;
	} else if ((s->symvalue.funcv.haswrapper == false)
		|| (s->language == Fortran && name[0] != '_')
		|| (s->language == langC && name[0] == '_')) {
		s->symvalue.funcv.beginaddr = addr;
		if ((name[0] == '_') || (s->class == FUNC) 
			|| (s->class == PROC)) {
			if ((!mainfunc) || 
				(addr != mainfunc->symvalue.funcv.beginaddr - 16)) {
				newfunc(s, codeloc(s));
			}
			findbeginning(s);
		}
	} else if (s->language == Fortran && name[0] == '_') {
		name[0] = '#';
		s = deffunc(identname(name));
		s->symvalue.funcv.beginaddr = addr;
		newfunc(s, codeloc(s));
	} else if (s->language == langC && name[0] != '_') {
		sprintf(buf, "#%s", name);
		s = deffunc(identname(buf));
		s->symvalue.funcv.beginaddr = addr;
		newfunc(s, codeloc(s));
	}
}

/*
 * Check to see if a global _name is already in the symbol table,
 * if not then insert it.
 */

private check_global (name, np)
String name;
register struct nlist *np;
{
    register Name n;
    register Symbol t, u;
    char buf[4096];
    boolean isextref;
    integer count;

    stripped_ = false;
    if (not streq(name, "_end")) {
	if (name[0] == '_') {
	    n = identname(&name[1], true);
            stripped_ = true;
	} else {
	    n = identname(name, true);
#ifndef SVS
	    if (lookup(n) != nil) {
		if (n->language == Fortran) {
			sprintf(buf, "$%s", name);
			n = identname(buf, false);
		}
	    }
#endif
	}
	if ((np->n_type&N_TYPE) == N_TEXT) {
	    count = 0;
	    t = findsym(n, &isextref);
	    while (isextref) {
		++count;
		updateTextSym(t, name, np->n_value, np);
		t = findsym(n, &isextref);
	    }
	    if (count == 0) {
		if (t == nil) {
		    t = deffunc(n);
		    if (name[0] != '%') {
		    	updateTextSym(t, name, np->n_value, np);
		    	if (tracesyms) {
				printdecl(t);
		    	}
		    }
		} else {
		    if (t->class == MODULE) {
			u = t;
			t = deffunc(n);
			t->block = u;
			if (tracesyms) {
			    printdecl(t);
			}
		    }
		    updateTextSym(t, name, np->n_value, np);
		}
	    }
	} else if (((np->n_type&N_TYPE) == N_BSS)
			|| ((np->n_type&N_TYPE) == N_DATA)) {
	    find(t, n) where
		t->class == COMMON
	    endfind(t);
	    if (t != nil) {
		u = (Symbol) t->symvalue.common.offset;
		while (u != nil) {
		    u->symvalue.offset = u->symvalue.common.offset+np->n_value;
		    u = u->symvalue.common.chain;
		}
            } else {
		check_var(np, n);
	    }
        } else {
	    check_var(np, n);
	}
    }
}

/*
 * Check to see if a namelist entry refers to a variable.
 * If not, create a variable for the entry.  In any case,
 * set the offset of the variable according to the value field
 * in the entry.
 *
 * If the external name has been referred to by several other symbols,
 * we must update each of them.
 */

private check_var (np, n)
struct nlist *np;
register Name n;
{
    register Symbol t, u, next;
    Symbol conflict;

    t = lookup(n);
    if (t == nil) {
	t = defvar(n);
	t->symvalue.offset = np->n_value;
	if (tracesyms) {
	    printdecl(t);
	}
    } else {
	conflict = nil;
	do {
	    next = t->next_sym;
	    if (t->name == n) {
		if (t->class == MODULE and t->block == program) {
		    conflict = t;
		} else if (t->class == EXTREF and t->level == program->level) {
		    u = t->symvalue.extref;
		    while (u != nil and u->class == EXTREF) {
			u = u->symvalue.extref;
		    }
		    u->symvalue.offset = np->n_value;
		    delete(t);
		} else if (t->level == program->level and
		    (t->class == VAR or t->class == PROC or t->class == FUNC)
		) {
		    conflict = nil;
		    t->symvalue.offset = np->n_value;
		}
	    }
	    t = next;
	} while (t != nil);
	if (conflict != nil) {
	    u = defvar(n);
	    u->block = conflict;
	    u->symvalue.offset = np->n_value;
	}
    }
}

/*
 * Check to see if a local _name is known in the current scope.
 * If not then enter it.
 */

private check_local (name, np)
String name;
register struct nlist *np;
{
    register Name n;
    register Symbol t, cur;

    n = identname(name, true);
    cur = ((np->n_type&N_TYPE) == N_TEXT) ? curmodule : curblock;
    find(t, n) where t->block == cur endfind(t);
    if (t == nil) {
	t = insert(n);
	t->language = findlanguage(".s");
	t->type = t_int;
	t->block = cur;
	t->storage = EXT;
	t->level = cur->level;
	if ((np->n_type&N_TYPE) == N_TEXT) {
	    t->class = FUNC;
	    t->symvalue.funcv.src = false;
	    t->symvalue.funcv.inline = false;
	    t->symvalue.funcv.beginaddr = np->n_value;
	    newfunc(t, codeloc(t));
	    findbeginning(t);
	} else {
	    t->class = VAR;
	    t->symvalue.offset = np->n_value;
	}
    }
}

/*
 * Check to see if a symbol corresponds to a object file name.
 * For some reason these are listed as in the text segment.
 */

private check_filename (name)
String name;
{
    register String mname;
    register integer i;
    Name n;
    Symbol s;

    mname = strdup(name);
    i = strlen(mname) - 2;
    if (i >= 0 and mname[i] == '.' and mname[i+1] == 'o') {
	mname[i] = '\0';
	--i;
	while (mname[i] != '/' and i >= 0) {
	    --i;
	}
	n = identname(&mname[i+1], true);
	find(s, n) where s->block == program and s->class == MODULE endfind(s);
	if (s == nil) {
	    s = insert(n);
	    s->language = findlanguage(".s");
	    s->class = MODULE;
	    s->symvalue.funcv.beginaddr = 0;
	    findbeginning(s);
	}
	if (curblock->class != PROG) {
	    exitblock();
	    if (curblock->class != PROG) {
		exitblock();
	    }
	}
	enterblock(s);
	curmodule = s;
    }
}

/*
 * Check to see if a symbol is about to be defined within an unnamed block.
 * If this happens, we create a procedure for the unnamed block, make it
 * "inline" so that tracebacks don't associate an activation record with it,
 * and enter it into the function table so that it will be detected
 * by "whatblock".
 */

public chkUnnamedBlock ()
{
    register Symbol s;
    static int bnum = 0;
    char buf[100];
    Address startaddr;

    if (nesting > 0 and addrstk[nesting] != NOADDR) {
	startaddr = (linep - 1)->addr;
	++bnum;
	sprintf(buf, "$b%d", bnum);
	s = insert(identname(buf, false));
	s->language = curlang;
	s->class = PROC;
	s->symvalue.funcv.src = false;
	s->symvalue.funcv.inline = true;
	s->symvalue.funcv.beginaddr = startaddr;
	enterblock(s);
	newfunc(s, startaddr);
	addrstk[nesting] = NOADDR;
    }
}

/*
 * Compilation unit.  C associates scope with filenames
 * so we treat them as "modules".  The filename without
 * the suffix is used for the module name.
 *
 * Because there is no explicit "end-of-block" mark in
 * the object file, we must exit blocks for the current
 * procedure and module.
 */

enterSourceModule (n, addr)
Name n;
Address addr;
{
    register Symbol s;
    Name nn;
    String mname, suffix;

    mname = strdup(Ident(n));
    if (rindex(mname, '/') != nil) {
	mname = rindex(mname, '/') + 1;
    }
    suffix = rindex(mname, '.');
    curlang = findlanguage(suffix);
    if (curlang == findlanguage(".f")) {
	strip_ = true;
    } 
    if (suffix != nil) {
	*suffix = '\0';
    }
    if (not (*language_op(curlang, L_HASMODULES))()) {
	if (curblock->class != PROG) {
	    exitblock();
	    if (curblock->class != PROG) {
		exitblock();
	    }
	}
	nn = identname(mname, true);
	if (curmodule == nil or curmodule->name != nn) {
	    s = insert(nn);
	    s->class = MODULE;
	    s->symvalue.funcv.beginaddr = 0;
	    findbeginning(s);
	} else {
	    s = curmodule;
	}
	s->language = curlang;
	enterblock(s);
	curmodule = s;
    }
    if (program->language == nil) {
	program->language = curlang;
    }
    warned = false;
    enterfile(Ident(n), addr);
    initTypeTable();
}

/*
 * Allocate file and line tables and initialize indices.
 */

private allocmaps (nf, nl)
integer nf, nl;
{
    if (filetab != nil) {
	dispose(filetab);
    }
    if (linetab != nil) {
	dispose(linetab);
    }
    filetab = newarr(Filetab, nf);
    linetab = newarr(Linetab, nl);
    filep = filetab;
    linep = linetab;
}

/*
 * Add a file to the file table.
 *
 * If the new address is the same as the previous file address
 * this routine used to not enter the file, but this caused some
 * problems so it has been removed.  It's not clear that this in
 * turn may not also cause a problem.
 */

private enterfile (filename, addr)
String filename;
Address addr;
{
    filep->addr = addr;
    filep->filename = filename;
    filep->lineindex = linep - linetab;
	filep->nlines = 0;
	elfilep = filep;
    ++filep;
}

/*
 * Since we only estimated the number of lines (and it was a poor
 * estimation) and since we need to know the exact number of lines
 * to do a binary search, we set it when we're done.
 */

private setnlines ()
{
    nlhdr.nlines = linep - linetab;
}

/*
 * Similarly for nfiles ...
 */

private setnfiles ()
{
    nlhdr.nfiles = filep - filetab;
    setsource(filetab[0].filename);
}



extern	DBXHEADER	dbxhead;


private
rdbinlt(f, dbxfiloff)
Fileid	f;
long	dbxfiloff;
{

	SRCFILE		srcfile;
	LINEHEADER	linehdr;
	LINEADDR	lineaddr;
	Symbol		funcsym;
	Address		lastaddr;
	Boolean		firstfunc;
	int	i;
	Filetab	*fp;

	if (lseek(f, dbxfiloff + sizeof(DBXHEADER) 
			+ (dbxhead.nsyms * sizeof(struct Symbol)) +
			dbxhead.stringsize , 0) <= 0) {
		fprintf(stderr, "cannot seek to dbx symbol table\n");
		exit(200);
	}
	
	for (i = 0; i < dbxhead.nfiles; i++) {
		firstfunc = true;
		if (read(f, &srcfile, sizeof(SRCFILE) ) <= 1) {
			fprintf(stderr, " cannot read SRCFILE entry\n");
			exit(100);
		}
		while (srcfile.nfuncs--) {
			Boolean	firstline;
	
			if (read(f, &linehdr, sizeof(LINEHEADER) ) <= 1) {
				fprintf(stderr, " cannot read line header\n");
				exit(100);
			}
			funcsym = symrefs[linehdr.funcnum];
			if (funcsym->symvalue.funcv.beginaddr 
					== mainfunc->symvalue.funcv.beginaddr) {
				lastaddr = funcsym->symvalue.funcv.beginaddr 
					- 16;
			} else {
				lastaddr = funcsym->symvalue.funcv.beginaddr
					- 4;
			}
			firstline = true;
			while (linehdr.nlines--) {
				if (read(f, &lineaddr, sizeof(LINEADDR) ) 
						<= 1) {
					fprintf(stderr, 
						" cannot read line/addrss\n");
					exit(100);
				}
				lastaddr += lineaddr.addroff;
				if (firstline) {
					funcsym->symvalue.funcv.beginaddr = 
						lastaddr;
					firstline = false;
				}
				if (firstfunc == true) {
					curmodule->symvalue.funcv.beginaddr = 
						lastaddr;
					fp = 
					setfileaddr((char *) (srcfile.filename
						+ (long) newstringtab),
						lastaddr);
					elfilep->nlines = 0;
					firstfunc = false;
				}
				enterline(lineaddr.lineno, lastaddr);
			}
		}
	}
}

Filetab *
setfileaddr(filename, addr)
String	filename;
Address	addr;
{
	Filetab	*fp;

	fp = filetab;
	while (fp <= filep - 1) {
		if (strcmp(fp->filename, filename) == 0) {
			fp->addr = addr;
			fp->lineindex = linep - linetab;
			elfilep = fp;
			break;
		}
		fp++;
	}
	return(fp);
}
