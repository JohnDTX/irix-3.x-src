static	char	*Endwin_c	= "@(#)endwin.c	1.3";
/*
 * Clean things up before exiting
 *
 * 1/26/81 (Berkeley) %W
 */

# include	"curses.ext"

endwin()
{
	char	buf[512];
	char	*p;

	resetty();
	_puts(VE);
	_puts(TE);
	if (curscr) {
		if (curscr->_flags & _STANDOUT) {
			_puts(SE);
			curscr->_flags &= ~_STANDOUT;
		}
		if (curscr->_charset != CHRNORM) {
			sprintf(buf,"%s",curscr->_charset == CHRALT ? AE : GE);
			for (p=buf; *p; )
				_putchar(*p++);
			curscr->_charset = CHRNORM;
		}
		_endwin = TRUE;
	}
}
