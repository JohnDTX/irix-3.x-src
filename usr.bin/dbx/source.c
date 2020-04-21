/* Copyright (c) 1982 Regents of the University of California */

static char sccsid[] = "@(#)source.c 1.9 8/5/83";

static char rcsid[] = "$Header: /d2/3.7/src/usr.bin/dbx/RCS/source.c,v 1.1 89/03/27 17:44:53 root Exp $";

/*
 * Source file management.
 */

#include "defs.h"
#include "source.h"
#include "object.h"
#include "mappings.h"
#include "machine.h"
#include "keywords.h"
#include "tree.h"
#include "eval.h"
#ifdef DBMEX
#include "main.h"
#include "dbxshm.h"
#endif
#ifdef sgi
#   define R_OK 04	/* read access */
#   define L_SET 0	/* absolute offset for seek */
#else
#   include <sys/file.h>
#endif

#ifndef public
typedef int Lineno;

String cursource;
Lineno curline;
Lineno cursrcline;

#define LASTLINE 0		/* recognized by printlines */

#include "lists.h"

List sourcepath;
#endif

#ifdef sgi
extern char *regcmp();

#   define re_comp(pat) regcmp(pat, 0)
#   define re_exec(re, buf) (regex(re, buf) != NULL)
#   define re_err(re) (re == nil)
#else
extern char *re_comp();

#   define re_exec(re, buf) regex(buf)
#   define re_err(re) (re != nil)
#endif

private Lineno lastlinenum;
private String prevsource = nil;

/*
 * Data structure for indexing source seek addresses by line number.
 *
 * The constraints are:
 *
 *  we want an array so indexing is fast and easy
 *  we don't want to waste space for small files
 *  we don't want an upper bound on # of lines in a file
 *  we don't know how many lines there are
 *
 * The solution is a "dirty" hash table.  We have NSLOTS pointers to
 * arrays of NLINESPERSLOT addresses.  To find the source address of
 * a particular line we find the slot, allocate space if necessary,
 * and then find its location within the pointed to array.
 */

typedef long Seekaddr;

#define NSLOTS 40
#define NLINESPERSLOT 500

#define slotno(line)    ((line) div NLINESPERSLOT)
#define index(line)	((line) mod NLINESPERSLOT)
#define slot_alloc()    newarr(Seekaddr, NLINESPERSLOT)
#define srcaddr(line)	seektab[slotno(line)][index(line)]

private File srcfp;
private Seekaddr *seektab[NSLOTS];

/*
 * Determine if the current source file is available.
 */

public boolean canReadSource ()
{
    boolean b;

    if (cursource == nil) {
	b = false;
    } else if (cursource != prevsource) {
	skimsource();
	b = (boolean) (lastlinenum != 0);
    } else {
	b = true;
    }
    return b;
}

/*
 * Print out the given lines from the source.
 */

public printlines(l1, l2)
Lineno l1, l2;
{
    register int c;
    register Lineno i, lb, ub;
    register File f;

    if (cursource == nil) {
	beginerrmsg();
	fprintf(stderr, "no source file\n");
    } else {
	if (cursource != prevsource) {
	    skimsource();
	}
	if (lastlinenum == 0) {
	    beginerrmsg();
	    fprintf(stderr, "couldn't read \"%s\"\n", cursource);
	} else {
	    lb = (l1 == LASTLINE) ? lastlinenum : l1;
	    ub = (l2 == LASTLINE) ? lastlinenum : l2;
	    if (lb < 1) {
		beginerrmsg();
		fprintf(stderr, "line number must be positive\n");
	    } else if (lb > lastlinenum) {
		beginerrmsg();
		if (lastlinenum == 1) {
		    fprintf(stderr, "\"%s\" has only 1 line\n", cursource);
		} else {
		    fprintf(stderr, "\"%s\" has only %d lines\n",
			cursource, lastlinenum);
		}
	    } else if (ub < lb) {
		beginerrmsg();
		fprintf(stderr, "second number must be greater than first\n");
	    } else {
		if (ub > lastlinenum) {
		    ub = lastlinenum;
		}
		f = srcfp;
		fseek(f, srcaddr(lb), 0);
		for (i = lb; i <= ub; i++) {
		    printf("%5d   ", i);
		    while ((c = getc(f)) != '\n') {
			putchar(c);
		    }
		    putchar('\n');
		}
		cursrcline = ub + 1;
	    }
	}
    }
}

/*
 * Search the sourcepath for a file.
 */

static char fileNameBuf[1024];

public String findsource(filename)
String filename;
{
    register String src, dir;

    if (filename[0] == '/') {
	src = filename;
    } else {
	src = nil;
	foreach (String, dir, sourcepath)
	    sprintf(fileNameBuf, "%s/%s", dir, filename);
	    if (access(fileNameBuf, R_OK) == 0) {
		src = fileNameBuf;
		break;
	    }
	endfor
    }
    return src;
}

/*
 * Open a source file looking in the appropriate places.
 */

public File opensource(filename)
String filename;
{
    String s;
    File f;

    s = findsource(filename);
    if (s == nil) {
	f = nil;
    } else {
	f = fopen(s, "r");
    }
    return f;
}

/*
 * Set the current source file.
 */

public setsource(filename)
String filename;
{
    if (filename != nil and filename != cursource) {
	prevsource = cursource;
	cursource = filename;
	cursrcline = 1;
#ifdef DBMEX
	if (winmode == true) {
		shm_write('f', filename, 0, 0);
	}
#endif
    }
}

/*
 * Read the source file getting seek pointers for each line.
 */

private skimsource()
{
    register int c;
    register Seekaddr count;
    register File f;
    register Lineno linenum;
    register Seekaddr lastaddr;
    register int slot;

    f = opensource(cursource);
    if (f == nil) {
	lastlinenum = 0;
    } else {
	if (prevsource != nil) {
	    free_seektab();
	    if (srcfp != nil) {
		fclose(srcfp);
	    }
	}
	prevsource = cursource;
	linenum = 0;
	count = 0;
	lastaddr = 0;
	while ((c = getc(f)) != EOF) {
	    ++count;
	    if (c == '\n') {
		slot = slotno(++linenum);
		if (slot >= NSLOTS) {
		    panic("skimsource: too many lines");
		}
		if (seektab[slot] == nil) {
		    seektab[slot] = slot_alloc();
		}
		seektab[slot][index(linenum)] = lastaddr;
		lastaddr = count;
	    }
	}
	lastlinenum = linenum;
	srcfp = f;
    }
}

/*
 * Erase information and release space in the current seektab.
 * This is in preparation for reading in seek pointers for a
 * new file.  It is possible that seek pointers for all files
 * should be kept around, but the current concern is space.
 */

private free_seektab()
{
    register int slot;

    for (slot = 0; slot < NSLOTS; slot++) {
	if (seektab[slot] != nil) {
	    dispose(seektab[slot]);
	}
    }
}

/*
 * Figure out current source position.
 */

public getsrcpos()
{
    String filename;

    curline = srcline(pc);
    filename = srcfilename(pc);
    setsource(filename);
    if (curline != 0) {
	cursrcline = curline;
    }
}

/*
 * Print out the current source position.
 */

public printsrcpos()
{
#ifdef DBMEX
	int	i;
#endif
    printf("at line %d", curline);
    if (nlhdr.nfiles > 1) {
	printf(" in file \"%s\"", cursource);
    }
#ifdef DBMEX
	if (winmode == true) {
		shm_write('S', cursource, 0, curline);
	}
#ifdef notdef
	do_display_list();
#endif
#endif
}

#define DEF_EDITOR  "vi"

/*
 * Invoke an editor on the given file.  Which editor to use might change
 * installation to installation.  For now, we use "vi".  In any event,
 * the environment variable "EDITOR" overrides any default.
 */

public edit(filename)
String filename;
{
    extern String getenv();
    String ed, src, s;
    Symbol f;
    Address addr;
    char lineno[10];
#ifdef DBMEX
	int line;
#endif;

    ed = getenv("EDITOR");
    if (ed == nil) {
	ed = DEF_EDITOR;
    }
    src = findsource((filename != nil) ? filename : cursource);
    if (src == nil) {
	f = which(identname(filename, true));
	if (not isblock(f)) {
	    error("can't read \"%s\"", filename);
	}
	addr = firstline(f);
	if (addr == NOADDR) {
	    error("no source for \"%s\"", filename);
	}
	src = srcfilename(addr);
	s = findsource(src);
	if (s != nil) {
	    src = s;
	}
	sprintf(lineno, "+%d", srcline(addr));
#ifdef DBMEX
	line = srcline(addr);
#endif
    } else {
	sprintf(lineno, "+1");
#ifdef DBMEX
	line = 1;
#endif
    }
#ifdef DBMEX
    if (winmode == true) {
		shm_write('e', src, 0, line);
    } else
#endif
    if (streq(ed, "vi") or streq(ed, "ex")) {
	call(ed, stdin, stdout, lineno, src, nil);
    } else {
	call(ed, stdin, stdout, src, nil);
    }
prevsource = nil;
}

/*
 * Strip away portions of a given pattern not part of the regular expression.
 */

static char prevpattern[200];

private String getpattern (pattern)
String pattern;
{
    register char *p, *r;

    p = pattern;
    while (*p == ' ' or *p == '\t') {
	++p;
    }
    r = p;
    while (*p != '\0') {
	++p;
    }
    --p;
    if (*p == '\n') {
	*p = '\0';
	--p;
    }
    if (*p == *r) {
	*p = '\0';
	--p;
    }
    ++r;
    if (*r == '\0') {
	r = prevpattern;
    } else {
	strcpy(prevpattern, r);
    }
    return r;
}

/*
 * Search the current file for a regular expression.
 */

public search (direction, pattern)
char direction;
String pattern;
{
    File f;
    String re, compiled;
    Lineno line, stopline;
    char buf[512];
    
    if (cursource == nil) {
	beginerrmsg();
	fprintf(stderr, "no source file\n");
    } else {
	if (cursource != prevsource) {
	    skimsource();
	}
	if (lastlinenum == 0) {
	    beginerrmsg();
	    fprintf(stderr, "couldn't read \"%s\"\n", cursource);
	} else {
	    re = getpattern(pattern);
	    /* circf = 0; */
	    if (re != nil and *re != '\0') {
		compiled = re_comp(re);
		if (re_err(compiled)) {
		    error("bad regular expression");
		}
	    } else {
			error("bad regular expression");
	    }
	    f = srcfp;
	    if (direction == '/') {
		line = cursrcline + 1;
		if (cursrcline > lastlinenum) {
		    stopline = cursrcline;
		} else {
		    stopline = line;
		}
	    } else {
		line = cursrcline - 1;
		stopline = line;
	    }
	    for (;;) {
		if (line > lastlinenum) {
		    line = 1;
		} else if (line < 1) {
		    line = lastlinenum;
		}
		fseek(f, srcaddr(line), L_SET);
		if (fgets(buf, sizeof(buf), f) == nil) {
		    error("unexpected end-of-file");
		}
		if (re_exec(compiled, buf)) {
		    printlines(line, line);
#ifdef DBMEX
		if (winmode == true) {
			shm_write('L', cursource, MIDDLE, line);
		}
#endif
		    cursrcline = line;
		    break;
		}
		if (direction == '/') {
		    ++line;
		} else {
		    --line;
		}
		if (line == stopline) {
		    beginerrmsg();
		    fprintf(stderr, "pattern not found\n");
		    break;
		}
	    }
	}
    }
}

/*
 * Compute a small window around the given line.
 */

public getsrcwindow (line, l1, l2)
Lineno line, *l1, *l2;
{
    Node s;
    integer size;

    s = findvar(identname("$listwindow", true));
    if (s == nil) {
	size = 10;
    } else {
	eval(s);
	size = pop(integer);
    }
    *l1 = line - (size div 2);
    if (*l1 < 1) {
	*l1 = 1;
    }
    *l2 = *l1 + size;
    if (lastlinenum != LASTLINE and *l2 > lastlinenum) {
	*l2 = lastlinenum;
    }
}
