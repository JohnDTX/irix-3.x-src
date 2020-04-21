/*
**		Routines for Iris 4010 emulator
**
**			Glen Williams
**			Modified by Jonathan Bowen - July/August 1984
**			Modified by Tom Davis - July 1985 for 
**			conversion to 2400 and integration with wsiris.
*/

#ifndef GL1

#include <gl.h>
#include <device.h>
#include <stdio.h>
#include <fcntl.h>
#include "term.h"
#include "hostio.h"
#include "4010.h"

static int penup = FALSE;
static int displayedvector = FALSE;
static int curx, cury;			/* current graphics position */
static int visx, visy;			/* where visible cursor is sitting */
static int alphx, alphy;		/* current alpha position    */
static int fontheight, fontwidth;
static int xcursincr, ycursincr;
static char prev = '\0';
static Screencoord crossx = 100;
static Screencoord crossy = 100;
static Screencoord oldx, oldy;
static int inleftcolumn = TRUE;

#define xmaxscreen 1020
#define ymaxscreen 764

#define LEFTMARGIN 0
#define ALTLEFTMARGIN (xmaxscreen / 2)
#define RIGHTMARGIN (xmaxscreen / 2)
#define ALTRIGHTMARGIN xmaxscreen


#define LTGREEN CYAN
#define CURSMASK  0x10
#define CURSCODE 0x10
#define CROSSHAIRMASK 0x8
#define CROSSHAIRCODE 0x8
#define ERASECROSSHAIRCODE 0x0
#define NORMALMASK 0x7

/* **********************************************************************
*									*
*		make objs etc.						*
*									*
* ******************************************************************** */
init4010()
{
    char chstg[2];

    chstg[0] = 'M'; chstg[1] = '\0';
    prefsize(1020, 764);
    gbegin();
    cursoff();
    ortho2(-0.5, 1023.5, -0.5, 779.5);
    fontheight =1 + getheight();
    fontwidth = strwidth(chstg);
    xcursincr = fontwidth -3;
    ycursincr = fontheight -2;
    clearall();
    mapplanes(CROSSHAIRCODE, CROSSHAIRMASK, 0, 175, 0);
    mapplanes(CURSCODE, CURSMASK, 0, 0, 0);
    blink(20, CURSCODE, 0, 175, 0);

    curx = 0; cury = ymaxscreen;
    visx = 0; visy = ymaxscreen - fontheight;
    homeAlphCurs();
}

/* **********************************************************************
*									*
*		called when entering Graph Mode				*
*									*
* ******************************************************************** */
graphmode4010()
{   char cur;
    short data;
    Device dev;

    penup = TRUE;		/* got into this proc by seeing a GS */
    displayedvector = FALSE;
    while (TRUE) {
	prev = cur;
	cur = (gethostchar());

	switch (cur) {
	    case GS:	penup = TRUE;
			break;
	    case CR:	if (displayedvector) doCR();
			return;
	    case US:	setAlphCurs(curx, MIN(cury,ymaxscreen-fontheight));
			return;
	    case LF:	doLF();
			break;
	    case BEL:	ringbell();
			break;
	    case ESC:	if (procGSesc(gethostchar())) return;
	    default:	procGSbyte(cur);
			break;
	}
    }
}

/* **********************************************************************
*									*
*		this is the proc that collects bytes			*
*		for assembly to real live coordinates			*
*									*
********************************************************************** */
static procGSbyte(ch)
char ch;
{
    static int prevx = 0, prevy = 0;
    static int lx = 0, ly = 0, hx = 0, hy = 0;
    register char chr;

    if ((chr = (ch & ASCIIMASK)) < 32)
	return;
    else if (chr >= 64 && chr <= 95) {
	/* low 'x' -- time to do something */
	displayedvector = TRUE;
	lx = chr;
	prevx = curx; prevy = cury;
	curx = (hx&0x1f)<<5 | (lx&0x1f);
	cury = (hy&0x1f)<<5 | (ly&0x1f);

	if (curx == prevx && cury == prevy && !penup)
	    pnt2i(curx, cury);

	if (penup) {
	    penup = FALSE;
	    move2i(curx, cury);
	}
	else
	    draw2i(curx, cury);
    }
    else if (chr >= 96 && chr <= 127)
	ly = chr;

/*  must be between 32 and 64, huh? */
    else if (prev >= 96 && prev <= 127)
	hx = chr;
    else
	hy = chr;
}
   /* *******************************************************************
   *									*
   *		handle escape sequences in graphics			*
   *			  mode						*
   *									*
   ******************************************************************* */
static procGSesc(ch)
short int ch;
{   switch (ch) {
	case FF:	clearall();
			homeAlphCurs();
			return(TRUE);  /* i.e., return to caller's caller */
	case ENQ:	sendstatus(curx, cury, GRAPHMODE);
			return(FALSE);	/* i.e., stay in Graph Mode */
	case SUB:	processSUB();
			return(TRUE);   /* i.e., go to Alpha Mode   */
	default:	return(FALSE);
    }
}

 /* *********************************************************************
 *									*
 *		Send status and the coordinates of the graphics		*
 *		position to the host.					*
 *									*
 ********************************************************************* */
sendstatus(X,Y,mode)
int X, Y, mode;
{   char status;
    char *st[40];

    status = ( 0xa0 | HCU | NOLI | (mode?0:GRAPH) | (inleftcolumn?0:MARGIN)
			| AUXSENSE );
    puthostchar(status);
    flushhost();
    sendcoords(X, Y);
}

/* **********************************************************************
*									*
*		Send coordinates to the host				*
*									*
*									*
********************************************************************** */
sendcoords(X,Y)
int X, Y;
{
    char hix, lox, hiy, loy;
    hix = ( 32 | ((X>>5) & 0x1f));
    lox = ( 32 | ( X     & 0x1f));
    hiy = ( 32 | ((Y>>5) & 0x1f));
    loy = ( 32 | ( Y     & 0x1f));
    puthostchar(hix);	/* High X */
    puthostchar(lox);
    puthostchar(hiy);	/* High Y */
    puthostchar(loy);
    flushhost();

}

/* **********************************************************************
*									*
*		This is called by procALesc and by procGSesc.		*
*		It calls crosshair to get coords from user,		*
*		reads 2 chars from the host to pick up the ESC, ENQ	*
*		that follows the ESC,SUB (how we got here), flushes	*
*		the read stream from the host (emulating Bypass)	*
*		then sends the coords to the host.			*
*									*
********************************************************************** */
processSUB()
{
    register int ch;

    fcntl(infile, F_SETFL, fcntl(infile, F_GETFL, 0) | O_NDELAY);
    escsub4010 = 1;
	/* non-blocking reads */

    gsync(); gsync();

    ch = gethostchar();
    fcntl(infile, F_SETFL, fcntl(infile, F_GETFL, 0) & ~O_NDELAY);
    escsub4010 = 0;	/* reads block again */

    if (ch == -2) /* nothing there */ {
	puthostchar(crosshair());
	sendcoords();
	return;
    }

    while (1) {
	while (ch != ESC)
	    ch = gethostchar();
	if ((ch = gethostchar()) == ENQ)
	    break;
    }

    sendcoords(crossx, crossy);
    return;
}


clearall()
{
    register i;

    writemask( (1 << getplanes()) - 1);
    color(GREEN);
    clear();
    for (i = 0; i < 6; i++)
	gsync();
    color(BLACK);
    clear();
    color(GREEN);
}

/* **********************************************************************
*									*
*		alpha mode input handler				*
*									*
* ******************************************************************** */
alphamode4010(chr)
char chr;
{   
    short data;
    Device dev;
    int slen;
    char chstg[2];

    switch (chr) {
	    case ESC:	procALesc();
			return;
	    case BEL:	ringbell();
			break;
	    case BS:	doBS();
			break;
	    case LF:	doLF();
			break;
	    case CR:	doCR();
			break;
	    case VT:	doVT();
			break;
	    case HT:	chr = '\40';	/* tabs are worth one space on 4010 */
	    default:
			if (chr < 32 || chr == DEL) return;
			chstg[0] = chr; 
			chstg[1] = '\0';
			if ((alphx+fontwidth) > ALTRIGHTMARGIN) {
			    doCR();
			    doLF();
			}
			charstr(chstg);
			setAlphCurs(alphx + fontwidth,alphy);
			redrawviscurs();
			break;
	}
}

/* **********************************************************************
*									*
*		alpha cursor positioning				*
*									*
* ******************************************************************** */

redrawviscurs()
{
    writemask(CURSMASK);
    color(BLACK);
    rectfi(visx, visy-2, visx+xcursincr, visy+ycursincr);
    color(CURSCODE);
    rectfi(alphx, alphy-2, alphx+xcursincr, alphy+ycursincr);
    writemask(NORMALMASK);
    color(GREEN);
    visx = alphx; visy = alphy;
}

/* **********************************************************************
*									*
*		alpha coords positioning				*
*									*
* ******************************************************************** */
setAlphCurs(x, y)
int x,y;
{
    /* check bounds */
    x = MAX(x,0);
    y = MAX(y,0);
    x = MIN(x,xmaxscreen-fontwidth);
    y = MIN(y,ymaxscreen-fontheight);

    alphx = x;  alphy = y;
    
    redrawviscurs();

    cmov2i(x,y);
}

homeAlphCurs()
{
    
    inleftcolumn = TRUE;
    setAlphCurs(0, ymaxscreen-fontheight);
    curx = alphx; cury = alphy;

}

/* **********************************************************************
*									*
*	ESC,ENQ -> send status and coords to host.			*
*	ESC,SUB -> (1) put up crosshair (2) readhost for ESC,ENQ	*
*		   (3) flush stream from the host (4) send coords.	*
*									*
* ******************************************************************** */
procALesc()
{
    switch (gethostchar()) {
	case FF:	clearall();
			homeAlphCurs();
			return;
	case ENQ:	sendstatus(alphx, alphy, ALPHAMODE);
			return;
	case SUB:	processSUB();
			return;
    }
}

/* **********************************************************************
*									*
*		This does the back space processing. It worries about	*
*		columns etc.						*
*									*
* ******************************************************************** */
doBS()
{    if ((alphx-fontwidth) < 0)
	return;
    setAlphCurs(alphx-fontwidth, alphy);
}

/* **********************************************************************
*									*
*		This does the vertical tab processing.			*
*									*
* ******************************************************************** */
doVT()
{
    setAlphCurs(alphx, MIN(alphy+fontheight,ymaxscreen-fontheight));
}

/* **********************************************************************
*									*
*		This does the line feed processing. It worries about	*
*		columns etc.						*
*									*
* ******************************************************************** */
doLF()
{
    int y;

    y = alphy - fontheight;

    if (y < 0) {
	if (inleftcolumn) {
	    inleftcolumn = FALSE;
	    setAlphCurs(ALTLEFTMARGIN, ymaxscreen-fontheight);
	    return;
	}
	else {
	    homeAlphCurs();
	    return;
	}
    }
    setAlphCurs(alphx, y);
}

doCR()
{
    setAlphCurs(inleftcolumn ? LEFTMARGIN : ALTLEFTMARGIN, alphy);
}


/* **********************************************************************
*									*
*		Put up cross hair and get the key stroke		*
*									*
* ******************************************************************** */
char crosshair()
{
    char response;
    short data;

    oldx = crossx;
    oldy = crossy;
    /* the only device queued is the keyboard */
    qdevice(KEYBD);   
    qreset();				/* empty queue */

    pushmatrix();
    pushviewport();
    pushattributes();

    drawcrosshair();

    while (!(qtest())) {		/* wait on a keystroke */
	if (!(oldx == crossx && oldy == crossy)) {
	    drawcrosshair();
	    gsync();
	    gsync();
	    gsync();
	}
	oldx = crossx; oldy = crossy;
	crossx = getvaluator(MOUSEX);
	crossy = getvaluator(MOUSEY);
	if (!(oldx == crossx && oldy == crossy))
	    erasecrosshair();
    }
    erasecrosshair();
    gsync();

    qread(&data);
    response = data;

    qreset();				/* empty queue */

    popattributes();
    popviewport();
    popmatrix();

    unqdevice(KEYBD);

    return (response);
}   
/* **********************************************************************
*									*
*		make objs 						*
*									*
* ******************************************************************** */

drawcrosshair()
{
	writemask(CROSSHAIRMASK);
	color(CROSSHAIRCODE);
	move2i(crossx, 0);
	draw2i(crossx, ymaxscreen);
	move2i(0, crossy);
	draw2i(xmaxscreen, crossy);
}

erasecrosshair()
{
 	color(ERASECROSSHAIRCODE);
	move2i(oldx, 0);
	draw2i(oldx, ymaxscreen);
	move2i(0, oldy);
	draw2i(xmaxscreen, oldy);
}

mapplanes(code, umask, r, g, b)
unsigned int code, umask;
RGBvalue r,g,b;
{   unsigned short mask;
    register int incr, adr;
    unsigned int allOnes = (1 << getplanes()) - 1;

    if(!umask)
	return;
    mask = umask;

    for ( incr=1; incr&mask; incr <<= 1 ) ;
		    /* skip least significant 0s for optimum increment */
    for ( adr = code; adr <= allOnes; adr += incr)
    	if ( (adr&mask) == code ) {
 	   mapcolor(adr,r, g, b);
	}
}

#endif not GL1
