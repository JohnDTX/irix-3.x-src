/*	"@(#)curses.h	1.12"	*/
/* char _cursesh_id[] = "@(#)curses.h:1.3"; */
/* 5/15/81 (Berkeley) @(#)curses.h	1.8 */
# ifndef WINDOW

# include	<stdio.h>

#ifdef	BSD41
#undef	BSD			/* in case it is already defined */
#define	BSD
#endif

#ifdef	BSD42
#undef	BSD			/* in case it is already defined */
#define	BSD
#endif

#ifdef	BSD
#undef	V7			/* in case it is already defined */
#define	V7
#endif

#ifdef	V7
# include	<sgtty.h>
#else
# include	<termio.h>
# include	<sys/ioctl.h>
#endif

#ifdef	iris
# include	<gl.h>		/* include graphics library */
# define	IRISCURSES	/* have Curses avoid conflicting names */
# undef		putchar		/* putchar was defined by stdio.h */
# undef		TRUE		/* TRUE was defined by gl.h */
# undef		FALSE		/* FALSE was defined by gl.h */
# define	putchar	putiris
# define	fflush	irisfflush
# define	exit	irisexit
				/*
				 * Qchar() returns true if input queue
				 * has data, used for busy wait!
				 */
# define	Qchar	qtest
# define	MUTEX	"/tmp/IRIS.lock"
# define	CHEIGHT	16	/* height of a char      in pixels */
# define	CWIDTH	 9	/* width  of a char      in pixels */
# define	CDESCEND 3	/* depth  of a descender in pixels */
# define	XMARGIN	10
# define	YMARGIN	10
#endif	iris
				/*
				 * Some brain-damaged systems don't have
				 * a proper tolower() or toupper() function.
				 */
#undef		tolower
#undef		toupper
#define		tolower(x)	(x>='A' && x<='Z' ? x-'A'+'a':x)
#define		toupper(x)	(x>='a' && x<='z' ? x-'a'+'A':x)

# define	bool	char
# define	reg	register

# define	FALSE	(0)
# define	TRUE	(!FALSE)
# define	ERR	(0)
# define	OK	(1)

# define	_SUBWIN		01
# define	_ENDLINE	02
# define	_FULLWIN	04
# define	_SCROLLWIN	010
# define	_FLUSH		020
# define	_STANDOUT	0200
# define	_NOCHANGE	(-1)
# define	_MAXCOLORS	8

# define	_puts(s)	tputs(s, 0, _putchar);

/* @@@@@@@@@@@@@@@@@@ Color Extensions @@@@@@@@@@@@@@@@@@@@@@@@ */

#define	Max(a,b)	(a>b?a:b)
#define	Min(a,b)	(a<b?a:b)

#define	_LOWINTENS	0x40 /* 0x0001 */
#define	_BLINK		0x80 /* 0x0002 */

#define	_FOREW	3			/* foreground attrib width in bits    */
#define	_BACKW	3			/* background attrib width in bits    */
#define	_FORES	0 /* 2 */		/* foreground attrib shift in bits    */
#define	_BACKS	(_FORES+_FOREW)		/* background attrib shift in bits    */
#define	_FOREM	(((1<<_FOREW)-1)<<_FORES)/* foreground attrib mask	      */
#define	_BACKM	(((1<<_BACKW)-1)<<_BACKS)/* background attrib mask	      */
#define	_COLORM	(_FOREM|_BACKM) 	/* color      attrib mask	      */
#define	_COLORS	Min(_FORES,_BACKS)	/* color      attrib shift in bits    */
#define	_COLORW	(_FOREW+_BACKW)		/* color      attrib width in bits    */
	/*
	 * Mask out all but foreground/background COLOR
	 */
#define	mfcolor(f)	((f)&_FOREM)
#define	mbcolor(b)	((b)&_BACKM)
	/*
	 * Index foreground/background COLOR with foreground/background
	 * color (convert color number into mask for setting/testing
	 * win->_attrib
	 */
#define	ifcolor(f)	((f)<<_FORES)
#define	ibcolor(b)	((b)<<_BACKS)
	/*
	 * Deindex foreground/background COLOR with foreground/background
	 * color (convert color mask into number)
	 */
#define	dfcolor(f)	(mfcolor(f)>>_FORES)
#define	dbcolor(b)	(mbcolor(b)>>_BACKS)
#define	Black	0	/* temporary use */
#define	Red	1
#define	Green	2
#define	Yellow	3
#define	Blue	4
#define	Magenta	5
#define	Cyan	6
#define	White	7
#ifdef	iris
#define	COLNORM	0		/* normal colors			*/
#define	COLSHIM	8		/* shimmering blue			*/
#define	COLBLACK 16		/* all black				*/
#endif	iris

	/*
	 * put (to actual CRT) the Foreground/Background color sequence
	 */
#define	pcolor(f,b)	_puts(scolor(f,b))
#define	CHRNORM	0		/* normal character set: must be 0	*/
#define	CHRALT	1		/* alternate character set		*/
#define	CHRGRAF	2		/* graphics character set		*/
extern	char	*scolor();
extern	int	_Debug;		/* debugging level */
extern	int	writethru;	/* true to turn off optimization	*/
#ifdef	iris
extern	Colorindex coloroff;	/* color offset for color maps		*/
#endif

typedef	unsigned char	PIXEL;	/* for attribute elements */

#ifdef	V7
typedef	struct sgttyb	SGTTY;
#endif

#ifndef	V7
typedef struct termio	SGTTY;
typedef	unsigned	UNSIGN;
#endif

/*
 * Capabilities from termcap
 */

extern bool     AM, BS, CA, DA, DB, EO, GT, HZ, IN, MI, MS, NC, OS, UL,
		XN;
extern char     *AL, *BC, *BT, *CD, *CE, *CL, *CM, *DC, *DL, *DM, *DO,
		*ED, *EI, *HO, *IC, *IM, *IP, *LL, *MA, *ND, *SE, *SF,
		*SO, *SR, *TA, *TE, *TI, *UC, *UE, *UP, *US, *VB, *VE,
		*VS, PC,
		*CZ, *CP,	/* color extensions			*/
		*AS, *AE, *GS, *GE,/* vt100 alt. char sets extensions	*/
		*Al,		/* cobol: AL: ALternate (low intensity)	*/
		*Nb,		/* cobol: NB: Normal Blink		*/
		*Nm,		/* cobol: NM: NorMal			*/
		*CF, *CN;	/* cursor on/off			*/

extern int	SG;		/* # of columns for standout		*/

/*
 * From the tty modes...
 */

extern bool	NONL, UPPERCASE, normtty, _pfast;

struct _win_st {
	short	_cury, _curx;
	short	_maxy, _maxx;
	short	_begy, _begx;
	short	_flags;
	bool	_clear;
	bool	_leave;
	bool	_scroll;
	char	**_y;
	short	*_firstch;
	short	*_lastch;
#ifdef	COLOR
	PIXEL	_attrib;	/* current attributes */
	PIXEL	**_Y;		/* attribute array */
#endif
	short	_charset;	/* 0->normal, 1->alternate, 2->graphics */
};

# define	WINDOW	struct _win_st

extern bool	My_term, _echoit, _rawmode, _endwin;

extern char	*Def_term, ttytype[];
extern char	_mapchr[];	/* map chars to alt. or graphics char sets */

#ifdef	iris
extern int	_iris;				/* using iris */
extern int	_mouse;				/* using mouse (it is queied) */
extern short	_mousex;			/* current mouse X coord */
extern short	_mousey;			/* current mouse Y coord */
extern int	_linblit[];			/* screen address of line */
extern int	_linfont[];			/* font of line */
extern int	_linvs[];			/* flag: variable spaced font */
extern int	_nowfont;			/* current font for refresh */
#endif

#ifdef	BSD
extern	int	dumb42;
#endif	BSD

extern int 	LINES, COLS, _tty_ch;
extern int	CT;				/* Color Terminal type */
extern int	Dforeground, Dbackground;	/* default colors */
extern int	Iforeground, Ibackground;	/* illegal colors */
extern int	Ncolors;			/* number of colors */
extern char	*Colors[_MAXCOLORS];		/* names of colors */
#ifdef	V7
extern short	_res_flg;
#else	V7
extern int 	_res_iflag, _res_oflag, _res_cflag, _res_lflag;
extern SGTTY	_tty_res;
#endif	V7
extern SGTTY	_tty;

extern WINDOW	*stdscr, *curscr;

/*
 *	Define VOID to stop lint from generating "null effect"
 * comments.
 */
# ifdef lint
int	__void__;
# define	VOID(x)	(__void__ = (int) (x))
# else
# define	VOID(x)	x
# endif

/*
 * psuedo functions for standard screen
 */
# define	addch(ch)	VOID(waddch(stdscr, ch))
# define	getch()		VOID(wgetch(stdscr))
# define	addstr(str)	VOID(waddstr(stdscr, str))
# define	getstr(str)	VOID(wgetstr(stdscr, str))
#ifdef	IRISCURSES
# define	Move(y, x)	VOID(wmove(stdscr, y, x))
# define	Clear()		VOID(wclear(stdscr))
#else
# define	move(y, x)	VOID(wmove(stdscr, y, x))
# define	clear()		VOID(wclear(stdscr))
#endif
# define	erase()		VOID(werase(stdscr))
# define	clrtobot()	VOID(wclrtobot(stdscr))
# define	clrtoeol()	VOID(wclrtoeol(stdscr))
# define	insertln()	VOID(winsertln(stdscr))
# define	deleteln()	VOID(wdeleteln(stdscr))
# define	refresh()	VOID(wrefresh(stdscr))
# define	inch()		VOID(winch(stdscr))
# define	insch(c)	VOID(winsch(stdscr,c))
# define	delch()		VOID(wdelch(stdscr))
# define	standout()	VOID(wstandout(stdscr))
# define	standend()	VOID(wstandend(stdscr))

/*
 * mv functions
 */
#define	mvwaddch(win,y,x,ch)	VOID(wmove(win,y,x)==ERR?ERR:waddch(win,ch))
#define	mvwgetch(win,y,x)	VOID(wmove(win,y,x)==ERR?ERR:wgetch(win))
#define	mvwaddstr(win,y,x,str)	VOID(wmove(win,y,x)==ERR?ERR:waddstr(win,str))
#define	mvwgetstr(win,y,x)	VOID(wmove(win,y,x)==ERR?ERR:wgetstr(win))
#define	mvwinch(win,y,x)	VOID(wmove(win,y,x) == ERR ? ERR : winch(win))
#define	mvwdelch(win,y,x)	VOID(wmove(win,y,x) == ERR ? ERR : wdelch(win))
#define	mvwinsch(win,y,x,c)	VOID(wmove(win,y,x) == ERR ? ERR:winsch(win,c))
#define	mvaddch(y,x,ch)		mvwaddch(stdscr,y,x,ch)
#define	mvgetch(y,x)		mvwgetch(stdscr,y,x)
#define	mvaddstr(y,x,str)	mvwaddstr(stdscr,y,x,str)
#define	mvgetstr(y,x)		mvwgetstr(stdscr,y,x)
#define	mvinch(y,x)		mvwinch(stdscr,y,x)
#define	mvdelch(y,x)		mvwdelch(stdscr,y,x)
#define	mvinsch(y,x,c)		mvwinsch(stdscr,y,x,c)

/*
 * psuedo functions
 */

#define	clearok(win,bf)	 (win->_clear = bf)
#define	leaveok(win,bf)	 (win->_leave = bf)
#define	scrollok(win,bf) (win->_scroll = bf)
#define flushok(win,bf)	 (bf ? (win->_flags |= _FLUSH):(win->_flags &= ~_FLUSH))
#define	getyx(win,y,x)	 y = win->_cury, x = win->_curx
/*
 * Changed winch to mask out high parity bit which is used for attributes
 * Bob Toxen 03/20/83
 *
 * #define winch(win)	 (win->_y[win->_cury][win->_curx])
 */
#define	winch(win)	 (win->_y[win->_cury][win->_curx]&0177)

WINDOW	*initscr(), *newwin(), *subwin();
char	*longname(), *getcap();
char	_putchar();

#endif	WINDOW
