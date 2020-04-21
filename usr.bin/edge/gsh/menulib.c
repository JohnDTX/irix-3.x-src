/*
 *	menu -
 *		A menu package for the iris.
 *
 *				Paul Haeberli - 1985
 *
 */

#define UNIX

#include <gl/gl.h>
#include <gl/device.h>
#include <stdio.h>
#include <gl/globals.h>
#include <gl/glipc.h>
#ifdef GL5
#define pupcolor(n)	color(n)
#else
#include <gl/grioctl.h>
#endif

#define MAXTOKEN	128
#define TITLEHEIGHT	(ENTRYHI+1)
#define TITLESEP	2

#define ENTRYHI		(entryheight)
#define SHADOW		5
#define ARROWWIDTH	16

#define SWEEPBACK	(-2)
#define NOSELECTION	(0x80000001)

/* values for functype */
#define ARG1		1	/* func(return_value) */
#define ARG2		2	/* func(return_value, &entry->value) */

static short shadow[PATTERN_16_SIZE] = {
    0x5555, 0xaaaa, 0x5555, 0xaaaa,
    0x5555, 0xaaaa, 0x5555, 0xaaaa,
    0x5555, 0xaaaa, 0x5555, 0xaaaa,
    0x5555, 0xaaaa, 0x5555, 0xaaaa
};

#define P_SHADOW    	1234

typedef int (*PFI) ();

typedef struct pupentry {
    struct pupentry *next;
    PFI             func;	/* %f, %f2 */
    short           functype;	/* ARG1, ARG2 */
    PFI             pupfunc;	/* %p */
    struct puphead *pup;	/* %m */
    char           *text;
    short           width;
    short           state;
    short           no;
    Boolean         toggle;	/* %b */
    long            group;	/* %g *//* 0 = no group */
    long            value;	/* %v */
    long            retno;	/* %x, %r */
    Object          userobj;	/* %o *//* 0 = no object */
} pupentry;

typedef struct puphead {
    struct pupentry *entries;
    struct puphead *subpup;
    PFI             func;	/* %F, %F2 */
    short           functype;	/* ARG1, ARG2 */
    short           nentries;
    short           rhsicon;	/* no. of entries with RHS icons */
    short           width;	/* entry width */
    short           height;	
    char           *title;  
    short           twidth;	/* title width */
    short           xorg;
    short           yorg;
} puphead;

short *gl_smallbufaddr();

#define POPUP		1
#define OVERLEFT	2
#define OVERMIDDLE	3
#define OVERRIGHT	4

static puphead *basepup;
static int oldwritemask;
static int oldbuffer;
static int menushift;
static int mexpups;
static int entryheight;
static int menubut;
static int menuretval;
static int qmx, qmy, qic;
static int inpupmode = 0;
static int indopup = 0;

#define SAVELEN	200

static int savep;
static short savedev[SAVELEN];
static short saveval[SAVELEN];

long
newpup()
{
    register puphead *pup;

    pup = (puphead *) calloc(1, sizeof(puphead));
    return (int) pup;
}

void
freepup(pup)
register puphead *pup;
{
    register pupentry *entry, *m;

    entry = pup->entries;
    while (entry) {
	m = entry;
	if (m->text)
	    free(m->text);
	entry = entry->next;
	free(m);
    }
    if (pup->title)
	free(pup->title);
    free(pup);
}

defpup(str, args)
register char *str;
long args;
{
    return do_addpup(newpup(), str, &args);
}

addtop(pup, str, len, arg)
puphead *pup;
char *str;
int len;
long arg;
{
    register char *cpy;

    cpy = (char *) malloc(len + 1);
    bcopy(str, cpy, len);
    cpy[len] = 0;
    addtopup(pup, cpy, arg);
    free(cpy);
}

void
addtopup(pup, str, args)
puphead *pup;
char *str;
long args;
{
    do_addpup(pup, str, &args);
}

static pupentry *
lastentry(pup)
puphead *pup;
{
    pupentry *entry;

    if (pup->entries) {
	for (entry = pup->entries; entry->next; entry = entry->next);
	return entry;
    }
    else
	return 0;
}

static
do_addpup(pup, str, argptr)
register char *str;
register long *argptr;
register puphead *pup;
{
    register int len;
    register pupentry *m, *entry;
    char token[MAXTOKEN];
    char mods[MAXTOKEN];
    short rhsicon, width, height;
    short nentries;
    char *cptr;
    int fontsave;

    fontsave = getfont();
    font(0);
    entryheight = getheight() + 1;
    nentries = 0;
    for (entry = pup->entries; entry; entry = entry->next)
	nentries++;
    while ((len = gettoken(str, token, mods))) {
	str += len;
	if (strlen(token) > 0) {
	    if (istitlespec(mods)) {
		pup->title = (char *) calloc(1, strlen(token) + 1);
		strcpy(pup->title, token);
		pup->twidth = strwidth(token);
	    }
	    else {
		m = (pupentry *) calloc(1, sizeof(pupentry));
		m->no = nentries++;
		m->retno = nentries;
		m->next = 0;
		m->text = (char *) calloc(1, strlen(token) + 1);
		strcpy(m->text, token);
		m->width = strwidth(token);
		m->func = 0;
		m->pup = 0;
		if (pup->entries) {
		    for (entry = pup->entries; entry->next; entry = entry->next);
		    entry->next = m;
		}
		else
		    pup->entries = m;
	    }
	}
	for (cptr = mods; *cptr; cptr++) {
	    if (*cptr == '%') {
		switch (cptr[1]) {
		  case 'F':
		    pup->func = (PFI) (*argptr++);
		    if (cptr[2] == '2') {
			pup->functype = ARG2;
			cptr++;
		    }
		    else
			pup->functype = ARG1;
		    break;

		  case 'f':
		    if ((entry = lastentry(pup)))
			entry->func = (PFI) (*argptr++);
		    if (cptr[2] == '2') {
			entry->functype = ARG2;
			cptr++;
		    }
		    else
			entry->functype = ARG1;
		    break;

		  case 'm':
		    if ((entry = lastentry(pup)))
			entry->pup = (puphead *) (*argptr++);
		    break;
    
		  case 'n':
		    break;

		  case 'p':
		    if ((entry = lastentry(pup)))
			entry->pupfunc = (PFI) (*argptr++);
		    break;

		  case 'x':
		    if ((entry = lastentry(pup)))
			entry->retno = atoi(cptr + 2);
		    break;

		  case 'r':
		    if ((entry = lastentry(pup)))
			entry->retno = atoi(*argptr++);
		    break;

		  case 'b':
		    if ((entry = lastentry(pup)))
			entry->toggle = TRUE;
		    break;

		  case 'g':
		    if ((entry = lastentry(pup)))
			entry->group = atoi(*argptr++);
		    break;

		  case 'v':
		    if ((entry = lastentry(pup)))
			entry->value = atoi(*argptr++);
		    break;

		  case 'o':
		    if ((entry = lastentry(pup)))
			entry->userobj = atoi(*argptr++);
		    break;
		}
	    }
	}
    }
    rhsicon = 0;
    width = 0;
    for (entry = pup->entries; entry; entry = entry->next) {
	if (entry->width > width)
	    width = entry->width;
	if (entry->pup || entry->pupfunc || entry->toggle)
	    rhsicon++;
    }
    if (pup->title) {
	if ((pup->twidth + 14) > width)
	    width = pup->twidth + 14;
    }
    width += 10;
    if (rhsicon)
	width += ARROWWIDTH;
    pup->nentries = nentries;
    pup->rhsicon = rhsicon;
    pup->width = width;
    pup->height = nentries * ENTRYHI;
    pup->subpup = 0;
    font(fontsave);
    return (int) pup;
}

static pupentry *
entryn(pup, sel)
puphead *pup;
register short sel;
{
    register pupentry *entry;
    register short i;

    if (sel < 0)
	return 0;
    entry = pup->entries;
    for (i = 0; i < sel; i++) {
	entry = entry->next;
	if (!entry)
	    return 0;
    }
    return entry;
}

long
dopup(pup)
register puphead *pup;
{
    static short firsted;
    short mx, my;

    if (!firsted) {
	defpattern(P_SHADOW, PATTERN_16, shadow);
	firsted++;
    }
    if (indopup) {
	fprintf(stderr, "dopup: this may not be called from a menu function\n");
	return -1;
    }
    intopup();
    menubut = 1;
    basepup = pup;
    indopup = 1;
    mx = getvaluator(MOUSEX);
    my = getvaluator(MOUSEY);
    if (gl_dopup(pup, mx, my, POPUP)) {
        setvaluator(MOUSEX, mx, 0, XMAXSCREEN);
        setvaluator(MOUSEY, my, 0, YMAXSCREEN);
    }
    indopup = 0;
    outofpup();
    if (menuretval == NOSELECTION)
	return -1;
    else
	return menuretval;
}

static int
setxyorg(pup, cx, cy)
register puphead *pup;
short cx, cy;
{
    register short xorg, yorg;
    register short neg;
    register moved = 0;

    xorg = cx - pup->width / 2;
    yorg = cy + ENTRYHI / 2;
    if (xorg < 0)
	xorg = 0;
    if (xorg + pup->width > XMAXSCREEN)
	xorg = XMAXSCREEN - pup->width;
    if (pup->title)
	neg = TITLEHEIGHT + TITLESEP;
    else
	neg = 0;
    if (yorg > (YMAXSCREEN - neg)) {
	yorg = YMAXSCREEN - neg;
	setvaluator(MOUSEY, yorg - ENTRYHI / 2, 0, YMAXSCREEN);
	moved++;
    }
    if (yorg - pup->height < 0) {
	yorg = pup->height;
	setvaluator(MOUSEY, yorg - ENTRYHI / 2, 0, YMAXSCREEN);
	moved++;
    }
    pup->xorg = xorg;
    pup->yorg = yorg;
    return moved;
}

static
drawpup(pup, init)
register puphead *pup;
short init;
{
    register short i;
    register pupentry *entry;
    register short xorg, yorg;
    register short y;
    register int nlines;

    xorg = pup->xorg;
    yorg = pup->yorg;
    pupcolor(PUP_BLACK);
    if (pup->title) {
	shadowrect(xorg, yorg + TITLESEP,
		xorg + pup->width, yorg + TITLESEP + TITLEHEIGHT);
    }
    shadowrect(xorg, yorg - pup->height, xorg + pup->width, yorg);
    if (pup->title) {
	pupcolor(PUP_WHITE);
	rectfi(xorg, yorg + TITLESEP,
		xorg + pup->width, yorg + TITLESEP + TITLEHEIGHT);
	pupcolor(PUP_BLACK);
	recti(xorg, yorg + TITLESEP,
		xorg + pup->width, yorg + TITLESEP + TITLEHEIGHT);

	nlines = (ENTRYHI - 5) / 2;
	for (i = 0; i < nlines; i++) {
	    y = yorg + TITLESEP + 4 + 2 * i;
	    move2i(xorg + 4 + i, y);
	    draw2i(xorg + (pup->width - pup->twidth) / 2 - 4, y);
	    move2i(xorg + (pup->width + pup->twidth) / 2 + 4, y);
	    draw2i(xorg + pup->width - 3 + i - nlines, y);
	}
	cmov2i(xorg + (pup->width - pup->twidth) / 2, yorg + TITLESEP + 4);
	charstr(pup->title);
    }
    for (entry = pup->entries; entry; entry = entry->next)
	drawent(pup, entry, 0, init);
}

static
drawallpups()
{
    register puphead *pupptr = basepup;

    while (pupptr) {
	drawpup(pupptr, 0);
	pupptr = pupptr->subpup;
    }
}

static
undrawpup(pup)
register puphead *pup;
{
    register short xorg, yorg;

    xorg = pup->xorg;
    yorg = pup->yorg;
    pupcolor(PUP_CLEAR);
    if (pup->title)
	rectfi(xorg, yorg + TITLESEP + TITLEHEIGHT,
		xorg + pup->width + SHADOW, yorg + TITLESEP - SHADOW);
    rectfi(xorg, yorg, xorg + pup->width + SHADOW, yorg - pup->height - SHADOW);
}

static int
gl_dopup(pup, xorg, yorg, mode)
register puphead *pup;
short xorg, yorg;
short mode;
{
    register pupentry *entry;
    int oldsel, cursel;
    pupentry *oldent, *curent;
    register short cx, cy;
    register short i;
    short val, dev;
    int nx, ny;
    int rv;

    pup->subpup = 0;
    rv = setxyorg(pup, xorg, yorg);
#ifndef GL5
    cursoff();
#endif
    drawpup(pup, 1);
#ifndef GL5
    curson();
#endif
    xorg = pup->xorg;
    yorg = pup->yorg;
    cursel = -1;
    curent = 0;
    cx = getvaluator(MOUSEX);
    cy = getvaluator(MOUSEY);
    while (1) {
	oldsel = cursel;
	oldent = curent;
	if (cy > yorg || cy < (yorg - pup->height)) {
	    cursel = -1;
	    curent = 0;
	}
	else if (cx < xorg || cx > (xorg + pup->width)) {
	    cursel = -1;
	    curent = 0;
	    entry = oldent;
	    if (entry) {	/* we're sliding off now */
		if (entry->pup) {	/* pop up a memu if one is defined */
		    if (cx < xorg)
			nx = xorg - 5;
		    else
			nx = xorg + pup->width + 5;
		    ny = (yorg - oldsel * ENTRYHI) - ENTRYHI / 3;
		    pup->subpup = entry->pup;
		    if (cx < xorg)
			gl_dopup(entry->pup, nx, ny, OVERLEFT);
		    else
			gl_dopup(entry->pup, nx, ny, OVERRIGHT);
		    pup->subpup = 0;
		    if (!menubut)
			goto out;
#ifndef GL5
		    cursoff();
#endif
		    drawallpups();
#ifndef GL5
		    curson();
#endif
		}
		else if (entry->pupfunc) {	/* maybe call slide off func */
		    menuretval = (entry->pupfunc) (entry->retno);
		    if (!menubut)
			goto out;
#ifndef GL5
		    cursoff();
#endif
		    drawallpups();
#ifndef GL5
		    curson();
#endif
		}
	    }
	}
	else {
	    cursel = (yorg - cy) / ENTRYHI;
	    curent = entryn(pup, cursel);
	}
	if (cursel != oldsel) {
#ifndef GL5
	    cursoff();
#endif
	    drawent(pup, oldent, 0, 1);
	    drawent(pup, curent, 1, 1);
#ifndef GL5
	    curson();
#endif
	}
	if (mode == OVERRIGHT) {
	    if (cx < xorg) {
		menuretval = NOSELECTION;
		goto out;
	    }
	}
	else if (mode == OVERLEFT) {
	    if (cx > xorg + pup->width) {
		menuretval = NOSELECTION;
		goto out;
	    }
	}
	do {
	    dev = qread(&val);
	    switch (dev) {
	      case MOUSEX:
		cx = val;
		break;
	      case MOUSEY:
		cy = val;
		break;
	      case MENUBUTTON:
		menubut = val;
		break;
	      case INPUTCHANGE:
		if (val == 0) {
		    menuretval = NOSELECTION;
		    goto out;
		}
		break;
	      default:
		savedev[savep] = dev;
		saveval[savep] = val;
		savep++;
		if (savep >= SAVELEN) {
		    fprintf(stderr, "menu save len exceeded\n");
		    savep--;
		}
		break;
	    }
	} while (qtest());
	if (menubut == 0) {
	    if (curent) {
		menuretval = curent->retno;
		if (curent->toggle)
		    curent->value = !(curent->value);
		if (curent->func) {
		    outofpup();
		    if (curent->functype == ARG2)
			menuretval = (curent->func) (menuretval, &curent->value);
		    else
			menuretval = (curent->func) (menuretval);
		    intopup();
		}
	    }
	    else
		menuretval = NOSELECTION;
	    goto out;
	}
    }

out:
    if (menuretval != NOSELECTION) {
	if (pup->entries && curent && curent->group) {
	    for (entry = pup->entries; entry->next; entry = entry->next) {
		if (entry != curent && entry->group == curent->group)
		    entry->value = 0;
	    }
	}
	if (pup->func) {
	    outofpup();
	    if (pup->functype == ARG2) {
		if (curent)
		    menuretval = (pup->func) (menuretval, &curent->value);
		else
		    menuretval = (pup->func) (menuretval, 0);
	    }
	    menuretval = (pup->func) (menuretval);
	    intopup();
	}
    }
#ifndef GL5
    cursoff();
#endif
    undrawpup(pup);
#ifndef GL5
    curson();
#endif
    return rv;
}


static
drawent(pup, entry, newstate, setstate)
register puphead *pup;
register pupentry *entry;
short newstate, setstate;
{
    register short i;
    short textcolor, backcolor;
    register short xorg, yorg;

    if (!entry)
	return;
    xorg = pup->xorg;
    yorg = pup->yorg - (entry->no + 1) * ENTRYHI;
    if (setstate)
	entry->state = newstate;
    if (entry->state) {
	backcolor = PUP_BLACK;
	textcolor = PUP_WHITE;
    }
    else {
	backcolor = PUP_WHITE;
	textcolor = PUP_BLACK;
    }
    pupcolor(backcolor);
    rectfi(xorg + 1, yorg, xorg + pup->width - 1, yorg + ENTRYHI);
    pupcolor(PUP_BLACK);
    recti(xorg, yorg, xorg + pup->width, yorg + ENTRYHI);
    pupcolor(textcolor);
    if (pup->rhsicon)
	cmov2i(xorg + (pup->width - entry->width - ARROWWIDTH) / 2, yorg + 3);
    else
	cmov2i(xorg + (pup->width - entry->width) / 2, yorg + 3);
    charstr(entry->text);
    if (entry->pup || entry->pupfunc || (entry->toggle && entry->value)) {
	pushmatrix();
	translate(xorg + pup->width - 12.0, yorg + ENTRYHI / 2 - 3.0, 0.0);
	if (entry->userobj)
	    callobj(entry->userobj);
	else {
	    if (entry->toggle)
		drawcheck();
	    else
		drawarrow();
	}
	popmatrix();
    }
}

static
gettoken(str, token, mods)
register char *str, *token, *mods;
{
    char *s, *t;

    s = str;
    t = token;
    while (*str && (*str == '|'))
	str++;
    while (*str && (*str != '|'))
	*t++ = *str++;
    *t = 0;
    if (token != t) {
	for (t = token; *t && (*t != '%'); t++);
	strcpy(mods, t);
	*t = 0;
	for (--t; (t >= token) && iswhite(*t); t--)
	    *t = 0;
	return str - s;
    }
    else
	return 0;
}

static int ahead[3][2] = {
	{5, 0},
	{8, 3},
	{5, 6},
};

static
drawarrow()
{
    rectfi(0, 2, 2, 4);
    rectfi(4, 2, 4, 4);
    polf2i(3, ahead);
}

static int acheck[7][2] = {
	{1, 0},
	{3, 0},
	{8, 5},
	{7, 6},
	{2, 1},
	{1, 3},
	{6, 2},
};

static
drawcheck()
{
    polf2i(7, acheck);
}

static
iswhite(c)
char c;
{
    return ((c == ' ') || (c == '\t'));
}

static
istitlespec(str)
char *str;
{
    while (*str) {
	if (str[0] == '%' && str[1] == 't')
	    return 1;
	str++;
    }
    return 0;
}

static
shadowrect(x1, y1, x2, y2)
register int x1, x2, y1, y2;
{
    setpattern(P_SHADOW);
    rectfi(x1 + SHADOW, y1 - SHADOW, x2 + SHADOW, y1);
    rectfi(x2, y1, x2 + SHADOW, y2 - SHADOW);
    setpattern(0);
}

setmexpups(n)
int n;
{
    menushift = n;
    mexpups++;
}

void pupmode();
void fullscrn();

static
intopup()
{
    if (!inpupmode) {
	qmx = isqueued(MOUSEX);
	qmy = isqueued(MOUSEY);
	qic = isqueued(INPUTCHANGE);
	qdevice(MOUSEX);
	qdevice(MOUSEY);
	qdevice(INPUTCHANGE);
	qdevice(MENUBUTTON);

	pupmode();
	fullscrn();
#ifndef GL5
	if (!mexpups)
	    setcursor(0, 1, 1);
#endif
	linewidth(1);
	setlinestyle(0);
	setpattern(0);
	depthcue(0);
	backface(0);
#ifndef GL5
	curson();
#endif
	font(0);
	savep = 0;
	inpupmode = 1;
    }
}

static
outofpup()
{
    int i;

    if (inpupmode) {
	void endpupmode();
	void endfullscrn();

	endfullscrn();
	endpupmode();

	for (i = 0; i < savep; i++)
	    qenter(savedev[i], saveval[i]);
	if (!qmx)
	    unqdevice(MOUSEX);
	if (!qmy)
	    unqdevice(MOUSEY);
	if (!qic)
	    unqdevice(INPUTCHANGE);
	inpupmode = 0;
    }
}

void
pupmode()
{
    pushattributes();
    pushmatrix();
    pushviewport();
#ifdef mips
    drawmode(PUPDRAW);
    writemask(3);
#else
    if (oldwritemask) {
	fprintf(stderr, "pupmode: can't nest these babes for now\n");
	exit(0);
    }
    if (!menushift)
	menushift = getplanes();
    if (!mexpups) {
	if (getdisplaymode() == 2) {
	    oldbuffer = getbuffer() + 8;
	    backbuffer(1);
	    frontbuffer(1);
	}
	else
	    oldbuffer = 0;
	oldwritemask = gl_wstatep->bitplanemask;
	gl_wstatep->bitplanemask = 3 << menushift;
	winfunc(BEGINPUPMODE);
    }
    writemask(3 << menushift);
#endif
}

void
endpupmode()
{
#ifdef GL5
    drawmode(NORMALDRAW);
    /* RGBwritemask(0xff,0xff,0xff); */
#else
    if (!mexpups) {

	gl_wstatep->bitplanemask = oldwritemask;
	winfunc(ENDPUPMODE);
	writemask(oldwritemask);
	if (oldbuffer) {
	    backbuffer(oldbuffer & 1);
	    frontbuffer((oldbuffer & 2) >> 1);
	}
    }
    oldwritemask = 0;
#endif
    popattributes();
    popmatrix();
    popviewport();
}


#ifdef GL5

#define GFNUM	(gl_wstatep->shmemp->context)

void
fullscrn()
{
    short data[1];

    if (!mexpups) {
	data[0] = GFNUM;
	gl_winfunc(BEGINFSMODE, data, 1 * sizeof(short));
    }
    viewport(0, XMAXSCREEN, 0, YMAXSCREEN);
    ortho2(-0.5, XMAXSCREEN.5, -0.5, YMAXSCREEN.5);
}

void
endfullscrn()
{
    short data[1];

    if (!mexpups) {
	data[0] = GFNUM;
	gl_winfunc(ENDFSMODE, data, 1 * sizeof(short));
    }
}

#else GL5

void
fullscrn()
{
    if (!mexpups)
	winfunc(BEGINFSMODE);
    viewport(0, XMAXSCREEN, 0, YMAXSCREEN);
    ortho2(-0.5, XMAXSCREEN.5, -0.5, YMAXSCREEN.5);
}

void
endfullscrn()
{
    if (!mexpups)
	winfunc(ENDFSMODE);
}

void
pupcolor(c)
int c;
{
    color(c << menushift);
}
#endif GL5


#ifdef NOTDEF
sigport(dev, val)
short dev, val;
{
    grioctl(GR_SIGCON, (dev << 16) + val);
}

replycon(inchanno)
int inchanno;
{
    grioctl(GR_REPLYCON, inchanno);
}

modcon(outic, outport, inic, inport, make)
int outic, outport, inic, inport, make;
{
    short *data = gl_smallbufaddr();

    data[0] = outic;
    data[1] = outport;
    data[2] = inic;
    data[3] = inport;
    data[4] = make;
    return grioctl(GR_MODCON, 0);
}

randpup(pup, n)
register puphead *pup;
register int n;
{
    register int x, y;
    register int i;

    for (i = 0; i < n; i++) {
	x = rand() % 1024;
	y = rand() % 768;
	pup->xorg = x;
	pup->yorg = y;
	drawpup(pup, 0);
    }
}

#endif
