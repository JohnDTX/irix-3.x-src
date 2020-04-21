char _Origin_[] = "System V";

/* Copyright (c) 1980 Regents of the University of California */

#include "sh.h"
#include <time.h>

/*
 * C Shell
 *
 * Bill Joy, UC Berkeley
 * October, 1978
 */

#define	TIMNULL	((struct tm *) 0)

char	*pathlist[] =	{ SRCHPATH, 0 };
char	*month[12] = {
	"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};
char	*days[7] = {
	"Sun","Mon","Tue","Wed","Thu","Fri","Sat"
};
struct	tm  *tim;
char	*dumphist[] =	{ "history", "-h", 0, 0 };
char	*loadhist[] =	{ "source", "-h", "~/.history", 0 };
int	phup();
bool	enterhist = 0;
int	whyle1st = 0;
int	prompt = 1;

main(c, av)
	int c;
	char **av;
{
	register char **v, *cp;
	int nofile = 0;
	int reenter = 0;
	bool nverbose = 0, nexececho = 0, quitit = 0, fast = 0;
	char *hp;

	settimes();			/* Immed. estab. timing base */
	hp = getenv("HOME");
	v = av;
	if (eq(v[0], "a.out"))		/* A.out's are quittable */
		quitit = 1;
	uid = getuid();
#ifdef V6
	loginsh = eq(*v, "-");		/* To do .login/.logout */
#else
	loginsh = **v == '-';
#endif
	if (c > 1 && !strcmp(v[1],"-l")) {
		**v = '-';
		loginsh = 1;
		c--, v++;
	}
	if (loginsh)
		time(&chktim);

	/*
	 * Move the descriptors to safe places.
	 * The variable didfds is 0 while we have only FSH* to work with.
	 * When didfds is true, we have 0,1,2 and prefer to use these.
	 */
	initdesc();

	/*
	 * Initialize the shell variables.
	 * ARGV and PROMPT are initialized later.
	 * STATUS is also munged in several places.
	 * CHILD is munged when forking/waiting
	 */

	set("status", "0");
	dinit(hp);			/* dinit thinks that HOME == cwd in a
					 * login shell */

	/*
	 * Grab other useful things from the environment.
	 */
	if ((cp = getenv("USER")) != NOSTR)
		set("user", savestr(cp));
					/* System V's equiv. of USER */
	if ((cp = getenv("LOGNAME")) != NOSTR)
		set("logname", savestr(cp));
	if ((cp = getenv("TERM")) != NOSTR)
		set("term", savestr(cp));

	/* Default history characters */
	HIST = '!'; HISTSUB = '^';

#ifdef	USG
	(void) tzset();			/* set timezone (for timepr() */
#endif

	if (hp == 0)
		fast++;			/* No home -> can't read scripts */
	else
		set("home", hp);
	if (uid == 0)
		pathlist[0] = "/etc";
	set1("path", saveblk(pathlist), &shvhed);
	/*
	 * Re-initialize path if set in environment
	 */
	cp = getenv("PATH");
	if (cp != 0) {
		importpath(cp);
	}

	set("shell", SHELLPATH);

	doldol = putn(getpid());		/* For $$ */
	shtemp = strspl("/tmp/sh", doldol);	/* For << */

	/*
	 * Record the interrupt states from the parent process.
	 * If the parent is non-interruptible our hand must be forced
	 * or we (and our children) won't be either.
	 * Our children inherit termination from our parent.
	 * We catch it only if we are the login shell.
	 */
	parintr = signal(SIGINT, SIG_IGN);	/* parents interruptibility */
	signal(SIGINT, parintr);			/* ... restore */
	parterm = signal(SIGTERM, SIG_IGN);	/* parents terminability */
	signal(SIGTERM, parterm);			/* ... restore */
	if (loginsh)
		signal(SIGHUP, phup);

	/*
	 * Process the arguments.
	 *
	 * Note that processing of -v/-x is actually delayed till after
	 * script processing.
	 *
	 * We set the first character of our name to be '-' if we are
	 * a shell running interruptible commands.  Many programs which
	 * examine ps'es use this to filter such shells out.
	 */
	c--, v++;
	while (c > 0 && (cp = v[0])[0] == '-') {
		do switch (*cp++) {

		case 0:			/* -	Interruptible, no prompt */
			prompt = 0;
			**av = '-';
			nofile++;
			break;

		case 'c':		/* -c	Command input from arg */
			if (c == 1)
				exit(0);
			c--, v++;
			arginp = v[0];
			prompt = 0;
			nofile++;
			break;

		case 'e':		/* -e	Exit on any error */
			exiterr++;
			break;

		case 'f':		/* -f	Fast start */
			fast++;
			break;

		case 'i':		/* -i	Interactive, even if !intty */
			intact++;
			**av = '-';
			nofile++;
			break;

		case 'n':		/* -n	Don't execute */
			noexec++;
			break;

		case 'q':		/* -q	(Undoc'd) ... die on quit */
			quitit = 1;
			break;

		case 's':		/* -s	Read from std input */
			nofile++;
			if (isatty(SHIN))
				**v = '-';
			break;

		case 't':		/* -t	Read one line from input */
			onelflg = 2;
			if (isatty(SHIN))
				**v = '-';
			prompt = 0;
			nofile++;
			break;

		case 'v':		/* -v	Echo hist expanded input */
			nverbose = 1;			/* ... later */
			break;

		case 'x':		/* -x	Echo just before execution */
			nexececho = 1;			/* ... later */
			break;

		case 'V':		/* -V	Echo hist expanded input */
			setNS("verbose");		/* NOW! */
			break;

		case 'X':		/* -X	Echo just before execution */
			setNS("echo");			/* NOW! */
			break;

		} while (*cp);
		v++, c--;
	}

	if (quitit)			/* With all due haste, for debugging */
		signal(SIGQUIT, SIG_DFL);

	/*
	 * Unless prevented by -, -c, -i, -s, or -t, if there
	 * are remaining arguments the first of them is the name
	 * of a shell file from which to read commands.
	 */
	if (nofile == 0 && c > 0) {
		nofile = open(v[0], 0);
		if (nofile < 0) {
			child++;		/* So this ... */
			Perror(v[0]);		/* ... doesn't return */
		}
		file = v[0];
		SHIN = dmove(nofile, FSHIN);	/* Replace FSHIN */
		prompt = 0;
		c--, v++;
	}

	/*
	 * Consider input a tty if it really is or we are interactive.
	 */
	intty = intact || isatty(SHIN);
#ifdef TELL
	settell();
#endif
	/*
	 * Commands are interruptible if we are interactive
	 * or the process which created us was.
	 */
	if (intact || parintr == SIG_DFL)
		**av = '-';

	/*
	 * Save the remaining arguments in ARGV.
	 * Normally the system-supplied argument list is ok as
	 * a zero terminated value block.
	 * On some version 6 systems, it is -1 terminated and setting it
	 * to zero messes up "ps" so we change it to zero, copy
	 * the block of pointers, and put it back the way it was.
	 */
/*
	if (c == 0)
		set("argv", 0);
	else
 */
	if ((int) v[c] == -1) {
		/* ick */
		v[c] = 0, setq("argv", copyblk(v), &shvhed), v[c] = (char *) -1;
	} else
		setq("argv", v, &shvhed);

	/*
	 * Set up the prompt.
	 */
	if (prompt)
		set("prompt", uid == 0 ? "# " : "% ");

	/*
	 * If we are an interactive shell, then start fiddling
	 * with the signals; this is a tricky game.
	 */
	if (**av == '-') {
		setintr++;
		if (!quitit)		/* Wary! */
			signal(SIGQUIT, SIG_IGN);
		signal(SIGINT, SIG_IGN);
		signal(SIGTERM, SIG_IGN);
	}

	/*
	 * Set an exit here in case of an interrupt or error reading
	 * the shell start-up scripts.
	 */
	setexit();
	haderr = 0;		/* In case second time through */
	if (!fast && reenter == 0) {
		reenter++;
		if (loginsh)
			srccat("/etc", "/cshrc");	/* timezone... */
		/* Will have value("home") here because set fast if don't */
		srccat(value("home"), "/.cshrc");
		if (!fast && !arginp && !onelflg)
			dohash();
		dosource(loadhist);
		if (loginsh)
#ifdef NOHELP
			srccat("", ".login");
#else
			srccat(value("home"), "/.login");
#endif
	}

	/*
	 * Now are ready for the -v and -x flags
	 */
	if (nverbose)
		setNS("verbose");
	if (nexececho)
		setNS("echo");

	/*
	 * All the rest of the world is inside this call.
	 * The argument to process indicates whether it should
	 * catch "error unwinds".  Thus if we are a interactive shell
	 * our call here will never return by being blown past on an error.
	 */
	process(setintr);

	/*
	 * Mop-up.
	 */
	if (loginsh) {
		printf("logout\n");
		close(SHIN);
		child++;
		goodbye();
	}
	rechist();	/* record history in ~/.history (4.1bsd) */
	exitstat();
}

importpath(cp)
char *cp;
{
	register int i = 0;
	register char *dp;
	register char **pv;
	int c;
	static char dot[2] = {'.', 0};

	for (dp = cp; *dp; dp++)
		if (*dp == ':')
			i++;
	/*
	 * i+2 where i is the number of colons in the path.
	 * There are i+1 directories in the path plus we need
	 * room for a zero terminator.
	 */
	pv = (char **) calloc(i+2, sizeof (char **));
	dp = cp;
	i = 0;
	if (dp)
	for (;;) {
		if ((c = *dp) == ':' || c == 0) {
			*dp = 0;
			pv[i++] = savestr(*cp ? cp : dot);
			if (c) {
				cp = dp + 1;
				*dp = ':';
			} else
				break;
		}
		dp++;
	}
	pv[i] = 0;
	set1("path", pv, &shvhed);
	dohash();
}

/*
 * Source to the file which is the catenation of the argument names.
 */
srccat(cp, dp)
	char *cp, *dp;
{
	register char *ep = strspl(cp, dp);
	register int unit = dmove(open(ep, 0), -1);

	/* ioctl(unit, FIOCLEX, NULL); */
	xfree(ep);
	srcunit(unit, 0, 0);
}

/*
 * Source to a unit.  If onlyown it must be our file or
 * we don't chance it.	This occurs on ".cshrc"s and the like.
 */
srcunit(unit, onlyown, hflg)
	register int unit;
	bool onlyown;
	bool hflg;
{
	/* We have to push down a lot of state here */
	/* All this could go into a structure */
	int oSHIN = -1, oldintty = intty;
	struct whyle *oldwhyl = whyles;
	char *ogointr = gointr, *oarginp = arginp;
	char *oevalp = evalp, **oevalvec = evalvec;
	int oonelflg = onelflg;
	bool oenterhist = enterhist;
	char OHIST = HIST;
#ifdef TELL
	bool otell = cantell;
#endif
	struct Bin saveB;

	/* The (few) real local variables */
	jmp_buf oldexit;
	int reenter;
#ifdef SVR3
	register void (*oldint)();
#else
	register int (*oldint)();
#endif

	if (unit < 0)
		return;
	if (onlyown) {
		struct stat stb;

#ifdef V69
		if (fstat(unit, &stb) < 0 || (stb.st_uid != uid && stb.st_uid != (uid &~ 0377)))
#else
		if (fstat(unit, &stb) < 0 || (stb.st_uid != uid && stb.st_gid != getgid()))
#endif
		{
			close(unit);
			return;
		}
	}

	/*
	 * There is a critical section here while we are pushing down the
	 * input stream since we have stuff in different structures.
	 * If we weren't careful an interrupt could corrupt SHIN's Bin
	 * structure and kill the shell.
	 *
	 * We could avoid the critical region by grouping all the stuff
	 * in a single structure and pointing at it to move it all at
	 * once.  This is less efficient globally on many variable references
	 * however.
	 */
	getexit(oldexit);
	reenter = 0;
	oldint = signal(SIGINT, SIG_IGN);
	setexit();
	reenter++;
	if (reenter == 1) {
		/* Setup the new values of the state stuff saved above */
		copy(&saveB, &B, sizeof saveB);
		fbuf = (u_char **) 0;
		fseekp = feobp = fblocks = 0;
		oSHIN = SHIN, SHIN = unit, arginp = 0, onelflg = 0;
		intty = isatty(SHIN), whyles = 0, gointr = 0;
		evalvec = 0; evalp = 0;
		enterhist = hflg;
		if (enterhist)
			HIST = '\0';
		/*
		 * Now if we are allowing commands to be interrupted,
		 * we let ourselves be interrupted.
		 */
#ifdef SVR3
		signal(SIGINT, setintr ? (void(*)())pintr : oldint);
#else
		signal(SIGINT, setintr ? pintr : oldint);
#endif
#ifdef TELL
		settell();
#endif
		process(0);		/* 0 -> blow away on errors */
	}
	signal(SIGINT, oldint);
	if (oSHIN >= 0) {
		register int i;

		/* We made it to the new state... free up its storage */
		/* This code could get run twice but xfree doesn't care */
		for (i = 0; i < fblocks; i++)
			xfree(fbuf[i]);
		xfree(fbuf);

		/* Reset input arena */
		copy(&B, &saveB, sizeof B);

		close(SHIN), SHIN = oSHIN;
		arginp = oarginp, onelflg = oonelflg;
		evalp = oevalp, evalvec = oevalvec;
		intty = oldintty, whyles = oldwhyl, gointr = ogointr;
		if (enterhist)
			HIST = OHIST;
		enterhist = oenterhist;
#ifdef TELL
		cantell = otell;
#endif
	}

	resexit(oldexit);
	/*
	 * If process reset() (effectively an unwind) then
	 * we must also unwind.
	 */
	if (reenter >= 2)
		error(0);
}

goodbye()
{

	if (loginsh) {
		signal(SIGQUIT, SIG_IGN);
		signal(SIGINT, SIG_IGN);
		signal(SIGTERM, SIG_IGN);
		setintr = 0;		/* No interrupts after "logout" */
		if (adrof("home"))
			srccat(value("home"), "/.logout");
	}
	rechist();
	exitstat();
}

exitstat()
{

	/*
	 * Note that if STATUS is corrupted (i.e. getn bombs)
	 * then error will exit directly because we poke child here.
	 * Otherwise we might continue unwarrantedly (sic).
	 */
	child++;
	exit(getn(value("status")));
}

/*
 * Catch an interrupt, e.g. during lexical input.
 * If we are an interactive shell, we reset the interrupt catch
 * immediately.  In any case we drain the shell output,
 * and finally go through the normal error mechanism, which
 * gets a chance to make the shell go away.
 */
#ifdef SVR3
void
#endif
pintr()
{
	register char **v;

	if (setintr)
		signal(SIGINT, SIG_IGN);
	draino();

	/*
	 * If we have an active "onintr" then we search for the label.
	 * Note that if one does "onintr -" then we shan't be interruptible
	 * so we needn't worry about that here.
	 */
	if (gointr) {
		search(ZGOTO, 0, gointr);
		timflg = 0;
		if (v = pargv)
			pargv = 0, blkfree(v);
		if (v = gargv)
			gargv = 0, blkfree(v);
		reset();
	} else if (intty)
		printf("\n");		/* Some like this, others don't */
	error(0);
}

/*
 * Process is the main driving routine for the shell.
 * It runs all command processing, except for those within { ... }
 * in expressions (which is run by a routine evalav in sh.exp.c which
 * is a stripped down process), and `...` evaluation which is run
 * also by a subset of this code in sh.glob.c in the routine backeval.
 *
 * The code here is a little strange because part of it is interruptible
 * and hence freeing of structures appears to occur when none is necessary
 * if this is ignored.
 *
 * Note that if catch is not set then we will unwind on any error.
 * In an end-of-file occurs, we return.
 */
process(catch)
	bool catch;
{
	register char *cp;
	jmp_buf osetexit;
	struct command *t;

	getexit(osetexit);
	for (;;) {
		paraml.next = paraml.prev = &paraml;
		paraml.word = "";
		t = 0;
		setexit();
		justpr = enterhist;	/* execute if not entering history */

		/*
		 * Interruptible during interactive reads
		 */
		if (setintr)
			signal(SIGINT, pintr);

		/*
		 * For the sake of reset()
		 */
		freelex(&paraml), freesyn(t), t = 0;

		if (haderr) {
			if (!catch) {
				/* unwind */
				doneinp = 0;
				resexit(osetexit);
				reset();
			}
			haderr = 0;
			/*
			 * Every error is eventually caught here or
			 * the shell dies.  It is at this
			 * point that we clean up any left-over open
			 * files, by closing all but a fixed number
			 * of pre-defined files.  Thus routines don't
			 * have to worry about leaving files open due
			 * to deeper errors... they will get closed here.
			 */
			closem();
			continue;
		}
		if (doneinp) {
			doneinp = 0;
			break;
		}
		if (intty && prompt && evalvec == 0) {
			mailchk();
			/*
			 * If we are at the end of the input buffer
			 * then we are going to read fresh stuff.
			 * Otherwise, we are rereading input and don't
			 * need or want to prompt.
			 */
			if (fseekp == feobp)
				if (!whyles) {
				    /*
				     * Zero out tim so that
				     * timepr() will read time
				     * the first time (if any).
				     */
				  tim = TIMNULL;
				  for (cp = value("prompt"); *cp; cp++)
				    if (*cp == '!')
				      printf("%d", eventno + 1);
				    else if (*cp == '#' && flipon)
				      printf("#%%");
				    else {
				      if (*cp == '\\') {
					if (cp[1] == '!' || cp[1] == '\\'
					  || cp[1] == '#') {
					  cp++;
					  goto prchr;
					} else if (cp[1] == TIMEPR && cp[2]) {
					  timepr(*(cp+=2));
					} else
					  goto prchr;
				      } else
prchr:				        putchar(*cp | QUOTE);
				    }
				} else {
					/*
					 * Prompt for forward reading loop
					 * body content.
					 */
					printf("? ");
					whyle1st = 1;
				}
			flush();
		}
		err = 0;

		/*
		 * Echo not only on VERBOSE, but also with history expansion.
		 * If there is a lexical error then we forego history echo.
		 */
		if (lex(&paraml) && !err && intty || adrof("verbose")) {
			haderr = 1;
			prlex(&paraml);
			haderr = 0;
		}

		/*
		 * The parser may lose space if interrupted.
		 */
		if (setintr)
			signal(SIGINT, SIG_IGN);

		/*
		 * Save input text on the history list if it
		 * is from the terminal at the top level and not
		 * in a loop.
		 */
		if (enterhist || catch && intty && (!whyles || whyle1st)) {
			savehist(&paraml);
			if (whyle1st > 0)
				whyle1st--;
		}

		/*
		 * Print lexical error messages.
		 */
		if (!enterhist && err)
			error(err);

		/*
		 * If had a history command :p modifier then
		 * this is as far as we should go
		 */
		if (justpr)
			reset();

		alias(&paraml);

		/*
		 * Parse the words of the input into a parse tree.
		 */
		t = syntax(paraml.next, &paraml, 0);
		if (err)
			error(err);

		/*
		 * Execute the parse tree
		 */
		execute(t);

		/*
		 * Made it!
		 */
		freelex(&paraml), freesyn(t);
	}
	resexit(osetexit);
}

dosource(t)
	register char **t;
{
	register char *f;
	register int u;
	bool hflg = 0;
	char buf[BUFSIZ];

	t++;
	if (*t && eq(*t, "-h")) {
		t++;
		hflg++;
	}
	strcpy(buf, *t);
	f = globone(buf);
	u = dmove(open(f, 0), -1);
	xfree(f);
	if (u < 0 && !hflg)
		Perror(f);
	srcunit(u, 0, hflg);
}

/*
 * Check for mail.
 * If we are a login shell, then we don't want to tell
 * about any mail file unless its been modified
 * after the time we started.
 * This prevents us from telling the user things he already
 * knows, since the login program insists on saying
 * "You have mail."
 */
mailchk()
{
	register struct varent *v;
	register char **vp;
	time_t t;
	int intvl, cnt;

	v = adrof("mail");
	if (v == 0)
		return;
	time(&t);
	vp = v->vec;
	cnt = blklen(vp);
	intvl = (cnt && number(*vp)) ? (--cnt, getn(*vp++)) : MAILINTVL;
	if (intvl < 1)
		intvl = 1;
	if (chktim + intvl > t)
		return;
	for (; *vp; vp++) {
		bool new;
		struct stat stb;

		if (stat(*vp, &stb) < 0)
			continue;
		/*
		 * We assume that a file has been read if the access time is
		 * greater than the mod time.
		 */
		if (stb.st_size == 0)
			continue;
		if (stb.st_atime > stb.st_mtime || (stb.st_atime < chktim && stb.st_mtime < chktim))
			continue;
		new = stb.st_mtime > time0;
		if (loginsh && !new)
			continue;
		if (cnt == 1)
			printf("You have %smail.\n", new ? "new " : "");
		else
			printf("%s in %s.\n", new ? "New mail" : "Mail", *vp);
	}
	chktim = t;
}

#include <pwd.h>
/*
 * Extract a home directory from the password file
 * The argument points to a buffer where the name of the
 * user whose home directory is sought is currently.
 * We write the home directory of the user back there.
 */
gethdir(home)
	char *home;
{
	register struct passwd *pp;

	pp = getpwnam(home);
	if (pp == 0)
		return (1);
	strcpy(home, pp->pw_dir);
	return (0);
}

/*
 * Move the initial descriptors to their eventual
 * resting places, closin all other units.
 */
initdesc()
{

	didcch = 0;			/* Havent closed for child */
	didfds = 0;			/* 0, 1, 2 aren't set up */
	SHIN = dcopy(0, FSHIN);
	SHOUT = dcopy(1, FSHOUT);
	SHDIAG = dcopy(2, FSHDIAG);
	OLDSTD = dcopy(SHIN, FOLDSTD);
	closem();
}

#ifndef V6
exit(i)
	int i;
{

	_exit(i);
}
#endif

timepr(c)
char	c;
{
	char	*aptr;
	int	h;
	int	hflag;
	long	tbuf;
	long	time();

	if (tim == TIMNULL) {
		(void) time(&tbuf);
		tim = localtime(&tbuf);
	}

	hflag = 0;
	switch (c) {
	  case 'n':
		printf("\n");
		break;
	  case 't':
		printf("\t");
		break;
	  case 'm' :
		printf("%02d",tim->tm_mon+1);
		break;
	  case 'd':
		printf("%02d",tim->tm_mday);
		break;
	  case 'y' :
		printf("%02d",tim->tm_year);
		break;
	  case 'D':
		printf("%02d/%02d/%02d",
		  tim->tm_mon+1,tim->tm_mday,tim->tm_year);
		break;
	  case 'H':
		printf("%02d",tim->tm_hour);
		break;
	  case 'M':
		printf("%02d",tim->tm_min);
		break;
	  case 'S':
		printf("%02d",tim->tm_sec);
		break;
	  case 'T':
		printf("%02d:%02d:%02d",
		  tim->tm_hour,tim->tm_min,tim->tm_sec);
		break;
	  case 'j':
		printf("%03d",tim->tm_yday+1);
		break;
	  case 'w':
		printf("%01d",tim->tm_wday);
		break;
	  case 'r':
		if ((h = tim->tm_hour) >= 12)
			hflag++;
		if ((h %= 12) == 0)
			h = 12;
		printf("%02d:%02d:%02d %cM",
		  h,tim->tm_min,tim->tm_sec,hflag ? 'P' : 'A');
		break;
	  case 'R':
		if ((h = tim->tm_hour) >= 12)
			hflag++;
		if ((h %= 12) == 0)
			h = 12;
		printf("%d:%02d%cM",
		  h,tim->tm_min,hflag ? 'P' : 'A');
		break;
	  case 'h':
		printf("%s",month[tim->tm_mon]);
		break;
	  case 'a':
		printf("%s",days[tim->tm_wday]);
		break;
	}
}

/*
 * in the event of a HUP we want to save the history
 */
phup()
{
	rechist();
	exit(1);
}

rechist()
{
	char buf[BUFSIZ];
	int fp, ftmp, oldidfds;

	if (value("savehist")[0] == '\0')
		return;
	strcpy(buf, value("home"));
	strcat(buf, "/.history");
	fp = creat(buf, 0777);
	if (fp == -1)
		return;
	oldidfds = didfds;
	didfds = 0;
	ftmp = SHOUT;
	SHOUT = fp;
	strcpy(buf, value("savehist"));
	dumphist[2] = buf;
	dohist(dumphist);
	close(fp);
	SHOUT = ftmp;
	didfds = oldidfds;
}
