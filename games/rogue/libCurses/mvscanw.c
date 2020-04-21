static	char	*Mvscanw_c	= "@(#)mvscanw.c	1.2";
# include	"curses.ext"

/*
 * implement the mvscanw commands.  Due to the variable number of
 * arguments, they cannot be macros.  Another sigh....
 *
 * 1/26/81 (Berkeley) @(#)mvscanw.c	1.1
 */

mvscanw(y, x, fmt, args)
reg int		y, x;
char		*fmt;
int		args; {

# ifdef	IRISCURSES
	return Move(y, x) == OK ? _sscanw(stdscr, fmt, &args) : ERR;
# else
	return move(y, x) == OK ? _sscanw(stdscr, fmt, &args) : ERR;
# endif
}

mvwscanw(win, y, x, fmt, args)
reg WINDOW	*win;
reg int		y, x;
char		*fmt;
int		args; {

	return wmove(win, y, x) == OK ? _sscanw(win, fmt, &args) : ERR;
}
