static	char	*Initscr_c	= "@(#)initscr.c	1.10";
# include	"curses.ext"
# include	<signal.h>
#ifdef	V7
# include	<sys/types.h>
# include	<sys/file.h>
#else
# include	<fcntl.h>
#endif	V7

extern char	*getenv();

#ifdef	iris
int	_linblit[49];		/* pixel row of text line */
int	_linfont[48];		/* font of text line */
int	_linvs[48];		/* flag: variable spaced font */
int	_nowfont;		/* current font - used by refresh */
#endif

/*
 *	This routine initializes the current and standard screen.
 *
 * 3/5/81 (Berkeley) @(#)initscr.c	1.2
 */
WINDOW *
initscr() {

	reg char	*sp;
	int		tstp();
	int		i;

# ifdef DEBUG
	fprintf(outf, "INITSCR()\n");
# endif
	if (!My_term && isatty(2)) {
		_tty_ch = 2;
		gettmode();
		if ((sp = getenv("TERM")) == NULL)
			sp = Def_term;
	} else
		sp = Def_term;
# ifdef DEBUG
	fprintf(outf, "INITSCR: term = %s\n", sp);
# endif
	setterm(sp);
#ifdef	iris
	if (CT == 2) {
		if (isatty(1) &&
		  (!strcmp(ttyname(1),"/dev/console")
		  || !strcmp(ttyname(1),"/dev/syscon"))
		  && close(open(MUTEX,O_EXCL|O_CREAT,0)) < 0) {
		    fprintf(stderr,"IRIS in use\n");
		    fflush(stderr);
		    exit(2);
		}
		_iris = 1;	/* do not separate this statement and ginit() */
		ginit();
		cursoff();
		CE = 0;			/* temporary */
					/* yes, we are a Color Terminal */
					/* here is our list of colors */
		if (!CZ)
			CZ="*Bblack,red,green,yellow,blue,magenta,cyan,*Fwhite";
		if (!_linblit[0])
			for (i=0; i<49; i++)
				_linblit[i] = (48-i)*CHEIGHT;
	}
# endif
	_puts(TI);
	_puts(VS);
# ifdef SIGTSTP
	signal(SIGTSTP, tstp);
# endif
	if (curscr != NULL) {
# ifdef DEBUG
		fprintf(outf, "INITSCR: curscr = 0%o\n", curscr);
# endif
		delwin(curscr);
	}
# ifdef DEBUG
	fprintf(outf, "LINES = %d, COLS = %d\n", LINES, COLS);
# endif
	if ((curscr = newwin(LINES, COLS, 0, 0)) == ERR)
		return ERR;
	curscr->_clear = TRUE;
# ifdef	COLOR
	curscr->_attrib = 0;
	if (CT)	/* Color Terminal: set colors */
		acolor(curscr,Dforeground,Dbackground);
# endif
	if (stdscr != NULL) {
# ifdef DEBUG
		fprintf(outf, "INITSCR: stdscr = 0%o\n", stdscr);
# endif
		delwin(stdscr);
	}
	stdscr = newwin(LINES, COLS, 0, 0);
	return stdscr;
}
