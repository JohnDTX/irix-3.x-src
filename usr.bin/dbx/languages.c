/* Copyright (c) 1982 Regents of the University of California */

static char sccsid[] = "@(#)languages.c 1.3 5/18/83";

static char rcsid[] = "$Header: /d2/3.7/src/usr.bin/dbx/RCS/languages.c,v 1.1 89/03/27 17:44:32 root Exp $";

/*
 * Language management.
 */

#include "defs.h"
#include "languages.h"
#include "c.h"
#include "pascal.h"
#include "modula-2.h"
#include "asm.h"

#ifndef public

typedef struct Language *Language;

typedef enum {
    L_PRINTDECL, L_PRINTVAL, L_TYPEMATCH, L_BUILDAREF, L_EVALAREF,
    L_MODINIT, L_HASMODULES, L_PASSADDR,
    L_ENDOP
} LanguageOp;

typedef LanguageOperation();

Language primlang;

#endif

struct Language {
    String name;
    String suffix;
    LanguageOperation *op[20];
    Language next;
};

private Language head;

/*
 * Initialize language information.
 *
 * The last language initialized will be the default one
 * for otherwise indistinguised symbols.
 */

public language_init()
{
    primlang = language_define("$builtin symbols", ".?");
    c_init();
    fortran_init();
    pascal_init();
    modula2_init();
    asm_init();
}

public Language findlanguage(suffix)
String suffix;
{
    Language lang;

    lang = head;
    if (suffix != nil) {
	while (lang != nil and not streq(lang->suffix, suffix)) {
	    lang = lang->next;
	}
	if (lang == nil) {
	    lang = head;
	}
    }
    return lang;
}

public String language_name(lang)
Language lang;
{
    return (lang == nil) ? "(nil)" : lang->name;
}

public Language language_define(name, suffix)
String name;
String suffix;
{
    Language p;

    p = new(Language);
    p->name = name;
    p->suffix = suffix;
    p->next = head;
    head = p;
    return p;
}

public language_setop(lang, op, operation)
Language lang;
LanguageOp op;
LanguageOperation *operation;
{
    checkref(lang);
    assert(ord(op) < ord(L_ENDOP));
    lang->op[ord(op)] = operation;
}

public LanguageOperation *language_op(lang, op)
Language lang;
LanguageOp op;
{
    LanguageOperation *o;

    checkref(lang);
    o = lang->op[ord(op)];
    checkref(o);
    return o;
}
