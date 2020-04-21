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

#include "globals.h"
#include "shmem.h"
#include "TheMacro.h"
#include "imdraw.h"
#include "glerror.h"

static long gl_picklen;
static short numberhits, numshorts;

void pick(buff,buflen)
short *buff;
long buflen;		/* length in names */
{
    if(!gl_fbmode) { /* if not already picking */
	gl_makepickmat();
	gl_picking = 1;
	gselect(buff, buflen);
    } else {
	gl_ErrorHandler(ERR_INPICK, WARNING, "pick");
	return;
    }
}

long endpick(buff)
short *buff;
{
    if(gl_fbmode) { /* if picking */
	gl_picking = 0;
	return(endselect(buff));
    } else {
	gl_ErrorHandler(ERR_NOTINPICK, WARNING, "endpick");
	return(0);
    }
}


void gselect(buff,buflen)
short *buff;
long buflen;		/* length in names */
{
    register struct shmem *sh = gl_shmemptr;

    if(!gl_fbmode) { /* if not already picking */
	im_setup;

	gl_pickselect = gl_fbmode = 1;
	sh->numhits = numberhits = numshorts = 0;

#ifdef UNDEF
	sh->intbuf = (short *)malloc(buflen*2);  /* length in */
							     /* bytes */
	if(!sh->intbuf) {
	    gl_outmem("gselect");
	    gl_GEbuflen = sh->intbuflen = 0;
	} else
	    gl_GEbuflen = sh->intbuflen = buflen; /* length in shorts */
#else
	sh->intbuf = buff;
	gl_GEbuflen = sh->intbuflen = buflen; /* length in shorts */
#endif

	gl_pickbuf = sh->intbuf;
	gl_picklen = sh->intbuflen = buflen;	
	gl_startfeed();

	im_outshort(GEsethitmode);
	im_initnames();		/* initialize name stack: */
	im_cleanup;
    } else {
	gl_ErrorHandler(ERR_INPICK, WARNING, "gselect");
	return;
    }
}


gl_suspendselect()
{
    register struct shmem *sh = gl_shmemptr;
    im_setup;
    im_passcmd(2,FBCpushname);	/* trigger last hit report */
    im_outshort(0);
    im_passcmd(1,FBCpopname);
    im_outshort(GEclearhitmode);
    gl_pickselect = gl_fbmode = 0;
    gl_WaitForEOF(1);
    im_freepipe;
    gl_endfeed();
    im_cleanup;
    numshorts = gl_picklen - sh->intbuflen;
    numberhits += sh->numhits;
}

gl_resumeselect()
{
    register struct shmem *sh = gl_shmemptr;
    im_setup;
    gl_pickselect = gl_fbmode = 1;
    sh->numhits = 0;
    sh->intbuf = &gl_pickbuf[numshorts];
    sh->intbuflen = gl_picklen - numshorts;
    gl_startfeed();
    im_outshort(GEsethitmode);
    im_freepipe;
    im_cleanup;
}

long endselect(buff)
short *buff;
{
    register struct shmem 	*sh = gl_shmemptr;

    if(gl_fbmode) { /* if picking */
	im_setup;

	im_passcmd(2,FBCloadname);	/* trigger last hit report */
	im_outshort(0);
	im_outshort(GEclearhitmode);
	gl_pickselect = gl_fbmode = 0;
	gl_WaitForEOF(1);
	im_freepipe;
	gl_endfeed();
	im_cleanup;
	numshorts += (gl_picklen - sh->intbuflen);

#ifdef UNDEF
	/* Copy names to user data buffer: */
	
	if(buff)
	    bcopy(gl_pickbuf,buff,numshorts*sizeof(short));
	if(gl_pickbuf)
	    free(gl_pickbuf);
#endif
/* hack to temporarily fix (microcode??) bug: 	CCR */
scrmask(WS->curvpdata.llx, WS->curvpdata.urx, WS->curvpdata.lly,
							WS->curvpdata.ury);
	return(numberhits+sh->numhits);
    } else {
	gl_ErrorHandler(ERR_NOTINPICK, WARNING, "endselect");
	return(0);
    }
}

long gethitcode()
{
    finish();
    return((gl_shmemptr->hitbits >> 8) & 0x3f);
}

void clearhitcode()
{
    finish();
    gl_shmemptr->hitbits = 0;
}

gl_makepickmat()
{
    short cursorx, cursory;
    float fx,fy;
    register windowstate *ws = gl_wstatep;

    /* this accounts for viewports being relative to the corner
    of the window */
    register float wsxmin, wsymin;
    wsxmin = ws->xmin;
    wsymin = ws->ymin;

    cursorx = getvaluator(CURSORX);
    cursory = getvaluator(CURSORY);
    fx = gl_picksizex;
    fy = gl_picksizey;

    gl_pickmat[0][1] = gl_pickmat[0][2] = gl_pickmat[0][3] =
    gl_pickmat[1][0] = gl_pickmat[1][2] = gl_pickmat[1][3] = 
    gl_pickmat[2][0] = gl_pickmat[2][1] = gl_pickmat[2][3] = 0.0;

    gl_pickmat[0][0] = (ws->curvpdata.urx - ws->curvpdata.llx + 1) / fx;
    gl_pickmat[1][1] = (ws->curvpdata.ury - ws->curvpdata.lly + 1) / fy;
    gl_pickmat[3][0] = (-2*cursorx + 2*wsxmin + 
			ws->curvpdata.llx + ws->curvpdata.urx)/fx;
    gl_pickmat[3][1] = (-2*cursory + 2*wsymin +
			ws->curvpdata.lly+ws->curvpdata.ury)/fy;
    gl_pickmat[2][2] = gl_pickmat[3][3] = 1.0;
}

ROOT_1S(loadname)
ROOT_1S(pushname)
ROOT_0(popname)
ROOT_0(initnames)

#include "interp.h"

INTERP_NAME(loadname);
INTERP_NAME(pushname);
INTERP_NAME(popname);
INTERP_NAME(initnames);

static bogus ()
{
    DECLARE_INTERP_REGS;

    INTERP_ROOT_1S(loadname);
    INTERP_ROOT_1S(pushname);
    INTERP_ROOT_0 (popname);
    INTERP_ROOT_0 (initnames);
}
