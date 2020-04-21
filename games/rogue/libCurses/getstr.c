static	char	*Getstr_c	= "@(#)getstr.c	1.2";
# include	"curses.ext"

/*
 *	This routine gets a string starting at (_cury,_curx)
 *
 * 4/29/81 (Berkeley) @(#)getstr.c	1.3
 */
wgetstr(win,str)
reg WINDOW	*win; 
reg char	*str; {

	while ((*str = wgetch(win)) != ERR && *str != '\n' && *str != '\r')
		str++;
	if (*str == ERR) {
		*str = '\0';
		return ERR;
	}
	*str = '\0';
	return OK;
}
