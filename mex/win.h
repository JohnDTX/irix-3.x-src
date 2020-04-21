/*
 * Data structures specific to window manipulation
 *
 * $Source: /d2/3.7/src/mex/RCS/win.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:08:24 $
 */
#include "gl.h"
#include "grioctl.h"
#include "device.h"
#include "glipc.h"

extern struct portreq gl_initportreq;

/*
 * When a window is obscured, it is broken up into pieces, which
 * are described by the structure below
 */
struct	windowpiece {
	struct	windowpiece *w_xnext, *w_xprev;
	struct	windowpiece *w_ynext, *w_yprev;
#ifdef HORIZPIECES
	struct	windowpiece *w_znext, *w_zprev;
#endif
	short	w_xmin, w_xmax;
	short	w_ymin, w_ymax;
};

/*
 * Each window on the screen is managed by the following structure
 */
struct	wm_window {
	short	w_state;		/* state of window */
	short	w_xmin, w_xmax;		/* physical x screen position */
	short	w_ymin, w_ymax;		/* physical y screen position */
	short	w_ncols, w_nrows;	/* txport info */
	struct	wm_window *w_next;	/* links to next window in chain */
	struct	wm_window *w_prev;	/* links to prev window in chain */
	struct	windowpiece *w_xhead;	/* windowpiece list stuff */
	struct	windowpiece *w_xtail;
	struct	windowpiece *w_yhead;
	struct	windowpiece *w_ytail;
#ifdef HORIZPIECES
	struct	windowpiece *w_zhead;
	struct	windowpiece *w_ztail;
#endif
	int 	w_shadow;		/* checksum of hardware pieces */
	short	w_txport;		/* # of attached textport */
	short	w_inchan;		/* # of attached inputchan */
	long	w_ownerpid;		/* process id of owner */
	struct	portreq	w_pr;		/* sizing hints for window */
	short	knowsize;		/* know the size of the window */
	short	knowposition;		/* know the position of the window */
	char	w_title[81];		/* they made me do it */
	char	w_name[81];		/* the name passed to getport */
};

/* window w_state's */

#define	WS_HASKEYBOARD	0x0001		/* window has keyboard */
#define	WS_OPEN		0x0002		/* window is open */
#define	WS_VISIBLE	0x0004		/* window is visible */
#define	WS_REDISPLAY	0x0008		/* repaint text in window */
#define	WS_NEWPIECES	0x0010		/* piece list changed */
#define WS_ISDUMMY	0x0020		/* fake window for whole screen */
#define	WS_FULLSCREEN	0x0040		/* window can draw everywhere */
#define	WS_USEDPUPMODE	0x0080		/* has used the pop up planes */
#define	WS_CLOSED	0x1000		/* have recieved WMGFCLOSE */
#define	WS_REBORDER	0x2000		/* repaint border, menus in window */
#define	WS_GRAPHPORT	0x4000		/* window contains graphics */
#define	WS_TEXTPORT	0x8000		/* window is a textport */

/* buffer used for reading/writing pieces to kernel */

#define MAXPIECES 500
struct	pbuf {
	struct grpiecehdr ph;
	struct grpiece piece[MAXPIECES];
};
