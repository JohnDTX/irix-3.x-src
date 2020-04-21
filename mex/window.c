/*
 * Window module:
 *	- this code manages the movement and management of rectangular
 *	  regions of the screen, windows.  No use is made of these windows
 *	  in this module, but it provides the primitives need for a window
 *	  manager to do part of its job.
 *
 * Written by: Kipp Hickman & Rocky Rhodes
 *
 * Exports:
 * wm_newwin:
 *	- create a new de-rezzed window, returning its window pointer
 * wm_init:
 *	- init the window manager data structures
 * wm_pop:
 *	- pop this window to the front of all the other windows
 * wm_push:
 *	- push this window in back of all other windows
 * wm_derez:
 *	- push this window logically in back of the display screen, as
 *	  well as the other windows.  Makes the window invisible.
 * wm_resize:
 *	- change the dimensions of a window.  Window must be de-rezzed
 *	  before being resized.
 * wm_move:
 *	- change the location of a window.  Window must be de-rezzed
 *	  before being moved.
 * wm_infront:
 *	- return the window pointer of the window in front of this window
 * wm_inback:
 *	- return the window pointer of the window in back of this window
 * wm_attachtx:
 *	- attach a textport to this window, returning the textport number
 *	  allocated
 * wm_detachtx:
 *	- detach textport from this window
 * wm_detachgf:
 *	- detach graphport from this window
 *
 * wm_freepiecelist:
 *	- free a list of pieces
 * wm_listsame:
 *	- compare a list of pieces
 * wm_copylist:
 *	- copy a piecelist
 *
 */
#include "win.h"
#include <stdio.h>
#include <gltypes.h>
#include "misc.h"
#include "window.h"

struct	wm_window *wm_allocwin();
struct	windowpiece *wm_allocpiece();

/*
 * wm_newwin:
 *	- create a new window, returning its window pointer
 */
struct wm_window *
wm_newwin()
{
	register struct wm_window *w;

	w = wm_allocwin();
    /* setup window parameters */
	w->w_state = WS_OPEN | WS_TEXTPORT | WS_VISIBLE;
	w->w_ncols = MAXCOLS;
	w->w_nrows = MAXROWS;
#ifdef FIXED_TXTPORT
	w->w_xmin = mousex;
	w->w_ymin = mousey;
#else
	w->w_xmin = 0;
	w->w_ymin = 0;
#endif
	w->w_xmax = w->w_xmin + XLEN(MAXCOLS) + 2*TXBORDER;
	w->w_ymax = w->w_ymin + YLEN(MAXROWS) + 2*TXBORDER;
	w->w_xhead = w->w_xtail = (struct windowpiece *)0;
	w->w_yhead = w->w_ytail = (struct windowpiece *)0;
#ifdef HORIZPIECES
	w->w_zhead = w->w_ztail = (struct windowpiece *)0;
#endif
	w->w_shadow = 0;
	w->w_txport = w->w_inchan = w->w_pr.gfnum = -1;
	w->w_title[0] = 0;
	w->w_ownerpid = -1;

    /* first put window at tail of list */
	win_last->w_next = w;
	w->w_prev = win_last;
	win_last = w;
	w->w_next = (struct wm_window *)0;
	return(w);
}

/*
 * wm_init:
 *	- init the window manager data structures
 */
wm_init()
{
	register struct wm_window *w;
	register struct windowpiece *wp;

	gl_charwidth = strwidth("a");
	gl_charheight = getheight();

    /* create dummy window covering entire screen */
	w = wm_allocwin();
	backgroundw = w;
	w->w_state = WS_ISDUMMY;
	w->w_txport = w->w_inchan = w->w_pr.gfnum = -1;
	win_first = win_last = w;

    /* this window has one piece covering the whole screen */
	wp = wm_allocpiece();
	w->w_xhead = w->w_xtail = wp; 
	w->w_yhead = w->w_ytail = wp; 
#ifdef HORIZPIECES
	w->w_zhead = w->w_ztail = wp; 
#endif
	w->w_shadow = 0;
	wp->w_xnext = wp->w_xprev = 0;
	wp->w_ynext = wp->w_yprev = 0;
#ifdef HORIZPIECES
	wp->w_znext = wp->w_zprev = 0;
#endif
	wp->w_xmin = wp->w_ymin = w->w_xmin = w->w_ymin = 0;
	wp->w_xmax = w->w_xmax = XMAXSCREEN;
	wp->w_ymax = w->w_ymax = YMAXSCREEN;
}

/*
 * wm_newpiece:
 *	- allocate a new piece, preset to a given screen position
 */
struct windowpiece *
wm_newpiece(w, xmin, ymin, xmax, ymax)
struct wm_window *w;
short xmin, ymin, xmax, ymax;
{
	register struct windowpiece *newp;

	newp = wm_allocpiece();
	newp->w_xmin = xmin;
	newp->w_ymin = ymin;
	newp->w_xmax = xmax;
	newp->w_ymax = ymax;
	addpiece(newp, w);
	return newp;
}

/*
 * wm_promote(w1, w2):
 *	- w1 and w2 are windows, with w1 currently lying "behind" w2
 *	- the goal of this procedure is to alter w1's priority so that
 *	  it will lie "in front of" w2.
 *	- to achieve this goal, w2 and all the windows below w2, but
 *	  above w1 are examined, and any piece of any of these windows
 *	  that intersects w1's window is "given" to w1
 *	- then if w1 picks up any new pieces, a mask is returned with w1's
 *	  id bit turned on, so that the caller will know to redraw w1
 */
wm_promote(w1, w2)
register struct wm_window *w1, *w2;
{
	register struct windowpiece *wp, *temppiece;
	register short temp;
	struct wm_window *w2save = w2;
	register short affected = 0, affectedthisone;

	if ((w1 == win_first) || (w1 == w2) || !w1 || !w2)
	    return;
	for(; w2 != w1; w2 = w2->w_next) {
	    if(!w2)
		return;

	/* If w2 and w1 don't intersect, contine */
	    if ((w2->w_xmin > w1->w_xmax) ||
		(w2->w_xmax < w1->w_xmin) ||
		(w2->w_ymin > w1->w_ymax) ||
		(w2->w_ymax < w1->w_ymin))
  		    continue;

	    affectedthisone = 0;
	    for (wp = w2->w_yhead; wp; ) {

	/* If wp and w1 don't intersect, wp = wp-w_next.  */
		if ((wp->w_xmin > w1->w_xmax) ||
		    (wp->w_xmax < w1->w_xmin) ||
		    (wp->w_ymin > w1->w_ymax) ||
		    (wp->w_ymax < w1->w_ymin))
			wp = wp->w_ynext;

	/* else split wp by one of w1's edges, if possible.  */
		else if (wp->w_ymin < w1->w_ymin) { /* w1's bottom edge */
			temp = wp->w_ymax;
			wp->w_ymax = w1->w_ymin - 1;
			wm_newpiece(w2, wp->w_xmin, w1->w_ymin,
							wp->w_xmax, temp);
			wp = wp->w_ynext;
		} else if (wp->w_ymax > w1->w_ymax) { /* w1's top edge */
			temp = wp->w_ymax;
			wp->w_ymax = w1->w_ymax;
			wm_newpiece(w2, wp->w_xmin, w1->w_ymax + 1,
							wp->w_xmax, temp);
		} else if (wp->w_xmin < w1->w_xmin) { /* w1's left edge */
			temp = wp->w_xmax;
			wp->w_xmax = w1->w_xmin - 1;
			wm_newpiece(w2, w1->w_xmin, wp->w_ymin,
							temp, wp->w_ymax);
			wp = wp->w_ynext;
		} else if (wp->w_xmax > w1->w_xmax) { /* w1's right edge */
			temp = wp->w_xmax;
			wp->w_xmax = w1->w_xmax;
			wm_newpiece(w2, w1->w_xmax + 1, wp->w_ymin,
							temp, wp->w_ymax);
		} else {
		    /*
		     * Else wp must fall entirely into w1's window.
		     * Take wp from w2 and give it to w1.
		     */
			affected = 1;
			affectedthisone = 1;
			temppiece = wp->w_ynext;
			removepiece(wp, w2);
			addpiece(wp, w1);
			wp = temppiece;
		}
	    }
	    if (affectedthisone)
		wm_coalesce(w2);
	}
	w1->w_prev->w_next = w1->w_next;
	if (w1->w_next)
	    w1->w_next->w_prev = w1->w_prev;
	else
	    win_last = w1->w_prev;
	w2 = w2save;
	w1->w_next = w2;
	w1->w_prev = w2->w_prev;
	if (w2->w_prev)
	    w2->w_prev->w_next = w1;
	else
	    win_first = w1;
	w2->w_prev = w1;

	if (affected) {
	    wm_coalesce(w1);
	    w1->w_state |= WS_REDISPLAY;
	}
}

/*
 * wm_coalesce:
 *	- Minimize the number of pieces in the window by making 3 passes 
 *	  through the window, once in increasing x, then increasing y, and
 *	  finally decreasing y. This procedure will result in a window
 *	  with the minimum number of rectangles, and edges biased
 *	  to being horizontal.
 *
 */
wm_coalesce(w)
register struct wm_window *w;
{
	register struct windowpiece *wp, *wpgone;
	register short savex, savey;

	if(!w)
	    return;
	if (!w->w_yhead)
	    return;

    /* do X pass */	
	wp = w->w_xhead;
	while (wp->w_xnext) {
	    if ((wp->w_xmin == wp->w_xnext->w_xmin) &&
			    (wp->w_ymax == wp->w_xnext->w_ymin - 1)) {
		if (wp->w_xmax == wp->w_xnext->w_xmax) {
		    wpgone = wp->w_xnext;
		    removepiece(wp, w);
		    wp->w_ymax = wp->w_xnext->w_ymax;
		    addpiece(wp, w);
		    removepiece(wpgone, w);
		    wm_freepiece(wpgone);
		} else if (wp->w_xmax < wp->w_xnext->w_xmax) {
		    wpgone = wp->w_xnext;
		    removepiece(wp, w);
		    wp->w_ymax = wpgone->w_ymax;
		    addpiece(wp, w);
		    removepiece(wpgone, w);
		    wpgone->w_xmin = wp->w_xmax + 1;
		    addpiece(wpgone, w);
		} else {
		    wpgone = wp->w_xnext;
		    savex = wp->w_xmax;
		    savey = wp->w_ymax;
		    removepiece(wp, w);
		    wp->w_ymax = wpgone->w_ymax;
		    wp->w_xmax = wpgone->w_xmax;
		    addpiece(wp, w);
		    removepiece(wpgone, w);
		    wpgone->w_ymin = wp->w_ymin;
		    wpgone->w_ymax = savey;
		    wpgone->w_xmin = wp->w_xmax + 1;
		    wpgone->w_xmax = savex;
		    addpiece(wpgone, w);
		}
	    } else
		wp = wp->w_xnext;
	}

    /* do Y pass */	
	wp = w->w_yhead;
	while (wp->w_ynext) {
	    if ((wp->w_ymin == wp->w_ynext->w_ymin) &&
		(wp->w_xmax == wp->w_ynext->w_xmin - 1)) {
		    if (wp->w_ymax == wp->w_ynext->w_ymax) {
			wpgone = wp->w_ynext;
			removepiece(wp, w);
			wp->w_xmax = wp->w_ynext->w_xmax;
			addpiece(wp, w);
			removepiece(wpgone, w);
			wm_freepiece(wpgone);
		    } else if (wp->w_ymax < wp->w_ynext->w_ymax) {
			wpgone = wp->w_ynext;
			removepiece(wp, w);
			wp->w_xmax = wpgone->w_xmax;
			addpiece(wp, w);
			removepiece(wpgone, w);
			wpgone->w_ymin = wp->w_ymax + 1;
			addpiece(wpgone, w);
		    } else {
			wpgone = wp->w_ynext;
			savey = wp->w_ymax;
			savex = wp->w_xmax;
			removepiece(wp, w);
			wp->w_xmax = wpgone->w_xmax;
			wp->w_ymax = wpgone->w_ymax;
			addpiece(wp, w);
			removepiece(wpgone, w);
			wpgone->w_xmin = wp->w_xmin;
			wpgone->w_xmax = savex;
			wpgone->w_ymin = wp->w_ymax + 1;
			wpgone->w_ymax = savey;
			addpiece(wpgone, w);
		    }
		} else
		    wp = wp->w_ynext;
	}

#ifdef HORIZPIECES
    /* do Z pass */	
	wp = w->w_zhead;
	while (wp->w_znext) {
	    if ((wp->w_ymax == wp->w_znext->w_ymax) &&
		(wp->w_xmax == wp->w_znext->w_xmin - 1)) {
		    if (wp->w_ymin == wp->w_znext->w_ymin) {
			wpgone = wp->w_znext;
			wp->w_xmax = wp->w_znext->w_xmax;
			removepiece(wpgone, w);
			wm_freepiece(wpgone);
		    } else if (wp->w_ymin > wp->w_znext->w_ymin) {
			wpgone = wp->w_znext;
			removepiece(wp, w);
			wp->w_xmax = wpgone->w_xmax;
			addpiece(wp, w);
			removepiece(wpgone, w);
			wpgone->w_ymax = wp->w_ymin - 1;
			addpiece(wpgone, w);
		    } else {
			wpgone = wp->w_znext;
			savey = wp->w_ymin;
			savex = wp->w_xmax;
			removepiece(wp, w);
			wp->w_xmax = wpgone->w_xmax;
			wp->w_ymin = wpgone->w_ymin;
			addpiece(wp, w);
			removepiece(wpgone, w);
			wpgone->w_xmin = wp->w_xmin;
			wpgone->w_xmax = savex;
			wpgone->w_ymax = wp->w_ymin - 1;
			wpgone->w_ymin = savey;
			addpiece(wpgone, w);
		    }
	    } else
		wp = wp->w_znext;
	}
#endif
}

/*
 * removepiece:
 *	- remove piece "wp" from "w"'s piece lists
 */
removepiece(wp, w)
register struct windowpiece *wp;
register struct wm_window *w;
{
    /* remove from X list */
	if (wp->w_xprev)
	    wp->w_xprev->w_xnext = wp->w_xnext;
	else
	    w->w_xhead = wp->w_xnext;
	if (wp->w_xnext)
	    wp->w_xnext->w_xprev = wp->w_xprev;
	else
	    w->w_xtail = wp->w_xprev;

    /* remove from Y list */
	if (wp->w_yprev)
	    wp->w_yprev->w_ynext = wp->w_ynext;
	else
	    w->w_yhead = wp->w_ynext;
	if (wp->w_ynext)
	    wp->w_ynext->w_yprev = wp->w_yprev;
	else
	    w->w_ytail = wp->w_yprev;

#ifdef HORIZPIECES
    /* remove from Z list */
	if (wp->w_zprev)
	    wp->w_zprev->w_znext = wp->w_znext;
	else
	    w->w_zhead = wp->w_znext;
	if (wp->w_znext)
	    wp->w_znext->w_zprev = wp->w_zprev;
	else
	    w->w_ztail = wp->w_zprev;
#endif
	w->w_state |= WS_NEWPIECES;
}

/*
 * addpiece:
 *	- add "wp" to "w"'s window piece lists
 *	- each piece is sorted into the x and y piece lists
 */
addpiece(wp, w)
register struct windowpiece *wp;
register struct wm_window *w;
{
	register struct windowpiece *p;

    /* add to X list */
	p = w->w_xhead;
	while (p && (p->w_xmin < wp->w_xmin))
	    p = p->w_xnext;
	while (p && (p->w_xmin == wp->w_xmin) && (p->w_ymin < wp->w_ymin))
	    p = p->w_xnext;
	wp->w_xnext = p;
	if (p) {
	    wp->w_xprev = p->w_xprev;
	    if (p->w_xprev)
		p->w_xprev->w_xnext = wp;
	    else
		w->w_xhead = wp;
	    p->w_xprev = wp;
	} else {
	    wp->w_xprev = w->w_xtail;
	    if (w->w_xtail)
		w->w_xtail->w_xnext = wp;
	    else
		w->w_xhead = wp;
	    w->w_xtail = wp;
	}

    /* add to Y list */
	p = w->w_yhead;
	while (p && (p->w_ymin < wp->w_ymin))
	    p = p->w_ynext;
	while (p && (p->w_ymin == wp->w_ymin) && (p->w_xmin < wp->w_xmin))
	    p = p->w_ynext;
	wp->w_ynext = p;
	if (p) {
	    wp->w_yprev = p->w_yprev;
	    if (p->w_yprev)
		p->w_yprev->w_ynext = wp;
	    else
		w->w_yhead = wp;
	    p->w_yprev = wp;
	} else {
	    wp->w_yprev = w->w_ytail;
	    if (w->w_ytail)
		w->w_ytail->w_ynext = wp;
	    else
		w->w_yhead = wp;
	    w->w_ytail = wp;
	}

#ifdef HORIZPIECES
    /* add to Z list */
	p = w->w_zhead;
	while (p && (p->w_ymax > wp->w_ymax))
	    p = p->w_znext;
	while (p && (p->w_ymax == wp->w_ymax) && (p->w_xmin < wp->w_xmin))
	    p = p->w_znext;
	wp->w_znext = p;
	if (p) {
	    wp->w_zprev = p->w_zprev;
	    if (p->w_zprev)
		p->w_zprev->w_znext = wp;
	    else
		w->w_zhead = wp;
	    p->w_zprev = wp;
	} else {
	    wp->w_zprev = w->w_ztail;
	    if (w->w_ztail)
		w->w_ztail->w_znext = wp;
	    else
		w->w_zhead = wp;
	    w->w_ztail = wp;
	}
#endif
	w->w_state |= WS_NEWPIECES;
}

/*
 * wm_pop:
 *	- put this window at the beginning of the window list
 */
wm_pop(w)
register struct wm_window *w;
{
	wm_promote(w, win_first);
}

/*
 * wm_push:
 *	- put this window just in front of the big dummy window representing
 *	  the display screen
 */
wm_push(w)
register struct wm_window *w;
{
	if(!w)
	    return;
	while ((w->w_next) && !(w->w_next->w_state & WS_ISDUMMY))
	    wm_promote(w->w_next, w);
}

/*
 * wm_derez:
 *	- put this window at the end of the window list
 */
wm_derez(w)
register struct wm_window *w;
{
	if(!w)
	    return;
	while (w->w_next)
	    wm_promote(w->w_next, w);
}

/*
 * wm_allocwin:
 *	- allocate a new window
 */
struct wm_window *
wm_allocwin()
{
	register struct wm_window *w;

	if (win_freelist == (struct wm_window *)0)
	    w = (struct wm_window *)getmemory(sizeof(struct wm_window));
	else {
	    w = win_freelist;
	    win_freelist = w->w_next;
	}
	return w;
}

/*
 * wm_freewin:
 *	- free a window
 */
wm_freewin(w)
register struct wm_window *w;
{
	if(!w)
	    return;
	if(w->w_next)
	    w->w_next->w_prev = w->w_prev;
	else
	    win_last = w->w_prev;
	if(w->w_prev)
	    w->w_prev->w_next = w->w_next;
	else
	    win_first = w->w_next;
	if (w->w_yhead)
	    fprintf(stderr,"mex: wm_freewin: yhead non-null\n");
	bzero(w, sizeof(struct wm_window));
	w->w_next = win_freelist;
	win_freelist = w;
}

/*
 * wm_allocpiece:
 *	- allocate a new windowpiece
 */
struct windowpiece *wm_allocpiece()
{
	register struct windowpiece *p;

	if (win_piecefreelist == (struct windowpiece *)0)
	    p = (struct windowpiece *)getmemory(sizeof(struct windowpiece));
	else {
	    p = win_piecefreelist;
	    win_piecefreelist = p->w_ynext;
	}
	return p;
}

/*
 * wm_freepiece:
 *	- free a windowpiece
 */
wm_freepiece(wp)
register struct windowpiece *wp;
{
	wp->w_ynext = win_piecefreelist;
	win_piecefreelist = wp;
}

/*
 * wm_freepiecelist:
 *	- free a list of pieces using ynext links
 */
wm_freepiecelist(wp)
register struct windowpiece *wp;
{
    register struct windowpiece *wpnext;

    while(wp) {
	wpnext = wp->w_ynext;
	wm_freepiece(wp);
	wp = wpnext;
    }
}

/*
 * wm_listsame:
 *	- compare the pieces in two lists, return true if same
 */
wm_listsame(wp1,wp2)
register struct windowpiece *wp1, *wp2;
{
    while(wp1 && wp2) {
	if(wp1->w_xmin != wp2->w_xmin)
	    return 0;
	if(wp1->w_xmax != wp2->w_xmax)
	    return 0;
	if(wp1->w_ymin != wp2->w_ymin)
	    return 0;
	if(wp1->w_ymax != wp2->w_ymax)
	    return 0;
	wp1 = wp1->w_ynext;
	wp2 = wp2->w_ynext;
    }
    if(wp1 || wp2)
	return 0;
    else
	return 1;
}

/*
 * wm_copylist:
 *	- copy the contents of a piece list. The returned structure is
 *	only linked in one direction.
 */
struct windowpiece *wm_copylist(wp)
register struct windowpiece *wp;
{
    register struct windowpiece *head, *cwp;

    cwp = head = 0;
    while(wp) {
	if(head == 0) 
	    head = cwp = wm_allocpiece();
	else {
	    cwp->w_ynext = wm_allocpiece();
	    cwp = cwp->w_ynext;
	}
	cwp->w_xmin = wp->w_xmin;
	cwp->w_xmax = wp->w_xmax;
	cwp->w_ymin = wp->w_ymin;
	cwp->w_ymax = wp->w_ymax;
	cwp->w_ynext = 0;
	wp = wp->w_ynext;
    }
    return head;
}

/*
 * wm_resize:
 *	- change the dimensions of a window.  Window must be de-rezzed
 *	  before being resized.
 */
wm_resize(w, xmin, xmax, ymin, ymax)
register struct wm_window *w;
short xmin, xmax, ymin, ymax;
{
    register struct wm_window *nw;

    if(!w)
	return;
    nw = w->w_next;
    while (nw && !(nw->w_state & WS_ISDUMMY))
	nw = nw->w_next;
    if(nw) {
	fprintf(stderr,"mex: attempt to resize un-derezzed window\n");
	return;
    }
    w->w_xmin = xmin;
    w->w_xmax = xmax;
    w->w_ymin = ymin;
    w->w_ymax = ymax;
    w->w_ncols = (xmax - xmin - XBORDER*2 + gl_charwidth - 1) / gl_charwidth;
    w->w_nrows = (ymax - ymin - YBORDER*2 + gl_charheight - 1) / gl_charwidth;
    if(w->w_ncols < MINCOLS)
	w->w_ncols = MINCOLS;
    else if(w->w_ncols > MAXCOLS)
	w->w_ncols = MAXCOLS;
    if(w->w_nrows < MINROWS)
	w->w_nrows = MINROWS;
    else if(w->w_nrows > MAXROWS)
	w->w_nrows = MAXROWS;
}

/*
 * wm_move:
 *	- change the location of a window.  Window must be de-rezzed
 *	  before being moved.
 */
wm_move(w, xmin, ymin)
register struct wm_window *w;
short xmin, ymin;
{
	register short	dx = xmin - w->w_xmin, dy = ymin - w->w_ymin;
	register struct wm_window *nw;

	if(!w)
	    return;
	nw = w->w_next;
	while (nw && !(nw->w_state & WS_ISDUMMY))
	    nw = nw->w_next;
	if(nw) {
	    fprintf(stderr,"mex: attempt to move un-derezzed window\n");
	    return;
	}
	w->w_xmin = w->w_xmin + dx;
	w->w_xmax = w->w_xmax + dx;
	w->w_ymin = w->w_ymin + dy;
	w->w_ymax = w->w_ymax + dy;
}

/*
 * wm_infront:
 *	- return the window pointer of the window in front of this window
 */
struct wm_window *
wm_infront(w)
struct wm_window *w;
{
	if(w)
	    return w->w_prev;
	else
	    return 0;
}

/*
 * wm_inback:
 *	- return the window pointer of the window in back of this window
 */
struct wm_window *
wm_inback(w)
struct wm_window *w;
{
	if(w)
	    return w->w_next;
	else
	    return 0;
}

/*
 * wm_attachtx:
 *	- attach a textport to this window, returning the textport number
 *	  allocated
 */
wm_attachtx(w)
struct wm_window *w;
{
	if(!w)
	    return;
	if(w->w_txport != -1)
	    return -1;
	w->w_txport = grioctl(GR_GETTXPORT, 0);
	    return w->w_txport;
}

/*
 * wm_detachtx:
 *	- detach textport from this window
 */
wm_detachtx(w)
struct wm_window *w;
{
	if(!w)
	    return;
	if(w->w_txport == -1)
	    return;
	grioctl(GR_PUTTXPORT, w->w_txport);
	w->w_txport = -1;
}

/*
 * wm_detachgf:
 *	- detach graphport from this window
 */
wm_detachgf(w)
struct wm_window *w;
{
	if(!w)
	    return;
	if(w->w_inchan == -1)
	    return;
	if (!(w->w_state & WS_CLOSED)) {
		w->w_state |= WS_CLOSED;
		grioctl(GR_PUTINCHAN, w->w_inchan);
	}
	w->w_pr.gfnum = w->w_inchan = -1;
}
