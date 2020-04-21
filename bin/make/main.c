/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef	sgi
#ident	"@(#)make:main.c	1.4.1.1"
#endif

# include "defs"
/*
command make to update programs.
Flags:	'd'  print out debugging comments
	'p'  print out a version of the input graph
	's'  silent mode--don't print out commands
	'f'  the next argument is the name of the description file;
	     makefile is the default
	'i'  ignore error codes from the shell
	'S'  stop after any command fails (normally do parallel work)
	'n'   don't issue, just print, commands
	't'   touch (update time of) files but don't issue command
	'q'   don't do anything, but check if object is up to date;
	      returns exit code 0 if up to date, -1 if not
*/

char makefile[] = "makefile";
char Nullstr[] = "";
char Makefile[] =	"Makefile";
char RELEASE[] = "RELEASE";
CHARSTAR badptr = (CHARSTAR)-1;

NAMEBLOCK mainname ;
NAMEBLOCK firstname;
LINEBLOCK sufflist;
VARBLOCK firstvar;
PATTERN firstpat ;
OPENDIR firstod;


#include <signal.h>
int sigivalue=0;
int sigqvalue=0;
int waitpid=0;

int Mflags=MH_DEP;
int ndocoms=0;
int okdel=YES;

#ifdef	sgi
CHARSTAR prompt="";	/* other systems -- pick what you want */
char *makepath;
#else	/* sgi */
CHARSTAR prompt="\t";	/* other systems -- pick what you want */
#endif	/* sgi */
char junkname[20];
char funny[128];




char Makeflags[]="MAKEFLAGS";

main(argc,argv)
int argc;
CHARSTAR argv[];
{
	register NAMEBLOCK p;
	int i, j;
	int descset, nfargs;
	TIMETYPE tjunk;
	char c;
	CHARSTAR s, *e;
#ifdef unix
	int intrupt();



#endif

#ifdef METERFILE
	meter(METERFILE);
#endif

	descset = 0;

	for(s = "#|=^();&<>*?[]:$`'\"\\\n" ; *s ; ++s)
		funny[*s] |= META;
	for(s = "\n\t :=;{}&>|" ; *s ; ++s)
		funny[*s] |= TERMINAL;
	funny['\0'] |= TERMINAL;

	TURNON(INTRULE);		/* Default internal rules, turned on */

/*
 *	Set command line flags
 */

	getmflgs();				/* Init $(MAKEFLAGS) variable */
	setflags(argc, argv);

	setvar("$","$");


/*
 *	Read command line "=" type args and make them readonly.
 */
	TURNON(INARGS|EXPORT);
#ifdef MKDEBUG
	if(IS_ON(DBUG))printf("Reading \"=\" type args on command line.\n");
#endif
	for(i=1; i<argc; ++i)
		if(argv[i]!=0 && argv[i][0]!=MINUS && (eqsign(argv[i]) == YES) )
			argv[i] = 0;
	TURNOFF(INARGS|EXPORT);

/*
 *	Read internal definitions and rules.
 */

	if( IS_ON(INTRULE) )
	{
#ifdef MKDEBUG
		if(IS_ON(DBUG))printf("Reading internal rules.\n");
#endif
		rdd1(NULL);
	}

/*
 *	Done with internal rules, now.
 */
	TURNOFF(INTRULE);
#ifdef	sgi
	/*
	 * Stash value of MAKEPATH variable, if any.  Get it early, so that
	 * it can be used to find the makefile.  Also, by getting it early
	 * we allow the user to over-ride via all possible means, the
	 * value that we assign to it.
	 */
	setup_makepath();
#endif	/* sgi */

/*
 *	Read environment args.  Let file args which follow override.
 *	unless 'e' in MAKEFLAGS variable is set.
 */
	if( any( (varptr(Makeflags))->varval, 'e') )
		TURNON(ENVOVER);
#ifdef MKDEBUG
	if(IS_ON(DBUG))printf("Reading environment.\n");
#endif
	TURNON(EXPORT);
	readenv();
	TURNOFF(EXPORT|ENVOVER);

/*
 *	Read command line "-f" arguments.
 */

	rdmakecomm();

	for(i = 1; i < argc; i++)
		if( argv[i] && argv[i][0] == MINUS && argv[i][1] == 'f' && argv[i][2] == CNULL)
		{
			argv[i] = 0;
			if(i >= argc-1)
				fatal("No description argument after -f flag");
			if( rddescf(argv[++i], YES) )
				fatal1("Cannot open %s", argv[i]);
			argv[i] = 0;
			++descset;
		}
#ifdef	sgi
	/*
	 * Recompute the makepath, now that the environment and explicit
	 * makefiles have been read.  Because of normal make behaviour,
	 * this won't overwrite any explicit users setting of MAKEPATH.
	 */
	setup_makepath();
#endif	/* sgi */


/*
 *	If no command line "-f" args then look for some form of "makefile"
 */
	if( !descset )
#ifdef unix
		if( rddescf(makefile, NO))
		if( rddescf(Makefile, NO))
		if( rddescf(makefile, YES))
			rddescf(Makefile, YES);

#endif
#ifdef gcos
		rddescf(makefile, NO);
#endif


	if(IS_ON(PRTR)) printdesc(NO);

	if( srchname(".IGNORE") ) TURNON(IGNERR);
	if( srchname(".SILENT") ) TURNON(SIL);
	if(p=srchname(".SUFFIXES")) sufflist = p->linep;
	if( !sufflist ) fprintf(stderr,"No suffix list.\n");

#ifdef unix
	sigivalue = (int)signal(SIGINT,1) & 01;
	sigqvalue = (int)signal(SIGQUIT,1) & 01;
	enbint(intrupt);
#endif

#ifdef	sgi
	/*
	 * Gasp.  Now that the makefile(s) have been read in, finally setup
	 * a cached version of MAKEPATH (etc).
	 */
	setup_makepath();
#endif	/* sgi */
	nfargs = 0;

	for(i=1; i<argc; ++i)
		if((s=argv[i]) != 0)
		{
			if((p=srchname(s)) == 0)
			{
				p = makename(s);
			}
			++nfargs;
			doname(p, 0, &tjunk);
#ifdef MKDEBUG
			if(IS_ON(DBUG)) printdesc(YES);
#endif
		}

/*
	If no file arguments have been encountered, make the first
	name encountered that doesn't start with a dot
	*/

	if(nfargs == 0)
		if(mainname == 0)
			fatal("No arguments or description file");
		else
		{
			doname(mainname, 0, &tjunk);
#ifdef MKDEBUG
			if(IS_ON(DBUG)) printdesc(YES);
#endif
		}

	exit(0);
}



#ifdef unix
intrupt()
{
	CHARSTAR p;
	NAMEBLOCK q;

	if(okdel && IS_OFF(NOEX) && IS_OFF(TOUCH) &&
	   (p=varptr("@")->varval) && 
	   (q=srchname(p)) && 
	   (exists(q)>0) &&
	   !isprecious(p) )
	{
		if(isdir(p))
			fprintf(stderr, "\n*** %s NOT REMOVED.",p);
		else if(unlink(p) == 0)
			fprintf(stderr, "\n***  %s removed.", p);
	}
	if(junkname[0])
		unlink(junkname);
	fprintf(stderr, "\n");
	exit(2);
}




isprecious(p)
CHARSTAR p;
{
	register NAMEBLOCK np;
	register LINEBLOCK lp;
	register DEPBLOCK dp;

	if(np = srchname(".PRECIOUS"))
	    for(lp = np->linep ; lp ; lp = lp->nextline)
		for(dp = lp->depp ; dp; dp = dp->nextdep)
			if(equal(p, dp->depname->namep))
				return(YES);

	return(NO);
}


enbint(k)
int (*k)();
{
	if(sigivalue == 0)
		signal(SIGINT,k);
	if(sigqvalue == 0)
		signal(SIGQUIT,k);
}
#endif

extern CHARSTAR builtin[];

CHARSTAR *linesptr=builtin;

FILE * fin;
int firstrd=0;

rdmakecomm()
{
#ifdef PWB
	register char *nlog;
	char s[128];
	CHARSTAR concat(), getenv();

	if(nlog=getenv("HOME")) {
		if(rddescf( concat(nlog,"/makecomm",s), NO))
			rddescf( concat(nlog,"/Makecomm",s), NO);
	}

	if(rddescf("makecomm", NO))
		rddescf("Makecomm", NO);
#endif
}

extern int yylineno;
extern CHARSTAR zznextc;
rddescf(descfile, flg)
CHARSTAR descfile;
int flg;			/* if YES try s.descfile */
{
	FILE * k;

/* read and parse description */

	if(equal(descfile, "-"))
		return( rdd1(stdin) );

retry:
	if( (k = fopen(descfile,"r")) != NULL)
	{
#ifdef MKDEBUG
		if(IS_ON(DBUG))printf("Reading %s\n", descfile);
#endif
		return( rdd1(k) );
	}

	if(flg == NO)
		return(1);
#ifdef	sgi
	if (makepath) {
		char *new;

		/*
		 * Try to find the makefile using the MAKEPATH
		 */
		new = findfl(descfile);
		if (new != (CHARSTAR)-1) {
			if ((k = fopen(new, "r")) != NULL) {
#ifdef MKDEBUG
				if (IS_ON(DBUG))
					printf("Reading %s\n", new);
#endif
				return(rdd1(k));
			}
		}
	}
#endif	/* sgi */
	if(get(descfile, CD, varptr(RELEASE)->varval) == NO)
		return(1);
	flg = NO;
	goto retry;

}




rdd1(k)
FILE * k;
{
	fin = k;
	yylineno = 0;
	zznextc = 0;

	if( yyparse() )
		fatal("Description file error");

	if(fin != NULL)
		fclose(fin);

	return(0);
}

printdesc(prntflag)
int prntflag;
{
	NAMEBLOCK p;
	DEPBLOCK dp;
	VARBLOCK vp;
	OPENDIR od;
	SHBLOCK sp;
	LINEBLOCK lp;

#ifdef unix
	if(prntflag)
	{
		fprintf(stderr,"Open directories:\n");
		for(od=firstod; od!=0; od = od->nextopendir)
#ifdef	sgi
			fprintf(stderr,"\t%d: %s\n", od->dirfc->dd_fd, od->dirn);
#else	/* sgi */
			fprintf(stderr,"\t%d: %s\n", fileno(od->dirfc), od->dirn);
#endif	/* sgi */
	}
#endif

	if(firstvar != 0) fprintf(stderr,"Macros:\n");
	for(vp=firstvar; vp!=0; vp=vp->nextvar)
		if(vp->v_aflg == NO)
			printf("%s = %s\n" , vp->varname , vp->varval);
		else
		{
			CHAIN pch;

			fprintf(stderr,"Lookup chain: %s\n\t", vp->varname);
			for(pch = (CHAIN)vp->varval; pch; pch = pch->nextchain)
				fprintf(stderr," %s",
					((NAMEBLOCK)pch->datap)->namep);
			fprintf(stderr,"\n");
		}

	for(p=firstname; p!=0; p = p->nextname)
		prname(p, prntflag);
	printf("\n");
	fflush(stdout);
}

prname(p, prntflag)
register NAMEBLOCK p;
{
	register LINEBLOCK lp;
	register DEPBLOCK dp;
	register SHBLOCK sp;

	if(p->linep != 0)
		printf("\n\n%s:",p->namep);
	else
		fprintf(stderr, "\n\n%s", p->namep);
	if(prntflag)
	{
		fprintf(stderr,"  done=%d",p->done);
	}
	if(p==mainname) fprintf(stderr,"  (MAIN NAME)");
	for(lp = p->linep ; lp!=0 ; lp = lp->nextline)
	{
		if( dp = lp->depp )
		{
			fprintf(stderr,"\n depends on:");
			for(; dp!=0 ; dp = dp->nextdep)
				if(dp->depname != 0)
				{
					printf(" %s", dp->depname->namep);
					printf(" ");
				}
		}
		if(sp = lp->shp)
		{
			printf("\n");
			fprintf(stderr," commands:\n");
			for( ; sp!=0 ; sp = sp->nextsh)
				printf("\t%s\n", sp->shbp);
		}
	}
}


setflags(ac, av)
int ac;
CHARSTAR *av;
{
	register int i, j;
	register char c;
	int flflg=0;			/* flag to note `-f' option. */

	for(i=1; i<ac; ++i)
	{
		if(flflg)
		{
			flflg = 0;
			continue;
		}
#ifdef	sgi
		/*
		 * This fixes a bug which later causes make to core dump,
		 * if the user says something like 'make ""' to the shell.
		 */
		if (av[i] != 0 && av[i][0] == CNULL) {
			av[i] = 0;
			continue;
		}
#endif
		if(av[i]!=0 && av[i][0]==MINUS)
		{
			if(any(av[i], 'f'))
				flflg++;
			for(j=1 ; (c=av[i][j])!=CNULL ; ++j)
				optswitch(c);
			if(flflg)
				av[i] = "-f";
			else
				av[i] = 0;
		}
	}
}


/*
 *	Handle a single char option.
 */
optswitch(c)
register char c;
{

	switch(c)
	{

	case 'e':	/* environment override flag */
		setmflgs(c);
		break;

	case 'd':	/* debug flag */
#ifdef MKDEBUG
		TURNON(DBUG);
		setmflgs(c);
#else
		printf("make: no debugging information available.\n");
#endif
		break;

	case 'p':	/* print description */
		TURNON(PRTR);
		break;

	case 's':	/* silent flag */
		TURNON(SIL);
		setmflgs(c);
		break;

	case 'i':	/* ignore errors */
		TURNON(IGNERR);
		setmflgs(c);
		break;

	case 'S':
		TURNOFF(KEEPGO);
		setmflgs(c);
		break;

	case 'k':
		TURNON(KEEPGO);
		setmflgs(c);
		break;

	case 'n':	/* do not exec any commands, just print */
		TURNON(NOEX);
		setmflgs(c);
		break;

	case 'r':	/* turn off internal rules */
		TURNOFF(INTRULE);
		break;

	case 't':	/* touch flag */
		TURNON(TOUCH);
		setmflgs(c);
		break;

	case 'q':	/* question flag */
		TURNON(QUEST);
		setmflgs(c);
		break;

	case 'g':	/* turn default $(GET) of files not found */
		TURNON(GET);
		setmflgs(c);
		break;

	case 'm':	/* print memory map */
		TURNON(MEMMAP);
		setmflgs(c);
		break;

	case 'b':	/* use MH version of test for whether a cmd exists */
		TURNON(MH_DEP);
		setmflgs(c);
		break;
	case 'B':	/* turn off -b flag */
		TURNOFF(MH_DEP);
		setmflgs(c);
		break;

	case 'f':	/* Named makefile; already handled by setflags(). */
		break;

	default:
		fatal1("Unknown flag argument %c", c);
	}
}

/*
 *	getmflgs() set the cmd line flags into an EXPORTED variable
 *	for future invocations of make to read.
 */


getmflgs()
{
	register VARBLOCK vpr;
	register CHARSTAR *pe;
	register CHARSTAR p;

	vpr = varptr(Makeflags);
	setvar(Makeflags, "ZZZZZZZZZZZZZZZZ");
	vpr->varval[0] = CNULL;
	vpr->envflg = YES;
	vpr->noreset = YES;
	optswitch('b');
	for(pe = environ; *pe; pe++)
	{
		if(sindex(*pe, "MAKEFLAGS=") == 0)
		{
			for(p = (*pe)+sizeof Makeflags; *p; p++)
				optswitch(*p);
			return;
		}
	}
}

/*
 *	setmflgs(c) sets up the cmd line input flags for EXPORT.
 */

setmflgs(c)
register char c;
{
	register VARBLOCK vpr;
	register CHARSTAR p;

	vpr = varptr(Makeflags);
	for(p = vpr->varval; *p; p++)
	{
		if(*p == c)
			return;
	}
	*p++ = c;
	*p = CNULL;
}
#ifdef	sgi

/*
 * Stash a copy of the MAKEPATH variable, if it exists.  Optionally,
 * if MAKEMAKEPATH is defined, construct MAKEPATH using SRCROOT and
 * BINROOT and the current working directory:
 *
 *	if (defined(MAKEMAKEPATH)) {
 *		if (notnull(SRCROOT) && notnull(BINROOT)) {
 *			SRCDIR= see below
 *			MAKEPATH = ".:SRCDIR:SRCDIR/RCS"
 *		}
 *	}
 */
setup_makepath()
{
	VARBLOCK srcroot, binroot;
	VARBLOCK vp;
	char *cwd;
	int len;
	char *cp1, *cp2;
	extern char *getcwd();

	vp = varptr("MAKEPATH");
	if (vp && *vp->varval) {
		/*
		 * Record value of make path, for quick access.
		 */
		makepath = vp->varval;
	} else {
		/*
		 * Provide a minimal default makepath.
		 */
		setvar("MAKEPATH", ".");
	}

	vp = varptr("MAKEMAKEPATH");
	if (vp && *vp->varval) {
		srcroot = varptr("SRCROOT");
		binroot = varptr("BINROOT");
		if ((srcroot == NULL) || (*srcroot->varval == CNULL) ||
		    (binroot == NULL) || (*binroot->varval == CNULL))
			fatal("${SRCROOT}/${BINROOT} not set");
		/*
		 * Get current directory
		 */
		cwd = getcwd(0, MAXNAMLEN+2);
		if (cwd == NULL)			/* huh? */
			fatal("can't find current working directory");
#ifdef	MKDEBUG
		if (IS_ON(DBUG))
			printf("cwd=%s\n", cwd);
#endif
		/*
		 * Using the BINROOT as a template, strip off the
		 * BINROOT prefix from cwd.
		 */
		len = strlen(binroot->varval);
		if (strncmp(binroot->varval, cwd, len)) {
			/*
			 * For some reason, the current directory
			 * is not in BINROOT.  Punt.
			 */
			fatal("${BINROOT} and cwd not related");
		}
		cwd += len;
		while (*cwd == '/')		/* skip past noisy /'s */
			cwd++;
		/*
		 * Allocate enough memory to hold the SRCROOT prefix
		 * plus a "/" plus the cwd tail component plus 1 for
		 * a null plus a few bytes for fudge room.
		 */
		cp1 = (char *) malloc(strlen(srcroot->varval) + 1 +
					       strlen(cwd) + 1 + 20);
		if (cp1 == NULL)
			fatal("out of memory");
		strcpy(cp1, srcroot->varval);
		if ((*(cp1 + strlen(srcroot->varval) - 1) != '/') && *cwd)
			strcat(cp1, "/");
		strcat(cp1, cwd);
		setvar("SRCDIR", cp1);
		/*
		 * Make MAKEPATH be ".:${SRCDIR}:${SRCDIR}/RCS".
		 */
		cp2 = (char *)malloc(sizeof(".:") + strlen(cp1)*2 +
				       sizeof(":") + sizeof("/RCS") + 1);
		sprintf(cp2, ".:%s:%s/RCS", cp1, cp1);
		setvar("MAKEPATH", cp2);
		makepath = cp2;
		/*
		 * Make the SRCDIR, MAKEPATH, etc. be in the environment.
		 */
		varptr("SRCDIR")->envflg = 1;
		varptr("MAKEPATH")->envflg = 1;
#ifdef	MKDEBUG
		if (IS_ON(DBUG)) {
			printf("SRCDIR=%s\n", cp1);
			printf("MAKEPATH=%s\n", cp2);
		}
#endif
	}
}
#endif
