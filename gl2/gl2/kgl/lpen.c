/*
 *	lpen -
 *		Lightpen Service Routines for Multibus board Light Pen
 *
 */
#include "addrs.h"
#include "device.h"
#include "shmem.h"
#define DNR			/* Define for DNR algorithm */
#define LPENIOADDR	0x6000	/* Structure lpen located here	*/
#define	PENHIT		0x4
#define	VALID		0x8
#define TIPSWITCH	0x20
#define	BIGSNAP		64	/* Move to new value immediately */
#define	MIDSNAP		32	/* Move to new value at half speed */
#define	LOWSNAP		16	/* Move to new value at 1/3 speed */
				/* Default: Move at quarter speed */

#define ABS( a )	(((a) > 0) ? (a) : -(a))

struct lpen {
    union {
	unsigned long xxaccum;
	struct {
	    unsigned short xxaccumlow;
	    unsigned short xxaccumhigh;
	} xa;
    } x;
    unsigned short ystart;
    unsigned short yend_xcount;
    unsigned short status;
};

#ifdef UNIX
#define	    LPEN  ((struct lpen *)(_MBIO_VBASE + LPENIOADDR))
#endif

#ifdef V
#define	    LPEN  0
#endif

#define	xaccum x.xxaccum
#define	xaccumlow x.xa.xxaccumlow
#define	xaccumhigh x.xa.xxaccumhigh

#define DEFAULT_XOFFSET	    233
#define DEFAULT_YOFFSET	     29
#define DEFAULT_XLINELENGTH 1023

static	int lpen_xoffset;    
static  int lpen_yoffset;
static  int lpen_yscreensize;
static  int lpen_xlinelength;
static  int fixlpen_yscreensize;
static  int fixlpen_xlinelength;
int lpenwarp = 0;
int lpen1delta = 1;
int lpen2delta = 2;
int lpen3delta = 3;
int lpen4delta = 4;

#define dx newx		/* register alloc hack */
#define dy newy

#define 	TOFIX(x)	((x)<<4)
#define 	FROMFIX(x)	(((x)+8)>>4)

int lpencheck( x, y, tipswitch )
register int *x, *y, *tipswitch;
{
    static int lpenfilterx;
    register int newx, newy, delta;
    register int cx, newys, newye;
    register struct lpen *p = LPEN;
    register valid;

    for(newx=0; newx<4000; newx++)
	if(p->status & VALID)
	    break;
    if ((p->status & (PENHIT|VALID)) == (PENHIT|VALID)) {
	valid = 1;
	newx = (((p->xaccumhigh&0xFFF)<<16)|p->xaccumlow);
/* Lets not divide by zero,  if thats the case the value is wrong anyway */
	if (p->yend_xcount>>8) 
	    newx = newx / (p->yend_xcount>>8);

	newys = p->ystart & 0xFFF;
	newye = p->yend_xcount & 0xff;
	if (newye < (newys & 0xff))
	    newye += ((newys & 0xf00) + 256);
	else if (newye == (newys & 0xff))
	    newye  = newys;
	else
	    newye += (newys & 0xf00);
	newy = (newys+newye)/2;

/* translate Make screen coordinates	*/
	newx -= lpen_xoffset; 
	newy -= lpen_yoffset;

/* Put 0 at bottom of screen,Lightpen Y counts up	*/
	newy = lpen_yscreensize - newy;
	if (newx < 0)
	    newx = 0;
	if (newx > lpen_xlinelength)
	    newx = lpen_xlinelength;
	if (newy < 0)
	   newy = 0;
	if (newy > lpen_yscreensize)
	    newy = lpen_yscreensize;

/* DNR Algorithm */
        if(lpenwarp) {
	    cx = lpenfilterx;
	    dx = TOFIX(newx) - cx; /* Reuse newx/newy as deltas */
	    delta = ABS(dx);

/* Bias the DNR algorithm based on delta */
	    if(delta > TOFIX(BIGSNAP))
		delta = lpen1delta;
	    else if(delta > TOFIX(MIDSNAP))
		delta = lpen2delta;
	    else if(delta > TOFIX(LOWSNAP))
		delta = lpen3delta;
	    else
		delta = lpen4delta;
	    cx += dx/delta;

/* set point to screenbounds	*/
	    if (cx < 0)
		cx = 0;
	    if (cx > fixlpen_xlinelength)
		cx = fixlpen_xlinelength;
	    lpenfilterx = cx;
	    *x = FROMFIX(cx);
	} else
	    *x = newx;
	*y = newy;
    } else
	valid = 0;
    p->status = 0; 

/* Set value for Tip Switch	*/
    if (p->status & TIPSWITCH)
       *tipswitch = 1;
    else
       *tipswitch = 0;
    return valid;
}		

gl_lpenset(val)
int val;
{
    if(havelpen)
        lpeninit(val,0,0,0);
}

lpeninit(deltaxoff, deltayoff, deltaxlength, deltayscreensize)
int deltaxoff, deltayoff, deltaxlength, deltayscreensize;
{
    register struct lpen *p = LPEN;

    lpen_xoffset = DEFAULT_XOFFSET + deltaxoff;
    lpen_yoffset = DEFAULT_YOFFSET + deltayoff;
    lpen_xlinelength = XMAXSCREEN + deltaxlength;
    lpen_yscreensize = YMAXSCREEN + deltayscreensize;
    fixlpen_xlinelength = TOFIX(lpen_xlinelength);
    fixlpen_yscreensize = TOFIX(lpen_yscreensize);

/* any write to lpen board resets Hit Status */
    p->status = 0; 
}

static int lpx, lpy, lps, lpv;
static int oldlpx, oldlpy, oldlps, oldlpv;

lpentick()
{
    register short didsomething = 0;
    static short firsted;

    if (!firsted) {
	lpeninit(0,0,0,0);
	lpx = lpy = lps = -1;
  	firsted++;
    } 
    lpv = lpencheck(&lpx,&lpy,&lps);
    if(lpv) {
	if (lpx != oldlpx) {
	    ChangeValuator(LPENX,lpx);
	    oldlpx = lpx;
	    didsomething = 1;
	}
	if (lpy != oldlpy) {
	    ChangeValuator(LPENY,lpy);
	    oldlpy = lpy;
	    didsomething = 1;
	}
    }
    if(lpv != oldlpv) {
	ChangeButton(LPENVALID,lpv);
	oldlpv = lpv;
	didsomething = 1;
    }
    if (lps != oldlps) {
	ChangeButton(LPENBUT,lps);
	oldlps = lps;
	didsomething = 1;
    }
    if (didsomething) {
	gl_lastupdate = gl_framecount;
	kblankscreen(0);
    }
}
