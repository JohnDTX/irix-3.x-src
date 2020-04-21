/*
 * Window manager
 *
 * Written by: Kipp Hickman
 * Rehacked like totally by Rocky Rhodes and Paul Haeberli
 *
 */
#include "stdio.h"
#include "win.h"
#include "gltypes.h"
#include "globals.h"
#include "misc.h"
#include "window.h"
#include "cursors.h"
#include "shmem.h"

struct windowpiece *wm_copylist();

/*
 * wn_destroy:
 *	- destroy the current window
 */
wn_destroy(w, kill)
register struct wm_window *w;
short kill;		/* kill the owner? */
{
    register short i;
    register struct wm_window *nextw;

    if(!w || (w==win_console))
	return;
    if(w->w_state & WS_USEDPUPMODE)
	wn_clearpupplanes();
    if(w->w_state & WS_HASKEYBOARD)
	movekbd(win_console);
    if(w != backgroundw)
	wn_derez(w);
    if(kill) {
	wm_detachtx(w);
	wm_detachgf(w);
    }
    w->w_ownerpid = -1;
    if(w != backgroundw)
	for(i=0; i<2; i++) {
	    nextw = wm_inback(w);
	    wm_freewin(w);
	    w = nextw;
	}
}

wn_derez(w)
register struct wm_window *w;
{
    register struct wm_window *nextw = wm_inback(w);

    wm_derez(w);		/* derez inside window */
    wm_derez(nextw);
}

wn_resize(w1, x1, x2, y1, y2)
struct wm_window *w1;
short x1, x2, y1, y2;
{
    register struct wm_window *w = w1, *w2, *bw;
    register short	i, adjust;

    /* hpm - Fri Apr  4 10:59:36 PST 1986 
    fixed to keep windows on the screen */
    if (x1 > (XMAXSCREEN-TITLEHEIGHT)) {
	adjust = x1 - (XMAXSCREEN - TITLEHEIGHT);
	x2 -= adjust;
	x1 -= adjust;
    }
    if (x2 < TITLEHEIGHT) {
	adjust = TITLEHEIGHT - x2;
	x2 += adjust;
	x1 += adjust;
    }
    if (y1 > (YMAXSCREEN-TITLEHEIGHT)) {
	adjust = y1 - (YMAXSCREEN - TITLEHEIGHT);
	y2 -= adjust;
	y1 -= adjust;
    }
    if (y2 < TITLEHEIGHT) {
	adjust = TITLEHEIGHT - y2;
	y2 += adjust;
	y1 += adjust;
    }
    /* hpm - Fri Apr  4 10:59:36 PST 1986 
    fixed to keep windows on the screen */

    w2 = wn_inback(w1);
    wn_derez(w1);
/* copy new position info into window */
    wm_resize(w1, x1, x2, y1, y2);
    bw = wm_inback(w1);
    if(w1->w_title[0])
	wm_resize(bw, x1-(2*BORDERWIDTH), x2+(2*BORDERWIDTH)+DROP,
	    y1-(2*BORDERWIDTH)-DROP, y2+(2*BORDERWIDTH)+TITLEHEIGHT);
    else
	wm_resize(bw, x1-(2*BORDERWIDTH), x2+(2*BORDERWIDTH)+DROP,
	    y1-(2*BORDERWIDTH)-DROP, y2+(2*BORDERWIDTH));

/* draw window in new position */
    if(w2) {
	bw = wm_inback(w1);		/* border window */
	wm_promote(w1, w2);
	wm_promote(bw, wm_inback(w1));
    }
}

/*
 * wn_newshape:
 *	- give a window a new shape
 */
wn_newshape(w, rows, cols)
register struct wm_window *w;
register short rows, cols;
{
    struct wm_window tempw;

    wn_resize(w, w->w_xmin, w->w_xmin + XLEN(cols) - 1 + 2*TXBORDER,
		     w->w_ymin, w->w_ymin + YLEN(rows) - 1 + 2*TXBORDER);
}

/*
 * wn_pop:
 *	- move given window to the top of the heap
 */
wn_pop(w)
register struct wm_window *w;
{
    register struct wm_window *bw, *nextbw;
    register short i;

    if(!w)
	return;
    bw = wm_inback(w);
    nextbw = wm_inback(bw);
    wm_pop(w);
    wm_promote(bw, wm_inback(w));
}

/*
 * wn_push:
 *	- move the given window to the bottom of the heap
 */
wn_push(w)
register struct wm_window *w;
{
    register struct wm_window *bw = w, *nextw;
    register short i;

    if(!w)
	return;
    bw = wm_inback(bw);
    w = wm_infront(bw);
    nextw = wm_infront(w);
    wm_push(bw);
    while (wm_inback(w) != bw)
	wm_promote(wm_inback(w), w);
}

/*
 * wn_attach:
 *	- attach keyboard to a specific window
 */
wn_attach(w)
struct wm_window *w;
{
    movekbd(w);
}

/*
 * wn_repaint:
 *	- repaint all the windows on the screen
 */
wn_repaint(how)
int how;
{
    register struct wm_window *w, *bw;

    gl_lockpipe();	/* don't update while inconsistent */
    for(w = win_first; w; w = wn_inback(w)) {
	bw = wm_inback(w);
	if (how) {
	    w->w_state |= WS_REDISPLAY;
	    w->w_shadow = 0;
	    if(bw) 
		bw->w_state |= WS_REDISPLAY;
	}
	if(w->w_state & WS_ISDUMMY) {
	    if(w->w_state & WS_REDISPLAY) 
		background(1);
	    else if(w->w_state & WS_NEWPIECES) 
		background(0);
	} else
	    wn_redraw(w);
    }
    grioctl(GR_SAFE, 0);
    gl_freepipe();
}

/*
 * wn_redraw:
 *	- redraw a window
 *	- then draw the border up
 *	- really need to check if the border window needs to be redisplayed 
 */
wn_redraw(w)
register struct wm_window *w;
{
    register struct wm_window *bw = wm_inback(w);
    register int doredraw;

    if(bw->w_state & WS_REDISPLAY) {
	wn_drawborder(w,1);
	bw->w_state &= ~WS_REDISPLAY;
    }
    doredraw = w->w_state & WS_REDISPLAY;
    if(w->w_shadow == piececheck(w))
	doredraw = 0;
    if(w->w_state & WS_NEWPIECES || doredraw) {
	wn_setpieces(w, doredraw);	
	w->w_state &= ~WS_NEWPIECES;
	w->w_state &= ~WS_REDISPLAY;
    }
}

struct wm_window *wn_inback(w)
register struct wm_window *w;
{
    register short i;

    if(w->w_state & WS_ISDUMMY)
	return wm_inback(w);		/* no border window for dummy win */
    return wm_inback(wm_inback(w));
}

struct wm_window *wn_newwin()
{
    register struct wm_window *w, *bw;

    w = wm_newwin();
    bw = wm_newwin();
    return w;
}

#define colorsdiff(name) (hilightcolors.name != stdcolors.name)

/*
 * wn_drawborder:
 *	- draw a windows border
 *	- assumes cursor is off
 *	- if how is 0, the header and outer border are only redrawn if
 *	  the color for attached is different from not attached.
 */
wn_drawborder(w1,how)
register struct wm_window *w1;
int how;
{
    register struct wm_window *w = wm_inback(w1);
    register short urx, ury, i, j;
    register struct colorarray	*c;
    int y, x1, x2, x3, x4;
    int nlines;

    wn_setrects(w);
    urx = w->w_xmax-w->w_xmin;
    ury = w->w_ymax-w->w_ymin;
    /* viewport(-llx, XMAXSCREEN-llx, -lly, YMAXSCREEN-lly); */
    viewport(-XMAXSCREEN, 2*XMAXSCREEN, 
			-YMAXSCREEN, 2*YMAXSCREEN);
    ortho2(-XMAXSCREEN-0.5,2*XMAXSCREEN+0.5,
	    		-YMAXSCREEN-0.5,2*YMAXSCREEN+0.5);
    if(w1->w_state & WS_HASKEYBOARD)
	c = &hilightcolors;
    else
	c = &stdcolors;
    if( (how == 1) || colorsdiff(binnercolor) ) {
	    color(c->binnercolor);
	    rectfi(BORDERWIDTH, BORDERWIDTH+DROP, 
				urx-BORDERWIDTH-DROP, ury-BORDERWIDTH);
    }
    if(w1->w_title[0]) {
        if( (how == 1) || colorsdiff(binnercolor) || colorsdiff(tinnercolor)
		|| colorsdiff(toutercolor) || colorsdiff(boutercolor) ) {
	    color(c->tinnercolor);
	    cmov2i(2*BORDERWIDTH+16, ury-2*BORDERWIDTH-TITLEHEIGHT+6);
	    font(titlefontno);
	    charstr(w1->w_title);
	    x1 = 2*BORDERWIDTH+2;
	    x2 = 2*BORDERWIDTH+12;
	    x3 = x2+strwidth(w1->w_title) + 6;
	    font(0);
	    x4 = urx-6;
	    y = ury-2*BORDERWIDTH-TITLEHEIGHT+6;
	    nlines = (TITLEHEIGHT+2)/4;
	    color(c->toutercolor);
	    for(i=0; i<nlines; i++) {
	        move2i(x1+i,y);
	        draw2i(x2,y);
	        move2i(x3,y);
	        draw2i(x4+i+4-(2*nlines),y);
	        y+=2;
	    }
	    color(c->boutercolor);
	    move2i(0,ury-2*BORDERWIDTH-TITLEHEIGHT+2);
	    draw2i(urx,ury-2*BORDERWIDTH-TITLEHEIGHT+2);
	}
    }
    if( (how == 1) || colorsdiff(boutercolor) ) {
        color(c->boutercolor);
   	for(i=0; i<BORDERWIDTH; i++)
	    recti(i, i, urx-i, ury-i);
        for(i=1; i<=DROP; i++)
	    recti(0, i, urx-i, ury);
    }
    wn_setrects(0);
    viewport(0, XMAXSCREEN, 0, YMAXSCREEN);
    ortho2(-0.5,XMAXSCREEN.0,-0.5,YMAXSCREEN.0);
}

/*
 * wn_initconsole:
 *	- initialize the console window
 */
wn_initconsole()
{
    register struct wm_window *w;
    short x1, x2, y1, y2;

    win_console = w = wn_newwin();
    strcpy(w->w_title, "console");
    wn_initport(w);
    w->w_state |= WS_TEXTPORT;
    w->w_txport = 0;
    w->w_inchan = -1;
    gettp(&x1, &x2, &y1, &y2);
    wn_resize(w, x1+2, x2-2, y1+2, y2-2);
    wn_pop(w);
    wn_attach(w);
}

/*
 * wn_setpieces:
 *	- tell the system about a window's pieces
 */
wn_setpieces(w, doredraw)
register struct wm_window *w;
short doredraw;
{
    register struct windowpiece *wp;
    register struct grpiece *p;
    register short npieces, fudge, i;
    struct pbuf pbuf;

    p = &pbuf.piece[0];
    npieces = 0;
    wp = w->w_yhead;
    if(w->w_state & WS_FULLSCREEN) {
	p->gr_xmin = 0;
	p->gr_ymin = 0;
	p->gr_xmax = XMAXSCREEN;
	p->gr_ymax = YMAXSCREEN;
	pbuf.ph.gr_no = w->w_pr.gfnum;
	pbuf.ph.gr_type = GFPORT;
	pbuf.ph.gr_llx = 0;
	pbuf.ph.gr_lly = 0;
	pbuf.ph.gr_urx = XMAXSCREEN;
	pbuf.ph.gr_ury = YMAXSCREEN;
	npieces = 1;
    } else {
	while (wp) {
	    p->gr_xmin = wp->w_xmin;
	    p->gr_ymin = wp->w_ymin;
	    p->gr_xmax = wp->w_xmax;
	    p->gr_ymax = wp->w_ymax;
	    p++;
	    npieces++;
	    wp = wp->w_ynext;
	}
	if (w->w_state & WS_TEXTPORT) {
	    pbuf.ph.gr_no = w->w_txport;
	    pbuf.ph.gr_type = TXPORT;
	    pbuf.ph.gr_llx = w->w_xmin - 2;
	    pbuf.ph.gr_lly = w->w_ymin - 2;
	    pbuf.ph.gr_urx = w->w_xmax + 2;
	    pbuf.ph.gr_ury = w->w_ymax + 2;
	} else {
	    pbuf.ph.gr_no = w->w_pr.gfnum;
	    pbuf.ph.gr_type = GFPORT;
	    pbuf.ph.gr_llx = w->w_xmin;
	    pbuf.ph.gr_lly = w->w_ymin;
	    pbuf.ph.gr_urx = w->w_xmax;
	    pbuf.ph.gr_ury = w->w_ymax;
	}
    }
    pbuf.ph.gr_pieces = npieces;
    pbuf.ph.gr_ncols = w->w_ncols;
    pbuf.ph.gr_nrows = w->w_nrows;
    pbuf.ph.gr_doredraw = doredraw;
    grioctl(GR_SETPIECE, &pbuf);
    w->w_shadow = piececheck(w);
}

#define hashit(x) 	{				\
	if((check^=(x)) < 0) {				\
	    check <<= 1;				\
	    check++;					\
	} else						\
	    check <<= 1;				\
}

/* 
 * piececheck -
 *	return a checksum calculated by the dimensions of each piece
 *	in a window's piecelist.
 *
 */
piececheck(w)
struct wm_window *w;
{
    register struct windowpiece *wp;
    register int check;

    check = 0;
    hashit(w->w_xmin);
    hashit(w->w_ymin);
    hashit(w->w_xmax);
    hashit(w->w_ymax);
    for(wp = w->w_yhead; wp; wp=wp->w_ynext) {
	hashit(wp->w_xmin);
	hashit(wp->w_ymin);
	hashit(wp->w_xmax);
	hashit(wp->w_ymax);
    }
    return check;
}

/*
 * wn_setrects:
 *	- set the window manager's piecelist to mimic someone else's window:
 */
wn_setrects(w)
struct wm_window *w;
{
    register struct windowpiece *wp;
    register short *coords;

    if(w) {
	gl_wstatep->numrects = 0;
	coords = gl_wstatep->rectlist;
	gl_wstatep->xmin = w->w_xmin;
	gl_wstatep->ymin = w->w_ymin;
	gl_wstatep->xmax = w->w_xmax;
	gl_wstatep->ymax = w->w_ymax;
	if(!w->w_yhead) {
	    *coords++ = 1;
	    *coords++ = 1;
	    *coords++ = 0;
	    *coords++ = 0;
	} else for(wp=w->w_yhead; wp; wp=wp->w_ynext) {
	    gl_wstatep->numrects++;
	    *coords++ = wp->w_xmin;
	    *coords++ = wp->w_ymin;
	    *coords++ = wp->w_xmax;
	    *coords++ = wp->w_ymax;
	}
    } else {
	gl_wstatep->numrects = 1;
	gl_wstatep->xmin = 0;
	gl_wstatep->ymin = 0;
	gl_wstatep->xmax = XMAXSCREEN;
	gl_wstatep->ymax = YMAXSCREEN;
	gl_wstatep->rectlist[0] = 0;
	gl_wstatep->rectlist[1] = 0;
	gl_wstatep->rectlist[2] = XMAXSCREEN;
	gl_wstatep->rectlist[3] = YMAXSCREEN;
    }
}

/*
 * wn_receive:
 *	- receive a message from another process
 */
wn_receive(icnum, sr)
short icnum;
struct sendrec *sr;
{
    struct portreq *pr;
    register struct wm_window *w;
    register short x1, x2, y1, y2;
    int gfnum, oldinput;

    switch (sr->msg) {
	case PORTREQ:
		pr = (struct portreq *)sr->data;	
		if(pr->imakebackground) {
		    if(backgroundw->w_inchan != -1)
			grioctl(GR_PUTINCHAN, backgroundw->w_inchan);
		    backgroundw->w_inchan = icnum;
		    backgroundw->w_pr.gfnum = pr->gfnum;
		    backgroundw->w_state |= WS_GRAPHPORT;
		    backgroundw->w_ownerpid = pr->pid;
		    wn_setpieces(backgroundw, 1);
		    grioctl(GR_REPLY, icnum);
		} else
		    wn_newgraph(icnum, sr->data);
		break;
	  case POPREQ:
		gfnum = sr->data[0];
		if(w = wn_findgf(gfnum)) {
		    wn_pop(w);
		    wn_repaint(0);
		}
		grioctl(GR_REPLY, icnum);
		break;
	  case PUSHREQ:
		gfnum = sr->data[0];
		if(w = wn_findgf(gfnum)) {
		    wn_push(w);
		    wn_repaint(0);
		}
		grioctl(GR_REPLY, icnum);
		break;
	  case ATTACHREQ:
		gfnum = sr->data[0];
		oldinput = inputwindow->w_pr.gfnum;
		if(w = wn_findgf(gfnum)) {
		    wn_attach(w);
		    wn_repaint(0);
		}
		sr->data[0] = oldinput;
		grioctl(GR_SHWRITE, icnum);
		grioctl(GR_REPLY, icnum);
		break;
	  case MOVEREQ:
		gfnum = sr->data[0];
		x1 = sr->data[1];
		y1 = sr->data[2];
		w = wn_findgf(gfnum);
		if(w) {
		    wn_resize(w, x1, x1 + (w->w_xmax - w->w_xmin),
					y1, y1 + (w->w_ymax - w->w_ymin));
		    wn_repaint(0);
		}
		grioctl(GR_REPLY, icnum);
		break;
	  case RESHAPEREQ:
		gfnum = sr->data[0];
		x1 = sr->data[1];
		x2 = sr->data[2];
		y1 = sr->data[3];
		y2 = sr->data[4];
		w = wn_findgf(gfnum);
		if(w) {
		    wn_resize(w, x1, x2, y1, y2);
		    wn_repaint(0);
		}
		grioctl(GR_REPLY, icnum);
		break;
	  case SETTITLEREQ:
		gfnum = sr->data[0];
		w = wn_findgf(gfnum);
		if(w) {
		    if(w->w_title[0]) {
			strcpy(w->w_title, &sr->data[1]);
			wn_drawborder(w,1);
		    } else {
			strcpy(w->w_title, &sr->data[1]);
			wn_resize(w,w->w_xmin,w->w_xmax,w->w_ymin,w->w_ymax);
			wn_repaint(0);
		    }
		}
		grioctl(GR_REPLY, icnum);
		break;
#ifdef NOTDEF
	  case FULLSCREENMODE:
		gfnum = sr->data[0];
		w = wn_findgf(gfnum);
		if(w) {
		    wn_derez(w);
		    w->w_state |= WS_FULLSCREEN;
		    w->w_state |= WS_NEWPIECES;
		    wn_repaint(0);
		}
		grioctl(GR_REPLY, icnum);
		break;
#endif
	  case BEGINPUPMODE:
		gfnum = sr->data[0];
		w = wn_findgf(gfnum);
		if(w) 
		    w->w_state |= WS_USEDPUPMODE;
		grioctl(GR_REPLY, icnum);
		break;
	  case ENDPUPMODE:
		grioctl(GR_REPLY, icnum);
		break;

	  case BEGINFSMODE:
		gfnum = sr->data[0];
		w = wn_findgf(gfnum);
		if(w) {
		    w->w_state |= (WS_FULLSCREEN | WS_NEWPIECES);
		    wn_repaint(0);
		}
		grioctl(GR_REPLY, icnum);
		break;
	  case ENDFSMODE:
		gfnum = sr->data[0];
		w = wn_findgf(gfnum);
		if(w) {
		    w->w_state &= ~WS_FULLSCREEN;
		    w->w_state |= WS_NEWPIECES;
		    wn_repaint(0);
		}
		grioctl(GR_REPLY, icnum);
		break;
	  case WINDOWAT:
		x1 = sr->data[0];
		y1 = sr->data[1];
		w = windowat(x1,y1);
		if(w)
		    sr->data[0] = w->w_pr.gfnum;
		else
		    sr->data[0] = -1;
		grioctl(GR_SHWRITE, icnum);
		grioctl(GR_REPLY, icnum);
		break;
	  case INCHANAT:
		x1 = sr->data[0];
		y1 = sr->data[1];
		w = windowat(x1,y1);
		if(w)
		    sr->data[0] = w->w_inchan;
		else
		    sr->data[0] = -1;
		grioctl(GR_SHWRITE, icnum);
		grioctl(GR_REPLY, icnum);
		break;
	  case SETNAMEREQ:
		gfnum = sr->data[0];
		w = wn_findgf(gfnum);
		if(w) 
		    strcpy(w->w_name, &sr->data[1]);
		grioctl(GR_REPLY, icnum);
		break;
	  case TXOPEN:
		grioctl(GR_REPLY, icnum);
		break;
	  case TXCLOSE:
		grioctl(GR_REPLY, icnum);
		break;
	  case NEWHINTS:
		pr = (struct portreq *)sr->data;	
		gfnum = pr->gfnum;
		w = wn_findgf(gfnum);
		if(w) 
		    w->w_pr = *pr;
		grioctl(GR_REPLY, icnum);
		break;
	  default:
		fprintf(stderr,"mex: screwy message received: %d\n", sr->msg);
		break;
	}
}

/*
 * wn_txclose:
 *	- close a tx port
 */
wn_txclose(txnum)
int txnum;
{
    register struct wm_window *w;

    w = win_first;
    while (w) {
	if ((w->w_state & WS_TEXTPORT) && (w->w_txport == txnum)) {
	    wn_destroy(w, 1);
	    return;
	}
	w = wn_inback(w);
    }
}

/*
 * wn_gfclose:
 *	- close window associated with a gfport
 */
wn_gfclose(gfnum)
int gfnum;
{
    register struct wm_window *w, *nextw;

    w = win_first;
    while (w) {
	nextw = wn_inback(w);
	if ((w->w_state & WS_GRAPHPORT) && (w->w_pr.gfnum == gfnum)) {
	    wn_destroy(w, 0);
	    return;
	}
	w = nextw;
    }
}

/*
 * wn_inclose:
 *	- close all gfports associated with an input channel
 */
wn_inclose(inchan)
int inchan;
{
    register struct wm_window *w, *nextw;

    w = win_first;
    while (w) {
	nextw = wn_inback(w);
	if ((w->w_state & WS_GRAPHPORT) && (w->w_inchan == inchan))
	    wn_destroy(w, 1);
	w = nextw;
    }
}

struct wm_window *wn_findgf(gfnum)
int gfnum;
{
    register struct wm_window *w;

    w = win_first;
    while (w) {
	if ((w->w_state & WS_GRAPHPORT) && (w->w_pr.gfnum == gfnum))
	    return w;
	w = wn_inback(w);
    }
    return 0;
}

wn_initport(w)
struct wm_window *w;
{
    register struct portreq *pr = &w->w_pr;

    pr->preforigin = 0;
    pr->prefsize = 0;
    pr->minsizex = 20;
    pr->minsizey = 20;
    pr->maxsizex = XLEN(MAXCOLS) + 2*TXBORDER;
    pr->maxsizey = YLEN(MAXROWS) + 2*TXBORDER;
    pr->keepaspect = 0;
    pr->xunit = gl_charwidth;
    pr->yunit = gl_charheight;
    pr->xunitfudge = 2*TXBORDER;
    pr->yunitfudge = 2*TXBORDER;
}

wn_clearpupplanes()
{
	writemask(menumask);
	color(0);	/* default background */
	cursoff();
	clear();
	curson();
	writemask(drawmask);
}

genhash(ptr,nbytes)
register unsigned short *ptr;
int nbytes;
{
    register int check;
    register int nshorts;

    if (nbytes&1) {
	fprintf(stderr,"genhash: must be even bytecount\n");
	return 0;
    }
    check = 0;
    nshorts = nbytes>>1; 
    while(nshorts--) {
	check = (check ^ *ptr++) << 1;
	if(check & 0x40000000)
	    check++;
    }
    return check;
}
