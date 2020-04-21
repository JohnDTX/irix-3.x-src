static	char	*Refresh_c	= "@(#)refresh.c	1.13";
/*
 * make the current screen look like "win" over the area coverd by
 * win.
 *
 * 3/5/81 (Berkeley) @(#)refresh.c	1.4
 */

# include	"curses.ext"

# ifdef DEBUG
# define	STATIC
# else
# define	STATIC	static
# endif

STATIC short	ly, lx;

STATIC bool	curwin;

WINDOW	*_win = NULL;

int	_Debug;		/* debugging level */


wrefresh(win)
reg WINDOW	*win;
{
	reg short	wy;
	reg int		retval;

	/*
	 * make sure were in visual state
	 */
# ifdef	DEBUG
	if (!outf)
		outf = fopen("/dev/ttyd1","w");
# endif
	if (_endwin) {
		_puts(VS);
		_puts(TI);
		_endwin = FALSE;
	}

	/*
	 * initialize loop parameters
	 */

	ly = curscr->_cury;
	lx = curscr->_curx;
	wy = 0;
	_win = win;
	curwin = (win == curscr);

	if (win->_clear || curscr->_clear || curwin) {
		if ((win->_flags & _FULLWIN) || curscr->_clear) {
# ifdef iris
			if (_iris) {
				color(Dbackground);
				glclear();
				color(Dforeground);
			} else
# endif
# ifdef	COLOR
				if (CT) {
					_puts(CL);	/* just to home */
					pcolor(dfcolor(mfcolor(win->_attrib)),
					  dbcolor(mbcolor(win->_attrib)));
				} else
# endif
					_puts(CL);
			ly = lx = curscr->_curx = curscr->_cury = 0;
			curscr->_clear = FALSE;
			if (!curwin)
				werase(curscr);
			touchwin(win);
		}
		win->_clear = FALSE;
	}
	if (!CA) {
		if (win->_curx != 0)
			putchar('\n');
		if (!curwin)
			werase(curscr);
	}
# ifdef DEBUG
	fprintf(outf, "REFRESH(%0.2o): curwin = %d\n", win, curwin);
	fprintf(outf, "REFRESH:\n\tfirstch\tlastch\n");
# endif
	for (wy = 0; wy < win->_maxy; wy++) {
# ifdef DEBUG2
		fprintf(outf, "%d\t%d\t%d\n", wy, win->_firstch[wy], win->_lastch[wy]);
# endif
		if (win->_firstch[wy] != _NOCHANGE)
			if (makech(win, wy) == ERR)
				return ERR;
			else
				win->_firstch[wy] = _NOCHANGE;
	}
	if (win->_leave) {
		curscr->_cury = ly;
		curscr->_curx = lx;
		ly -= win->_begy;
		lx -= win->_begx;
		if (ly >= 0 && ly < win->_maxy && lx >= 0 && lx < win->_maxx) {
			win->_cury = ly;
			win->_curx = lx;
		} else
			win->_cury = win->_curx = 0;
	} else {
		mvcur(ly, lx, win->_cury + win->_begy, win->_curx + win->_begx);
		curscr->_cury = win->_cury + win->_begy;
		curscr->_curx = win->_curx + win->_begx;
	}
	retval = OK;
ret:
	_win = NULL;
	fflush(stdout);
	return retval;
}

/*
 * make a change on the screen
 */
makech(win, wy)
reg WINDOW	*win;
short		wy;
{
	reg char	*nsp, *csp, *ce;
	reg short	wx, lch, y;
	reg int		nlsp, clsp;	/* last space in lines		*/
# ifdef	COLOR
	PIXEL		Attrib;		/* current color mode		*/
	PIXEL		anAttrib;	/* current color mode		*/
	PIXEL		*Nsp, *Csp;
# endif
# ifdef iris
	int		xx, yy;
	int		c1;		/* color			*/
	static char	putit[2];
	int		x1, x2;
	char		mybuf[133];
	char		*p;
# endif

# ifdef	COLOR
	Attrib = win->_attrib;
# endif
	wx = win->_firstch[wy];
	y = wy + win->_begy;
	lch = win->_lastch[wy];
# ifdef	iris
	if (_iris) {
		Icoord	h1, h2, w1, w2;

		if (_nowfont != _linfont[y])
			font(_nowfont = _linfont[y]);
		h2 = _linblit[y]-1;		/* TOP side		      */
		h1 = _linblit[y+1];		/* BOTTOM side		      */
		yy = y;				/* yy chr pos on screen       */
		x1 = win->_firstch[wy];
						/* clear to background color  */
		if (_linvs[y]) {
						/*
						 * first char determines back-
						 * ground color for entire line
						 */
			color(dbcolor(win->_Y[wy][0])+coloroff);
			w1 = 0;			/* LEFT side		      */
			w2 =  XMAXSCREEN;	/* RIGHT side		      */
			rectfi(w1, h1-8, w2, h2); /* 8 is for descenders    */
			win->_firstch[wy] = 0;
			win->_lastch[wy] = win->_maxx-1;
		} else
			while (x1 <= win->_lastch[wy]) {
						/* til end of same back color */
				c1 = dbcolor(win->_Y[wy][x1]);
				for (xx=x1;
				  xx<=win->_lastch[wy]
				    && c1==dbcolor(win->_Y[wy][xx]);
				  xx++)
					x2 = xx;
				color(c1+coloroff);	/* background color   */
						/* clear area to back color   */
				rectfi((Icoord) x1*CWIDTH /* -1 */,
				  (Icoord) h1-CDESCEND,
				  (Icoord) x2*CWIDTH+(CWIDTH-1),
				  (Icoord) h2-CDESCEND);
				x1 = x2 + 1;
			}
		x1 = win->_firstch[wy];
						/* cursor address	      */
		cmov2i((x1+win->_begx)*CWIDTH+(_linvs[y] != 0),_linblit[yy+1]);
						/* strip off trailing spaces  */
		xx = win->_lastch[wy];
		while (win->_y[wy][xx] == ' ' && xx)
			xx--;
		win->_lastch[wy] = xx;
		while (x1 <= win->_lastch[wy]) {/* put chars in foregr color  */
						/* til end of same fore color */
			c1 = dfcolor(win->_Y[wy][x1]);
			p = mybuf;		/* make string to print	      */
			for (xx=x1;
			  xx<=win->_lastch[wy] && c1==dfcolor(win->_Y[wy][xx]);
			  xx++) {
				x2 = xx;
				*p++ = win->_y[wy][xx];
			}
			*p = 0;
			color(c1+coloroff);	/* set foreground color       */
						/* clear area to back color   */
			charstr(mybuf);
			x1 = x2 + 1;
		}
		return OK;
	}
# endif
	if (curwin)
		csp = " ";
	else
		csp = &curscr->_y[wy + win->_begy][wx + win->_begx];
	nsp = &win->_y[wy][wx];
#ifdef	COLOR
	if (curwin)
		Csp = &Attrib;
	else
		Csp = &curscr->_Y[wy + win->_begy][wx + win->_begx];
	Csp = &curscr->_Y[wy + win->_begy][wx + win->_begx];
	Nsp = &win->_Y[wy][wx];
#endif	COLOR
	if (CE && !curwin) {
		for (ce = &win->_y[wy][win->_maxx - 1]; *ce == ' '; ce--)
			if (ce <= win->_y[wy])
				break;
		nlsp = ce - win->_y[wy];
	}
	if (curwin)
		ce = NULL;
	else
		ce = CE;
	while (wx <= lch) {
		if (*nsp != *csp
#ifdef	COLOR
		  || *Nsp != *Csp /***** y *****/
#endif	COLOR
		) {
			mvcur(ly, lx, y, wx + win->_begx);
# ifdef DEBUG
			fprintf(outf, "MAKECH: 1: wx = %d, lx = %d\n", wx, lx);
# endif	
			ly = y;
			lx = wx + win->_begx;
			while ((*nsp != *csp
#ifdef	COLOR
			  || *Nsp != *Csp /***** y *****/
#endif	COLOR
			  ) && wx <= lch
			) {
				if (ce != NULL && wx >= nlsp && *nsp == ' '
#ifdef	COLORx
					/* we could probably deal with it */
				  && !CT
#endif	COLOR
				) {
					/*
					 * check for clear to end-of-line
					 */
					ce = &curscr->_y[ly][COLS - 1];
					while (*ce == ' ')
						if (ce-- <= csp)
							break;
					clsp = ce - curscr->_y[ly] - win->_begx;
# ifdef DEBUG
					fprintf(outf, "MAKECH: clsp = %d, nlsp = %d\n", clsp, nlsp);
# endif
					if (clsp - nlsp >= strlen(CE)
					  && clsp < win->_maxx
#ifdef	COLOR
					  && !CT	/* 10/06/84 22:00 */
#endif	COLOR
					) {
# ifdef DEBUG
						fprintf(outf, "MAKECH: using CE\n");
# endif
						_puts(CE);
						lx = wx + win->_begx;
						while (wx++ <= clsp)
							*csp++ = ' ';
						goto ret;
					}
					ce = NULL;
				}
				/*
				 * enter/exit standout mode as appropriate
				 */
				if (SO && (*nsp&_STANDOUT) != (curscr->_flags&_STANDOUT)) {
					if (*nsp & _STANDOUT) {
						_puts(SO);
						curscr->_flags |= _STANDOUT;
					}
					else {
						_puts(SE);
						curscr->_flags &= ~_STANDOUT;
					}
				}
				/*
				 * change color mode as appropriate
				 */
# ifdef	COLOR
				anAttrib = win->_Y[wy][nsp-win->_y[wy]];
# endif
# ifdef	COLOR
if (_Debug > 1) {
  fprintf(stderr,"nsp-win->_y[wy:%d]=%u\n",wy,nsp-win->_y[wy]);
  fprintf(stderr,"anAttrib=0x%x Attrib=0x%x win->_attrib=0x%x\n",
    anAttrib,Attrib,win->_attrib);
  fprintf(stderr,"anAttrib(char):fore=%d back=%d\n\n",
    dfcolor(mfcolor(anAttrib)),
    dbcolor(mbcolor(anAttrib)));
}
# endif
# ifdef	COLOR
				if (CT && (anAttrib&_COLORM)
				  != (Attrib&_COLORM) || _Debug) {
					pcolor(dfcolor(mfcolor(anAttrib)),
					  dbcolor(mbcolor(anAttrib)));
					Attrib &= ~_COLORM;
					Attrib |= anAttrib&_COLORM;
				}
# endif
				wx++;
				if (wx >= win->_maxx && wy == win->_maxy - 1)
					if (win->_scroll) {
					    if ((win->_flags&(_ENDLINE|_STANDOUT)) == (_ENDLINE|_STANDOUT))
						if (!MS) {
						    _puts(SE);
						    win->_flags &= ~_STANDOUT;
						}
					    if (!curwin)
						Eputchar((*csp = *nsp) & 0177);
					    else
						Eputchar(*nsp & 0177);
					    scroll(win);
					    if (win->_flags&_FULLWIN && !curwin)
						scroll(curscr);
					    ly = win->_begy+win->_cury;
					    lx = win->_begx+win->_curx;
					    return OK;
					}
					else if (win->_flags&_SCROLLWIN) {
					    lx = --wx;
					    return ERR;
					}
				if (!curwin) {
					Eputchar((*csp++ = *nsp) & 0177);
#ifdef	COLOR
					Csp++;
#endif	COLOR
				} else
					Eputchar(*nsp & 0177);
				if (UC && (*nsp & _STANDOUT)) {
					putchar('\b');
					_puts(UC);
				}
				nsp++;
#ifdef	COLOR
				Nsp++;
#endif	COLOR
			}
# ifdef DEBUG
			fprintf(outf, "MAKECH: 2: wx = %d, lx = %d\n", wx, lx);
# endif	
			if (lx == wx + win->_begx)	/* if no change */
				break;
			lx = wx + win->_begx;
		}
		else if (wx < lch)
			while (*nsp == *csp
#ifdef	COLORx
			  && *Nsp == *Csp
#endif	COLOR
			) {
				nsp++;
#ifdef	COLOR
				Nsp++;
#endif	COLOR
				if (!curwin) {
					csp++;
#ifdef	COLOR
					Csp++;
#endif	COLOR
				}
				wx++;
			}
		else
			break;
# ifdef DEBUG
		fprintf(outf, "MAKECH: 3: wx = %d, lx = %d\n", wx, lx);
# endif	
	}
ret:
	if ((win->_flags & _STANDOUT) && !MS) {
		_puts(SE);
		win->_flags &= ~_STANDOUT;
	}
# ifdef	COLOR
	if (CT && (win->_attrib&_COLORM) != (Attrib&_COLORM))
		pcolor(dfcolor(mfcolor(win->_attrib)),
		  dbcolor(mbcolor(win->_attrib)));
# endif
	return OK;
}

/*
 * set COLOR Attribute for Window to (Foreground,Background)
 */
acolor(w,f,b)
reg WINDOW *w;
int f;
int b;
{

#ifdef	COLOR

	w->_attrib &= ~_COLORM;
	if (Iforeground < 0 || Iforeground != f)
		w->_attrib |= ifcolor(f);
	else
		w->_attrib |= ifcolor(b);
	if (Ibackground < 0 || Ibackground != b)
		w->_attrib |= ibcolor(b);
	else
		w->_attrib |= ibcolor(f);

#endif	COLOR

}

/*
 * switch colors (on color terminal)
 */
# ifdef	COLOR
char	*
scolor(fore,back)
int	fore;
int	back;
{
	char	*tgoto();

	return tgoto(CP,back,fore);
}
# endif

/*
 * handle font changes (altername & graphic) on vt100 and equiv.
 */
Eputchar(chr)
register char	chr;
{
	register char	c;
	register char	*p;
	char buf[512];

	chr &= 0177;		/* strip parity bit (probably unneeded) */
	buf[0] = 0;
	if (c=_mapchr[chr]) {	/* Yes, assignment! */
		if (c&0200) {	/* alternate char set */
			if (curscr && curscr->_charset != CHRALT) {
				sprintf(buf,"%s%s",curscr->_charset == CHRGRAF ?
				  GE : "",AS);
				curscr->_charset = CHRALT;
			}
		} else {	/* graphics char set */
			if (curscr && curscr->_charset != CHRGRAF) {
				sprintf(buf,"%s%s",curscr->_charset == CHRALT ?
				  AE : "",GS);
				curscr->_charset = CHRGRAF;
			}
		}
		chr = c;
	} else {
		if (curscr && curscr->_charset != CHRNORM) {
			sprintf(buf,"%s",curscr->_charset == CHRALT ? AE : GE);
			curscr->_charset = CHRNORM;
		}
	}
	for (p=buf; *p; )
		_putchar(*p++);
	_putchar(chr & 0177);
}
