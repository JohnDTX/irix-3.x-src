static	char	*Clrtoeol_c	= "@(#)clrtoeol.c	1.3";
# include	"curses.ext"

/*
 *	This routine clears up to the end of line
 *
 * 3/5/81 (Berkeley) @(#)clrtoeol.c	1.2
 */
wclrtoeol(win)
reg WINDOW	*win; {

	reg char	*sp, *end;
	reg int		y, x;
	reg char	*maxx;
	reg int		minx;
#ifdef	COLOR
	PIXEL		*Sp;
	PIXEL		attrib;
#endif

	y = win->_cury;
	x = win->_curx;
	end = &win->_y[y][win->_maxx];
	minx = _NOCHANGE;
	maxx = &win->_y[y][x];
#ifdef	COLOR
	attrib = win->_attrib;
	for (sp = maxx, Sp = &win->_Y[y][x]; sp < end; sp++, Sp++)
		if (*sp != ' ' || *Sp != attrib)
#else
	for (sp = maxx; sp < end; sp++)
		if (*sp != ' ')
#endif
		{
			maxx = sp;
			if (minx == _NOCHANGE)
				minx = sp - win->_y[y];
			*sp = ' ';
#ifdef	COLOR
			*Sp = attrib;
#endif	COLOR
		}
	/*
	 * update firstch and lastch for the line
	 */
# ifdef DEBUG
	fprintf(outf, "CLRTOEOL: minx = %d, maxx = %d, firstch = %d, lastch = %d\n", minx, maxx - win->_y[y], win->_firstch[y], win->_lastch[y]);
# endif
	if (minx != _NOCHANGE) {
		if (win->_firstch[y] > minx || win->_firstch[y] == _NOCHANGE)
			win->_firstch[y] = minx;
		if (win->_lastch[y] < maxx - win->_y[y])
			win->_lastch[y] = maxx - win->_y[y];
	}
}
