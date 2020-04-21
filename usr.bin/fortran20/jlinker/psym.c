/* Copyright (c) 1982 Regents of the University of California */

static char sccsid[] = "@(#)printsym.c 1.12 8/10/83";

static char rcsid[] = "$Header: /d2/3.7/src/usr.bin/fortran20/jlinker/RCS/psym.c,v 1.1 89/03/27 18:01:36 root Exp $";

/*
 * Printing of symbolic information.
 */

#include "defs.h"
#include "symbols.h"
#include "languages.h"
#include "printsym.h"
#include "tree.h"
#include "names.h"
#include "readj.h"

#ifndef public
#endif

/*
 * Maximum number of arguments to a function.
 * This is used as a check for the possibility that the stack has been
 * overwritten and therefore a saved argument pointer might indicate
 * to an absurdly large number of arguments.
 */

#define MAXARGSPASSED 20

/*
 * Return a pointer to the string for the name of the class that
 * the given symbol belongs to.
 */

private String clname[] = {
    "bad use", "constant", "type", "variable", "array", "@dynarray",
    "@subarray", "fileptr", "record", "field",
    "procedure", "function", "funcvar",
    "ref", "pointer", "file", "set", "range", "label", "withptr",
    "scalar", "string", "program", "improper", "variant",
    "procparam", "funcparam", "module", "tag", "common", "extref", "typeref"
};

public String classname(s)
Symbol s;
{
    return clname[ord(s->class)];
}




/*
 * Straight dump of symbol information.
 */

public psym(s)
Symbol s;
{
if (s && s->name)
    printf("name\t%s at %x\n", symname(s), s);
    printf("lang\t%s\n", language_name(s->language));
    printf("level\t%d\n", s->level);
    printf("class\t%s\n", classname(s));
	printf("symnum=%d\n", s->symnum);
    printf("type\t0x%x", s->type);
    if (s->type != nil and s->type->name != nil) {
	printf(" (%s)", symname(s->type));
    }
    printf("\nchain\t0x%x", s->chain);
    if (s->chain != nil and s->chain->name != nil) {
	printf(" (%s)", symname(s->chain));
    }
    printf("\nblock\t0x%x", s->block);
    if (s->block != nil and s->block->name != nil) {
	printf(" (");
	printname(stdout, s->block);
	putchar(')');
    }
    putchar('\n');
    switch (s->class) {
	case TYPE:
	    printf("type:size\t\n" /*,size(s)*/);
	    break;

	case VAR:
	case REF:
	    switch (s->storage) {
		case INREG:
		    printf("reg\t%d\n", s->symvalue.offset);
		    break;

		case STK:
		    printf("offset\t%d\n", s->symvalue.offset);
		    break;

		case EXT:
		    printf("address\t0x%x\n", s->symvalue.offset);
		    break;
	    }
	    printf("size\t%d\n" /*,size(s)*/);
	    break;

	case RECORD:
	case VARNT:
	    printf("size\t%d\n", s->symvalue.offset);
	    break;

	case FIELD:
	    printf("offset\t%d\n", s->symvalue.field.offset);
	    printf("size\t%d\n", s->symvalue.field.length);
	    break;

	case PROG:
	case PROC:
	case FUNC:
	    printf("address\t0x%x\n", s->symvalue.funcv.beginaddr);
	    if (isinline(s)) {
		printf("inline procedure\n");
	    }
	    if (nosource(s)) {
		printf("does not have source information\n");
	    } else {
		printf("has source information\n");
	    }
	    break;

	case RANGE:
	    prangetype(s->symvalue.rangev.lowertype);
	    printf("lower\t%d\n", s->symvalue.rangev.lower);
	    prangetype(s->symvalue.rangev.uppertype);
	    printf("upper\t%d\n", s->symvalue.rangev.upper);
	    break;

	default:
	    /* do nothing */
	    break;
    }
}

private prangetype(r)
Rangetype r;
{
    switch (r) {
	case R_CONST:
	    printf("CONST");
	    break;

	case R_ARG:
	    printf("ARG");
	    break;

	case R_TEMP:
	    printf("TEMP");
	    break;

	case R_ADJUST:
	    printf("ADJUST");
	    break;
    }
}

/*
 * Print out the name of a symbol.
 */

public printname(f, s)
File f;
Symbol s;
{
    if (s == nil) {
	fprintf(f, "(noname)");
    } else if (s == program) {
	fprintf(f, ".");
/*
    } else if (isredirected() or isambiguous(s)) {
	printwhich(f, s);
*/
    } else {
	fprintf(f, "%s", symname(s));
    }
}

public String language_name(lang)
short	lang;
{
    return (lang == nil) ? "(nil)" : langstrs[lang];
}

