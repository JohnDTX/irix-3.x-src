/* Copyright (c) 1982 Regents of the University of California */

static char sccsid[] = "@(#)main.c 1.5 5/17/83";

static char rcsid[] = "$Header: /d2/3.7/src/usr.bin/dbx/RCS/main.c,v 1.1 89/03/27 17:44:36 root Exp $";

/*
 * Debugger main routine.
 */

#include "defs.h"
#include <setjmp.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include "main.h"
#include "tree.h"
#include "eval.h"
#include "debug.h"
#include "symbols.h"
#include "scanner.h"
#include "keywords.h"
#include "process.h"
#include "runtime.h"
#include "source.h"
#include "object.h"
#include "mappings.h"
#include "coredump.h"


#ifndef public

#define isterm(file)	(interactive or isatty(fileno(file)))

#ifdef sgi
#   include <termio.h>

    typedef struct termio Ttyinfo;
#else
#   include <sgtty.h>
#   include <fcntl.h>

    typedef struct {
	struct sgttyb sg;		/* standard sgttyb structure */
	struct tchars tc;		/* terminal characters */
	struct ltchars ltc;		/* local special characters */
	integer ldisc;			/* line discipline */
	integer local;			/* TIOCLGET */
	integer fcflags;		/* fcntl(2) F_GETFL, F_SETFL */
    } Ttyinfo;
#endif

#endif

public boolean coredump;		/* true if using a core dump */
public boolean runfirst;		/* run program immediately */
public boolean interactive;		/* standard input IS a terminal */
public boolean lexdebug;		/* trace scanner return values */
public boolean tracebpts;		/* trace create/delete breakpoints */
public boolean traceexec;		/* trace execution */
public boolean tracesyms;		/* print symbols are they are read */
public boolean traceblocks;		/* trace blocks while reading symbols */
public boolean quiet;			/* map addresses through page tables */
public boolean vaddrs;			/* map addresses through page tables */
#ifdef DBMEX1
public DBXSHM *shmptr;
public int	shm_id;
public	int	syncfd;
public	int	syncfdin;
public	int	dbx_shmkey;
#endif
#ifdef DBMEX
public boolean winmode;			/* run in window mode */
public	char	*winfile;
#endif

public File corefile;			/* File id of core dump */

#define FIRST_TIME 0			/* initial value setjmp returns */

private Boolean initdone = false;	/* true if initialization done */
private jmp_buf env;			/* setjmp/longjmp data */
private char outbuf[BUFSIZ];		/* standard output buffer */
private char namebuf[512];		/* possible name of object file */
private int firstarg;			/* first program argument (for -r) */

private Ttyinfo ttyinfo;
private String corename;		/* name of core file */

private catchintr();

/*
 * Main program.
 */

main(argc, argv)
int argc;
String argv[];
{
    register integer i;
    extern String date;
    extern integer versionNumber;
    extern integer subversionNumber;

    cmdname = argv[0];
    catcherrs();
    onsyserr(EINTR, nil);
    scanargs(argc, argv);
    setbuf(stdout, outbuf);
	if (quiet == false) {
    		printf("dbx version %d.%d of %s.\nType 'help' for help.\n",
				versionNumber, subversionNumber ,date);
    		fflush(stdout);
	}
    language_init();
    symbols_init();
    process_init();
    if (runfirst) {
	if (setjmp(env) == FIRST_TIME) {
	    arginit();
	    for (i = firstarg; i < argc; i++) {
		newarg(argv[i]);
	    }
	    run();
	    /* NOTREACHED */
	} else {
	    runfirst = false;
	}
    } else {
	init();
    }
    if (setjmp(env) != FIRST_TIME) {
	restoretty(stdout, &ttyinfo);
    }
    i=signal(SIGINT, catchintr);
    yyparse();
    putchar('\n');
    quit(0);
}

/*
 * Initialize the world, including setting initial input file
 * if the file exists.
 */

public init()
{
    File f;
    String home;
    char buf[100];
    extern String getenv();
	extern	String	find1stsrcline();

    savetty(stdout, &ttyinfo);
    enterkeywords();
    scanner_init();
    if (not coredump and not runfirst) {
	start(nil, nil, nil);
    }
    printf("reading symbolic information ...");
    fflush(stdout);
    readobj(objname);
    printf("\n");
    fflush(stdout);
    if (coredump) {
	int	lineno;
	String	file;

	printf("[using memory image in %s]\n", corename);
	if (vaddrs) {
	    coredump_getkerinfo();
	}
	getsrcpos();
#ifdef DBMEX
	if (winmode == true) {

		file = find1stsrcline(&lineno);
		if (file != (String) 0) {
			shm_write('S', file, 0, lineno);
		}
	}
#endif
	setcurfunc(whatblock(pc));
    } else {
	setcurfunc(program);
    }
    bpinit();
    f = fopen(initfile, "r");
    if (f != nil) {
	fclose(f);
	setinput(initfile);
    } else {
	home = getenv("HOME");
	if (home != nil) {
	    sprintf(buf, "%s/%s", home, initfile);
	    f = fopen(buf, "r");
	    if (f != nil) {
		fclose(f);
		setinput(strdup(buf));
	    }
	}
    }
    initdone = true;
}

/*
 * Re-initialize the world, first de-allocating all storage.
 * This is necessary when the symbol information must be re-read
 * from the object file when it has changed.
 *
 * Before "forgetting" things, we save the current tracing/breakpoint
 * information to a temp file.  Then after re-creating the world,
 * we read the temp file as commands.  This isn't always the right thing;
 * if a procedure that was being traced is deleted, an error message
 * will be generated.
 *
 * If the argument vector is not nil, then this is re-initialize is being
 * done in preparation for running the program.  Since we want to process
 * the commands in the temp file before running the program, we add the
 * run command at the end of the temp file.  In this case, reinit longjmps
 * back to parsing rather than returning.
 */

public reinit(argv, infile, outfile)
String *argv;
String infile;
String outfile;
{
    register Integer i;
    String tmpfile;
    extern String mktemp();

    tmpfile = mktemp("/tmp/dbxXXXX");
    setout(tmpfile);
    status();
    alias(nil, nil, nil);
    if (argv != nil) {
	printf("run");
	for (i = 1; argv[i] != nil; i++) {
	    printf(" %s", argv[i]);
	}
	if (infile != nil) {
	    printf(" < %s", infile);
	}
	if (outfile != nil) {
	    printf(" > %s", outfile);
	}
	putchar('\n');
    }
    unsetout();
    bpfree();
    objfree();
    symbols_init();
    process_init();
    enterkeywords();
    scanner_init();
    readobj(objname);
    bpinit();
    fflush(stdout);
    setinput(tmpfile);
    unlink(tmpfile);
    if (argv != nil) {
	longjmp(env, 1);
	/* NOTREACHED */
    }
}

/*
 * After a non-fatal error we skip the rest of the current input line, and
 * jump back to command parsing.
 */

public erecover()
{
    if (initdone) {
	gobble();
	longjmp(env, 1);
    }
}

/*
 * This routine is called when an interrupt occurs.
 */

private catchintr()
{
    if (isredirected()) {
	fflush(stdout);
	unsetout();
    }
    putchar('\n');
    longjmp(env, 1);
}

/*
 * Scan the argument list.
 */

private scanargs(argc, argv)
int argc;
String argv[];
{
    register int i, j;
    register Boolean foundfile;
    register File f;
    char *tmp;

    runfirst = false;
    interactive = false;
    lexdebug = false;
    tracebpts = false;
    traceexec = false;
    tracesyms = false;
    traceblocks = false;
    vaddrs = false;
    foundfile = false;
    corefile = nil;
    coredump = true;
    sourcepath = list_alloc();
    list_append(list_item("."), nil, sourcepath);
    i = 1;
#ifdef GB_FIXES
    if ((argc == 2) && (argv[1][0] == '-') && (argv[1][1] == '?')) {
	/* inquiry for flags. */
	printf("dbx: <progname> <corefilename> \n");
	printf("\t{-I <dir>}\tlook in <dir> for source files\n");
	printf("\t-c <file>\tlook in <file> for commands\n");
	printf("\t-r\trun <prog> before accepting commands\n");
	printf("\t-i\trun in interactive mode\n");
	printf("else trace:\n");
	printf("\t-b bkpts, -s syms, -n blocks, -e exec, -l lex\n\n");
	fatal("only flags query");
    }
#endif
    while (i < argc and (not foundfile or (coredump and corefile == nil))) {
	if (argv[i][0] == '-') {
	    if (streq(argv[i], "-I")) {
		++i;
		if (i >= argc) {
		    fatal("missing directory for -I");
		}
		list_append(list_item(argv[i]), nil, sourcepath);
#ifdef DBMEX
		if (winmode == true) {
			shm_write('U', argv[i], 0, 0 , 0);
		}
#endif
	    } else if (streq(argv[i], "-c")) {
		++i;
		if (i >= argc) {
		    fatal("missing command file name for -c");
		}
		initfile = argv[i];
#ifdef DBMEX
	    } else if (streq(argv[i], "-w")) {
		winmode = true;
		++i;
		shm_init(argv[i], argv[i+1]);
		i += 2;
		winfile = argv[i];
#endif
	    } else {
		for (j = 1; argv[i][j] != '\0'; j++) {
		    setoption(argv[i][j]);
		}
	    }
	} else if (not foundfile) {
	    objname = argv[i];
	    foundfile = true;
	} else if (coredump and corefile == nil) {
	    corefile = fopen(argv[i], "r");
	    corename = argv[i];
	    if (corefile == nil) {
		coredump = false;
	    }
	}
	++i;
    }
    if (i < argc and not runfirst) {
	fatal("extraneous argument %s", argv[i]);
    }
    firstarg = i;
    if (not foundfile and isatty(0)) {
	printf("enter object file name (default is `%s'): ", objname);
	fflush(stdout);
	gets(namebuf);
	if (namebuf[0] != '\0') {
	    objname = namebuf;
	}
    }
    f = fopen(objname, "r");
    if (f == nil) {
	fatal("can't read %s", objname);
    } else {
	fclose(f);
    }
    if (rindex(objname, '/') != nil) {
	tmp = strdup(objname);
	*(rindex(tmp, '/')) = '\0';
	list_append(list_item(tmp), nil, sourcepath);
#ifdef DBMEX
	if (winmode == true) {
		shm_write('U', tmp, 0, 0 , 0);
	}
#endif
    }
    if (coredump and corefile == nil) {
	if (vaddrs) {
	    corefile = fopen("/dev/mem", "r");
	    corename = "/dev/mem";
	    if (corefile == nil) {
		panic("can't open /dev/mem");
	    }
	} else {
	    corefile = fopen("core", "r");
	    corename = "core";
	    if (corefile == nil) {
		coredump = false;
	    }
	}
    }
}

/*
 * Take appropriate action for recognized command argument.
 */

private setoption(c)
char c;
{
    switch (c) {
	case 'r':   /* run program before accepting commands */
	    runfirst = true;
	    coredump = false;
	    break;

	case 'i':
	    interactive = true;
	    break;

	case 'b':
	    tracebpts = true;
	    break;

	case 'e':
	    traceexec = true;
	    break;
	case 'q':
		quiet = true;
		break;

	case 's':
	    tracesyms = true;
	    break;

	case 'n':
	    traceblocks = true;
	    break;

	case 'k':
	    vaddrs = true;
	    break;


	case 'l':
#   	    ifdef LEXDEBUG
		lexdebug = true;
#	    else
		fatal("\"-l\" only applicable when compiled with LEXDEBUG");
#	    endif
	    break;

	default:
	    fatal("unknown option '%c'", c);
    }
}

/*
 * Save/restore the state of a tty.
 */

public savetty(f, t)
File f;
Ttyinfo *t;
{
#   ifdef sgi
	ioctl(fileno(f), TCGETA, t);
#   else
	ioctl(fileno(f), TIOCGETP, &(t->sg));
	ioctl(fileno(f), TIOCGETC, &(t->tc));
	ioctl(fileno(f), TIOCGLTC, &(t->ltc));
	ioctl(fileno(f), TIOCGETD, &(t->ldisc));
	ioctl(fileno(f), TIOCLGET, &(t->local));
	t->fcflags = fcntl(fileno(f), F_GETFL, 0);
#   endif
}

public restoretty(f, t)
File f;
Ttyinfo *t;
{
#   ifdef sgi
	ioctl(fileno(f), TCSETA, t);
#   else
	ioctl(fileno(f), TIOCSETN, &(t->sg));
	ioctl(fileno(f), TIOCSETC, &(t->tc));
	ioctl(fileno(f), TIOCSLTC, &(t->ltc));
	ioctl(fileno(f), TIOCSETD, &(t->ldisc));
	ioctl(fileno(f), TIOCLSET, &(t->local));
	(void) fcntl(fileno(f), F_SETFL, t->fcflags);
#   endif
}

/*
 * Exit gracefully.
 */

public quit(r)
Integer r;
{
    pterm(process);
    exit(r);
}
