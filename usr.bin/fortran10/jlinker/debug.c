
/* Copyright (c) 1982 Regents of the University of California */

static char sccsid[] = "@(#)symbols.c 1.10 8/10/83";

static char rcsid[] = "$Header: /d2/3.7/src/usr.bin/fortran10/jlinker/RCS/debug.c,v 1.1 89/03/27 17:57:03 root Exp $";

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


#include "machine.h"
#include "names.h"
#include "languages.h"
#include "tree.h"



dump_array(s)
Symbol	s;
{
	char bounds[130], *p1, **p;
	String typename();

	p1 = bounds;
	p = &p1;
	mksubs(p,s);
	*p -= 1; 
	**p = '\0';   /* get rid of trailing ',' */
	printf(" %s %s[%s] \n",typename(s), symname(s), bounds);
}

mksubs(pbuf,st)
Symbol st;
char  **pbuf;
{   
	int lb, ub;
	Symbol r, eltype;
	String typename();

	if ((st->type) && st->type->class == ARRAY) {
	        mksubs(pbuf,st->type);
	}

	r = st->chain;
	lb = r->symvalue.rangev.lower;
	sprintf(*pbuf,"%d:",lb);
	*pbuf += strlen(*pbuf);

	ub = r->symvalue.rangev.upper;
	sprintf(*pbuf,"%d,",ub);
	*pbuf += strlen(*pbuf);
}


private String typename(s)
Symbol s;
{
int ub;
static char buf[20];
char *pbuf;
Symbol st,sc;

	if ((s == nil) || s->type == nil) {
		return("void");
	}
     if(s->type->class == TYPE) return(symname(s->type));

     for(st = s->type; st->type->class != TYPE; st = st->type);

     pbuf=buf;

     if(istypename(st->type,"char"))  { 
	  sprintf(pbuf,"character*");
          pbuf += strlen(pbuf);
	  sc = st->chain;
 	  sprintf(pbuf,"%d",sc->symvalue.rangev.upper);
     } else {
          sprintf(pbuf,"%s ",symname(st->type));
     }
     return(buf);
}


/*
 * Check for a type of the given name.
 */

public Boolean istypename(type, name)
Symbol type;
String name;
{
    register Symbol t;
    Boolean b;

    t = type;
    if (t == nil) {
	b = false;
    } else {
	b = (Boolean) (
	    t->class == TYPE and streq(ident(t->name), name)
	);
    }
    return b;
}


/*
 * Print out the declaration of a FORTRAN variable.
 */

public fortran_printdecl(s)
Symbol s;
{


Symbol eltype;

    switch (s->class) {

	case CONST:
	    
	    printf("parameter %s = ", symname(s));
	    break;

        case REF:
            printf(" (dummy argument) ");

	case VAR:
	    if (s->type->class == ARRAY &&
		 (not istypename(s->type->type,"char")) ) {
                char bounds[130], *p1, **p;
		p1 = bounds;
                p = &p1;
                mksubs(p,s->type);
                *p -= 1; 
                **p = '\0';   /* get rid of trailing ',' */
		printf(" %s %s[%s] ",typename(s), symname(s), bounds);
	    } else {
		printf("%s %s", typename(s), symname(s));
	    }
	    break;

	case FUNC:
	    if (not istypename(s->type, "void")) {
                printf(" %s function ", typename(s) );
	    }
	    else printf(" subroutine");
	    printf(" %s ", symname(s));
	    fortran_listparams(s);
	    break;

	case MODULE:
	    printf("source file \"%s.c\"", symname(s));
	    break;

	case PROG:
	    printf("executable file \"%s\"", symname(s));
	    break;

	default:
	    fprintf(stderr, "class %s in fortran_printdecl", classname(s));
    }
    putchar('\n');
}

/*
 * List the parameters of a procedure or function.
 * No attempt is made to combine like types.
 */

public fortran_listparams(s)
Symbol s;
{
    register Symbol t;

    putchar('(');
    for (t = s->chain; t != nil; t = t->chain) {
	printf("%s", symname(t));
	if (t->chain != nil) {
	    printf(", ");
	}
    }
    putchar(')');
    if (s->chain != nil) {
	printf("\n");
	for (t = s->chain; t != nil; t = t->chain) {
	    if (t->class != REF) {
		fprintf(stderr, "unexpected class %d for parameter", t->class);
		exit (1);
	    }
	printf("%s %s", typename(t), symname(t));
	}
    } else {
	putchar('\n');
    }
}

