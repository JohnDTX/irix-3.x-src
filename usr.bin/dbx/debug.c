
/* Copyright (c) 1982 Regents of the University of California */

static char sccsid[] = "@(#)debug.c	1.3	5/18/83";

static char rcsid[] = "$Header: /d2/3.7/src/usr.bin/dbx/RCS/debug.c,v 1.1 89/03/27 17:44:25 root Exp $";

/*
 *  Debug routines
 */

#include "defs.h"
#include "tree.h"
#include "operators.h"
#include "eval.h"
#include "events.h"
#include "symbols.h"
#include "scanner.h"
#include "source.h"
#include "object.h"
#include "main.h"
#include "mappings.h"
#include "process.h"
#include "machine.h"
#include "debug.h"
#include <signal.h>

public boolean tracetree;	/* trace building of parse trees */
public boolean traceeval;	/* trace tree evaluation */

/*
 * Dynamically turn on/off a debug flag, or display some information.
 */

public debug (p)
Node p;
{
    int code;

    code = p->value.lcon;
    switch (code) {
	case 0:
	    puts("debugging flags:");
	    puts("    1        trace scanner return values");
	    puts("    2        trace breakpoints");
	    puts("    3        trace execution");
	    puts("    4        trace tree building");
	    puts("    5        trace tree evaluation");
	    puts("   -[12345]  turns off corresponding flag");
	    puts("    6        dump function table");
	    break;

	case 1:
	case -1:
#           ifdef LEXDEBUG
		lexdebug = (boolean) (code > 0);
#           else
		error("can't debug scanner (not compiled with LEXDEBUG)");
#           endif
	    break;

	case 2:
	case -2:
	    tracebpts = (boolean) (code > 0);
	    break;

	case 3:
	case -3:
	    traceexec = (boolean) (code > 0);
	    break;

	case 4:
	case -4:
	    tracetree = (boolean) (code > 0);
	    break;

	case 5:
	case -5:
	    traceeval = (boolean) (code > 0);
	    break;

	case 6:
	    dumpfunctab();
	    break;

	default:
	    error("unknown debug flag");
	    break;
    }
}

private String leafname[] = {
    "nop", "name", "sym", "lcon", "fcon", "scon", "rval", "index"
};

public String opname (op)
Operator op;
{
    String s;
    static char buf[100];

    switch (op) {
	case O_ITOF:
	    s = "itof";
	    break;

	case O_ENDX:
	    s = "endx";
	    break;

	case O_QLINE:
	    s = "qline";
	    break;

	default:
	    if (ord(op) <= ord(O_INDEX)) {
		s = leafname[ord(op)];
	    } else {
		s = opinfo[ord(op)].opstring;
		if (s == nil) {
		    sprintf(buf, "[op %d]", op);
		    s = buf;
		}
	    }
	    break;
    }
    return s;
}
