#define MAIN

#include "win.h"
#include "signal.h"
#include "globals.h"
#include "stdio.h"
#include "misc.h"
#include "window.h"
#include "shmem.h"
#include "cursors.h"

#define NUMDEVS	600
#define YOFFSET	6

struct devfunction {
    struct devfunction 	*next;
    long		(*func)();
};

struct devfunction devtab[NUMDEVS];

struct wm_window *currentw;

short resizex, resizey;
static short confirmed;

int	(*saveint)(), (*savehup)(), (*savequit)(), (*saveterm)();

int	newshell(), wmexit(), attachwman(), killwindow(),
	movewindow(), reshapewindow(), popandattachwin(),
	popwindow(), pushwindow(), attachwindow(); 

int window_menu, wm_menu, confirm_menu, more_menu;
char aux_default[] = "no entries";

defmenus()
{
	more_menu = defpup("help|set|tell");
	window_menu = defpup("mex %t|attach|select|move");
	addtopup(window_menu,"reshape|pop|push|kill");
	wm_menu = defpup("mex %t|attach|new shell|exit");
	confirm_menu = defpup("confirm %t|Do it. I'm sure|Nah... forget it.");
}

int dbmode = 0;

sigkid()		/* called when child has finished crucial setup */
{
    exit(0);
}

main(argc, argv)
int argc;
char *argv[];
{
    register struct devfunction *ptr;
    struct wm_window *w;
    register short t, i;
    short v;
    int parent = getpid();
    int f;

    for(i=1; i<argc; i++) {
	if(strcmp(argv[i],"-d") == 0)
	    dbmode = 1;
	else if(strcmp(argv[i],"-t") == 0) {
	    if(++i<argc)
		titlefont = argv[i];
	} else {
	    fprintf(stderr,"mex: wierd option %s\n",argv[i]);
	    exit(1);
	}
    }

/* put wman in the background */
#define SIGKID	SIGUSR1
    signal(SIGKID, sigkid);	/* probably exit through sigkid */
    if(((f = fork()) != 0) && (f != -1)) {	/* we are succesful parent */
	pause();		/* till child tells us through critical part */
	exit(0);
    }
    if(f == -1) {
	fprintf(stderr,"mex: no more processes\n");
	exit(1);
    }
    signal(SIGKID, SIG_IGN);
    saveint = signal(SIGINT, SIG_IGN);
    savequit = signal(SIGQUIT, SIG_IGN);
    savehup = signal(SIGHUP, SIG_IGN);
    saveterm = signal(SIGTERM, SIG_IGN);

/* set up default colors */
    stdcolors.binnercolor = WHITE;
    stdcolors.boutercolor = BLACK;
    stdcolors.tinnercolor = BLUE;
    stdcolors.toutercolor = BLUE;

    hilightcolors.binnercolor = WHITE;
    hilightcolors.boutercolor = RED;
    hilightcolors.tinnercolor = RED;
    hilightcolors.toutercolor = RED;
    cursorr = 255; cursorg = 0; cursorb = 0;
    menur = 20; menug = 20; menub = 20;
    menubr = 250; menubg = 250; menubb = 250;

/* kill off any graphics programs already running: */
    for(t=0; t<NINCHANS; t++)
	grioctl(GR_PUTINCHAN, t);
    ginit();
    if(titlefont) {
	titlefontno = 1;
	loadfont(titlefont,1);
	font(1);
	titleheight = getheight() + 2;
	font(0);
    } else
	titleheight = getheight() + 2;
    softqreset();
    if(grioctl(GR_MEWMAN,0)<0)
	panic("only one wman at a time!!\n");
    if(dbmode) {
	doublebuffer();
	gconfig();
	frontbuffer(1);
	grioctl(GR_SWAPANYTIME,0);
    }
    defmenus();
    initdevtab();
    readinit();
    cursorsetup();
    gl_reservebitplanes(2);
    resetmasks();
    wn_setrects(0);
    viewport(0, XMAXSCREEN, 0, YMAXSCREEN);
    cursoff();
    setcursor(C_IDLE, cursorcolor, cursormask);
    deflinestyle(1, 0xF0F0);

/* init the window manager */
    wm_init();
    wn_initconsole();
    procinit();
    background(1);
    curson();
    qdevice(MODECHANGE);
    qdevice(MENUBUTTON);
    qdevice(WINCLOSE);
    getmousepos();
    wn_repaint(0);
    kill(parent,SIGKID);		/* tell parent ok to exit now */
    for (;;) {
	t = qread(&v);
	ptr = &devtab[t];
	while(ptr) {
	    (*(ptr->func))(t, v);
	    ptr = ptr->next;
	}
	purgeq();
    }
}

/*
**  A.L. 9/2/87  Purge cursor position data from the input queue, and
**	reserialize the input queue with events from the software queue
**	before calling dopup().  This is necessary because dopup() does
**	not know that it should read the software queue.
*/
purgeq()
{
    register int i;
    short dev, val;

	/*
	** First, empty the hard queue onto the soft queue, purging
	** the all cursor events; then copy all other events onto the
	** event queue, including a possible menu button up event needed
	** to terminate the pop up.  Thus the order of events in the
	** queue is preserved.
	*/
	while (qtest()) {
	    dev = qread(&val);
	    if ((dev != CURSORX) && (dev != CURSORY)) {
		softqenter(dev, val);
	    }
	}
	/*
	** Also purge cursor events that may have already been put
	** on the soft queue.
	*/
	while (softqtest()) {
	    dev = softqread(&val);
	    if ((dev != CURSORX) && (dev != CURSORY)) {
		qenter(dev, val);
	    }
	}
}

defaultfunc(t, v)
short t, v;
{
}

modechangefunc(t, v)
short t, v;
{
    resetmasks();
    wn_repaint(1);	/* redraw all windows after mode change */
}

wmsendfunc(t, v)
short t, v;
{
    grioctl(GR_SHREAD, v);
    wn_receive(v, (struct sendrec *)gl_shmemptr->smallbuf);
    wn_repaint(0);
}

wmtxclosefunc(t, v)
short t, v;
{
    wn_txclose(v);
    wn_repaint(0);
}

wmgfclosefunc(t, v)
short t, v;
{
    register struct wm_window *w;

#ifdef NOTDEF
    w = wn_findgf(v);
    if (w == NULL)
	return;
    w->w_state |= WS_CLOSED;
#endif
    wn_gfclose(v);
    wn_repaint(0);
}

wminclosefunc(t, v)
short t, v;
{
    register struct wm_window *w;

#ifdef NOTDEF
    w = wn_findgf(v);
    if (w == NULL)
	return;
    w->w_state |= WS_CLOSED;
#endif
    wn_inclose(v);
    wn_repaint(0);
}

menufunc(origt, v)
register short origt;
short v;
{
    struct wm_window *savedinputw;
    int selno;

    gettiedstuff(origt);				/* sets currentw */
    if(!v)
	return;
    savedinputw = movekbd(0); 
    setcursor(0, cursorcolor, cursormask);

    purgeq();

    if(currentw) {
	menuwindow = currentw;
        selno = dopup(window_menu);
	getmousepos();
        if((selno != 1) && (!inputwindow) && savedinputw)
	    movekbd(savedinputw);
        switch(selno) {
	    case 1: 
		attachwindow();
		break;
	    case 2: 
		popandattachwin();
		break;
	    case 3: 
		movewindow();
		break;
	    case 4: 
		reshapewindow();
		break;
	    case 5: 
		popwindow();
		break;
	    case 6: 
		pushwindow();
		break;
	    case 7: 
		killwindow();
		break;
	} 
    } else {
	menuwindow = backgroundw;
        selno = dopup(wm_menu);
	getmousepos();
        if((selno != 1) && (!inputwindow) && savedinputw)
	    movekbd(savedinputw);
        switch(selno) {
	    case 1: 
		attachwman();
		break;
	    case 2: 
		newshell();
		break;
	    case 3: 
		wmexit();
		break;
	}
    }
    setcursor(C_IDLE, cursorcolor, cursormask);
}

hogmodefunc(t, v)
short t, v;
{
    if(!v)
	movekbd(0);
}

hogwhiledownfunc(t, v)
short t, v;
{
    static struct wm_window *savedinputw = 0;

    if(v) {
	if(!savedinputw) {
	    savedinputw = inputwindow;
	    movekbd(0);
	}
    } else {
	if((!inputwindow) && savedinputw)
	    movekbd(savedinputw);
	savedinputw = 0;
    }
}

killfunc(t, v)
{
    gettiedstuff(t);
    if(v)
	return;
    if(currentw) 
	killproc(currentw);
}

pushfunc(t, v)
short t, v;
{
    gettiedstuff(t);
    if(v) {
	setcursor(C_PUSH, cursorcolor, cursormask);
	return;
    }
    if(currentw) {
	wn_push(currentw);
	wn_repaint(0);
    } else
	ringbell();
    setcursor(C_IDLE, cursorcolor, cursormask);
}

popfunc(t, v)
short t, v;
{
    gettiedstuff(t);
    if(v) {
	setcursor(C_POP, cursorcolor, cursormask);
	return;
    } 
    if(currentw) {
	wn_pop(currentw);
	wn_repaint(0);
    } else 
	ringbell();
    setcursor(C_IDLE, cursorcolor, cursormask);
}

popattachfunc(t, v)
short t, v;
{
    gettiedstuff(t);
    if(v) {
	setcursor(C_POP, cursorcolor, cursormask);
	return;
    }
    if(currentw) {
	wn_pop(currentw);
	wn_attach(currentw);
	wn_repaint(0);
    } else
	ringbell();
    setcursor(C_IDLE, cursorcolor, cursormask);
}

attachfunc(t, v)
short t, v;
{
    gettiedstuff(t);
    if(v)
	return;
    if(currentw) 
	wn_attach(currentw);
}


/*
** While an application window is the current input focus, a button up for
** movefunc() or movegrowfunc() can occur.  In this case the button up
** event will not go to the window manager, thus leaving the window drag
** mechanism in an illegal state; so don't process the button down in
** this condition, i.e. ignore the drag request.
*/

movefunc(origt, v)
short origt;
short v;
{
    register struct wm_window *savedinputw;
    register short t;

    gettiedstuff(origt);
    if(!v)
	return;
    if (inputwindow)		/* quit if not window mgr.  A.L. 9/2/87 */
	return;
    if(!currentw) {
	savedinputw = movekbd(0);
	do_moving(currentw);
	dothedrag(origt);
	dragging->knowposition = 1;
	reset_dragbox();
	wn_undrawdragbox();
	wn_resize(dragging, dragx1, dragx2, dragy1, dragy2);
	dragging = 0;
	setcursor(C_IDLE, cursorcolor, cursormask);
	movekbd(savedinputw);
	wn_repaint(0);
    } else
	ringbell();
}

movegrowfunc(origt, v)
short origt;
short v;
{
    register struct wm_window *savedinputw, *w;
    register short t, knowsize = 0;

    gettiedstuff(origt);
    if(!v)
	return;
    if(!(w = currentw)) {
	ringbell();
	return;
    }
    if (inputwindow)		/* quit if not window mgr.  A.L. 9/2/87 */
	return;
    savedinputw = movekbd(0);
    dragging = w;
    w->knowposition = 0;
    w->knowsize = 1;		/* draw initial box correctly */
    wn_drawdragbox();
    if(w->w_pr.prefsize) {
	setcursor(C_MOVE, cursorcolor, cursormask);
    } else if( (w->w_xmax - mousex < 24) && (mousey - w->w_ymin < 24) ) {
	dragx1 = w->w_xmin;
	dragy2 = w->w_ymax;
	dragx2 = w->w_xmax;
	dragy1 = w->w_ymin;
	w->knowsize = 0;
	setcursor(C_LRIGHT, cursorcolor, cursormask);
    } else if((w->w_ymax - mousey < 24) && (mousex - w->w_xmin < 24)){
	dragx1 = w->w_xmax;
	dragy2 = w->w_ymin;
	dragx2 = w->w_xmin;
	dragy1 = w->w_ymax;
	w->knowsize = 0;
	setcursor(C_ULEFT, cursorcolor, cursormask);
    } else if((w->w_xmax - mousex < 24) && (w->w_ymax - mousey < 24)){
	dragx1 = w->w_xmin;
	dragy2 = w->w_ymin;
	dragx2 = w->w_xmax;
	dragy1 = w->w_ymax;
	w->knowsize = 0;
	setcursor(C_URIGHT, cursorcolor, cursormask);
    } else if((mousey - w->w_ymin < 24) && (mousex - w->w_xmin < 24)){
	dragx1 = w->w_xmax;
	dragy2 = w->w_ymax;
	dragx2 = w->w_xmin;
	dragy1 = w->w_ymin;
	w->knowsize = 0;
	setcursor(C_LLEFT, cursorcolor, cursormask);
    } else {
	setcursor(C_MOVE, cursorcolor, cursormask);
    }
    dothedrag(origt);
    dragging->knowposition = 1;
    reset_dragbox();
    wn_undrawdragbox();
    wn_resize(dragging, dragx1, dragx2, dragy1, dragy2);
    dragging = 0;
    setcursor(C_IDLE, cursorcolor, cursormask);
    movekbd(savedinputw);
    wn_repaint(0);
}

/*
** A.L. 9/2/87  Major rewrite of the original:
**	If the next event on the queue is for the button that originated
**	    the current action, do nothing (leave it on the queue);
**	else try to remove up to 2 events, assuming that they are for the
**	    cursor x & y; if so, update the mouse position & current window;
**	if any other event, copy it to the software queue.
*/
gettiedstuff(origt)
int origt;
{
    register int i, dev;
    short val;

    for (i = 0; i < 2; i++) {
	if (dev = qtest()) {
	    if((dev == origt) || ((!origt) && ISBUTTON(dev)))
		return;
	    dev = qread(&val);
	    switch(dev) {
	    case CURSORX:
		mousex = val;
		break;
	    case CURSORY:
		mousey = val;
		break;
	    default:
		softqenter(dev, val);
	    }
	}
    }
    currentw = windowat(mousex,mousey);
}

getacorner()
{
    register short t;
    short v;

    while(1) {
	t = qread(&v);
	if(ISBUTTON(t)) {
	    gettiedstuff(t);
	    if(v)
		return t;
	} else 
	    softqenter(t, v);
    }
}

dothedrag(origt)
register short origt;
{
    register short t;
    short v;

    qdevice(CURSORX);
    qdevice(CURSORY);
    while(1) {
	redraw_dragbox();
	do {
	    t = qread(&v);
	    if(t ==  CURSORX)
		mousex = v;
	    else if(t ==  CURSORY)
		mousey = v;
	    else if((t == origt) || ((!origt) && ISBUTTON(t))) {
		gettiedstuff(origt);
		if(!v) {
		    unqdevice(CURSORX);
		    unqdevice(CURSORY);
		    redraw_dragbox();
		    return;
		}
	    } else 
		softqenter(t, v);
	} while(qtest());
    }
}

initdevtab()
{
    short i;

    for(i=0; i<NUMDEVS; i++)
	devtab[i].func = defaultfunc;
    bindfunc(modechangefunc, MODECHANGE);
    bindfunc(wmsendfunc, WMSEND);
    bindfunc(wmtxclosefunc, WMTXCLOSE);
    bindfunc(wmgfclosefunc, WMGFCLOSE);
    bindfunc(wminclosefunc, WINCLOSE);
}

bindfunc(func, but)
int (*func)();
short but;
{
    if(but < NUMDEVS)
	devtab[but].func = func;
    if(ISBUTTON(but)) {
	qdevice(but);
	tie(but, CURSORX, CURSORY);
    }
}

panic(msg)
char *msg;
{
    fprintf(stderr,"mex: panic: %s\n", msg);
    exit(1);
}

/*
 * getmemory:
 *	- allocate n bytes of memory
 */
char *
getmemory(n)
int n;
{
    register char *cp;

    cp = (char *)malloc(n);
    if (cp == (char *)0)
	panic("out of memory");
    bzero(cp, n);
    return cp;
}

do_moving(w)
register struct wm_window *w;
{
    dragging = w;
    w->knowposition = 0;
    w->knowsize = 1;
    wn_drawdragbox();
    setcursor(C_MOVE, cursorcolor, cursormask);
}

do_resize(w)
register struct wm_window *w;
{
    dragging = w;
    w->knowposition = 0;
    w->knowsize = 1;	/* con the dragging code to use the */
    wn_drawdragbox();
    w->knowsize = 0;
    if(dragx1 != resizex) {
        dragx2 = dragx1;
        dragx1 = resizex;	/* make the fixed point be the one we want */
    }
    if(dragy2 != resizey) {
        dragy1 = dragy2;
        dragy2 = resizey;
    }
}

redraw_dragbox()
{
    if(!dragging)
	return;
    wn_undrawdragbox();
    wn_drawdragbox();
}

struct wm_window *windowat(x,y)
register short x, y;
{
    register struct wm_window *w = win_first;

    while (w) {
	if (w->w_state & WS_ISDUMMY) 
	    return 0;		 	/* only look through visible windows */
	if (inwindow(w,x,y))		/* graphics window */
		return w;
	if (inwindow(wm_inback(w),x,y))	/* border window */
		return w;
	w = wn_inback(w);
    }
    return 0;
}

inwindow(w,x,y)
struct wm_window *w;
register short x, y;
{
    register struct windowpiece *wp;

    if (w) {
	wp = w->w_yhead;
	while (wp) {
	    if ((wp->w_xmin < x) && (wp->w_xmax >= x) &&
				(wp->w_ymin < y) && (wp->w_ymax >= y))
		return 1;
	    wp = wp->w_ynext;
	}
	return 0;
    } else 
	return 0;
}

resetmasks()
{
    static numplanes;
    register long i;

    i = getplanes();
    setmexpups(i-2);
    cursormask = 3 << (i - 2);
    menucolor = 1 << (i - 1);
    menubcolor = cursormask;
    cursorcolor = 1 << (i - 2);
    drawmask = cursorcolor - 1;
    cursoff();
    color(0);
    writemask(cursormask);
    clear();
    curson();
    setcursor(C_IDLE, cursorcolor, cursormask);
    writemask(drawmask);
    if(i == numplanes)
	return;
    numplanes = i;
    for(i=0; i<=drawmask; i++) {
	mapcolor((i | menucolor), menur, menug, menub);
	mapcolor((i | menubcolor), menubr, menubg, menubb);
	mapcolor((i | cursorcolor), cursorr, cursorg, cursorb);
    }
#if 0		/* the following is not necessary */
    mapcolor(drawmask, 255, 255, 255);	/* white text for kernel */
#endif
}

newshell()
{
    short t, origt;
    short v;
    register struct wm_window *w, *savedinputw;

    savedinputw = movekbd(0);
    setcursor(C_ULEFT, cursorcolor, cursormask);
    origt = getacorner();
    dragging = w = wn_newwin();
    w->w_title[0] = 0;
    wn_initport(w);
    strcpy(w->w_name,"textport");
    w->knowsize = 0;
    w->knowposition = 0;
    dragx1 = mousex;
    dragy1 = mousey;
#ifdef NOTYET
    w->aux_menu_string = aux_default;
#endif NOTYET
    dodesk(w->w_name);
    wn_drawdragbox();
    dothedrag(origt);
    w->knowsize = 1;
    w->knowposition = 1;
    reset_dragbox();
    wn_undrawdragbox();
    wn_resize(w, dragx1, dragx2, dragy1, dragy2);
    wn_pop(w);
    wn_repaint(0); /* has side effect of setting pieces for application */
    if(w->w_state & WS_GRAPHPORT)
	grioctl(GR_REPLY, w->w_inchan);
    else if(!newproc(w)) {
        wn_destroy(w, 1);
        fprintf(stderr, "mex: no more ttyw's for new shell\n");
	w = savedinputw;
    }
    setcursor(C_IDLE, cursorcolor, cursormask);
    movekbd(w);
    wn_repaint(0);
}

/*
 * wn_newgraph:
 *	- create a new graph port
 */
wn_newgraph(icnum, pr)
short icnum;
register struct portreq *pr;
{
    short t, origt;
    short v;
    register struct wm_window *w, *savedinputw;
    int haveinput;

    creating = w = wn_newwin();
    w->w_title[0] = 0;
    w->w_pr = *pr;	/* this is pointing to the smallbuf!!!! */
    pr = &w->w_pr;	/* now it is safe */
    w->w_ownerpid = pr->pid;
    w->w_inchan = icnum;
    strncpy(w->w_name,pr->name,PORTNAMESIZE);
    w->w_state &= ~WS_TEXTPORT;
    w->w_state |= WS_GRAPHPORT;
#ifdef NOTYET
    w->aux_menu_string = aux_default;
#endif NOTYET
    getmousepos();
    dodesk(w->w_name);
    haveinput = 0;
    if(pr->preforigin) {
	if(pr->prefsize) {	/* give gfport w/o interaction */
	    creating = 0;
	    if((pr->prefsizex != 0) || (pr->prefsizey != 0)) {
		wn_resize(w, pr->preforiginx,
				pr->preforiginx+pr->prefsizex-1,
				pr->preforiginy,
				pr->preforiginy+pr->prefsizey-1);
		wn_pop(w);
	    }
	    wn_repaint(0);
	    grioctl(GR_REPLY, w->w_inchan);
	} else {		/* start dragging w/ specified org */
	    if(!haveinput) {
		savedinputw = movekbd(0);
		haveinput = 1;
	    }
	    dragging = w;
	    w->knowsize = 0;
	    w->knowposition = 0;
	    dragx1 = mousex;
	    dragy1 = mousey;
	    wn_drawdragbox();
	    dothedrag(0);
	    w->knowsize = 1;
	    w->knowposition = 1;
	    reset_dragbox();
	    wn_undrawdragbox();
	    wn_resize(w, dragx1, dragx2, dragy1, dragy2);
	    wn_pop(w);
/* has side effect of setting pieces for application: */
	    wn_repaint(0);
	    grioctl(GR_REPLY, w->w_inchan);
	    setcursor(C_IDLE, cursorcolor, cursormask);
	}
    } else if(pr->prefsize) {	/* start dragging w/ specified size */
	if(!haveinput) {
	    savedinputw = movekbd(0);
	    haveinput = 1;
	}
	dragx1 = mousex;
	dragy1 = mousey;
	dragging = w;
	w->knowposition = 0;
	w->knowsize = 0;	/* will be set by initdrag */
	wn_drawdragbox();
	setcursor(C_MOVE, cursorcolor, cursormask);
	dothedrag(0);
	dragging->knowposition = 1;
	reset_dragbox();
	wn_undrawdragbox();
	wn_resize(dragging, dragx1, dragx2, dragy1, dragy2);
	wn_pop(w);
/* has side effect of setting pieces for application: */
	wn_repaint(0);
	dragging = 0;
	grioctl(GR_REPLY, w->w_inchan);
	setcursor(C_IDLE, cursorcolor, cursormask);
    } else {			/* wait for origin */
	if(!haveinput) {
	    savedinputw = movekbd(0);
	    haveinput = 1;
	}
	setcursor(C_ULEFT, cursorcolor, cursormask);
	origt = getacorner();
	dragging = w;
	w->knowsize = 0;
	w->knowposition = 0;
	dragx1 = mousex;
	dragy1 = mousey;
	wn_drawdragbox();
	dothedrag(origt);
	w->knowsize = 1;
	w->knowposition = 1;
	reset_dragbox();
	wn_undrawdragbox();
	wn_resize(w, dragx1, dragx2, dragy1, dragy2);
	wn_pop(w);
/* has side effect of setting pieces for application: */
	wn_repaint(0);
	grioctl(GR_REPLY, w->w_inchan);
	setcursor(C_IDLE, cursorcolor, cursormask);
    }
#ifdef ASKPAUL
    w->w_pr.preforigin = 0;   /* ok to move the port after creation */
#endif
    if(haveinput) 
	movekbd(savedinputw);
}

attachwman()
{
    movekbd(0);
    wn_repaint(0);
}

wmexit()
{
    register struct wm_window *w, *nextw;

    getconfirm();
    if(confirmed) {
	for(w=win_first, nextw=wn_inback(w); w;) {
	    if(w!=win_console) {
		wn_derez(w);
		killproc(w); 
		wn_destroy(w, 1);
	    }
	    w = nextw;
	    if(w) 
		nextw = wn_inback(w);
        }
        wn_destroy(backgroundw, 1);
	backgroundw = win_last;
	backgroundw->w_inchan = -1;
	win_first->w_title[0] = '\0';
	wn_resize(win_first, win_first->w_xmin, win_first->w_xmax,
				win_first->w_ymin, win_first->w_ymax);
	wn_repaint(1);
	sleep(1);
	exit(0);
    }
}

getconfirm()
{
    struct wm_window *savedinputw;
    register short t, i;
    short v, xorg, yorg;
    struct menu *m;

    savedinputw = movekbd(0);
    setcursor(0, cursorcolor, cursormask);

    purgeq();

    switch(dopup(confirm_menu)) {
	case 1: 
	    confirmed = 1;
	    break;
	case 2: 
	    confirmed = 0;
	    break;
 	default: 
	    confirmed = 0;
    }
    getmousepos();
    movekbd(savedinputw);
}

killwindow()
{
    getconfirm();
    if(confirmed) {
        killproc(menuwindow);
        wn_repaint(0);
    }
}

movewindow()
{
    register short t;
    short v;
    register struct wm_window *savedinputw;

    savedinputw = movekbd(0);
    do_moving(menuwindow);
    dothedrag(0);
    dragging->knowposition = 1;
    reset_dragbox();
    wn_undrawdragbox();
    wn_resize(dragging, dragx1, dragx2, dragy1, dragy2);
    dragging = 0;
    setcursor(C_IDLE, cursorcolor, cursormask);
    movekbd(savedinputw);
    wn_repaint(0);
}

reshapewindow()
{
    register struct wm_window *w, *savedinputw;
    short t, origt;
    short v;

    if(menuwindow->w_pr.prefsize) {
	movewindow();
	return;
    }
    savedinputw = movekbd(0);
    setcursor(C_ULEFT, cursorcolor, cursormask);
    origt = getacorner();
    dragging = w = menuwindow;
    w->knowsize = 0;
    w->knowposition = 0;
    dragx1 = mousex;
    dragy1 = mousey;
    wn_drawdragbox();
    dothedrag(origt);
    w->knowsize = 1;
    w->knowposition = 1;
    reset_dragbox();
    wn_undrawdragbox();
    wn_resize(w, dragx1, dragx2, dragy1, dragy2);
    setcursor(C_IDLE, cursorcolor, cursormask);
    movekbd(savedinputw);
    wn_repaint(0);
}

popwindow()
{
    setcursor(C_POP, cursorcolor, cursormask);
    wn_pop(menuwindow);
    wn_repaint(0);
}

pushwindow()
{
    setcursor(C_PUSH, cursorcolor, cursormask);
    wn_push(menuwindow);
    wn_repaint(0);
}

popandattachwin()
{
    wn_pop(menuwindow);
    wn_attach(menuwindow);
    wn_repaint(0);
}

attachwindow()
{
    wn_attach(menuwindow);
    wn_repaint(0);
}

cursorsetup()
{
    defcursor(C_LLEFT, lleft);
    curorigin(C_LLEFT, X_LLEFT, Y_LLEFT);
    defcursor(C_LRIGHT, lright);
    curorigin(C_LRIGHT, X_LRIGHT, Y_LRIGHT);
    defcursor(C_ULEFT, uleft);
    curorigin(C_ULEFT, X_ULEFT, Y_ULEFT);
    defcursor(C_URIGHT, uright);
    curorigin(C_URIGHT, X_URIGHT, Y_URIGHT);
    defcursor(C_SELIN, selin);
    curorigin(C_SELIN, X_SELIN, Y_SELIN);
    defcursor(C_MOVE, movew);
    curorigin(C_MOVE, X_MOVE, Y_MOVE);
    defcursor(C_QUESTION, question);
    curorigin(C_QUESTION, X_QUESTION, Y_QUESTION);
    defcursor(C_PUSH, push);
    curorigin(C_PUSH, X_PUSH, Y_PUSH);
    defcursor(C_POP, pop);
    curorigin(C_POP, X_POP, Y_POP);
    defcursor(C_KILL, ckill);
    curorigin(C_KILL, X_KILL, Y_KILL);
    defcursor(C_MENU, menu);
    curorigin(C_MENU, X_MENU, Y_MENU);
    defcursor(C_IDLE, idle);
    curorigin(C_IDLE, X_IDLE, Y_IDLE);
    defpattern(P_SHADOW, 16, shadow);
}

gl_lockpipe()
{
    grioctl(GR_LOCK, 0);
}

gl_freepipe()
{
    grioctl(GR_UNLOCK, 0);
}

getmousepos()
{
    mousex = getvaluator(CURSORX);
    mousey = getvaluator(CURSORY);
}

wmdoc()
{
    showdoc("wman");
}

auxwindow()
{
    int aux_menu;
    int aux_choice;

#ifdef NOTYET
    showdoc(currentw->w_name);
    aux_menu = defpup(w->aux_menu_string);
    setcursor(0, cursorcolor, cursormask);
    if(aux_choice = dopup(aux_menu)) {
	system(w->aux_cmds[aux_choice]);
    }
    freepup(aux_menu);
#endif NOTYET
}

showdoc(name)
    char *name;
{
    char buf[512];

    strcpy(buf,"ivyview ");
    strcat(buf,"/usr/lib/gl2/doc/");
    strcat(buf,name);
    if(access(buf+8,4)) 
	ringbell();
    else
	dosystem(buf);
}
