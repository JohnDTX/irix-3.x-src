static	char	*Getch_c	= "@(#)getch.c	1.8";
# include	"curses.ext"
# ifdef	iris
# include <device.h>
# endif
# include <errno.h>

/*
 *	This routine reads in a character from the window.
 *
 * 1/26/81 (Berkeley) @(#)getch.c	1.1
 */
wgetch(win)
reg WINDOW	*win; {

	reg bool	weset = FALSE;
	reg char	inp;

	if (!win->_scroll && (win->_flags&_FULLWIN)
	    && win->_curx == win->_maxx && win->_cury == win->_maxy)
		return ERR;
# ifdef DEBUG
	fprintf(outf, "WGETCH: _echoit = %c, _rawmode = %c\n", _echoit ? 'T' : 'F', _rawmode ? 'T' : 'F');
# endif
	if (_echoit && !_rawmode) {
		raw();
		weset++;
	}
	inp = getchar();
# ifdef DEBUG
	fprintf(outf,"WGETCH got '%s'\n",unctrl(inp));
# endif
	if (_echoit) {
		mvwaddch(curscr, win->_cury, win->_curx, inp);
		waddch(win, inp);
	}
	if (weset)
		noraw();
	return inp;
}

# ifdef	iris
usemouse()
{
	_mouse = 1;		/* flag that we are using mouse stuff */
	_mousex = 32000;
	_mousey = 32000;
	setvaluator((Device) MOUSEX,_mousex,(short) -1000,(short) 1000);
	setvaluator((Device) MOUSEY,_mousey,(short) -1000,(short) 1000);
#ifdef	gl1
	qkeyboard();
	qbutton(SETUPKEY);
	qbutton(LEFTMOUSE);
	qbutton(MIDDLEMOUSE);
	qbutton(RIGHTMOUSE);
/*
 *	qvaluator(MOUSEX);
 *	qvaluator(MOUSEY);
 */
#endif	gl1
#ifdef	gl2
	qdevice(KEYBD);
	qdevice(SETUPKEY);
	qdevice(LEFTMOUSE);
	qdevice(MIDDLEMOUSE);
	qdevice(RIGHTMOUSE);
/*
 *	qvaluator(MOUSEX);
 *	qvaluator(MOUSEY);
 */
#endif	gl2
/*
 *	noise((Device) MOUSEX,(short) 50);
 *	noise((Device) MOUSEY,(short) 50);
 */
}
# endif

_Getchar()
{
	static	char	c;
# ifdef	iris
	static	Device	dev;
	static	short	data;
	static	short	res;
# endif
	extern	int	errno;

# ifdef	iris
	if (_mouse) {
		for (;;) {
#ifndef	STAND
			errno = 0;
#endif	STAND
#ifdef	MOUSE
			if ((data=(getvaluator(MOUSEX)/100)) != _mousex) {
				dev = MOUSEX;
				if (data < -500 || data > 500) {
					setvaluator((Device) MOUSEX,_mousex,
					  (short) -1000,(short) 1000);
					_mousex = 32000;
				}
			} else
			  if ((data=(getvaluator(MOUSEY)/100)) != _mousey) {
				dev = MOUSEY;
				if (data < -500 || data > 500) {
					setvaluator((Device) MOUSEY,_mousey,
					  (short) -1000,(short) 1000);
					_mousey = 32000;
				}
			} else if (qtest())
				dev = qread(&data);
#ifndef	STAND
			else if (errno == EINTR)
				return -1;
#endif	STAND
			else
				continue;
#else	MOUSE
			dev = qread(&data);
#endif	MOUSE
#ifndef	STAND
			if (errno == EINTR)
				return -1;
#endif	STAND
			switch (dev) {
			  case KEYBD:
				return data;
			  case SETUPKEY:
				if (data)
					return 'o';
				break;
			  case MOUSEX:
				res = data - _mousex;
				_mousex = data;
				return res>0 ? 'l' : 'h';
			  case MOUSEY:
				res = data - _mousey;
				_mousey = data;
				return res>0 ? 'k' : 'j';
			  case LEFTMOUSE:
				if (data)
					return 'r';
				break;
			  case MIDDLEMOUSE:
				if (data)
					return 'q';
				break;
			  case RIGHTMOUSE:
				if (data)
					return 'i';
				else
					return ' ';
			}
		}
	} else
# endif
	{
	    while (read(0, &c, 1) < 0)
		continue;
	    return c;
	}
}
