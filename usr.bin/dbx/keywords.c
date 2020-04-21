/* Copyright (c) 1982 Regents of the University of California */

static char sccsid[] = "@(#)keywords.c 1.3 5/18/83";

static char rcsid[] = "$Header: /d2/3.7/src/usr.bin/dbx/RCS/keywords.c,v 1.1 89/03/27 17:44:31 root Exp $";

/*
 * Keywords, variables, and aliases (oh my!).
 */

#include "defs.h"
#include "keywords.h"
#include "scanner.h"
#include "names.h"
#include "symbols.h"
#include "tree.h"
#include "lists.h"
#include "main.h"
#include "y.tab.h"

#ifndef public

#include "scanner.h"
#include "tree.h"

#endif

private String reserved[] ={
    "alias", "and", "assign", "at", "call", "catch", "cont",
    "debug", "delete", "div", "down", "dump", "edit", "file", "func",
    "gripe", "help", "if", "ignore", "in",
    "list", "mod", "next", "nexti", "nil", "not", "or",
    "print", "psym", "quit", "rerun", "return", "run",
    "set", "sh", "skip", "source", "status", "step", "stepi",
    "stop", "stopi", "trace", "tracei", "unalias", "unset", "up", "use",
    "disass", "whatis", "when", "where", "whereis", "which", "display",
    "INT", "CHAR", "REAL", "NAME", "STRING", "->"
};

/*
 * The keyword table is a traditional hash table with collisions
 * resolved by chaining.
 */

#define HASHTABLESIZE 1007

typedef enum { ISKEYWORD, ISALIAS, ISVAR } KeywordType;

typedef struct Keyword {
    Name name;
    KeywordType class : 16;
    union {
	/* ISKEYWORD: */
	    Token toknum;

	/* ISALIAS: */
	    struct {
		List paramlist;
		String expansion;
	    } alias;

	/* ISVAR: */
	    Node var;
    } value;
    struct Keyword *chain;
} *Keyword;

typedef unsigned int Hashvalue;

private Keyword hashtab[HASHTABLESIZE];

#define hash(n) ((((unsigned) n) >> 2) mod HASHTABLESIZE)

/*
 * Enter all the reserved words into the keyword table.
 *
 * If the vaddrs flag is set (through the -k command line option) then
 * set the special "$mapaddrs" variable.  This assumes that the
 * command line arguments are scanned before this routine is called.
 */

public enterkeywords()
{
    register integer i;

    for (i = ALIAS; i <= DISPLAY; i++) {
	keyword(reserved[ord(i) - ord(ALIAS)], i);
    }
    defalias("c", "cont");
    defalias("d", "delete");
    defalias("h", "help");
    defalias("e", "edit");
    defalias("l", "list");
    defalias("n", "next");
    defalias("p", "print");
    defalias("q", "quit");
    defalias("r", "run");
    defalias("s", "step");
    defalias("st", "stop");
    defalias("j", "status");
    defalias("t", "where");
    defalias("dis", "disass");
    if (vaddrs) {
	defvar(identname("$mapaddrs", true), nil);
    }
}

/*
 * Deallocate the keyword table.
 */

public keywords_free()
{
    register Integer i;
    register Keyword k, nextk;

    for (i = 0; i < HASHTABLESIZE; i++) {
	k = hashtab[i];
	while (k != nil) {
	    nextk = k->chain;
	    dispose(k);
	    k = nextk;
	}
	hashtab[i] = nil;
    }
}

/*
 * Insert a name into the keyword table and return the keyword for it.
 */

private Keyword keywords_insert (n)
Name n;
{
    Hashvalue h;
    Keyword k;

    h = hash(n);
    k = new(Keyword);
    k->name = n;
    k->chain = hashtab[h];
    hashtab[h] = k;
    return k;
}

/*
 * Find the keyword associated with the given name.
 */

private Keyword keywords_lookup (n)
Name n;
{
    Hashvalue h;
    register Keyword k;

    h = hash(n);
    k = hashtab[h];
    while (k != nil and k->name != n) {
	k = k->chain;
    }
    return k;
}

/*
 * Delete the given keyword of the given class.
 */

private boolean keywords_delete (n, class)
Name n;
KeywordType class;
{
    Hashvalue h;
    register Keyword k, prevk;
    boolean b;

    h = hash(n);
    k = hashtab[h];
    prevk = nil;
    while (k != nil and (k->name != n or k->class != class)) {
	prevk = k;
	k = k->chain;
    }
    if (k != nil) {
	b = true;
	if (prevk == nil) {
	    hashtab[h] = k->chain;
	} else {
	    prevk->chain = k->chain;
	}
	dispose(k);
    } else {
	b = false;
    }
    return b;
}

/*
 * Enter a keyword into the table.  It is assumed to not be there already.
 * The string is assumed to be statically allocated.
 */

private keyword (s, t)
String s;
Token t;
{
    Keyword k;
    Name n;

    n = identname(s, true);
    k = keywords_insert(n);
    k->class = ISKEYWORD;
    k->value.toknum = t;
}

/*
 * Define a builtin command name alias.
 */

private defalias (s1, s2)
String s1, s2;
{
    alias(identname(s1, true), nil, s2);
}

/*
 * Look for a word of a particular class.
 */

private Keyword findword (n, class)
Name n;
KeywordType class;
{
    register Keyword k;

    k = keywords_lookup(n);
    while (k != nil and (k->name != n or k->class != class)) {
	k = k->chain;
    }
    return k;
}

/*
 * Return the token associated with a given keyword string.
 * If there is none, return the given default value.
 */

public Token findkeyword (n, def)
Name n;
Token def;
{
    Keyword k;
    Token t;

    k = findword(n, ISKEYWORD);
    if (k == nil) {
	t = def;
    } else {
	t = k->value.toknum;
    }
    return t;
}

/*
 * Return the associated string if there is an alias with the given name.
 */

public boolean findalias (n, pl, str)
Name n;
List *pl;
String *str;
{
    Keyword k;
    boolean b;

    k = findword(n, ISALIAS);
    if (k == nil) {
	b = false;
    } else {
	*pl = k->value.alias.paramlist;
	*str = k->value.alias.expansion;
    }
    return b;
}

/*
 * Return the string associated with a token corresponding to a keyword.
 */

public String keywdstring (t)
Token t;
{
    return reserved[ord(t) - ord(ALIAS)];
}

/*
 * Process an alias command, either entering a new alias or printing out
 * an existing one.
 */

public alias (newcmd, args, str)
Name newcmd;
List args;
String str;
{
    Keyword k;

    if (str == nil) {
	print_alias(newcmd);
    } else {
	k = findword(newcmd, ISALIAS);
	if (k == nil) {
	    k = keywords_insert(newcmd);
	}
	k->class = ISALIAS;
	k->value.alias.paramlist = args;
	k->value.alias.expansion = str;
    }
}

/*
 * Print out an alias.
 */

private print_alias (cmd)
Name cmd;
{
    register Keyword k;
    register Integer i;
    Name n;

    if (cmd == nil) {
	for (i = 0; i < HASHTABLESIZE; i++) {
	    for (k = hashtab[i]; k != nil; k = k->chain) {
		if (k->class == ISALIAS) {
		    if (isredirected()) {
			printf("alias %s", Ident(k->name));
			printparams(k->value.alias.paramlist);
			printf("\t\"%s\"\n", k->value.alias.expansion);
		    } else {
			printf("%s", Ident(k->name));
			printparams(k->value.alias.paramlist);
			printf("\t%s\n", k->value.alias.expansion);
		    }
		}
	    }
	}
    } else {
	k = findword(cmd, ISALIAS);
	if (k == nil) {
	    printf("\n");
	} else {
	    printparams(k->value.alias.paramlist);
	    printf("%s\n", k->value.alias.expansion);
	}
    }
}

private printparams (pl)
List pl;
{
    Name n;

    if (pl != nil) {
	printf("(");
	foreach(Name, n, pl)
	    printf("%s", Ident(n));
	    if (not list_islast()) {
		printf(", ");
	    }
	endfor
	printf(")");
    }
}

/*
 * Remove an alias.
 */

public unalias (n)
Name n;
{
    if (not keywords_delete(n, ISALIAS)) {
	error("%s is not aliased", Ident(n));
    }
}

/*
 * Define a variable.
 */

public defvar (n, val)
Name n;
Node val;
{
    Keyword k;

    if (n == nil) {
	print_vars();
    } else {
	if (lookup(n) != nil) {
	    error("\"%s\" is a program symbol -- use assign", Ident(n));
	}
	k = findword(n, ISVAR);
	if (k == nil) {
	    k = keywords_insert(n);
	}
	k->class = ISVAR;
	k->value.var = val;
	if (n == identname("$mapaddrs", true)) {
	    vaddrs = true;
	}
    }
}

/*
 * Return the value associated with a variable.
 */

public Node findvar (n)
Name n;
{
    Keyword k;
    Node val;

    k = findword(n, ISVAR);
    if (k == nil) {
	val = nil;
    } else {
	val = k->value.var;
    }
    return val;
}

/*
 * Return whether or not a variable is set.
 */

public boolean varIsSet (s)
String s;
{
    return (boolean) (findword(identname(s, false), ISVAR) != nil);
}

/*
 * Delete a variable.
 */

public undefvar (n)
Name n;
{
    if (not keywords_delete(n, ISVAR)) {
	error("%s is not set", Ident(n));
    }
    if (n == identname("$mapaddrs", true)) {
	vaddrs = false;
    }
}

/*
 * Print out all the values of set variables.
 */

private print_vars ()
{
    register integer i;
    register Keyword k;

    for (i = 0; i < HASHTABLESIZE; i++) {
	for (k = hashtab[i]; k != nil; k = k->chain) {
	    if (k->class == ISVAR) {
		if (isredirected()) {
		    printf("set ");
		}
		printf("%s", Ident(k->name));
		if (k->value.var != nil) {
		    printf("\t");
		    prtree(stdout, k->value.var);
		}
		printf("\n");
	    }
	}
    }
}
