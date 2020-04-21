static	char	*Printw_c	= "@(#)printw.c	1.3";
/*
 * printw and friends
 *
 * 1/26/81 (Berkeley) @(#)printw.c	1.1
 */

# include	"curses.ext"

/*
 * #ifdef	V7
 * int _print();
 * extern FILE *_pfile;
 * #endif
 */

/*
 *	This routine implements a printf on the standard screen.
 */
printw(fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
char	*fmt;
int	*a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9, *a10;
{

	return wprintw(stdscr, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}

/*
 *	This routine actually executes the printf and adds it to the window
 *	This is really a modified version of "sprintf".  
 *
 */

wprintw(win, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
WINDOW *win;
char *fmt;
int *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9, *a10;
{
	char buf[512];

	sprintf(buf, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	return waddstr(win, buf);
}
