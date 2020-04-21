static	char	*Mvprintw_c	= "@(#)mvprintw.c	1.3";
# include	"curses.ext"

/*
 * implement the mvprintw commands.  Due to the variable number of
 * arguments, they cannot be macros.  Sigh....
 *
 * 1/26/81 (Berkeley) @(#)mvprintw.c	1.1
 */

mvprintw(y, x, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
reg int		y, x;
char		*fmt;
int		*a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9, *a10;
{

#ifdef	IRISCURSES
	if (Move(y, x) != OK)
		return ERR;
#else	IRISCURSES
	if (move(y, x) != OK)
		return ERR;
#endif	IRISCURSES
	return wprintw(stdscr, fmt, a1, a2, a3, a4, a5, a6,
	  a7, a8, a9, a10);
}

mvwprintw(win, y, x, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
reg WINDOW	*win;
reg int		y, x;
char		*fmt;
int		*a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9, *a10;
{

	return wmove(win, y, x) == OK ? wprintw(win, fmt, a1, a2, a3, a4, a5,
	  a6, a7, a8, a9, a10) : ERR;
}
