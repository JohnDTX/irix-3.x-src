/*
 * printw and friends
 *
 * 1/26/81 (Berkeley) @(#)printw.c	1.1
 */

# include	"curses.ext"

/*
 *	This routine implements a printf on the standard screen.
 */
printw(fmt, args)
char	*fmt;
int	args; {

	return _sprintw(stdscr, fmt, &args);
}

/*
 *	This routine implements a printf on the given window.
 */
wprintw(win, fmt, args)
WINDOW	*win;
char	*fmt;
int	args; {

	return _sprintw(win, fmt, &args);
}
/*
 *	This routine actually executes the printf and adds it to the window
 *
 *	This is really a modified version of "sprintf".  As such,
 * it assumes that sprintf interfaces with the other printf functions
 * in a certain way.  If this is not how your system works, you
 * will have to modify this routine to use the interface that your
 * "sprintf" uses.
 *	Version modified for 68-System 3; using "vsprintf" as model 
 * instead -- C. Lai 8-18-82
 */

#include <varargs.h>
#include <values.h>

int
_sprintw(string, format, va_alist)
char *string, *format;
va_dcl
{
	register int count;
	FILE siop;
	va_list ap;
	register FILE *sp;
	char buf[2048];

	sp = &siop;
	sp->_cnt = MAXINT;
	sp->_base = sp->_ptr = (unsigned char *)buf;
	sp->_flag = _IOWRT;
	sp->_file = _NFILE;
	count = _doprnt(format, va_alist, sp);
	*sp->_ptr = '\0';	/* plant terminating null character */
	waddstr(string, buf);
	return(count);
}
