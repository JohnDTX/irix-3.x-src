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

#ifdef	ASSERT
#ifdef	UNIX
#include <machine/pte.h>
#endif
#endif

extern	short txneedsrepaint;
struct	inputchan *syncwaiters;
long	numreadytoswap;
short	gl_swapininter;             	/* HACK FOR NOW */
short	gl_fastfeed;
extern	short gl_mapcount;
static short gl_alive;

char gl_version[] = GLVERSION;		/* so adb and strings can find it */

/*
 * gr_allocpieces:
 *	- alloc a bunch of pieces
 */
struct piece *
gr_allocpieces(n)
    register int n;
{
    register struct piece *head, *pp;

    head = 0;
    while (n--) {
	if (gl_piecefreelist) {
	    pp = gl_piecefreelist;
	    gl_piecefreelist = pp->p_next;
	    pp->p_next = head;
	    head = pp;
	    continue;
	}
	panic("out of pieces");
    }
    return head;
}

/*
 * gr_freepieces:
 *	- free a list of pieces
 */
gr_freepieces(pp)
    register struct piece *pp;
{
    register struct piece *next;

    while (pp) {
	next = pp->p_next;
	pp->p_next = gl_piecefreelist;
	gl_piecefreelist = pp;
	pp = next;
    }
}

#ifdef RV_8BIT			/* reverse video & 8 bit data chars */
u_char display_flags[MAXROWS][MAXCOLS];
#endif

gr_init()
{
	gr_doreset(1);
}

gr_reset()
{
	gr_doreset(0);
}

gr_doreset(fullReset)
    int fullReset;
{
    register struct gfport *gf;
    register struct shmem *sh;
    register short i;

    /* set the default charwidth, height, and descender */
    if (fullReset)
	set_defont_sizes();
    gl_dialport = 3;
    gl_bpadport = 3;

    /* init piece free list */
    if (fullReset) {
	register struct piece *p;

	gl_piecefreelist = NULL;
	p = &piece[0];
	for (i=0; i<NPIECES; i++, p++) {
	    p->p_next = gl_piecefreelist;
	    gl_piecefreelist = p;
	}
    }

    /* init gfports and inputchans */
    if (fullReset) {
	register struct inputchan *ic;

	gf = &gfport[0];
	for (i=0; i<NGFPORTS; i++, gf++) {
	    gf->gf_no = i;
	    gf->gf_ic = 0;
	}
	ic = &inchan[0];
	for (i=0; i<NINCHANS; i++, ic++) {
	    ic->ic_no = i;
	    ic->ic_shmemptr = 0;
	    ic->ic_next = 0;
	    ic->ic_tx = &txport[0];
	    ic->ic_oshandle = 0;
	    ic->ic_qreadwait = 0;
	    ic->ic_gf = 0;
	}
    }

    /* get to kernel's shared memory */
    gl_gfport = 0;
    gl_lock = 0;
    gl_curric = 0;
    gl_wmbutton = PADPF4;
    gl_wmport = 0;
    sh = gl_shmemptr = gl_kernelshmemptr;
    (void) gr_setshmem(0);

    /* initialize software state for console gfport */
    txinputchannel(0);
    gl_initretrace();
    gl_initinput(&inchan[0]);
    fbcshmem();
    gl_softkeyboard = kb_issoft();

    /* init hardware state (load microcode, etc.) */
    if (fullReset)
	ginit();
    else
	greset();
    color(0);
    rectfi(0, 0, XMAXSCREEN, YMAXSCREEN);

    /*
     * Initialize textports and assign memory for the display text (r_data).
     * This code reuses the once-only loaded microcode memory for hold the
     * display data.
     */
    if (fullReset) {
	extern short ucode[];
	register struct txport *tx = &txport[0];
	register char *cp = (char *)&ucode[0];
	register short j;

	for (i=0; i<NTXPORTS; i++, tx++) {
	    tx->tx_no = i;
	    for (j = 0; j < MAXROWS; j++) {
		tx->tx_display[j].r_data = cp;
#ifdef RV_8BIT
		tx->tx_display[j].r_flags = &display_flags[j][0];
#endif
		cp += MAXCOLS;
	    }
	    tx_init(i);
	}
	tx_open(0);
    } else {
	txport[0].tx_state = TX_ON | TX_REDISPLAY | TX_SCROLLED |
				TX_BORDER | TX_DRAWBORDER;
	tx_textport(&txport[0], 0, XMAXSCREEN, 0, YMAXSCREEN);
	tx_repaint(0);
    }
    ch_init();
    gl_alive = 1;
}

/*
 * gr_ok:
 *	- figure out if we can give the graphics to the named gfport
 */
gr_ok(ic)
	register struct inputchan *ic;
{
	if (!ic->ic_shmemptr)
		panic("gr_ok !shmemptr");
	if ((gl_gfport == ic) && (gl_lock || PIPEISBUSY))
		return 1;	/* already have hardware state */
    /*
     * See if all double buffered process's are waiting for a swapbuffer;
     * if so, then don't take the pipe till they have swapped.
     */
	if (gl_needtodoswaps && !(ic->ic_displaymode == MD_DOUBLE))
		return 0;
	if (gl_lock || PIPEISBUSY)
		return 0;	/* pipe locked by someone else */
	return 1;		/* we can get it */
}

/*
 * gr_switchstate:
 *	- get graphics hardware for a process
 */
gr_switchstate(ic)
	register struct inputchan *ic;
{
	register int pri;

	if (gl_gfport == ic) {		/* already have it */
		gl_shmemptr->isblanked = gl_isblanked | gl_blankmode;
		return;
	}
#ifdef	ASSERT
	if (gl_lock)
		panic("gr_switchstate gl_lock");
	if (PIPEISBUSY)
		panic("gr_switchstate pipebusy");
#endif

    /* block repaint code during save & restore */
	gl_lock = 1;
	if (gl_gfport)            /* save other guys state */
		saveeverything();
	pri = spl6();
	gl_gfport = ic;
	gl_shmemptr = ic->ic_shmemptr;
	(void) gr_setshmem(ic->ic_oshandle);
	splx(pri);
	restoreeverything();
	gl_shmemptr->isblanked = gl_isblanked | gl_blankmode;

    /* okay to repaint now */
	gl_lock = 0;
}

/*
 * gr_repaint:
 *	- called to do a repaint only if a previous repaint had been
 *	  blocked because of the pipe being busy
 */
gr_repaint()
{
    if (txneedsrepaint)
	tx_repaint(0);
}

/*
 * gr_alloc:
 *	- allocate shared memory for the current process
 *	- returns the users virtual address of the shared memory region
 */
gr_alloc()
{
    register struct inputchan *ic;
    register short i, portno;
    register int pri;

    if(!gl_alive)
	return -1;
    if ((ic = getic()) != 0) {	/* already have one */
	iprintf("already have gf port!\n");
	return 0;
    }
    pri = spl6();
    ic = &inchan[0];
    for (i=0; i<NINCHANS; i++, ic++) {
	if (!ic->ic_shmemptr && !ic->ic_holding) {
	    ic->ic_oshandle = gr_getoshandle();
	    gr_setgrhandle(ic->ic_oshandle, (long)ic);
	    splx(pri);
	    gl_initinput(ic);
#ifdef ASSERT
	    if(ic->ic_gf)
		panic("gr_alloc: new ic with gf already!!");
#endif
	    pri = spl6();
	    getgfport();	/* links gf onto end of ic_gf list */
	    ic->ic_gf->gf_ic = ic;
	    gf_init(ic->ic_gf);
	    if ((portno = gr_txport()) != -1)
	    	ic->ic_tx = &txport[portno];
	    else
		ic->ic_tx = 0;
	    splx(pri);
	    gr_shmeminit(ic);
	    if(!gl_wmport)
		gfinputchannel(ic->ic_gf->gf_no);
	    return ic->ic_no;
	}
    }
    splx(pri);
    return -1;
}

gf_init(gf)
register struct gfport *gf;
{
    register struct piece *pp;

    gf->gf_piecelist = pp = gr_allocpieces(1);
    gf->gf_llx = gf->gf_lly = pp->p_xmin = pp->p_ymin = 0;
    gf->gf_urx = pp->p_xmax = XMAXSCREEN;
    gf->gf_ury = pp->p_ymax = XMAXSCREEN;	/* GARY was right! */
}

gr_shmeminit(ic)
register struct inputchan *ic;
{
    register struct shmem *sh;
    struct shmem *saveshmem;
    register int pri;
    long ctx;

    sh = gr_getshmem();
    ic->ic_qreadwait = 0;
    ic->ic_shmemptr = sh;
#ifdef NOTDEF
    ic->ic_oshandle = gr_getoshandle();
#endif
    ic->ic_curoffsetx = 0;
    ic->ic_curoffsety = 15;
    ic->ic_sendwait = 0;
    ic->ic_swapwaiting = 0;
    ic->ic_bpadused = 0;
    ic->ic_dialused = 0;

/* initialize shared memory stuff */
    pri = spl5();
    ctx = gr_setshmem(ic->ic_oshandle);
    saveshmem = gl_shmemptr;
    gl_shmemptr = sh;
    sh->ws.softstacktop = sh->ws.matrixlevel = 0;
    sh->ws.hdwrstackbottom = 1;
    sh->ws.matrixstatep = 0;
    sh->ws.curatrdata.myconfig = gl_cfr;
    sh->ws.fontrambase = gl_fontslot();
    sh->ws.fontramlength = FONTRAM_STARTSIZE;
    sh->inputchan = ic->ic_no;
    sh->gfnum = ic->ic_gf->gf_no;
    strcpy(sh->smallbuf, gl_version);
    gf_copypieces(ic->ic_gf);
    fbcshmem();
    gl_shmemptr = saveshmem;
    gr_restoreshmem(ctx);
    tx_onepiece(ic->ic_tx);
    splx(pri);

#ifdef	UNIX
    gr_os_stopproc();            	/* force scheduler to run */
#endif
}

/*
 * gr_free:
 *	- release gf process resources
 *	- this code should NOT use the dying process's shared memory unless
 *		it is the current gfport
 */
gr_free(oshandle)
    long oshandle;
{
    register struct inputchan *ic;
    register struct gfport *gf;
    register struct inputchan *icptr;
    register int pri;
    short oldlock;

    if ((ic = gr_getgrhandle(oshandle)) == 0)
	return;
    if (ic->ic_oshandle != oshandle)		/* not the graphics proc */
	return;
    gl_deletetimerevents(ic);

    if (ic == gl_curric) 
            gfinputchannel(-1);

    /*
     * Lock out repaint code while we straighten out the software state
     */
    oldlock = gl_lock;
    gl_lock = 1;

    /* bump textport state, if we have one, to force a border repaint */
    if (ic->ic_tx)
	ic->ic_tx->tx_state |= TX_SCROLLED|TX_DRAWBORDER;

    /* if we had the graphics state, give it to the kernel */
    if(gl_gfport == ic) {
#ifndef ALIAS
	if (PIPEISBUSY || oldlock || (gl_fbwn == ic) || gl_shmemptr->fastfeed)
	    greset();

	else if (gl_shmemptr->ws.myzbuffer ||
		(gl_shmemptr->ws.curatrdata.myconfig & (UC_DEPTHCUE << 16))) {
	    im_setup;
	    extern short geconfigtab[];

	    im_outshort(GEreconfigure);
	    justconfigure(geconfigtab);
	    im_outshort(0xff08);
	    im_passcmd(3, FBCdrawmode);
	    im_last_outlong(0);	
	}
	if (PIPEISBUSY)
	    panic("pipe stayed busy after greset");
#else ALIAS
	if (PIPEISBUSY)
	    GETOKEN = GEnoop;
#endif
	pri = spl6();
	gl_gfport = 0;
	gl_shmemptr = gl_kernelshmemptr;
	(void) gr_setshmem(0);
	splx(pri);
    }

    gl_ksetfontmem(0,ic);

    /*
     * If we are the gfport manager, undo state variable.  Anybody left
     * waiting on a GR_SEND will continue to wait...
     */
    if (gl_wmport == ic)
	wmbyebye();
    else 				/* tell everyone about the death */
	gr_qenterall(WINCLOSE,ic->ic_no); 

    /*
     *	If we are the keyboard manager, clear the kbport variable.
     *
     */
    if (gl_kbport == ic)
	gl_kbport = 0;

    /*
     * Clean up feedback in progress; greset() above should have
     * stopped any junk coming down the pipe from causing any new
     * feedback interrupts, so its okay to clear the state.
     */
    if (gl_fbwn == ic) {
	pri = spl6();
	gl_fbwn = 0;
	gl_fastfeed = 0;
	gl_fbcount = 0;
	gl_fbaddr = 0;
	splx(pri);
	iprintf("gr_free: fixing feedback\n");
	gr_unlockmem(ic->ic_oshandle, gl_origfbaddr,
            	     gl_origfbcount*sizeof(short));
    }

/* now its okay to unlock */
    gl_lock = 0;

/* release this process's gf resources */
    gr_setgrhandle(ic->ic_oshandle, 0);		/* free up shared memory */
    ic->ic_oshandle = 0;
    ic->ic_qreadwait = 0;
    ic->ic_shmemptr = 0;
    for(gf = ic->ic_gf; gf; gf = gf->gf_next) {
	gf->gf_ic = 0;
	gr_freepieces(gf->gf_piecelist);
	gf->gf_piecelist = 0;
    }
    ic->ic_gf = 0;

/* remove from syncwaiters list if on it */
    if(syncwaiters) {
	if (ic == syncwaiters) 
	    syncwaiters = ic->ic_next;
	else {
	    icptr = syncwaiters;	
	    while(icptr->ic_next && ic != icptr->ic_next)
		icptr = icptr->ic_next; 
	    if(ic == icptr->ic_next)
		icptr->ic_next = ic->ic_next;	
	}	
    }

/* clean up doublebuffer/singlebuffer state */
    if(ic->ic_displaymode == MD_DOUBLE) {
	if (ic->ic_swapwaiting)
		numreadytoswap--;
	/*
	 * If there aren't any more processes ready to swap, then we don't
	 * need to swap before graphics can be done again.  Clear out the
	 * gl_needtodoswaps flag to allow graphics to be used again.
	 */
	if (numreadytoswap == 0)
		gl_needtodoswaps = 0;
	ic->ic_swapwaiting = 0;
	gl_numdoublebufferers--;
#ifndef ALIAS
	if((gl_numdoublebufferers == 0) && (!gl_numrgbs)) {
	    gl_needtodoswaps = 0;
	    gl_sbtxport();
	    gl_mode(MD_SINGLE);
	}
#endif
	ic->ic_displaymode = MD_SINGLE;
    }

/* free the bitpad port or the dial port */
    freebitpadport(ic);
    freedialport(ic);

/*
 * Tag inchan as being held for the window manager to release,
 * IFF the window manager is running.
 */
    if (gl_wmport)
	ic->ic_holding = 1;
}

struct inputchan *
getic()
{
    return gr_getgrhandle(gr_getoshandle());
}

/*
 * saveeverything - save the state of the ge in the shared memory structure.
 *		    All we have to save is gpos, cpos, and the matrix stack,
 *		    the other things like the current color and the current 
 *		    write mask are already in the wstate structure.
 *
 */
saveeverything()
{
    im_setup;
    register windowstate *ws;
    register short i, j;
    register short *fbaddr, fbcount;
    register short *fbcaddr = (short *)&FBCdata;
    register unsigned short *fbcclearint = &FBCpixel;
    register struct shmem *sh = gl_shmemptr;
    short dum;
    long pri;

    ws = &sh->ws;
#ifdef	ASSERT
{
#ifdef	UNIX
    struct pte *pte;
    long xxx;
    extern short shmem_pa;

    if (!gl_gfport)
	panic("saveeverything gl_gfport");
    xxx = gr_setshmem(gl_gfport->ic_oshandle);
    pte = (struct pte *)&xxx;
    if (pte->pg_pfnum == shmem_pa)
	panic("saveeverything shmem_pa");
#endif
#ifdef	V
    if (!gl_gfport)
	panic("saveeverything gl_gfport");
#endif
    if (PIPEISBUSY)
	panic("saveeverything pipebusy");
/* XXX could check the point ranges here */
    if (ws->matrixstatep && (ws->matrixlevel < ws->softstacktop))
	panic("saveeverything stack");
    pri = spl7();
    if ((pri & 0x700) >= 0x400)
	panic("saveeverything pri");
    splx(pri);
}
#endif
    while (sh->EOFpending & EOFPENDINGBITS)
	;		/* wait for all user's EOF commands to complete */

/* this is true during ginit only! */
    if (!ws->matrixstatep) 
	return;

/*
 * Send a nothing down the pipe, then wait for an interrupt from the fbc,
 * if the cursor is halfway down
 */
    im_last_passthru(0);
    pri = spl3();
    while(gl_fbcstatus & HOSTFLAG) {
	splx(pri);
	im_last_passthru(0);
	for (i = 0; i < 10; i++)	/* interrupt window here */
		;
	spl3();
    }

/* Save all matrices not already saved in software: */
    FBCflags = gl_fbcstatus;
    GEflags = gl_gestatus | ENABFBCINT_BIT_;
    if(i=ws->matrixlevel - ws->softstacktop) {
	fbcount = i*33;
    /* assumes that matrixstatep points at an empty slot */
	ws->matrixstatep -= (--i);
	fbaddr = (short *)USERTOKERNEL(ws->matrixstatep);
	im_outlong(0x80000 | FBCfeedback);
	for(; i>0; --i) {
	    im_outlong((GEstoremm<<16) | GEpopmm);
	}
	im_outlong((GEstoremm<<16) | FBCEOF1);
	im_outlong((FBCEOF2<<16) | FBCEOF3);	/* don't free pipe! */
	while(FBCflags & INTERRUPT_BIT_) ;	/* wait for interrupt bit */
	FBCflags = READOUTRUN;
	if(!(*fbcaddr == _INTFEEDBACK))
	    panic("something bad wrong with savestate!!");
	FBCflags = gl_fbcstatus;
	FBCclrint;
	GEflags = gl_gestatus | AUTOCLEAR_BIT | ENABFBCINT_BIT_;
	if(!(*fbcaddr == (fbcount+3)))
	    panic("something real badd wrong with savestate!!");
	for(i=0; i<fbcount; i++) {
		*fbaddr++ = *fbcaddr;
	}
	dum = *fbcaddr;
	dum = *fbcaddr;
	dum = *fbcaddr;
	ws->matrixstatep--;
    }
    ws->softstacktop = ws->matrixlevel;
    ws->hdwrstackbottom = ws->matrixlevel + 1;
    GEflags = gl_gestatus | ENABFBCINT_BIT_;	/* disable autoclear */

/* get graphics position */
	/* First reconfigure the pipeline with the first four clippers posing
	   as matrix multipliers.
	 */
    im_outshort(GEreconfigure);
    if ((gemask&0x1ffe) == 0x1ffe) {
	im_outlong(0x0c000b09);
	im_outlong(0x0a0a090b);
	im_outlong(0x080c0709);
	im_outlong(0x060a050b);
	im_outlong(0x040c0313);
	im_outlong(0x02140120);
	im_outlong(0x0021ff60);
    } else {	/* 10 chip system */
	im_outlong(0x0a000909);
	im_outlong(0x080a070b);
	im_outlong(0x060c0509);
	im_outlong(0x040a030b);
	im_outlong(0x020c0120);
	im_outlong(0x0021ff60);
    }
    im_outlong(0x80000 | FBCfeedback);
/*
 * Stripping passthru commands to give popmm and storemm commands
 *  to the first clipper chip.
 */
    im_outlong(0x04390339);
    im_outlong(0x02390139);
    im_outshort(GEpopmm);
/* 
 * Get the "matrix".
 * The first column is the current graphics position. 
 */
    im_outlong((GEstoremm<<16) | FBCEOF1);
    im_outlong((FBCEOF2<<16) | FBCEOF3);

    while(FBCflags & INTERRUPT_BIT_)	/* wait for interrupt bit */
	;
    FBCflags = READOUTRUN;
    if(*fbcaddr != _INTFEEDBACK) {
	iprintf("gl_fbcstatus: %x",gl_fbcstatus);
	iprintf("getgpos(%x)",*fbcaddr);
	panic("something bad wrong with getgpos!!");
    }
    FBCflags = gl_fbcstatus;
    *fbcclearint = 1;
    GEflags = gl_gestatus | AUTOCLEAR_BIT | ENABFBCINT_BIT_;
    if(!(*fbcaddr == 36)) {
	iprintf("gl_fbcstatus: %x",gl_fbcstatus);
	iprintf("getgpos(%x)",*fbcaddr);
	panic("something real badd wrong with getgpos!!");
    }
    dum = *fbcaddr;        /* storemm token */
    fbaddr = (short *)(ws->gpos);
	/* first column of matrix contains the gpos: */
    *fbaddr++ = *fbcaddr;
    *fbaddr++ = *fbcaddr;
    *fbaddr++ = *fbcaddr;
    *fbaddr++ = *fbcaddr;
    *fbaddr++ = *fbcaddr;
    *fbaddr++ = *fbcaddr;
    *fbaddr++ = *fbcaddr;
    *fbaddr++ = *fbcaddr;
	/* throw away the rest: */
    for(j=0; j<3; j++) {
	*fbcclearint = 1;
	*fbcclearint = 1;
	*fbcclearint = 1;
	*fbcclearint = 1;
	*fbcclearint = 1;
	*fbcclearint = 1;
	*fbcclearint = 1;
	*fbcclearint = 1;
    }
    *fbcclearint = 1;	/* eof1 */
    *fbcclearint = 1;	/* eof2 */
    *fbcclearint = 1;	/* eof3 */
 
    GEflags = gl_gestatus | ENABFBCINT_BIT_;
/* Reconfigure the system normally. */

    im_outshort(GEreconfigure);
    if ((gemask&0x1ffe) == 0x1ffe) {
	im_outlong(0x0c000b09);
	im_outlong(0x0a0a090b);
	im_outlong(0x080c0715);
	im_outlong(0x06100511);
	im_outlong(0x04120313);
	im_outlong(0x02140120);
	im_outlong(0x0021ff08);
    } else {	/* 10 chip system */
	im_outlong(0x0a000909);
	im_outlong(0x080a070b);
	im_outlong(0x060c0510);
	im_outlong(0x04110312);
	im_outlong(0x02130120);
	im_outlong(0x0021ff08);
    }

/* get the current character position */
    im_last_outlong((GEpassthru<<16) | FBCreadcharposn);
    while(FBCflags & INTERRUPT_BIT_) ;	/* wait for interrupt bit */
    FBCflags = READOUTRUN;
    if(!(*fbcaddr == _INTCHPOSN))
	panic("something bad wrong with getcpos!!");
    FBCflags = gl_fbcstatus;
    *fbcclearint = 1;
    GEflags = gl_gestatus | AUTOCLEAR_BIT | ENABFBCINT_BIT_;
    fbaddr = (short *)ws->cpos;
    *fbaddr++ = *fbcaddr;
    *fbaddr++ = *fbcaddr;
    *fbaddr++ = *fbcaddr;

    GEflags = gl_gestatus;
    FBCflags = gl_fbcstatus;
    splx(pri);
    if(ws->myzbuffer) {
	im_passcmd(3, FBCdrawmode);
	im_last_outlong(0);		/* no shade or zbuffer modes */
    }
}

/*
 * restoreeverything - restore the hardware state described by 
 *
 * 1. the window state structure within the shared memory struct:
 *	a. texture
 *	b. configreg 
 *		1. buffermode stuff
 *		2. front/back buffer
 *		2. lsbackup
 *		3. resetls
 *	c. linewidth
 *	d. linestyle
 *	e. color
 *	f. writemask
 *	g. viewport
 *	h. scrmask
 *	i. currentcursor
 *	j. cursorcolor
 *	k. cursorwritemask
 *	l. pipeconfig(pick/feedback)
 *	m. fontrambase
 * 2. cpos
 * 3. gpos
 * 4. the matrix stack
 *
 */
restoreeverything()
{
    im_setup;
    register struct shmem *sh = gl_shmemptr;
    register windowstate *ws;
    register ushort *bufptr;
    register long llx, lly, urx, ury;
    register int s;

    ws = &sh->ws;
#ifdef	ASSERT
{
#ifdef	UNIX
    struct pte *pte;
    long xxx;
    extern short shmem_pa;

    if (!gl_gfport)
	panic("restoreeverything gl_gfport");
    xxx = gr_setshmem(gl_gfport->ic_oshandle);
    pte = (struct pte *)&xxx;
    if (pte->pg_pfnum == shmem_pa)
	panic("restoreeverything shmem_pa");
#endif
#ifdef	V
    if (!gl_gfport)
	panic("restoreeverything gl_gfport");
#endif
    if (PIPEISBUSY)
	panic("restoreeverything pipebusy");
/* XXX could maybe check pointer ranges */
    if (ws->matrixstatep && ((ws->softstacktop != ws->matrixlevel) ||
			     (ws->hdwrstackbottom != ws->matrixlevel + 1))) {
	panic("restoreeverything stack");
    }
    s = spl7();
    if ((s & 0x0700) >= 0x0300) {
	iprintf("pri=%x\n", s);
	panic("restoreeverything pri");
    }
    splx(s);
}
#endif
    sh->EOFpending &= ~EOFPENDINGBITS;

/* this is true during ginit only! */
    if (!ws->matrixstatep)
	return;

    gl_lock = 1;
    llx = ws->xmin;
    lly = ws->ymin;
    urx = ws->xmax;
    ury = ws->ymax;
/* Restore current matrix saved away in soft stack: */
    ws->hdwrstackbottom -= 1;
    bufptr = (ushort *)USERTOKERNEL((ws->matrixstatep + 1)->mat);
    im_outshort(GEloadmm);	/* this matrix is already in GE format */
    im_outlong(*(long *)bufptr++);
    im_outlong(*(long *)bufptr++);
    im_outlong(*(long *)bufptr++);
    im_outlong(*(long *)bufptr++);
    im_outlong(*(long *)bufptr++);
    im_outlong(*(long *)bufptr++);
    im_outlong(*(long *)bufptr++);
    im_outlong(*(long *)bufptr++);
    im_outlong(*(long *)bufptr++);
    im_outlong(*(long *)bufptr++);
    im_outlong(*(long *)bufptr++);
    im_outlong(*(long *)bufptr++);
    im_outlong(*(long *)bufptr++);
    im_outlong(*(long *)bufptr++);
    im_outlong(*(long *)bufptr++);
    im_outlong(*(long *)bufptr);

/* restore GE configuration for z buffer or depth cue modes */
    if((ws->curatrdata.myconfig & (UC_DEPTHCUE << 16)) ||
		ws->myzbuffer) {
	im_outshort(GEreconfigure);
	if ((gemask&0x1ffe) == 0x1ffe) {
	    im_outlong(0x0c000b09);
	    im_outlong(0x0a0a090b);
	    im_outlong(0x080c0715);
	    im_outlong(0x06100511);
	    im_outlong(0x04120313);
	    im_outlong(0x02140122);
	    im_outlong(0x0023ff0a);
	} else {	/* 10 chip system */
	    im_outlong(0x0a000909);
	    im_outlong(0x080a070b);
	    im_outlong(0x060c0510);
	    im_outlong(0x04110312);
	    im_outlong(0x02130122);
	    im_outlong(0x0023ff0a);
	}
	if(ws->myzbuffer) {
	    im_passcmd(3, FBCdrawmode);
	    im_outlong(1);			/* zbuffer mode */
	    im_passcmd(3, FBCcdcolorwe);
	    im_outlong(0x7fffffff);		/* we on for cd planes */
	}
    }
    if(!ws->myzbuffer) {
	im_passcmd(3, FBCcdcolorwe);
	im_outlong(0);				/* we off for cd planes */
    }
/* restore config reg */
    if((gl_cfr & (DISPLAYA | DISPLAYB)) !=
			(ws->curatrdata.myconfig & (DISPLAYA | DISPLAYB)))
	ws->curatrdata.myconfig = swapconfig(ws->curatrdata.myconfig);
/* XXX this can be sped up by concatinating lots of im_passcmd's */
    im_passcmd(3, FBCconfig);
    if(gl_blankmode || gl_isblanked)
	im_outlong(ws->curatrdata.myconfig & ~(DISPLAYA | DISPLAYB));
    else
	im_outlong(ws->curatrdata.myconfig);

/* restore linewidth */
    im_passcmd(2, FBClinewidth);
    im_outshort(ws->curatrdata.mylwidth); 

/* restore linestyle */
    im_passcmd(3, FBClinestipple);
    im_outshort(ws->curatrdata.mylsrepeat);
    im_outshort(ws->mylscode);

/* restore color */
    im_passcmd(2, FBCcolor);
    im_outshort(ws->curatrdata.mycolor); 

/* restore writemask */
    im_passcmd(2, FBCwrten);
    im_outshort(ws->curatrdata.mywenable); 

/* restore fontrambase: must be done before restoring poly texture */
    im_passcmd(2, FBCbaseaddress);
    im_outshort(ws->fontrambase); 

/* restore poly texture */
    im_passcmd(2, FBCpolystipple);
    im_outshort(ws->mytexcode);

/* restore viewport */
    im_outshort(GEloadviewport);
    im_outlong(ws->curvpdata.vcx + (llx<<8)); 
    im_outlong(ws->curvpdata.vcy + (lly<<8));
    im_outlong(ws->curvpdata.vsx); 
    im_outlong(ws->curvpdata.vsy);
    im_outlong(ws->curvpdata.vcs);
    im_outlong(ws->curvpdata.vcz);
    im_outlong(ws->curvpdata.vss);
    im_outlong(ws->curvpdata.vsz);

/* restore shade range */
    im_passcmd(7,FBCdepthsetup);
    im_outshort (ws->a);
    im_outshort (ws->b);
    im_outshort (ws->imin);
    im_outshort (ws->imax);
    im_outshort (ws->z1);
    im_outshort (ws->z2);

/* restore back face mode */
    im_passcmd(2,FBCsetbackfacing);
    im_outshort(ws->mybackface);

/* restore scrmask */
/* screen coord=window min+relative coord */
    llx = ws->xmin + ws->curvpdata.llx;
    lly = ws->ymin + ws->curvpdata.lly;
    urx = ws->xmin + ws->curvpdata.urx;
    ury = ws->ymin + ws->curvpdata.ury;
    im_passcmd(5,FBCloadviewport);
    im_outshort(llx);
    im_outshort(lly);
    im_outshort(urx);
    im_outshort(ury);
    {
    register short i, numrects;
    typedef struct {
	short xmin;
	short ymin;
	short xmax;
	short ymax;
    } rect;
    register rect *r;

#ifdef ASSERT
    if((ws->numrects < 0) || (ws->numrects > MAXWSPIECES))
	panic("restoreeverything: numrects");
#endif
    for(i=0, numrects=0, r=(rect *)ws->rectlist;
            	i < ws->numrects; i++, r++) {
	if((r->xmin > urx) || (r->ymin > ury) || (r->xmax < llx) ||
		(r->ymax < lly))
		continue;	/* scrmask and this piece don't overlap */
	if(numrects++ == 0) {
	    im_passthru(7);
	    im_outlong(FBCmasklist << 16);		/* first rect */
	} else {
	    im_outshort(1);	/* flag indicating more than one rect */
	    im_passthru(7);
	    im_outlong((FBCmasklist << 16) | 1);	/* next rect */
	}
	if(llx >= r->xmin)
	    im_outshort(llx);
	else
	    im_outshort(r->xmin);
	if(lly >= r->ymin)
	    im_outshort(lly);
	else
	    im_outshort(r->ymin);
	if(urx <= r->xmax)
	    im_outshort(urx);
	else
	    im_outshort(r->xmax);
	if(ury <= r->ymax)
	    im_outshort(ury);
	else
	    im_outshort(r->ymax);
    }
    if(numrects == 0) {
	im_passcmd(7, FBCmasklist);
	im_outlong((0 << 16) | 1);
	im_outlong((1 << 16) | 0);		/* if no visible pieces. */
	im_outlong((0 << 16) | 0);
    } else if(numrects == 1)
	im_outshort(0);	/* only one rect */
    else
	im_outshort(1);	/* more than one rect */
    }

/* Restore graphics position. */
    bufptr = (ushort *)ws->gpos;
    im_outlong((0xb39<<16) | 0xa39);	/* bypass the matrix multipliers */
    im_outlong((0x939<<16) | 0x839);
    im_outshort(GEmove);
    im_outlong(*(long *)bufptr++);
    im_outlong(*(long *)bufptr++);
    im_outlong(*(long *)bufptr++);
    im_outlong(*(long *)bufptr++);

/* Restore character position. */
    bufptr = (ushort *)ws->cpos;
    if(*(bufptr+2) == -1) {	/* This means good character position */
	if((ws->curatrdata.myconfig & (UC_DEPTHCUE << 16)) ||
		ws->myzbuffer) {
	    im_passcmd(6, FBCcharposnabs);
	    im_outshort(GEpoint);
	    im_outshort(*bufptr++);
	    im_outshort(*bufptr);
	    im_last_outlong(0);		/* stereo x and z */
		/* z.  Does anyone care about the z values of chars?? */
	} else {
	    im_passcmd(4, FBCcharposnabs);
	    im_outshort(GEpoint);
	    im_outshort(*bufptr++);
	    im_last_outshort(*bufptr);
	}
    } else {
	im_passcmd(1, FBCcharposnabs);
	im_freepipe;
    }
    gl_lock = 0;
}

dogsync()
{
    register struct inputchan *ic;
    register int pri;
    register long oshandle;

    if( !(ic=getic()) )
	return;
    pri = spl6();
    ic->ic_next = syncwaiters;		/* link onto sync waiters */
    syncwaiters = ic;
#ifdef UNIX
    while(syncwaiters)
        gr_sleep(ic->ic_oshandle, ic);
#else
    gr_sleep(ic->ic_oshandle, ic);
#endif
    splx(pri);
}

retraceevent()
{
    register struct inputchan *ic;
    short neededtoswap;
    
/* wakeup any processes blocked in gsync */
    ic = syncwaiters;
    syncwaiters = 0;
    while(ic) {            
	gr_wakeup(ic->ic_oshandle, ic);
	ic = ic->ic_next;
    }

/* count down for next swap */
    if (gl_SwapCount > 0)
	--gl_SwapCount;	
    else if(numreadytoswap>0) { 		/* if procs ready to swap */
	neededtoswap = gl_numdoublebufferers;
	if(gl_wmswapanytime)
	    neededtoswap--;
	if(neededtoswap > 0) {
	    if(numreadytoswap >= neededtoswap) {
		if (PIPEISBUSY || gl_lock) {
		    gl_needtodoswaps = 1;
		    fbc_setpipeint();
		} else 
		    wakeupswappers();
	    }
	}
    }
}

doqread()
{
    register struct inputchan *ic = getic();
    short dev, val;
    register int pri;

    if(!ic) 
	return;
    pri = spl6();
    if( (dev = qread(&val)) == 0) {
	ic->ic_qreadwait = 1; 
#ifdef UNIX
	while(ic->ic_qreadwait)
	    gr_sleep(ic->ic_oshandle, ic);
#else
	gr_sleep(ic->ic_oshandle, ic);
#endif
    } else {
	ic->ic_shmemptr->qdev = dev;
	ic->ic_shmemptr->qvalue = val;
    }
    splx(pri);
}

addqevent(ic)
register struct inputchan *ic;
{
    short dev, val;
    register long oshandle;
    register int pri;
    register long ctx;

    pri = spl6();
    oshandle = ic->ic_oshandle;
    if(oshandle && (ic->ic_qreadwait)) {
	dev = inter_qread(&val,ic);
	ctx = gr_setshmem(oshandle);
	ic->ic_shmemptr->qdev = dev;
	ic->ic_shmemptr->qvalue = val;
	gr_restoreshmem(ctx);
	ic->ic_qreadwait = 0;
	gr_wakeup(oshandle, ic);
    }
    splx(pri);
}

setqtop(ic, type)
register struct inputchan *ic;
int type;
{
    register int pri;
    register long ctx;

    pri = spl6();
    if(ic->ic_shmemptr) {
	ctx = gr_setshmem(ic->ic_oshandle);
	if (ic->ic_queuein == ic->ic_queueout) 
	    ic->ic_shmemptr->qtop = 0;
	else
	    ic->ic_shmemptr->qtop = ic->ic_queueout->type;
	gr_restoreshmem(ctx);
    }
    splx(pri);
}

dowaitforswap()
{
    register int pri;
    register struct inputchan *ic;

    if( !(ic=getic()) )
        return;
    pri = spl6();
    ic->ic_swapwaiting = 1;
    numreadytoswap++;
#ifdef UNIX
    while(ic->ic_swapwaiting)
        gr_sleep(ic->ic_oshandle, ic);
#else
    gr_sleep(ic->ic_oshandle, ic);
#endif
    splx(pri);
}

/*
 * Swaps front and back buffers in double-buffer mode only.
 * Flush before swap to insure that all drawing commands have been
 * completed.  gl_SwapCount stuff included.
 * Both display and update enables in the configuration (gl_cfr) are
 * swapped, thus front/back buffer enables remain consistent
 * across calls to swapbuffers.
 */
gl_doswapbuffers () 
{
    register long config;

    gl_cfr = swapconfig(gl_cfr);
    if (gl_cfr & DISPLAYA) {
	gl_cursorconfig = (UC_DOUBLE<<16) | UPDATEA | DISPLAYA;
    } else {
	gl_cursorconfig = (UC_DOUBLE<<16) | UPDATEB | DISPLAYB;
    }
    UpdateConfigs();
    if(gl_mapcount == 0) {		/* only do this if not cycleing maps */
	gl_dcr ^= DCMBIT;			/* wiggle bit for glasses */
	DCflags = gl_dcr;
	gl_dcr ^= DCMBIT;			/* wiggle back for glasses */
	DCflags = gl_dcr;
    }
    gl_didswap = 1;
}

static
UpdateConfigs() 
{
    im_setup;
    register short addr;
    register short curswason;

    if(curswason = gl_autocursor) {
	gl_WaitForEOF(1);
	cursoff();
    }
    im_passcmd(14, FBCconfig);
    if(gl_blankmode || gl_isblanked)
	im_outlong( (gl_cfr & ~(DISPLAYA | DISPLAYB)) | UPDATEA | UPDATEB);
    else
	im_outlong(gl_cfr | UPDATEA | UPDATEB);
    im_outshort(FBCselectcursor);
    im_outshort(gl_cursoraddr);
    if(gl_blankmode || gl_isblanked)
	im_outlong(gl_cursorconfig & ~(DISPLAYA | DISPLAYB));
    else
	im_outlong(gl_cursorconfig);
    im_outlong(gl_cursorcolor);
    im_outlong(gl_cursorwenable);
    im_outshort(FBCpixelsetup);
    if(gl_cfr & DISPLAYA)
	im_last_outlong(0x203ff);
    else
	im_last_outlong(0x103ff);
    if(curswason)
	curson(0);
}

gl_calcmaxsi()
{
    register struct inputchan *ic = &inchan[0];
    register short i, maxsi = -1000;

    for(i=0; i<NINCHANS; i++, ic++)
	if((ic->ic_shmemptr) && (ic->ic_SwapInterval > maxsi))
	    maxsi = ic->ic_SwapInterval;
    return maxsi;
}

wakeupswappers()
{
    register short i;
    register struct shmem *sh;
    register struct inputchan *ic = &inchan[0];
    register struct gfport *gf;
    register long currentconfig = 0;
    long ctx;

    /* quickly figure out what configuration to send to the dcr */
    gl_lock = 1;	/* block textport repaint code? */
    ctx = gr_setshmem(ic->ic_oshandle);	/* save shmem state */
    for(i=0; i<NINCHANS; i++, ic++) {
	if( (sh = ic->ic_shmemptr) ) {
	    (void) gr_setshmem(ic->ic_oshandle);
	    sh->ws.curatrdata.myconfig = swapconfig(sh->ws.curatrdata.myconfig);
	    if(gl_gfport == ic)
		currentconfig = sh->ws.curatrdata.myconfig;
	    if(ic->ic_swapwaiting) {
		ic->ic_swapwaiting = 0; 
		gr_wakeup(ic->ic_oshandle, ic);
	    }
	}
    }
    gl_doswapbuffers();
    gr_restoreshmem(ctx);            /* restore shmem state */

    /* set config value to state that hardware owner wants */
    if(currentconfig)
	setconfig(currentconfig);

    gl_SwapCount = gl_MaxSwapInterval;
    numreadytoswap = 0;
    gl_needtodoswaps = 0;
    gl_lock = 0;
}

gr_startfeed()
{
    register struct inputchan *ic = getic();
    register struct shmem *sh = gl_shmemptr;
    register int pri;

    if(!ic)
	return 1;		/* non graphics process */
    pri = spl6();
    if (gr_lockmem(sh->intbuf, sh->intbuflen*sizeof(short))) {
	splx(pri);
	return 1;
    }
    gl_fbwn = ic;
    gl_lock = 1;
    gl_fbaddr = gl_origfbaddr = sh->intbuf;
    gl_fbcount = gl_origfbcount = sh->intbuflen;
    gl_fastfeed = 0;
    sh->fastfeed = 0;
    splx(pri);
    return 0;
}

gr_endfeed()
{
    register struct inputchan *ic = getic();
    register struct shmem *sh = gl_shmemptr;

    if(!ic)
	return 1;		/* non graphics process */
    if (!gl_fbwn)
	return 1;		/* no feedback in progress */
    gr_unlockmem(gl_fbwn->ic_oshandle, gl_origfbaddr,
				       gl_origfbcount*sizeof(short));
    sh->intbuflen = gl_fbcount;
    sh->fastfeed = 0;
    gl_fastfeed = 0;
    gl_fbwn = 0;
    gl_lock = 0;
}

/*
 * gf_copypieces:
 *	- copy the pieces from the gfport struct into the shared memory
 */
gf_copypieces(gf)
    register struct gfport *gf;
{
    register struct inputchan *ic = gf->gf_ic;
    register struct shmem *sh;
    register struct piece *pp;
    register short *to;
    register int s;
    long ctx;

    if((!ic) || ((sh = ic->ic_shmemptr) == 0))
	return;

    s = spl6();				/* block tx_repaint's saveeverything */
    ctx = gr_setshmem(ic->ic_oshandle);
    if(sh->gfnum == gf->gf_no) {
	sh->ws.xmin = gf->gf_llx;
	sh->ws.ymin = gf->gf_lly;
	sh->ws.xmax = gf->gf_urx;
	sh->ws.ymax = gf->gf_ury;

	to = &sh->ws.rectlist[0];
	pp = gf->gf_piecelist;
	sh->ws.numrects = 0;
	while (pp && sh->ws.numrects < MAXWSPIECES) {
	    *to++ = pp->p_xmin;
	    *to++ = pp->p_ymin;
	    *to++ = pp->p_xmax;
	    *to++ = pp->p_ymax;
	    sh->ws.numrects++;
	    pp = pp->p_next;
	}
    }
    gr_restoreshmem(ctx);
    splx(s);				/* okay to repaint now */
}


gr_qenterall(t,v) 	/* special treatment for wmport until later HACK HACK */
{
    register short i;
    register struct inputchan *ic;

    ic = &inchan[0]; 
    for(i=0; i<NINCHANS; i++, ic++) 
	if( ic->ic_shmemptr && isqueued(ic,t) )  
		gr_qenter(ic, t, v);
}

#ifdef NOTDEF
char dbevent[2000];
short dbinp;

addevent(c)
char c;
{
register int pri;

    pri = spl7();
    dbevent[dbinp++] = c;
    if(dbinp == 2000)
	dbinp = 0;
    splx(pri);
}

printevents()
{
int i;
register int pri;

    pri = spl7();
    for(i=0; i<2000; i++) {
	duputchar(dbevent[dbinp++]);
	if(dbinp == 2000)
	    dbinp = 0;
    }
}
#endif

/*
 *   	gl_unreservebuttons - unreserve all the buttons
 *
 */
gl_unreservebuttons()
{
    register short i;
    register buttondata *but;
    
    but = &gl_buttons[0];
    for (i=0; i<BUTCOUNT; i++) {
	but->reserved = 0;
    	but++;
    }
}

getgfport()
{
    register short i;
    register struct gfport *gf, *gf2;
    register struct inputchan *ic = getic();

    if(!ic)
	return -1;
    gf = &gfport[0];
    for(i=0; i<NGFPORTS; i++, gf++) {
	if (!gf->gf_ic) {
	    gf->gf_ic = ic;
	    if(!ic->ic_gf)
		ic->ic_gf = gf;
	    else {
		for(gf2 = ic->ic_gf; gf2->gf_next; gf2 = gf2->gf_next) ;
		gf2->gf_next = gf;
	    }
	    gf->gf_next = 0;
	    return i;
	}
    }
    return -1;
}

setgfport(newws, oldws, gfnum)
windowstate *newws, *oldws;
register short gfnum;
{
    register struct gfport *gf;
    register struct inputchan *ic = getic();
    short oldlock = gl_lock;

    for(gf = ic->ic_gf; gf; gf = gf->gf_next)
	if(gf->gf_no == gfnum)
	    break;
    if(!gf)
{
iprintf("setting gfport %d, with no ownership\n",gfnum);
	return -1;
}
    gl_lock = 1;
    if(gl_gfport == ic)
	saveeverything();	/* ws in shmem is now up to date */
				/* copy the old shmem out to user land: */
    if(copyout(&gl_shmemptr->ws, oldws, sizeof(windowstate))) {
	gl_lock = oldlock;
	gl_ioerror = EFAULT;
iprintf("setgfport: bad copyout\n");
	return -1;
    }
    if(copyin(newws, &gl_shmemptr->ws, sizeof(windowstate))) { 
	gl_lock = oldlock;
	gl_ioerror = EFAULT;
iprintf("setgfport: bad copyin\n");
	return -1;
    }
    gl_shmemptr->gfnum = gfnum;
    gf_copypieces(&gfport[gfnum]);
    if(gl_gfport == ic)
	restoreeverything();
    gl_lock = oldlock;
    return 0;
}

/*
 * wmbyebye:
 *	- clean up after the window manager, now that its gone bye bye
 */
wmbyebye()
{
	register int i;

	gl_wmport = 0;
	txinputchannel(0);
	gl_unreservebuttons();
	reservebits(0);
	gl_wmswapanytime = 0;
	/*
	 * Check any clogged inchans now that the window manager is dead.
	 * Free up any held ones.
	 */
	for (i=0; i<NINCHANS; i++)
		inchan[i].ic_holding = 0;
}
