# include	<signal.h>

# ifdef SIGTSTP

# include	"curses.ext"

/*
 * handle stop and start signals
 *
 * 3/5/81 (Berkeley) @(#)tstp.c	1.1
 */

tstp() {

	SGTTY	tty;
# ifdef DEBUG
	if (outf)
		fflush(outf);
# endif
	tty = _tty;
	mvcur(0, COLS - 1, LINES - 1, 0);
	endwin();
	fflush(stdout);
	kill(0, SIGTSTP);
	signal(SIGTSTP, tstp);
	_tty = tty;
	ioctl(_tty_ch,TCSETA,&_tty);
	wrefresh(curscr);
}

#else

TSTP() {				/* so that there is a symbol table */
}

# endif
