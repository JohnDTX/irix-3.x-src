/**************************************************************************
 *									  *
 * 		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/
#include "sys/types.h"
#include "shmem.h"
#include "window.h"
#include "gf2.h"
#include "glerror.h"
#include "get.h"
#include "dcdev.h"
#include "uc4.h"

/*	
 * This file contains the routines implementing the process
 * which triggers actions dependent upon the display retrace interrupt
 * from the FBC.  Although the FBC gets these interrupts 60 times a
 * second, we are only interested in every other one since the display
 * refresh is interlaced.
 */

#define ERROR -1

long	gl_timeout = (15*60*67); /* fifteen minutes */

typedef struct  {
    short count;
    short duration;
    char nextmap;
    char orgmap;
} bmap;

static bmap maps[16];
static short curmap = 0;	/* wich map is current for cyclemap */
short gl_mapcount = 0;		/* how many maps are cycling */

/*
 * 	gl_initretrace - called once on startup.
 *
 */
gl_initretrace()
{
    register short i;
    register retrevent *event;
    register short pri;

    pri = spl7();
    event = &gl_retrevents[0];
    for (i = 0; i < MAX_RETRACE_EVENTS; i++) {
	event->type = 0;
	event++;
    }
    splx(pri);
}

retrace_softintr()
{
    register short i;
    register retrevent *event;
    static short doqueuecount = 0;

/* do nice mapping of colors */
    gl_domapcolors();

/* do blinking of colors */
    event = &gl_retrevents[0]; 
    for (i = 0; i < MAX_RETRACE_EVENTS; i++) {
	if ((event->type != 0) && (--(event->count) == 0)) {
	        event->count = event->repeat;
		if (event->type == BLINK_EVENT) {
		    if (event->data.colors.currentcolor == 0)
			mapcolor(event->data.colors.colorindex,
					 event->data.colors.r0,
					 event->data.colors.g0,
					 event->data.colors.b0);
		    else
			mapcolor(event->data.colors.colorindex,
					 event->data.colors.r1,
					 event->data.colors.g1,
					 event->data.colors.b1);
		    event->data.colors.currentcolor =
					    1-event->data.colors.currentcolor;
		} else if (event->type == TIMER_EVENT) 
		   gr_qenter(event->ic,event->data.timer.dnum,gl_framecount);
	}
	event++;
    }

/* switch to new map if we have timed out */
    if (gl_mapcount != 0) {
	register bmap *mapp = &maps[curmap];
	if(--(mapp->count) <= 0) {
	    mapp = &maps[curmap = mapp->nextmap];
	    mapp->count = mapp->duration;
	    gl_setmap(curmap);
	}
    }

/*
 * Advance global time counter.  If we haven't taken any input events in the
 * last few minutes, then blank the screen (saves phosphor).  Moving the
 * mouse, touching a mouse button, or touching the keyboard will cause the
 * screen to re-appear.
 */
    gl_framecount++;
    if (!gl_isblanked && (gl_timeout != 0)
	&& (gl_framecount - gl_lastupdate > gl_timeout))
	    kblankscreen(1);
    else {
	if (gl_isblanked < 0) {
	    if (!(gl_fbcstatus & HOSTFLAG))
		kblankscreen(0);
	}
    }

/* unblock anyone waiting on retrace */
    retraceevent();

/* check the mouse */
    mousetick();		

/* check valuators */
    if(++doqueuecount >= 3) {   
	DoQueueValuators();
	doqueuecount = 0;
    }

/* light pen poll */
    if(havelpen)
	lpentick();
}

void swapinterval(n)
short n;
{
    register struct inputchan *ic = getic();

    if(!ic)
	return;
    ic->ic_SwapInterval = (n > 0 ? n : 0);
    if(ic->ic_SwapInterval >= gl_MaxSwapInterval)
	gl_MaxSwapInterval = ic->ic_SwapInterval;
    else
	gl_MaxSwapInterval = gl_calcmaxsi();
    if(gl_SwapCount > gl_MaxSwapInterval)
	gl_SwapCount = gl_MaxSwapInterval;
}

gl_initblinkevents()
{
    register short i;
    register retrevent *re = &gl_retrevents[0];

    for (i = 0; i < MAX_RETRACE_EVENTS; i++) {
	if (re->type == BLINK_EVENT)
	    gl_deleteretraceevent(re);
	re++;
    }
}

gl_deletetimerevents(ic)
struct inputchan *ic;
{
    register short i;
    register retrevent *re = &gl_retrevents[0];

    for (i = 0; i < MAX_RETRACE_EVENTS; i++) {
	if (re->ic == ic)
	    gl_deleteretraceevent(re);
	re++;
    }
}

gl_addretraceevent(event)
register retrevent *event;
{
    register short i;
    register retrevent *re;
    register short pri;

    re = &gl_retrevents[0];
    for (i = 0; i < MAX_RETRACE_EVENTS; i++) {
	if (re->type == 0) 
	    break;
	re++;
    }
    if (i == MAX_RETRACE_EVENTS) {
	gl_ErrorHandler(ERR_MAXRETRACE, WARNING, 0);
	return 0;
    }

    pri = spl5();
    *re = *event;
    if(event->type == BLINK_EVENT) {
	re = &gl_retrevents[0];
	for (i = 0; i < MAX_RETRACE_EVENTS; i++) {
	    if(re->type == BLINK_EVENT) {
		re->data.colors.currentcolor = 0;
		re->count = 1;
	    }
	    re++;
	}
    }
    splx(pri);
    return(1);
}

gl_deleteretraceevent(event)
register retrevent *event;
{
    register i;
    register retrevent *re = &gl_retrevents[0];

    for (i = 0; i < MAX_RETRACE_EVENTS; i++) {
	if (re->type == event->type)
	    switch (re->type) {
		case BLINK_EVENT:
		    if (re->data.colors.colorindex == 
					event->data.colors.colorindex) {
		    /* restore the old colors: */
			mapcolor(re->data.colors.colorindex,
					 re->data.colors.r0,
					 re->data.colors.g0,
					 re->data.colors.b0);
			re->type = 0;
			return 1;
		    }
		    break;
		case TIMER_EVENT:
		    if( (re->ic  == event->ic )  &&
		           (re->data.timer.dnum == event->data.timer.dnum) ) {
		        re->type = 0;
		        return 1;
		    }
		    break;
	    }
	re++;
    }
    return 0;
}

void blink(rate, colorindex, red, green, blue)
register short rate, colorindex;
short red, green, blue;
{
    short oldred, oldgreen, oldblue;
    retrevent revent;

/* in case it was already blinking: */

    revent.data.colors.colorindex = colorindex;
    revent.type = BLINK_EVENT;
    gl_deleteretraceevent(&revent);
    if(rate == 0)
	return;

/* get the old color values for this index: */

    getmcolor(colorindex, &oldred, &oldgreen, &oldblue);

    revent.data.colors.currentcolor = 0;
    revent.data.colors.r0 = oldred;
    revent.data.colors.g0 = oldgreen;
    revent.data.colors.b0 = oldblue;

    revent.data.colors.r1 = red;
    revent.data.colors.g1 = green;
    revent.data.colors.b1 = blue;

    revent.count = revent.repeat = rate;
    revent.ic = 0;			/* no specific owner */
    gl_addretraceevent(&revent);
}

modtimer(dnum,val)
{
    retrevent revent;
    register struct inputchan *ic = getic();
    register short rate;

    revent.type = TIMER_EVENT;
    revent.ic = ic;
    revent.data.timer.dnum = dnum;
    gl_deleteretraceevent(&revent);
    ic->ic_proctimers[dnum-TIMOFFSET].doqueue = val;
    if(val) {
	    rate = ic->ic_proctimers[dnum-TIMOFFSET].noise;
	    revent.count = revent.repeat = rate;
	    gl_addretraceevent(&revent);
    }
}

void gl_initcyclemap()
{
    register i;

    for (i = 0; i < 16; i++)
	cyclemap(0, i, 0);
}

void cyclemap(duration, map, nextmap)
register short duration, map, nextmap;
{
    bmap *mapp;
    short gl_getmap();

    if((map < 0 || 16 <= map) || (nextmap < 0 || 16 <= nextmap)) {
	gl_ErrorHandler(ERR_CYCLEMAP, WARNING, 0);
	return;
    }
    mapp = &maps[map];
    if (duration == 0) {
	if(mapp->duration != 0)	{	/* was this one in use before */
	    gl_mapcount--;
	    mapp->nextmap = mapp->orgmap; /* so when we hit it it will reset */
	}
	mapp->count = mapp->duration = duration;
    } else {
	if(mapp->duration == 0)		/* is this one new */
	    gl_mapcount++;
	mapp->nextmap = nextmap;
	mapp->orgmap = gl_getmap();
	mapp->count = mapp->duration = duration;
	gl_setmap(map);
    }
}

gl_setsinglebuffer()
{
    register struct inputchan *ic = getic();
    register struct shmem *sh;

    if(!ic || !(sh = ic->ic_shmemptr))
	return;
    if (ic->ic_displaymode == MD_DOUBLE) {
	if((--gl_numdoublebufferers == 0) && (!gl_numrgbs)) {
	    gl_sbtxport();
	    gl_mode(MD_SINGLE);
	}
    } else if (ic->ic_displaymode == MD_RGB) {
	if(--gl_numrgbs == 0)
	    if(gl_numdoublebufferers) {
		gl_dbtxport();
		gl_mode(MD_DOUBLE);
	    } else {
		gl_sbtxport();
		gl_mode(MD_SINGLE);
	    }
    }
    ic->ic_displaymode = MD_SINGLE;
    sh->ws.curatrdata.myconfig &= ~(((UC_DOUBLE | UC_SWIZZLE)<<16) |
			DISPLAYA | DISPLAYB | UPDATEA | UPDATEB);
    sh->ws.curatrdata.myconfig |= (gl_cfr & (((UC_DOUBLE | UC_SWIZZLE)<<16) |
					DISPLAYA | DISPLAYB));
    sh->ws.curatrdata.myconfig |= (UPDATEA | UPDATEB | VIEWPORTMASK);
    if(ic == gl_wmport) {
	if(gl_cfr & (UC_DOUBLE << 16))
	    sh->ws.bitplanemask = gl_kdbwritemask;
	else
	    sh->ws.bitplanemask = gl_kwritemask;
    } else {
	if(gl_cfr & (UC_DOUBLE << 16))
	    sh->ws.bitplanemask = gl_userdbwritemask;
	else
	    sh->ws.bitplanemask = gl_userwritemask;
    }
    setconfig(sh->ws.curatrdata.myconfig);
}

gl_setdoublebuffer()
{
    register struct inputchan *ic = getic();
    register struct shmem *sh;

    if(!ic || !(sh = ic->ic_shmemptr))
	return;
    if (ic->ic_displaymode == MD_SINGLE) {
	if((gl_numdoublebufferers++ == 0) && (gl_numrgbs == 0)) {
	    gl_dbtxport();
	    gl_mode(MD_DOUBLE);
	}
    } else if (ic->ic_displaymode == MD_RGB) {
	++gl_numdoublebufferers;
	if(--gl_numrgbs == 0) {
	    gl_dbtxport();
	    gl_mode(MD_DOUBLE);
	}
    } else if (ic->ic_displaymode != MD_DOUBLE) {
	if((gl_numdoublebufferers++ == 0) && (gl_numrgbs == 0)) {
	    gl_dbtxport();
	    gl_mode(MD_DOUBLE);
	}
    }
    ic->ic_displaymode = MD_DOUBLE;
    sh->ws.curatrdata.myconfig &= ~(((UC_DOUBLE | UC_SWIZZLE)<<16) |
				DISPLAYA | DISPLAYB | UPDATEA | UPDATEB);
    sh->ws.curatrdata.myconfig |= (gl_cfr & (((UC_DOUBLE | UC_SWIZZLE)<<16) |
				DISPLAYA | DISPLAYB | UPDATEA | UPDATEB)) |
				VIEWPORTMASK;
    if(ic == gl_wmport) {
	if(gl_cfr & (UC_DOUBLE << 16))
	    sh->ws.bitplanemask = gl_kdbwritemask;
	else
	    sh->ws.bitplanemask = gl_kwritemask;
    } else {
	if(gl_cfr & (UC_DOUBLE << 16))
	    sh->ws.bitplanemask = gl_userdbwritemask;
	else
	    sh->ws.bitplanemask = gl_userwritemask;
    }
    setconfig(sh->ws.curatrdata.myconfig);
}

gl_setrgbmode()
{
    register struct inputchan *ic = getic();
    register struct shmem *sh;

    if(!ic || !(sh = ic->ic_shmemptr))
	return;
    if (ic->ic_displaymode == MD_DOUBLE)
	--gl_numdoublebufferers;
    if (ic->ic_displaymode != MD_RGB) {
	if(gl_numrgbs++ == 0) {
	    gl_rgbtxport();
	    gl_mode(MD_RGB);
	}
    }
    ic->ic_displaymode = MD_RGB;
    sh->ws.curatrdata.myconfig &= ~(((UC_DOUBLE | UC_SWIZZLE)<<16) |
				DISPLAYA | DISPLAYB | UPDATEA | UPDATEB);
    sh->ws.curatrdata.myconfig |= (gl_cfr & (((UC_DOUBLE | UC_SWIZZLE)<<16) |
				DISPLAYA | DISPLAYB));
    sh->ws.curatrdata.myconfig |= (UPDATEA | UPDATEB | VIEWPORTMASK);
    sh->ws.bitplanemask = gl_kwritemask;
    setconfig(sh->ws.curatrdata.myconfig);
}

gl_getnumdbers()
{
    if(gl_numrgbs)
	return 0;
    else
	return gl_numdoublebufferers;
}

gl_getnumrgbers()
{
    return gl_numrgbs;
}

gl_setonemap()
{
    gl_setmap(0); 
    gl_dcr &= ~DCREGADRMAP;
    DCflags = gl_dcr;
    setbitplanemasks(0);
}

gl_setmultimap()
{
    gl_dcr |= DCREGADRMAP;
    DCflags = gl_dcr;
    setbitplanemasks(0);
}

gl_setmap (mapnumber)
short mapnumber;
{
    curmap = mapnumber;
    if (gl_dcr | DCREGADRMAP) {
	gl_dcr &= ~DCNumToReg (DCMAPNUM-1);	/* clear mapnumber field */
	gl_dcr |= DCNumToReg (mapnumber);	/* set to new mapnumber	 */
	gl_dcr ^= DCMBIT;			/* wiggle bit for glasses */
	DCflags = gl_dcr;
	gl_dcr ^= DCMBIT;			/* wiggle back for glasses */
	DCflags = gl_dcr;
    }
}

short gl_getmap ()
{
    if(gl_dcr & DCMULTIMAP)
	return (DCRegToNum (gl_dcr));
    else
	return 0;
}

/* HACK ALERT HACK ALERT HACK ALERT */
#ifdef	V
#define DCRMODES	*(unsigned char *)0x23e    /* EPROM's common area */
#define DCRFLAGS	*(unsigned short *)0x228
char	_dcrsize = 0;					/* best guess */
#endif
#ifdef	UNIX
char	_dcrmodes;			/* filled in by locore.s */
char	_dcrsize;			/* filled in by locore.s */
short	_dcrflags;
#define	DCRMODES	_dcrmodes
#define	DCRFLAGS	_dcrflags
#endif

static short gl_monitor;

/*
 * gl_montobits:
 *	- convert from a defined number (get.h) to the dcr bits needed.
 */
short
gl_montobits(m)
register short m;
{
    register short newbits;

    switch (m & ~MONSPECIAL) {	/* don't consider special bit for now */
	case HZ30:
	    newbits = 0;
	    break;
	case HZ60:
	    newbits = DCPIPE4;
	    break;
	case PAL:			/* identical dcr bits */
	case NTSC:
	    newbits = DCOPTCLK;
	    break;
	case HZ50:
	    newbits = DCD1K | DCPIPE4;
	    break;
	case MONA:
	    newbits = DCOPTCLK | DCPIPE4;
	    break;
	case MONB:
	    newbits = DCD1K;
	    break;
	case MONC:
	    newbits = DCD1K | DCOPTCLK;
	    break;
	case MOND:
	    newbits = DCD1K | DCOPTCLK | DCPIPE4;
	    break;
	default:
	    gl_ErrorHandler (ERR_SETMONITOR,WARNING,0);
    }
    return newbits;
}

/*
 * gl_bitstomon:
 *	- convert from dcr bits to monitor number (get.h type).
 */
short
gl_bitstomon(bits)
register int bits;
{
    short mon;

    switch (bits & ~DCPROM) {
	case 0:
		mon = HZ30;
		break;
	case DCPIPE4:
		mon = HZ60;
		break;
	case (DCPIPE4 | DCD1K):
		mon = HZ50;
		break;
	case DCOPTCLK:
		mon = NTSC;
		if(_dcrsize == 1)
		    mon = PAL;
		break;
	case (DCPIPE4 | DCOPTCLK):
		mon = MONA;
		break;
	case DCD1K:
		mon = MONB;
		break;
	case (DCD1K | DCOPTCLK):
		mon = MONC;
		break;
	case (DCD1K | DCOPTCLK | DCPIPE4):
		mon = MOND;
		break;
	default:
	    gl_ErrorHandler (ERR_SETMONITOR,WARNING,0);
    }
    return mon;
}		

/*
 * gl_init_dcr:
 *	- set global dcr value to boot proms value
 */
gl_init_dcr()
{
    if (DCRFLAGS & 0x200)
	gl_dcr = (DCRMODES & 0xf0) << 7;
    else
	gl_dcr = (DCRMODES & 0xf) << 11;

/* now make the user's version correspond */
    gl_monitor = gl_bitstomon(gl_dcr);
}

gl_setmonitor(m)
    register short m;
{
    register short sel0, sel1, newbits;

    sel0 = (DCRMODES & 0xf) <<11;
    sel1 = (DCRMODES & 0xf0) <<7;
				/* eprom code reflects the 2 choices */
    newbits = gl_montobits(m);
    if ( (sel0 & ~DCPROM) == newbits) {	/* desired selection installed */
	gl_monitor = m;
	gl_dcr &= ~(DCPIPE4 | DCPROM | DCOPTCLK | DCD1K | DCMBIT);
	gl_dcr |= newbits;
	if (sel0 & DCPROM) gl_dcr |= DCPROM;
	if (m & MONSPECIAL)
		gl_dcr |= DCMBIT;
    } else if ( (sel1 & ~DCPROM) == newbits) {
	gl_monitor = m;
	gl_dcr &= ~(DCPIPE4 | DCPROM | DCOPTCLK | DCD1K | DCMBIT);
	gl_dcr |= newbits;
	if (sel1 & DCPROM) gl_dcr |= DCPROM;
	if (m & MONSPECIAL)
		gl_dcr |= DCMBIT;
    }
    DCflags = gl_dcr;
}

short gl_getmonitor()
{
    return gl_monitor;
}

gl_getothermonitor()
{
    register short sel0, sel1, newbits;

    sel0 = ((DCRMODES & 0xf) <<11) & ~DCPROM;
    sel1 = ((DCRMODES & 0xf0) <<7) & ~DCPROM;
				/* eprom code reflects the 2 choices */
    newbits = gl_montobits(gl_monitor);
    if(sel0 == newbits)
	return gl_bitstomon(sel1);
    else if(sel1 == newbits)
	return gl_bitstomon(sel0);
    else
	gl_ErrorHandler (ERR_SETMONITOR,WARNING,0);
}

gl_scrtimeout(n)
{
    gl_timeout = n;
}

gl_domapcolors()
{
    register short n, ptr;
    register short index;
    register short *mptr;
    register short *sptr;
    register unsigned char *rptr, *gptr, *bptr;

    if( gl_shmemptr->outp != gl_shmemptr->inp) {
	ptr = gl_shmemptr->outp;
	n = gl_shmemptr->inp - ptr;
 	if(n<0) n += MAPTABSIZE;
	ptr++;
	ptr &= MAPTABSIZE-1;
	sptr = gl_shmemptr->indices+ptr;
	rptr = gl_shmemptr->rs+ptr;
	gptr = gl_shmemptr->gs+ptr;
	bptr = gl_shmemptr->bs+ptr;
	if (gl_dcr & DCMULTIMAP) {
	    DCflags = gl_dcr | DCBUSOP;
	    while(n--) {
		index = *sptr++ & DCMULTIMASK;
		mptr = DCramRedAddr(index);
		if(!((*UCRAddr) & UCR_VERTICAL))
		    break;
		mptr[0] = *rptr++;
		mptr[DCRAMRED>>1] = *gptr++;
		mptr[DCRAMRED] = *bptr++;
		if(++ptr >= MAPTABSIZE) {
		    ptr = 0;
		    sptr = gl_shmemptr->indices;
		    rptr = gl_shmemptr->rs;
		    gptr = gl_shmemptr->gs;
		    bptr = gl_shmemptr->bs;
		}
	    }
	} else {
	    while(n--) {
		index = *sptr++;
		DCflags = gl_dcr | DCBUSOP | DCIndexToReg (index);
		index &= DCMULTIMASK;
		mptr = DCramRedAddr(index);
		if(!((*UCRAddr) & UCR_VERTICAL))
		    break;
		mptr[0] = *rptr++;
		mptr[DCRAMRED>>1] = *gptr++;
		mptr[DCRAMRED] = *bptr++;
		if(++ptr >= MAPTABSIZE) {
		    ptr = 0;
		    sptr = gl_shmemptr->indices;
		    rptr = gl_shmemptr->rs;
		    gptr = gl_shmemptr->gs;
		    bptr = gl_shmemptr->bs;
		}
	    }
	}
	DCflags = gl_dcr;
	if(--ptr < 0) ptr += MAPTABSIZE;
	gl_shmemptr->outp = ptr;
    } 
}

swapconfig(ocfr)
register int ocfr;
{
	register int config;

	config = ocfr & ~(UPDATEA | UPDATEB | DISPLAYA | DISPLAYB);
	if (ocfr & DISPLAYA)			/* pgb sept 5 1985 */
		config |= DISPLAYB;
	else
		config |= DISPLAYA;
	if (ocfr & UPDATEA)
		config |= UPDATEB;
	if (ocfr & UPDATEB)
		config |= UPDATEA;
	return config;
}
