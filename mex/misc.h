/*
 * Miscellaneous variables
 *
 * $Source: /d2/3.7/src/mex/RCS/misc.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:08:16 $
 */

/* procedures */
char	*getmemory();
struct	wm_window *wm_newwin();
struct	wm_window *wm_infront();
struct	wm_window *wm_inback();
struct	wm_window *wn_inback();
struct	wm_window *wn_newwin();
struct	wm_window *wn_findgf();
struct	wm_window *movekbd();
struct	wm_window *windowat();

/* variables */
struct	wm_window *win_first, *win_last, *win_freelist, *win_console;
struct	wm_window *backgroundw, *inputwindow, *menuwindow;
struct	windowpiece *win_piecefreelist;
struct	wm_window *dragging, *destroying, *creating;
short	mousex, mousey;
short	dragx1, dragx2, dragy1, dragy2;
short	menucolor, menubcolor;
short 	gl_charwidth, gl_charheight;
unsigned short	cursormask, cursorcolor, drawmask, menumask;
unsigned char	menur, menug, menub;
unsigned char	menubr, menubg, menubb;
unsigned char	cursorr, cursorg, cursorb;
short titleheight;
short titlefontno;
char  *titlefont;
int window_menu, wm_menu, confirm_menu;

/* etc. */
#define BORDERWIDTH	1
#define TITLEHEIGHT	(titleheight)
#define ENTRYHI 	(gl_charheight+2)
#define TXBORDER	2
#define DROP		1

struct colorarray {
	short	boutercolor;
	short	binnercolor;
	short	toutercolor;
	short	tinnercolor;
};

struct colorarray stdcolors, hilightcolors;

