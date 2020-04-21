char _Version_[] = "(C) Copyright 1983 UniSoft Corp. Version V.1.0";
char _Origin_[] = "UC Berkeley";

static	char *sccsid = "@(#)more.c	4.4 (Berkeley) 81/04/23";

/*
** more.c - General purpose tty output filter and file perusal program
**
**	by Eric Shienbrood, UC Berkeley
**
**	modified by Geoff Peck, UCB to add underlining, single spacing
**	modified by John Foderaro, UCB to add -c and MORE environment variable
*/

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <termio.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <varargs.h>	/* for execute */
#ifdef SVR3
#include <filehdr.h>	/* for coff magic numbers */
#endif SVR3

/* Help file will eventually go in libpath(more.help) on all systems */


#ifdef SVR3
#define HELPFILE	"/usr/bsd/more.help"
#define VI		"/usr/bin/vi"
#else
#define HELPFILE	"/usr/lib/more.help"
#define VI		"/bin/vi"
#endif SVR3

#ifdef SVR3
#define Fdopen(s,m)	(Currline = 0,file_pos=0,fdopen(s,m))
#endif SVR3
#define Fopen(s,m)	(Currline = 0,file_pos=0,fopen(s,m))
#define Ftell(f)	file_pos
#define Fseek(f,off)	(file_pos=off,fseek(f,off,0))
#define Getc(f)		(++file_pos, getc(f))
#define Ungetc(c,f)	(--file_pos, ungetc(c,f))

#define stty(fd,argp)	ioctl(fd,TCSETAW,argp)

#define TBUFSIZ	1024
#define LINSIZ	256
#define ctrl(letter)	('letter' & 077)
#define RUBOUT	'\177'
#define ESC	'\033'
#define QUIT	'\034'

struct termio 	otty, ntty;
long		file_pos, file_size;
int		fnum, no_intty, no_tty, slow_tty;
int		dum_opt, dlines, onquit(), end_it();
#ifdef SIGTSTP
int		onsusp();
#endif
int		nscroll = 11;	/* Number of lines scrolled by 'd' */
int		fold_opt = 1;	/* Fold long lines */
int		stop_opt = 1;	/* Stop after form feeds */
int		ssp_opt = 0;	/* Suppress white space */
int		ul_opt = 1;	/* Underline as best we can */
int		promptlen;
int		Currline;	/* Line we are currently at */
int		startup = 1;
int		firstf = 1;
int		notell = 1;
int		bad_so;	/* True if overwriting does not turn off standout */
int		inwait, Pause, errors;
int		within;	/* true if we are within a file,
			false if we are between files */
int		hard, dumb, noscroll, hardtabs, clreol;
int		catch_susp;	/* We should catch the SIGTSTP signal */
char		**fnames;	/* The list of file names */
int		nfiles;		/* Number of files left to process */
char		*shell;		/* The name of the shell to use */
int		shellp;		/* A previous shell command exists */
char		ch;
jmp_buf		restore;
char		obuf[BUFSIZ];	/* stdout buffer */
char		Line[LINSIZ];	/* Line buffer */
int		Lpp = 24;	/* lines per page */
char		*Clear;		/* clear screen */
char		*eraseln;	/* erase line */
char		*Senter, *Sexit;/* enter and exit standout mode */
char		*ULenter, *ULexit;	/* enter and exit underline mode */
char		*chUL;		/* underline character */
char		*chBS;		/* backspace character */
char		*Home;		/* go to home */
char		*cursorm;	/* cursor movement */
char		cursorhome[40];	/* contains cursor movement to home */
char		*EodClr;	/* clear rest of screen */
char		*tgetstr();
int		Mcol = 80;	/* number of columns */
int		Wrap = 1;	/* set if automargins */
long		fseek();
char		*getenv();
int 		readfd = -1;	/* read file descriptor */
int 		writefd = -1;	/* write file descriptor */
struct {
    long chrctr, line;
} context, screen_start;
extern char	PC;		/* pad character */
extern short	ospeed;


main(argc, argv)
int argc;
char *argv[];
{
    register FILE	*f;
    register char	*s;
    register char	*p;
    register char	ch;
    register int	left;
    int			prnames = 0; 
    int			initopt = 0;
    int			srchopt = 0;
    int			clearit = 0;
    int			initline;
    char		initbuf[80];
    FILE		*checkf();

    nfiles = argc;
    fnames = argv;
    initterm ();
    if(s = getenv("MORE")) argscan(s);
    while (--nfiles > 0) {
	if ((ch = (*++fnames)[0]) == '-') {
	    argscan(*fnames+1);
	}
	else if (ch == '+') {
	    s = *fnames;
	    if (*++s == '/') {
		srchopt++;
		for (++s, p = initbuf; p < initbuf + 79 && *s != '\0';)
		    *p++ = *s++;
		*p = '\0';
	    }
	    else {
		initopt++;
		for (initline = 0; *s != '\0'; s++)
		    if (isdigit (*s))
			initline = initline*10 + *s -'0';
		--initline;
	    }
	}
	else break;
    }
    /* allow clreol only if Home and eraseln and EodClr strings are
     *  defined, and in that case, make sure we are in noscroll mode
     */
    if(clreol)
    {
	if (!no_tty && 
	    ((*Home == '\0') || (*eraseln == '\0') || (*EodClr == '\0')))
	    clreol = 0;
	else noscroll = 1;
    }

    if (dlines == 0)
	dlines = Lpp - (noscroll ? 1 : 2);
    left = dlines;
    if (nfiles > 1)
	prnames++;
    if (!no_intty && nfiles == 0) {
	fputs("Usage: ",stderr);
	fputs(argv[0],stderr);
	fputs(" [-dfln] [+linenum | +/pattern] name1 name2 ...\n",stderr);
	exit(1);
    }
    else
	f = stdin;
    if (!no_tty) {
	signal(SIGQUIT, onquit);
	signal(SIGINT, end_it);
#ifdef SIGTSTP
	if (signal (SIGTSTP, SIG_IGN) == SIG_DFL) {
	    signal(SIGTSTP, onsusp);
	    catch_susp++;
	}
#endif
	stty (2, &ntty);
    }
    if (no_intty) {
	if (no_tty)
	    copy_file (stdin);
	else {
	    if ((ch = Getc (f)) == '\f')
		doclear();
	    else {
		Ungetc (ch, f);
		if (noscroll && (ch != EOF)) {
		    if (clreol)
			home ();
		    else
			doclear ();
		}
	    }
	    if (srchopt)
	    {
		search (initbuf, stdin, 1);
		if (noscroll)
		    left--;
	    }
	    else if (initopt)
		skiplns (initline, stdin);
	    screen (stdin, left);
	}
	no_intty = 0;
	prnames++;
	firstf = 0;
    }

    while (fnum < nfiles) {
	if ((f = checkf (fnames[fnum], &clearit)) != NULL) {
	    context.line = context.chrctr = 0;
	    Currline = 0;
	    if (firstf) setjmp (restore);
	    if (firstf) {
		firstf = 0;
		if (srchopt)
		{
		    search (initbuf, f, 1);
		    if (noscroll)
			left--;
		}
		else if (initopt)
		    skiplns (initline, f);
	    }
	    else if (fnum < nfiles && !no_tty) {
		setjmp (restore);
		left = command (fnames[fnum], f);
	    }
	    if (left != 0) {
		if ((noscroll || clearit) && (file_size != 0x7fffffffffffffffL))
		    if (clreol)
			home ();
		    else
			doclear ();
		if (prnames) {
		    if (bad_so)
			erase (0);
		    if (clreol)
			cleareol ();
		    pr("::::::::::::::");
		    if (promptlen > 14)
			erase (14);
		    printf ("\n");
		    if(clreol) cleareol();
		    printf("%s\n", fnames[fnum]);
		    if(clreol) cleareol();
		    printf("::::::::::::::\n", fnames[fnum]);
		    if (left > Lpp - 4)
			left = Lpp - 4;
		}
		if (no_tty)
		    copy_file (f);
		else {
		    within++;
		    screen(f, left);
		    within = 0;
		}
	    }
	    setjmp (restore);
	    fflush(stdout);
	    fclose(f);
	    screen_start.line = screen_start.chrctr = 0L;
	    context.line = context.chrctr = 0L;
	}
	fnum++;
	firstf = 0;
    }
    reset_tty ();
    exit(0);
}

argscan(s)
register char *s;
{
	    for (dlines = 0; *s != '\0'; s++)
		if (isdigit(*s))
		    dlines = dlines*10 + *s - '0';
		else if (*s == 'd')
		    dum_opt = 1;
		else if (*s == 'l')
		    stop_opt = 0;
		else if (*s == 'f')
		    fold_opt = 0;
		else if (*s == 'p')
		    noscroll++;
		else if (*s == 'c')
		    clreol++;
		else if (*s == 's')
		    ssp_opt = 1;
		else if (*s == 'u')
		    ul_opt = 0;
}


/*
** Check whether the file named by fs is an ASCII file which the user may
** access.  If it is, return the opened file. Otherwise return NULL.
*/

FILE *
checkf (fs, clearfirst)
register char *fs;
int *clearfirst;
{
    struct stat stbuf;
    register FILE *f;
    char c;
#ifdef SVR3
    int fd;
    unsigned short magic;
#endif SVR3

    if (stat (fs, &stbuf) == -1) {
	fflush(stdout);
	if (clreol)
	    cleareol ();
	perror(fs);
	return (NULL);
    }
    if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
	printf("\n*** %s: directory ***\n\n", fs);
	return (NULL);
    }

#ifdef SVR3
    if ((fd = open(fs,O_RDONLY)) <0 ) {
	fflush(stdout);
	perror(fs);
	return (NULL);
    }
    (void) read(fd, &magic, 2);
    (void) lseek(fd, 0, 0); /* rewind file pointer */
    if (ISCOFF(magic)) {
	printf("\n******** %s: Not a text file ********\n\n", fs);
	close (fd);
	return (NULL);
    }

    if ((f=Fdopen(fd, "r")) == NULL)
#else not  SVR3
    if ((f=Fopen(fs, "r")) == NULL)
#endif not  SVR3
    {
	fflush(stdout);
	perror(fs);
	return (NULL);
    }
    c = Getc(f);

    /* Try to see whether it is an ASCII file */

    switch ((c | *f->_ptr << 8) & 0177777) {
    case 0405:
    case 0407:
    case 0410:
    case 0411:
    case 0413:
    case 0177545:
	printf("\n******** %s: Not a text file ********\n\n", fs);
	fclose (f);
	return (NULL);
    default:
	break;
    }
    if (c == '\f')
	*clearfirst = 1;
    else {
	*clearfirst = 0;
	Ungetc (c, f);
    }
    if ((file_size = stbuf.st_size) == 0)
	file_size = 0x7fffffffffffffffL;
    return (f);
}

/*
** A real function, for the tputs routine in termlib
*/

putch (ch)
char ch;
{
    putchar (ch);
}

/*
** Print out the contents of the file f, one screenful at a time.
*/

#define STOP -10

screen (f, num_lines)
register FILE *f;
register int num_lines;
{
    register int c;
    register int nchars;
    int length;			/* length of current line */
    static int prev_len = 1;	/* length of previous line */

    for (;;) {
	while (num_lines > 0 && !Pause) {
	    if ((nchars = getline (f, &length)) == EOF)
	    {
		if (clreol)
		    clreos();
		return;
	    }
	    if (ssp_opt && length == 0 && prev_len == 0)
		continue;
	    prev_len = length;
	    if (bad_so || (Senter && *Senter == ' ') && promptlen > 0)
		erase (0);
	    /* must clear before drawing line since tabs on some terminals
	     * do not erase what they tab over.
	     */
	    if (clreol)
		cleareol ();
	    prbuf (Line, length);
	    if (nchars < promptlen)
		erase (nchars);	/* erase () sets promptlen to 0 */
	    else promptlen = 0;
	    /* is this needed?
	     * if (clreol)
	     *	cleareol();	/* must clear again in case we wrapped *
	     */
	    if (nchars < Mcol || !fold_opt)
		putchar('\n');
	    if (nchars == STOP)
		break;
	    num_lines--;
	}
	fflush(stdout);
	if ((c = Getc(f)) == EOF)
	{
	    if (clreol)
		clreos ();
	    return;
	}

	if (Pause && clreol)
	    clreos ();
	Ungetc (c, f);
	setjmp (restore);
	Pause = 0; startup = 0;
	if ((num_lines = command (NULL, f)) == 0)
{
	    return;
}
	if (hard && promptlen > 0)
		erase (0);
	if (noscroll && num_lines == dlines)
	{ 
	    if (clreol)
		home();
	    else
		doclear ();
	}
	screen_start.line = Currline;
	screen_start.chrctr = Ftell (f);
    }
}

/*
** Come here if a quit signal is received
*/

onquit()
{
    signal(SIGQUIT, SIG_IGN);
    if (!inwait) {
	putchar ('\n');
	if (!startup) {
	    signal(SIGQUIT, onquit);
	    longjmp (restore, 1);
	}
	else
	    Pause++;
    }
    else if (!dum_opt && notell) {
	writes (2, "[Use q or Q to quit]", 20);
	promptlen += 20;
	notell = 0;
    }
    signal(SIGQUIT, onquit);
}

/*
** Clean up terminal state and exit. Also come here if interrupt signal received
*/

end_it ()
{

    reset_tty ();
    if (clreol) {
	putchar ('\r');
	clreos ();
	fflush (stdout);
    }
    else if (!clreol && (promptlen > 0)) {
	kill_line ();
	fflush (stdout);
    }
    else
	writes (2, "\n", 1);
    _exit(0);
}

copy_file(f)
register FILE *f;
{
    register int c;

    while ((c = getc(f)) != EOF)
	putchar(c);
}

/* Simplified printf function */

#ifdef sgi
printf (fmt, va_alist)
register char *fmt;
va_dcl
#else
printf (fmt, args)
register char *fmt;
int args;
#endif
{
	register int *argp;
	register char ch;
	register int ccount;

	ccount = 0;
#ifdef sgi
	argp = &va_alist;
#else
	argp = &args;
#endif
	while (*fmt) {
		while ((ch = *fmt++) != '%') {
			if (ch == '\0')
				return (ccount);
			ccount++;
			putchar (ch);
		}
		switch (*fmt++) {
		case 'd':
			ccount += printd (*argp);
			break;
		case 's':
			ccount += pr ((char *)*argp);
			break;
		case '%':
			ccount++;
			argp--;
			putchar ('%');
			break;
		case '0':
			return (ccount);
		default:
			break;
		}
		++argp;
	}
	return (ccount);

}

/*
** Print an integer as a string of decimal digits,
** returning the length of the print representation.
*/

printd (n)
int n;
{
    int a, nchars;

    if (a = n/10)
	nchars = 1 + printd(a);
    else
	nchars = 1;
    putchar (n % 10 + '0');
    return (nchars);
}

/* Put the print representation of an integer into a string */
static char *sptr;

scanstr (n, str)
int n;
char *str;
{
    sptr = str;
    sprintf (n);
    *sptr = '\0';
}

sprintf (n)
{
    int a;

    if (a = n/10)
	sprintf (a);
    *sptr++ = n % 10 + '0';
}

static char bell = ctrl(G);

strlen (s)
char *s;
{
    register char *p;

    p = s;
    while (*p++)
	;
    return (p - s - 1);
}

/* See whether the last component of the path name "path" is equal to the
** string "string"
*/

tailequ (path, string)
char *path;
register char *string;
{
	register char *tail;

	tail = path + strlen(path);
	while (tail >= path)
		if (*(--tail) == '/')
			break;
	++tail;
	while (*tail++ == *string++)
		if (*tail == '\0')
			return(1);
	return(0);
}

prompt (filename)
char *filename;
{
    if (clreol)
	cleareol ();
    else if (promptlen > 0)
	kill_line ();
    if (!hard) {
	promptlen = 8;
	if (Senter && Sexit)
	    tputs (Senter, 1, putch);
	if (clreol)
	    cleareol ();
	pr("--More--");
	if (filename != NULL) {
	    promptlen += printf ("(Next file: %s)", filename);
	}
	else if (!no_intty) {
	    promptlen += printf ("(%d%%)", (int)((file_pos * 100) / file_size));
	}
	if (dum_opt) {
	    promptlen += pr("[Hit space to continue, Rubout to abort]");
	}
	if (Senter && Sexit)
	    tputs (Sexit, 1, putch);
	if (clreol)
	    clreos ();
	fflush(stdout);
    }
    else
	writes (2, &bell, 1);
    inwait++;
}

/*
** Get a logical line
*/

getline(f, length)
register FILE *f;
int *length;
{
    register int	c;
    register char	*p;
    register int	column;
    static int		colflg;

    p = Line;
    column = 0;
    c = Getc (f);
    if (colflg && c == '\n') {
	Currline++;
	c = Getc (f);
    }
    while (p < &Line[LINSIZ - 1]) {
	if (c == EOF) {
	    if (p > Line) {
		*p = '\0';
		*length = p - Line;
		return (column);
	    }
	    *length = p - Line;
	    return (EOF);
	}
	if (c == '\n') {
	    Currline++;
	    break;
	}
	*p++ = c;
	if (c == '\t')
	    if (hardtabs && column < promptlen && !hard) {
		if (eraseln && !dumb) {
		    column = 1 + (column | 7);
		    tputs (eraseln, 1, putch);
		    promptlen = 0;
		}
		else {
		    for (--p; column & 7 && p < &Line[LINSIZ - 1]; column++) {
			*p++ = ' ';
		    }
		    if (column >= promptlen) promptlen = 0;
		}
	    }
	    else
		column = 1 + (column | 7);
	else if (c == '\b') {
	    if (column != 0)
		    column--;
	} else if (c == '\r')
	    column = 0;
	else if (c == '\f' && stop_opt) {
		p[-1] = '^';
		*p++ = 'L';
		column += 2;
		Pause++;
	}
	else if (c == EOF) {
	    *length = p - Line;
	    return (column);
	}
	else if (c >= ' ' && c != RUBOUT)
	    column++;
	if (column >= Mcol && fold_opt) break;
	c = Getc (f);
    }
    if (column >= Mcol && Mcol > 0) {
	if (!Wrap) {
	    *p++ = '\n';
	}
    }
    colflg = column == Mcol && fold_opt;
    *length = p - Line;
    *p = 0;
    return (column);
}

/*
** Erase the rest of the prompt, assuming we are starting at column col.
*/

erase (col)
register int col;
{

    if (promptlen == 0)
	return;
    if (hard) {
	putchar ('\n');
    }
    else {
	if (col == 0)
	    putchar ('\r');
	if (!dumb && eraseln)
	    tputs (eraseln, 1, putch);
	else
	    for (col = promptlen - col; col > 0; col--)
		putchar (' ');
    }
    promptlen = 0;
}

/*
** Erase the current line entirely
*/

kill_line ()
{
    erase (0);
    if (!eraseln || dumb) putchar ('\r');
}

/*
 * force clear to end of line
 */
cleareol()
{
    tputs(eraseln, 1, putch);
}

clreos()
{
    tputs(EodClr, 1, putch);
}

/*
**  Print string and return number of characters
*/

pr(s1)
char	*s1;
{
    register char	*s;
    register char	c;

    for (s = s1; c = *s++; )
	putchar(c);
    return (s - s1 - 1);
}


/* Print a buffer of n characters */

prbuf (s, n)
register char *s;
register int n;
{
    char c;				/* next ouput character */
    register int state;			/* next output char's UL state */
    static int pstate = 0;		/* current terminal UL state (off) */

    while (--n >= 0)
	if (!ul_opt)
	    putchar (*s++);
	else {
	    if (n >= 2 && s[0] == '_' && s[1] == '\b') {
		n -= 2;
	        s += 2;
		c = *s++;
		state = 1;
	    } else if (n >= 2 && s[1] == '\b' && s[2] == '_') {
		n -= 2;
		c = *s++;
		s += 2;
		state = 1;
	    } else {
		c = *s++;
		state = 0;
	    }
	    if (state != pstate)
		tputs(state ? ULenter : ULexit, 1, putch);
	    pstate = state;
	    putchar(c);
	    if (state && *chUL) {
		pr(chBS);
		tputs(chUL, 1, putch);
	    }
	}
}

/*
**  Clear the screen
*/

doclear()
{
    if (Clear && !hard) {
	tputs(Clear, 1, putch);

	/* Put out carriage return so that system doesn't
	** get confused by escape sequences when expanding tabs
	*/
	putchar ('\r');
	promptlen = 0;
    }
}

/*
 * Go to home position
 */
home()
{
    tputs(Home,1,putch);
}

static int lastcmd, lastarg, lastp;
static int lastcolon;
char shell_line[132];

/*
** Read a command and do it. A command consists of an optional integer
** argument followed by the command character.  Return the number of lines
** to display in the next screenful.  If there is nothing more to display
** in the current file, zero is returned.
*/

command (filename, f)
char *filename;
register FILE *f;
{
    register int nlines;
    register int retval;
    register char c;
    char colonch;
    FILE *helpf;
    int done;
    char comchar, cmdbuf[80], *p;

#define ret(val) retval=val;done++;break

    done = 0;
    if (!errors)
	prompt (filename);
    else
	errors = 0;
    for (;;) {
	nlines = number (&comchar);
	lastp = colonch = 0;
	if (comchar == '.') {	/* Repeat last command */
		lastp++;
		comchar = lastcmd;
		nlines = lastarg;
		if (lastcmd == ':')
			colonch = lastcolon;
	}
	lastcmd = comchar;
	lastarg = nlines;
	if (comchar == otty.c_cc[VERASE]) {
	    kill_line ();
	    prompt (filename);
	    continue;
	}
	switch (comchar) {
	case ':':
	    retval = colon (filename, colonch, nlines);
	    if (retval >= 0)
		done++;
	    break;
	case ' ':
	case 'z':
	    if (nlines == 0) nlines = dlines;
	    else if (comchar == 'z') dlines = nlines;
	    ret (nlines);
	case 'd':
	case ctrl(D):
	    if (nlines != 0) nscroll = nlines;
	    ret (nscroll);
	case RUBOUT:
	case 'q':
	case 'Q':
	    end_it ();
	case 's':
	case 'f':
	    if (nlines == 0) nlines++;
	    if (comchar == 'f')
		nlines *= dlines;
	    putchar ('\r');
	    erase (0);
	    printf ("\n");
	    if (clreol)
		cleareol ();
	    printf ("...skipping %d line", nlines);
	    if (nlines > 1)
		pr ("s\n");
	    else
		pr ("\n");

	    if (clreol)
		cleareol ();
	    pr ("\n");

	    while (nlines > 0) {
		while ((c = Getc (f)) != '\n')
		    if (c == EOF) {
			retval = 0;
			done++;
			goto endsw;
		    }
		    Currline++;
		    nlines--;
	    }
	    ret (dlines);
	case '\n':
	    if (nlines != 0)
		dlines = nlines;
	    else
		nlines = 1;
	    ret (nlines);
	case '\f':
	    if (!no_intty) {
		doclear ();
		Fseek (f, screen_start.chrctr);
		Currline = screen_start.line;
		ret (dlines);
	    }
	    else {
		writes (2, &bell, 1);
		break;
	    }
	case '\'':
	    if (!no_intty) {
		kill_line ();
		pr ("\n***Back***\n\n");
		Fseek (f, context.chrctr);
		Currline = context.line;
		ret (dlines);
	    }
	    else {
		writes (2, &bell, 1);
		break;
	    }
	case '=':
	    kill_line ();
	    promptlen = printd (Currline);
	    fflush (stdout);
	    break;
	case 'n':
	    lastp++;
	case '/':
	    if (nlines == 0) nlines++;
	    kill_line ();
	    pr ("/");
	    promptlen = 1;
	    fflush (stdout);
	    if (lastp) {
		writes (2,"\r", 1);
		search (NULL, f, nlines);	/* Use previous r.e. */
	    }
	    else {
		ttyin (cmdbuf, 78, '/');
		writes (2, "\r", 1);
		search (cmdbuf, f, nlines);
	    }
	    ret (dlines-1);
	case '!':
	    do_shell (filename);
	    break;
	case 'h':
	    if ((helpf = fopen (HELPFILE, "r")) == NULL)
		error ("Can't open help file");
	    if (noscroll) doclear ();
	    copy_file (helpf);
	    close (helpf);
	    prompt (filename);
	    break;
	case 'v':	/* This case should go right before default */
	    if (!no_intty) {
		kill_line ();
		cmdbuf[0] = '+';
		scanstr (Currline, &cmdbuf[1]);
		pr ("vi "); pr (cmdbuf); putchar (' '); pr (fnames[fnum]);
		execute (filename, VI, "vi", cmdbuf, fnames[fnum], 0);
		break;
	    }
	default:
	    writes (2, &bell, 1);
	    break;
	}
	if (done) break;
    }
    putchar ('\r');
endsw:
    inwait = 0;
    notell++;
    return (retval);
}

char ch;

/*
 * Execute a colon-prefixed command.
 * Returns <0 if not a command that should cause
 * more of the file to be printed.
 */

colon (filename, cmd, nlines)
char *filename;
int cmd;
int nlines;
{
	if (cmd == 0)
		ch = readch ();
	else
		ch = cmd;
	lastcolon = ch;
	switch (ch) {
	case 'f':
		kill_line ();
		if (!no_intty)
			promptlen = printf ("\"%s\" line %d", fnames[fnum], Currline);
		else
			promptlen = printf ("[Not a file] line %d", Currline);
		fflush (stdout);
		return (-1);
	case 'n':
		if (nlines == 0) {
			if (fnum >= nfiles - 1)
				end_it ();
			nlines++;
		}
		putchar ('\r');
		erase (0);
		skipf (nlines);
		return (0);
	case 'p':
		if (no_intty) {
			writes (2, &bell, 1);
			return (-1);
		}
		putchar ('\r');
		erase (0);
		if (nlines == 0)
			nlines++;
		skipf (-nlines);
		return (0);
	case '!':
		do_shell (filename);
		return (-1);
	case 'q':
	case 'Q':
		end_it ();
	default:
		writes (2, &bell, 1);
		return (-1);
	}
}

/*
** Read a decimal number from the terminal. Set cmd to the non-digit which
** terminates the number.
*/

number(cmd)
char *cmd;
{
	register int i;

	i = 0; ch = otty.c_cc[VKILL];
	for (;;) {
		ch = readch ();
		if (ch >= '0' && ch <= '9')
			i = i*10 + ch - '0';
		else if (ch == otty.c_cc[VKILL])
			i = 0;
		else {
			*cmd = ch;
			break;
		}
	}
	return (i);
}

do_shell (filename)
char *filename;
{
	char cmdbuf[80];

	kill_line ();
	pr ("!");
	fflush (stdout);
	promptlen = 1;
	if (lastp)
		pr (shell_line);
	else {
		ttyin (cmdbuf, 78, '!');
		if (expand (shell_line, cmdbuf)) {
			kill_line ();
			promptlen = printf ("!%s", shell_line);
		}
	}
	fflush (stdout);
	writes (2, "\n", 1);
	promptlen = 0;
	shellp = 1;
	execute (filename, shell, shell, "-c", shell_line, 0);
}

/*
** Search for nth ocurrence of regular expression contained in buf in the file
*/

search (buf, file, n)
char buf[];
FILE *file;
register int n;
{
    long startline = Ftell (file);
    register long line1 = startline;
    register long line2 = startline;
    register long line3 = startline;
    register int lncount;
    int saveln, rv, re_exec();
    char *s, *re_comp();

    context.line = saveln = Currline;
    context.chrctr = startline;
    lncount = 0;
    if ((s = re_comp (buf)) != 0)
	error (s);
    while (!feof (file)) {
	line3 = line2;
	line2 = line1;
	line1 = Ftell (file);
	rdline (file);
	lncount++;
	if ((rv = re_exec (Line)) == 1)
		if (--n == 0) {
		    if (lncount > 3 || (lncount > 1 && no_intty))
		    {
			pr ("\n");
			if (clreol)
			    cleareol ();
			pr("...skipping\n");
		    }
		    if (!no_intty) {
			Currline -= (lncount >= 3 ? 3 : lncount);
			Fseek (file, line3);
			if (noscroll)
			    if (clreol) {
				home ();
				cleareol ();
			    } 
			    else
				doclear ();
		    }
		    else {
			kill_line ();
			if (noscroll)
			    if (clreol) {
			        home (); 
			        cleareol ();
			    } 
			    else
				doclear ();
			pr (Line);
			putchar ('\n');
		    }
		    break;
		}
	else if (rv == -1)
	    error ("Regular expression botch");
    }
    if (feof (file)) {
	if (!no_intty) {
	    Currline = saveln;
	    Fseek (file, startline);
	}
	else {
	    pr ("\nPattern not found\n");
	    end_it ();
	}
	error ("Pattern not found");
    }
}

execute (filename, cmd, va_alist)
va_dcl
char *filename;
char *cmd;
{
	int id;

	fflush (stdout);
	reset_tty ();
	while ((id = fork ()) < 0)
	    sleep (5);
	if (id == 0) {
	    execv (cmd, &va_alist);
	    writes (2, "exec failed\n", 12);
	    exit (1);
	}
	signal (SIGINT, SIG_IGN);
	signal (SIGQUIT, SIG_IGN);
#ifdef SIGTSTP
	if (catch_susp)
	    signal(SIGTSTP, SIG_DFL);
#endif
	wait (0);
	signal (SIGINT, end_it);
	signal (SIGQUIT, onquit);
#ifdef SIGTSTP
	if (catch_susp)
	    signal(SIGTSTP, onsusp);
#endif
	set_tty ();
	pr ("------------------------\n");
	prompt (filename);
}
/*
** Skip n lines in the file f
*/

skiplns (n, f)
register int n;
register FILE *f;
{
    register char c;

    while (n > 0) {
	while ((c = Getc (f)) != '\n')
	    if (c == EOF)
		return;
	    n--;
	    Currline++;
    }
}

/*
** Skip nskip files in the file list (from the command line). Nskip may be
** negative.
*/

skipf (nskip)
register int nskip;
{
    if (nskip == 0) return;
    if (nskip > 0) {
	if (fnum + nskip > nfiles - 1)
	    nskip = nfiles - fnum - 1;
    }
    else if (within)
	++fnum;
    fnum += nskip;
    if (fnum < 0)
	fnum = 0;
    pr ("\n...Skipping ");
    pr ("\n");
    if (clreol)
	cleareol ();
    pr ("...Skipping ");
    pr (nskip > 0 ? "to file " : "back to file ");
    pr (fnames[fnum]);
    pr ("\n");
    if (clreol)
	cleareol ();
    pr ("\n");
    --fnum;
}

/*----------------------------- Terminal I/O -------------------------------*/

initterm ()
{
    char	buf[TBUFSIZ];
    char	clearbuf[100];
    char	*clearptr, *padstr;
    int		ldisc;

    setbuf(stdout, obuf);
    ioctl(1, TCGETA, &otty);
    ntty = otty;
    if (!(no_tty = isnotatty(1))) {
	if ((padstr = getenv("TERM"))==0 || tgetent(buf, padstr) <= 0) {
	    dumb++; ul_opt = 0;
	}
	else {
	    if (((Lpp = tgetnum("li")) < 0) || tgetflag("hc")) {
		hard++;	/* Hard copy terminal */
		Lpp = 24;
	    }
	    if (tailequ (fnames[0], "page") || !hard && tgetflag("ns"))
		noscroll++;
	    if ((Mcol = tgetnum("co")) < 0)
		Mcol = 80;
	    Wrap = tgetflag("am");
	    bad_so = tgetflag ("xs");
	    clearptr = clearbuf;
	    eraseln = tgetstr("ce",&clearptr);
	    Clear = tgetstr("cl", &clearptr);
	    Senter = tgetstr("so", &clearptr);
	    Sexit = tgetstr("se", &clearptr);

	    /*
	     *  Set up for underlining:  some terminals don't need it;
	     *  others have start/stop sequences, still others have an
	     *  underline char sequence which is assumed to move the
	     *  cursor forward one character.  If underline sequence
	     *  isn't available, settle for standout sequence.
	     */

	    if (tgetflag("ul") || tgetflag("os"))
		ul_opt = 0;
	    if ((chUL = tgetstr("uc", &clearptr)) == NULL )
		chUL = "";
	    if ((ULenter = tgetstr("us", &clearptr)) == NULL &&
		(!*chUL) && (ULenter = tgetstr("so", &clearptr)) == NULL)
		ULenter = "";
	    if ((ULexit = tgetstr("ue", &clearptr)) == NULL &&
		(!*chUL) && (ULexit = tgetstr("se", &clearptr)) == NULL)
		ULexit = "";
	    
	    if (padstr = tgetstr("pc", &clearptr))
		PC = *padstr;
	    Home = tgetstr("ho",&clearptr);
	    if (Home == 0 || *Home == '\0')
	    {
		if ((cursorm = tgetstr("cm", &clearptr)) != NULL) {
		    strcpy(cursorhome, tgoto(cursorm, 0, 0));
		    Home = cursorhome;
	       }
	    }
	    EodClr = tgetstr("cd", &clearptr);
	}
	if ((shell = getenv("SHELL")) == NULL)
	    shell = "/bin/sh";
    }
    no_intty = isnotatty(0);
    ioctl(2, TCGETA, &otty);
    ntty = otty;
    ospeed = otty.c_cflag&CBAUD;
    slow_tty = ospeed < B1200;
    hardtabs =  !(otty.c_oflag & TAB3);
    if (!no_tty) {
	ntty.c_iflag |= ISTRIP | ICRNL;
	ntty.c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHONL);
	ntty.c_lflag &= ~ICANON;
	ntty.c_lflag |= ISIG;
	ntty.c_cc[VMIN] = ntty.c_cc[VTIME] = 1;
    }
}

readch ()
{
	char ch;
	extern int errno;
	register fdstat;

	if (readfd<0) {
		fdstat=fcntl(2, F_GETFL);
		if (isatty(2) && (fdstat==O_RDONLY || fdstat==O_RDWR))
			readfd = 2;
		else {
			fdstat=fcntl(0, F_GETFL);
			if (isatty(0) && (fdstat==O_RDONLY || fdstat==O_RDWR))
			readfd = 0;
			else if ((readfd = open("/dev/tty",O_RDWR))<0) {
				fprintf(stderr,
				  	"\nmore: Bad file descriptor.\n");
					end_it();
			}
		}
	}
	if ((fdstat = read (readfd, &ch, 1)) < 0) {
		if (errno != EINTR) {
			fprintf(stderr,"\nmore: Bad read (readfd = %d).\n", readfd);
			end_it();
		}
		else
			ch = otty.c_cc[VKILL];
	}
	return (ch);
}

static char BS = '\b';
static char CARAT = '^';

ttyin (buf, nmax, pchar)
char buf[];
register int nmax;
char pchar;
{
    register char *sptr;
    register char ch;
    register int slash = 0;
    int	maxlen;
    char cbuf;

    sptr = buf;
    maxlen = 0;
    while (sptr - buf < nmax) {
	if (promptlen > maxlen) maxlen = promptlen;
	ch = readch ();
	if (ch == '\\') {
	    slash++;
	}
	else if ((ch == otty.c_cc[VERASE]) && !slash) {
	    if (sptr > buf) {
		--promptlen;
		writes (2, &BS, 1);
		--sptr;
		if ((*sptr < ' ' && *sptr != '\n') || *sptr == RUBOUT) {
		    --promptlen;
		    writes (2, &BS, 1);
		}
		continue;
	    }
	    else {
		if (!eraseln) promptlen = maxlen;
		longjmp (restore, 1);
	    }
	}
	else if ((ch == otty.c_cc[VKILL]) && !slash) {
	    if (hard) {
		show (ch);
		putchar ('\n');
		putchar (pchar);
	    }
	    else {
		putchar ('\r');
		putchar (pchar);
		if (eraseln)
		    erase (1);
		promptlen = 1;
	    }
	    sptr = buf;
	    fflush (stdout);
	    continue;
	}
	if (slash && (ch == otty.c_cc[VKILL] || ch == otty.c_cc[VERASE])) {
	    writes (2, &BS, 1);
	    --sptr;
	}
	if (ch != '\\')
	    slash = 0;
	*sptr++ = ch;
	if ((ch < ' ' && ch != '\n' && ch != ESC) || ch == RUBOUT) {
	    ch += ch == RUBOUT ? -0100 : 0100;
	    writes (2, &CARAT, 1);
	    promptlen++;
	}
	cbuf = ch;
	if (ch != '\n' && ch != ESC) {
	    writes (2, &cbuf, 1);
	    promptlen++;
	}
	else
	    break;
    }
    *--sptr = '\0';
    if (!eraseln) promptlen = maxlen;
    if (sptr - buf >= nmax - 1)
	error ("Line too long");
}

expand (outbuf, inbuf)
char *outbuf;
char *inbuf;
{
    register char *instr;
    register char *outstr;
    register char ch;
    char temp[200];
    int changed = 0;

    instr = inbuf;
    outstr = temp;
    while ((ch = *instr++) != '\0')
	switch (ch) {
	case '%':
	    if (!no_intty) {
		strcpy (outstr, fnames[fnum]);
		outstr += strlen (fnames[fnum]);
		changed++;
	    }
	    else
		*outstr++ = ch;
	    break;
	case '!':
	    if (!shellp)
		error ("No previous command to substitute for");
	    strcpy (outstr, shell_line);
	    outstr += strlen (shell_line);
	    changed++;
	    break;
	case '\\':
	    if (*instr == '%' || *instr == '!') {
		*outstr++ = *instr++;
		break;
	    }
	default:
	    *outstr++ = ch;
	}
    *outstr++ = '\0';
    strcpy (outbuf, temp);
    return (changed);
}

show (ch)
register char ch;
{
    char cbuf;

    if ((ch < ' ' && ch != '\n' && ch != ESC) || ch == RUBOUT) {
	ch += ch == RUBOUT ? -0100 : 0100;
	writes (2, &CARAT, 1);
	promptlen++;
    }
    cbuf = ch;
    writes (2, &cbuf, 1);
    promptlen++;
}

error (mess)
char *mess;
{
    if (clreol)
	cleareol ();
    else
	kill_line ();
    promptlen += strlen (mess);
    if (Senter && Sexit) {
	tputs (Senter, 1, putch);
	pr(mess);
	tputs (Sexit, 1, putch);
    }
    else
	pr (mess);
    fflush(stdout);
    errors++;
    longjmp (restore, 1);
}


set_tty ()  /* "cbreak" mode */
{
	ntty = otty;
	ntty.c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHONL);
	ntty.c_lflag &= ~ICANON;
	ntty.c_lflag |= ISIG;
	ntty.c_cflag &= ~(CSIZE|PARENB);
	ntty.c_cflag |= CS8;
	ntty.c_iflag &= (ICRNL|ISTRIP);
	ntty.c_cc[VMIN] = ntty.c_cc[VTIME] = 1;
	stty(2, &ntty);
}

reset_tty ()
{
    stty(2, &otty);
}

rdline (f)
register FILE *f;
{
    register char c;
    register char *p;

    p = Line;
    while ((c = Getc (f)) != '\n' && c != EOF && p - Line < LINSIZ - 1)
	*p++ = c;
    if (c == '\n')
	Currline++;
    *p = '\0';
}

/* Come here when we get a suspend signal from the terminal */

#ifdef SIGTSTP
onsusp ()
{
    reset_tty ();
    fflush (stdout);
    /* Send the TSTP signal to suspend our process group */
    kill (0, SIGTSTP);
    /* Pause for station break */

    /* We're back */
    signal (SIGTSTP, onsusp);
    set_tty ();
    if (inwait)
	    longjmp (restore);
}
#endif

/* @(#)regex.c	4.1 (Berkeley) 12/21/80 */
#

/*
 * routines to do regular expression matching
 *
 * Entry points:
 *
 *	re_comp(s)
 *		char *s;
 *	 ... returns 0 if the string s was compiled successfully,
 *		     a pointer to an error message otherwise.
 *	     If passed 0 or a null string returns without changing
 *           the currently compiled re (see note 11 below).
 *
 *	re_exec(s)
 *		char *s;
 *	 ... returns 1 if the string s matches the last compiled regular
 *		       expression, 
 *		     0 if the string s failed to match the last compiled
 *		       regular expression, and
 *		    -1 if the compiled regular expression was invalid 
 *		       (indicating an internal error).
 *
 * The strings passed to both re_comp and re_exec may have trailing or
 * embedded newline characters; they are terminated by nulls.
 *
 * The identity of the author of these routines is lost in antiquity;
 * this is essentially the same as the re code in the original V6 ed.
 *
 * The regular expressions recognized are described below. This description
 * is essentially the same as that for ed.
 *
 *	A regular expression specifies a set of strings of characters.
 *	A member of this set of strings is said to be matched by
 *	the regular expression.  In the following specification for
 *	regular expressions the word `character' means any character but NUL.
 *
 *	1.  Any character except a special character matches itself.
 *	    Special characters are the regular expression delimiter plus
 *	    \ [ . and sometimes ^ * $.
 *	2.  A . matches any character.
 *	3.  A \ followed by any character except a digit or ( )
 *	    matches that character.
 *	4.  A nonempty string s bracketed [s] (or [^s]) matches any
 *	    character in (or not in) s. In s, \ has no special meaning,
 *	    and ] may only appear as the first letter. A substring 
 *	    a-b, with a and b in ascending ASCII order, stands for
 *	    the inclusive range of ASCII characters.
 *	5.  A regular expression of form 1-4 followed by * matches a
 *	    sequence of 0 or more matches of the regular expression.
 *	6.  A regular expression, x, of form 1-8, bracketed \(x\)
 *	    matches what x matches.
 *	7.  A \ followed by a digit n matches a copy of the string that the
 *	    bracketed regular expression beginning with the nth \( matched.
 *	8.  A regular expression of form 1-8, x, followed by a regular
 *	    expression of form 1-7, y matches a match for x followed by
 *	    a match for y, with the x match being as long as possible
 *	    while still permitting a y match.
 *	9.  A regular expression of form 1-8 preceded by ^ (or followed
 *	    by $), is constrained to matches that begin at the left
 *	    (or end at the right) end of a line.
 *	10. A regular expression of form 1-9 picks out the longest among
 *	    the leftmost matches in a line.
 *	11. An empty regular expression stands for a copy of the last
 *	    regular expression encountered.
 */

/*
 * constants for re's
 */
#define	CBRA	1
#define	CCHR	2
#define	CDOT	4
#define	CCL	6
#define	NCCL	8
#define	CDOL	10
#define	CEOFN	11
#define	CKET	12
#define	CBACK	18

#define	CSTAR	01

#define	ESIZE	512
#define	NBRA	9

static	char	expbuf[ESIZE], *braslist[NBRA], *braelist[NBRA];
static	char	circf;

/*
 * compile the regular expression argument into a dfa
 */
char *
re_comp(sp)
	register char	*sp;
{
	register int	c;
	register char	*ep = expbuf;
	int	cclcnt, numbra = 0;
	char	*lastep = 0;
	char	bracket[NBRA];
	char	*bracketp = &bracket[0];
	static	char	*retoolong = "Regular expression too long";

#define	comerr(msg) {expbuf[0] = 0; numbra = 0; return(msg); }

	if (sp == 0 || *sp == '\0') {
		if (*ep == 0)
			return("No previous regular expression");
		return(0);
	}
	if (*sp == '^') {
		circf = 1;
		sp++;
	}
	else
		circf = 0;
	for (;;) {
		if (ep >= &expbuf[ESIZE])
			comerr(retoolong);
		if ((c = *sp++) == '\0') {
			if (bracketp != bracket)
				comerr("unmatched \\(");
			*ep++ = CEOFN;
			*ep++ = 0;
			return(0);
		}
		if (c != '*')
			lastep = ep;
		switch (c) {

		case '.':
			*ep++ = CDOT;
			continue;

		case '*':
			if (lastep == 0 || *lastep == CBRA || *lastep == CKET)
				goto defchar;
			*lastep |= CSTAR;
			continue;

		case '$':
			if (*sp != '\0')
				goto defchar;
			*ep++ = CDOL;
			continue;

		case '[':
			*ep++ = CCL;
			*ep++ = 0;
			cclcnt = 1;
			if ((c = *sp++) == '^') {
				c = *sp++;
				ep[-2] = NCCL;
			}
			do {
				if (c == '\0')
					comerr("missing ]");
				if (c == '-' && ep [-1] != 0) {
					if ((c = *sp++) == ']') {
						*ep++ = '-';
						cclcnt++;
						break;
					}
					while (ep[-1] < c) {
						*ep = ep[-1] + 1;
						ep++;
						cclcnt++;
						if (ep >= &expbuf[ESIZE])
							comerr(retoolong);
					}
				}
				*ep++ = c;
				cclcnt++;
				if (ep >= &expbuf[ESIZE])
					comerr(retoolong);
			} while ((c = *sp++) != ']');
			lastep[1] = cclcnt;
			continue;

		case '\\':
			if ((c = *sp++) == '(') {
				if (numbra >= NBRA)
					comerr("too many \\(\\) pairs");
				*bracketp++ = numbra;
				*ep++ = CBRA;
				*ep++ = numbra++;
				continue;
			}
			if (c == ')') {
				if (bracketp <= bracket)
					comerr("unmatched \\)");
				*ep++ = CKET;
				*ep++ = *--bracketp;
				continue;
			}
			if (c >= '1' && c < ('1' + NBRA)) {
				*ep++ = CBACK;
				*ep++ = c - '1';
				continue;
			}
			*ep++ = CCHR;
			*ep++ = c;
			continue;

		defchar:
		default:
			*ep++ = CCHR;
			*ep++ = c;
		}
	}
}

/* 
 * match the argument string against the compiled re
 */
int
re_exec(p1)
	register char	*p1;
{
	register char	*p2 = expbuf;
	register int	c;
	int	rv;

	for (c = 0; c < NBRA; c++) {
		braslist[c] = 0;
		braelist[c] = 0;
	}
	if (circf)
		return((advance(p1, p2)));
	/*
	 * fast check for first character
	 */
	if (*p2 == CCHR) {
		c = p2[1];
		do {
			if (*p1 != c)
				continue;
			if (rv = advance(p1, p2))
				return(rv);
		} while (*p1++);
		return(0);
	}
	/*
	 * regular algorithm
	 */
	do
		if (rv = advance(p1, p2))
			return(rv);
	while (*p1++);
	return(0);
}

/* 
 * try to match the next thing in the dfa
 */
static	int
advance(lp, ep)
	register char	*lp, *ep;
{
	register char	*curlp;
	int	ct, i;
	int	rv;

	for (;;)
		switch (*ep++) {

		case CCHR:
			if (*ep++ == *lp++)
				continue;
			return(0);

		case CDOT:
			if (*lp++)
				continue;
			return(0);

		case CDOL:
			if (*lp == '\0')
				continue;
			return(0);

		case CEOFN:
			return(1);

		case CCL:
			if (cclass(ep, *lp++, 1)) {
				ep += *ep;
				continue;
			}
			return(0);

		case NCCL:
			if (cclass(ep, *lp++, 0)) {
				ep += *ep;
				continue;
			}
			return(0);

		case CBRA:
			braslist[*ep++] = lp;
			continue;

		case CKET:
			braelist[*ep++] = lp;
			continue;

		case CBACK:
			if (braelist[i = *ep++] == 0)
				return(-1);
			if (backref(i, lp)) {
				lp += braelist[i] - braslist[i];
				continue;
			}
			return(0);

		case CBACK|CSTAR:
			if (braelist[i = *ep++] == 0)
				return(-1);
			curlp = lp;
			ct = braelist[i] - braslist[i];
			while (backref(i, lp))
				lp += ct;
			while (lp >= curlp) {
				if (rv = advance(lp, ep))
					return(rv);
				lp -= ct;
			}
			continue;

		case CDOT|CSTAR:
			curlp = lp;
			while (*lp++)
				;
			goto star;

		case CCHR|CSTAR:
			curlp = lp;
			while (*lp++ == *ep)
				;
			ep++;
			goto star;

		case CCL|CSTAR:
		case NCCL|CSTAR:
			curlp = lp;
			while (cclass(ep, *lp++, ep[-1] == (CCL|CSTAR)))
				;
			ep += *ep;
			goto star;

		star:
			do {
				lp--;
				if (rv = advance(lp, ep))
					return(rv);
			} while (lp > curlp);
			return(0);

		default:
			return(-1);
		}
}

backref(i, lp)
	register int	i;
	register char	*lp;
{
	register char	*bp;

	bp = braslist[i];
	while (*bp++ == *lp++)
		if (bp >= braelist[i])
			return(1);
	return(0);
}

int
cclass(set, c, af)
	register char	*set, c;
	int	af;
{
	register int	n;

	if (c == 0)
		return(0);
	n = *set++;
	while (--n)
		if (*set++ == c)
			return(af);
	return(! af);
}

isnotatty(fd)
{
	return (isatty(fd) ? 0 : 1);
}

writes(fd, ptr, ct)
char *ptr;
{
	register fdstat;

	if (writefd<0) {
		fdstat=fcntl(2, F_GETFL);
		if (fdstat==O_WRONLY || fdstat==O_RDWR)
			writefd = 2;
		else {
			fdstat=fcntl(1, F_GETFL);
			if (isatty(1) && (fdstat==O_WRONLY || fdstat==O_RDWR))
			writefd = 1;
			else if ((writefd = open("/dev/tty",O_RDWR))<0) {
				fprintf(stderr,
				  	"\nmore: Bad file descriptor.\n");
					end_it();
			}
		}
	}
	write(writefd, ptr, ct);
}
