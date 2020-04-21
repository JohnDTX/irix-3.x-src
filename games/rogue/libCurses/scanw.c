static	char	*Scanw_c	= "@(#)scanw.c	1.2";
/*
 * scanw and friends
 *
 * 1/26/81 (Berkeley) @(#)scanw.c	1.1
 */

# include	"curses.ext"

/*
 *	This routine implements a scanf on the standard screen.
 */
scanw(fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
char	*fmt;
int	*a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9, *a10;
{

	return wscanw(stdscr, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}

/*
 *	This routine actually executes the scanf from the window.
 *	This is really a modified version of "sscanf".
 */

wscanw(win, fmt, a1, a2, a3a4, a5, a6, a7, a8, a9, a10)
WINDOW	*win;
char	*fmt;
int	*a1, *a2, *a3a4, *a5, *a6, *a7, *a8, *a9, *a10;
{

	char	buf[100];

	if (wgetstr(win, buf) == ERR)
		return ERR;
	return sscanf(buf, fmt, a1, a2, a3a4, a5, a6, a7, a8, a9, a10);
}
