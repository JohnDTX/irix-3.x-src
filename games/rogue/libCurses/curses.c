static	char	*Curses_c	= "@(#)curses.c	1.10";
/*
 * Define global variables
 *
 * 3/5/81 (Berkeley) @(#)curses.c	1.2
 *
 * This code is public domain except for Silicon Graphics specific code.
 * Silicon Graphics specific code (between #ifdef iris and either #else or
 * #endif or the #else portion of an #ifndef iris) is
 * Copyright 1984 Silicon Graphics, Inc.
 */
# include	"curses.h"

bool	_echoit		= TRUE,	/* set if stty indicates ECHO		*/
	_rawmode	= FALSE,/* set if stty indicates RAW mode	*/
	My_term		= FALSE,/* set if user specifies terminal type	*/
	_endwin		= FALSE;/* set if endwin has been called	*/

char	ttytype[10],		/* long name of tty			*/
	*Def_term	= "unknown";	/* default terminal type	*/

int	_tty_ch		= 1,	/* file channel which is a tty		*/
	LINES,			/* number of lines allowed on screen	*/
	COLS;			/* number of columns allowed on screen	*/

#ifdef	BSD
int	dumb42;
#endif	BSD

int	Dforeground;		/* foreground color			*/
int	Dbackground;		/* background color			*/
int	Iforeground = -1;	/* illegal foreground color e.g. black	*/
int	Ibackground = -1;	/* illegal background color e.g. white	*/
int	Ncolors;
int	writethru;		/* true to write even if "up to date"	*/
char	*Colors[_MAXCOLORS];
char	_mapchr[128];		/* map to alt. & graphics char sets	*/

WINDOW	*stdscr		= NULL,
	*curscr		= NULL;

# ifdef	iris
int	_mouse;			/* using mouse (it is queued)		*/
short	_mousex;		/* current mouse X coord		*/
short	_mousey;		/* current mouse Y coord		*/
Colorindex coloroff = COLNORM;	/* color offsets for special situations	*/
# endif

# ifdef DEBUG
FILE	*outf;			/* debug output file			*/
# endif

SGTTY	_tty;			/* tty modes				*/
# ifdef	V7
short	_res_flg;		/* sgtty flags for reseting later	*/
# else
SGTTY	_tty_res;		/* original tty modes			*/
# endif	V7

bool	AM, BS, CA, DA, DB, EO, GT, HZ, IN, MI, MS, NC, OS, UL, XN,
	NONL, UPPERCASE, normtty, _pfast;
				/*
				 * Color extension:type of color term.
				 * Non-zero implies a color terminal.
				 * The number indicates general capabilities.
				 * 1 -> can control fore/background colors of
				 *      each char on screen independently.
				 * No other numbers are currently defined.
				 * The "iris" terminal is special cased in
				 *      order that we don't limit its usefulness
				 */
int	CT;
#ifdef	iris
int	_iris;
#endif

char	*AL, *BC, *BT, *CD, *CE, *CL, *CM, *DC, *DL, *DM, *DO, *ED,
	*EI, *HO, *IC, *IM, *IP, *LL, *MA, *ND, *SE, *SF, *SO, *SR,
	*TA, *TE, *TI, *UC, *UE, *UP, *US, *VB, *VE, *VS, PC,
	*CZ, *CP,		/* color extensions			*/
	*AS, *AE, *GS, *GE,	/* vt100 alt. & char sets extensions	*/
	*Al, *Nb, *Nm,		/* Cobol extensions			*/
	*CF, *CN;		/* cursor on/off			*/

int	SG;			/* # of columns for standout		*/
