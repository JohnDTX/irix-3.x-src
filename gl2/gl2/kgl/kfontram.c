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
#include "gltypes.h"
#include "immed.h"
#include "dcdev.h"
#include "uc4.h"
#include "gr.h"
#include "device.h"
#include "errno.h"

/* the hole should always be between low and high */
static	long	top = (16*1024)-1;
static	long	high = (16*1024)-1;	/* last empty slot of hole */
static	long	low = 0;		/* first empty slot of hole */
static	long	fkernel = 0;		/* last slot used by kernel */

/*
 * move whatever is in the fontram around to make the biggest possible
 * hole from the current user on up, setting it to at least length
 * or returning error (-1).
 */
gl_setfontmem(length)
    short length;
{
    struct inputchan *ic = getic();

    return gl_ksetfontmem(length,ic);
}

gl_ksetfontmem(length,ic)
    short length;
    register struct inputchan *ic;
{
    register struct shmem *sh;
    long ctx;
    short holesize;
    short howmuch;
    short oldhigh;

    if(ic == 0) 
	return;

    oldhigh = high;
    ctx = gr_setshmem(ic->ic_oshandle);		/* save shmem state */
    length = (length + 255) & ~255;		/* keep lengths alingable */
    sh = ic->ic_shmemptr;
    holesize = high - low + 1;
    if((length - sh->ws.fontramlength) > holesize) {
	gr_restoreshmem(ctx);            /* restore shmem state */
	return -1;
    }
    
    /* figure which way to move so hole is just after current */
    if(sh->ws.fontrambase > high){
	/* move this and other fonts down, hole up */
	howmuch = (sh->ws.fontrambase + sh->ws.fontramlength - 1) - high;
	gl_copyfont(high + 1, low, howmuch);
	low += howmuch + length - sh->ws.fontramlength;
	high += howmuch;
    } else if(sh->ws.fontrambase < low){
	/* move other fonts up, hole down */
	howmuch = low - (sh->ws.fontrambase + sh->ws.fontramlength);
	gl_copyfont(sh->ws.fontrambase + sh->ws.fontramlength,
		    (high - howmuch) + 1, howmuch);
	low = sh->ws.fontrambase + length;
	high -= howmuch;
    } else {
	iprintf("kernel fontram managment messed up (some font inside hole)\n");
	gr_restoreshmem(ctx);            /* restore shmem state */
        return -1;
    }
    gl_adjustbase(sh->ws.fontrambase + sh->ws.fontramlength, oldhigh, holesize);
    sh->ws.fontramlength = length;
    gr_restoreshmem(ctx);            /* restore shmem state */
    return 0;
}

/* depends on static global values of low,high etc */
gl_adjustbase(start,fin,howmuch)
    short start;
    short fin;
    short howmuch;
{
    register short i;
    register struct shmem *sh;
    register struct inputchan *ic;
    long ctx;

    ic = &inchan[0];
    ctx = gr_setshmem(ic->ic_oshandle);	/* save shmem state */
    for(i=0; i<NINCHANS; i++, ic++) {
	if( (sh = ic->ic_shmemptr) ) {
	    (void) gr_setshmem(ic->ic_oshandle);

/* hack. need fin do this for all the saved ws's in the gfports as well: */
	    if(sh->ws.fontrambase>=start && sh->ws.fontrambase<=fin) {
		sh->ws.fontrambase  += howmuch;
		if(sh->ws.mytexcode < 0)	/* trying to be absolute 0 */
		    sh->ws.mytexcode  = -sh->ws.fontrambase;
		if(sh->ws.cursorbase > fkernel)
		    sh->ws.cursorbase  += howmuch;
		if(sh->ws.fontbase > fkernel)
		    sh->ws.fontbase  += howmuch;
	    }
	    else
	    if(sh->ws.fontrambase>fin && sh->ws.fontrambase<start) {
		sh->ws.fontrambase  -= howmuch;
		if(sh->ws.mytexcode < 0)	/* trying to be absolute 0 */
		    sh->ws.mytexcode  = -sh->ws.fontrambase;
		if(sh->ws.cursorbase > fkernel)
		    sh->ws.cursorbase  -= howmuch;
		if(sh->ws.fontbase > fkernel)
		    sh->ws.fontbase  -= howmuch;
	    }
	}
    }
    gr_restoreshmem(ctx);            /* restore shmem state */
}

gl_copyfont(src,dest,nwds)
    short src,dest,nwds;
{
    im_setup;

/* ???????????? */

    im_passcmd(1, FBCpopname);
    im_passcmd(4,FBCcopyfont);
    im_outshort(src);
    im_outshort(dest);
    im_last_outshort(nwds);
    im_cleanup;
}

gl_fontslot()
{
    if((high - low) > 256) {
	low = (low + 256) & ~255;	/* next lowest aligned slot */
	return low - 256;
    }
    return low;				/* we've run out so double up */
}

gl_lowfont(size)
{
/* this is a hack, should slide everything up and put hole above kernel space */
/* only used by kernel when it sets the font up */
    low = (size + 255) & ~255;
    fkernel = low;
}
