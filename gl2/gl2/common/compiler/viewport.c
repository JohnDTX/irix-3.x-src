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
#include "TheMacro.h"
#include "imattrib.h"
#include "uc4.h"
#include "get.h"
#include "glerror.h"
#include "shmem.h"

extern int i_viewport(), i_scrmask(), i_setdepth();
extern int i_pushviewport(), i_popviewport();
extern int i_pushattributes(), i_popattributes();

#define VIEWPORT_SIZE	14
#define SCRMASK_SIZE	6
#define SETDEPTH_SIZE	4

void viewport(left, right, bottom, top)
register short	left, right, bottom, top;
{
    register windowstate *ws = gl_wstatep;

    if(left < -XMAXSCREEN)
	left = -XMAXSCREEN;
    if(right > 2*XMAXSCREEN)
	right = 2*XMAXSCREEN;
    if(bottom < -YMAXSCREEN)
	bottom = -YMAXSCREEN;
    if(top > 2*YMAXSCREEN)
	top = 2*YMAXSCREEN;
    beginpicmandef(VIEWPORT_SIZE);
    BEGINCOMPILE(VIEWPORT_SIZE);
    ADDADDR(i_viewport);
    ADDLONG((left+right+1)<<7);
    ADDLONG((bottom+top+1)<<7);
    ADDLONG((right-left+1)<<7);
    ADDLONG((top-bottom+1)<<7);
    /* Make sure scrmask is only 10 bits: */
    if(left > XMAXSCREEN) left = XMAXSCREEN;
    else if(left < 0) left = 0;
    if(right > XMAXSCREEN) right = XMAXSCREEN;
    else if(right < 0) right = 0;
    if(bottom > XMAXSCREEN) bottom = XMAXSCREEN;
    else if(bottom < 0) bottom = 0;
    if(top > XMAXSCREEN) top = XMAXSCREEN;
    else if(top < 0) top = 0;
    ADDSHORT(left);
    ADDSHORT(bottom);
    ADDSHORT(right);
    ADDSHORT(top);

    ENDCOMPILE;
    endpicmandef;
}

void scrmask(left, right, bottom, top)
register Screencoord	left, right, bottom, top;
{
    register windowstate *ws = gl_wstatep;

    beginpicmandef(SCRMASK_SIZE);
    BEGINCOMPILE(SCRMASK_SIZE);
    ADDADDR(i_scrmask);
    /* Make sure scrmask is only 10 bits: */
    if(left > XMAXSCREEN) left = XMAXSCREEN;
    else if(left < 0) left = 0;
    if(right > XMAXSCREEN) right = XMAXSCREEN;
    else if(right < 0) right = 0;
    if(bottom > XMAXSCREEN) bottom = XMAXSCREEN;
    else if(bottom < 0) bottom = 0;
    if(top > XMAXSCREEN) top = XMAXSCREEN;
    else if(top < 0) top = 0;
    ADDSHORT(left);
    ADDSHORT(bottom);
    ADDSHORT(right);
    ADDSHORT(top);

    ENDCOMPILE;
    endpicmandef;
}

void setdepth(zmin, zmax)
register short	zmin, zmax;
{
    beginpicmandef(SETDEPTH_SIZE);
    BEGINCOMPILE(SETDEPTH_SIZE);
    ADDADDR(i_setdepth);
    ADDSHORT(zmin);
    ADDSHORT(zmax);
    ENDCOMPILE;
    endpicmandef;
}

void popviewport()
{
    beginpicmandef(2);
    BEGINCOMPILE(2);
    ADDADDR(i_popviewport);
    ENDCOMPILE;
    endpicmandef;
}

void pushviewport()
{
    beginpicmandef(2);
    BEGINCOMPILE(2);
    ADDADDR(i_pushviewport);
    ENDCOMPILE;
    endpicmandef;
}

void pushattributes()
{
    beginpicmandef(2);
    BEGINCOMPILE(2);
    ADDADDR(i_pushattributes);
    ENDCOMPILE;
    endpicmandef;
}

void popattributes()
{
    beginpicmandef(2);
    BEGINCOMPILE(2);
    ADDADDR(i_popattributes);
    ENDCOMPILE;
    endpicmandef;
}

#include "interp.h"

INTERP_NAME(viewport);
INTERP_NAME(scrmask);
INTERP_NAME(setdepth);
INTERP_NAME(pushattributes);
INTERP_NAME(popattributes);
INTERP_NAME(pushviewport);
INTERP_NAME(popviewport);

static bogus()
{
    DECLARE_INTERP_REGS;
{
    register long wxmin, wymin, llx, lly, urx, ury;

INTERP_LABEL(viewport, 14); /* VIEWPORT_SIZE == 14 */
    WS->curvpdata.vcx = *PC++;
    WS->curvpdata.vcy = *PC++;
    WS->curvpdata.vsx = *PC++;
    WS->curvpdata.vsy = *PC++;
    WS->curvpdata.llx = *(short *)PC++;
    WS->curvpdata.lly = *(short *)PC++;
    WS->curvpdata.urx = *(short *)PC++;
    WS->curvpdata.ury = *(short *)PC++;
asm("_i_vpdoit:");
    wxmin = WS->xmin;
    wymin = WS->ymin;
    im_outshort(GEloadviewport);
    im_outlong(WS->curvpdata.vcx + (wxmin<<8));
    im_outlong(WS->curvpdata.vcy + (wymin<<8));
    im_outlong(WS->curvpdata.vsx);
    im_outlong(WS->curvpdata.vsy);
    im_outlong(WS->curvpdata.vcs);
    im_outlong(WS->curvpdata.vcz);
    im_outlong(WS->curvpdata.vss);
    im_outlong(WS->curvpdata.vsz);
    im_passcmd(5,FBCloadviewport);
    im_outshort(WS->curvpdata.llx+wxmin);
    im_outshort(WS->curvpdata.lly+wymin);
    im_outshort(WS->curvpdata.urx+wxmin);
    im_outshort(WS->curvpdata.ury+wymin);
    bra(_i_scrmaskdoit);
    thread;

INTERP_LABEL(scrmask, 6); /* SCRMASK_SIZE == 6 */
    WS->curvpdata.llx = *(short *)PC++;
    WS->curvpdata.lly = *(short *)PC++;
    WS->curvpdata.urx = *(short *)PC++;
    WS->curvpdata.ury = *(short *)PC++;
    wxmin = WS->xmin;
    wymin = WS->ymin;

asm("_i_scrmaskdoit:");
    llx = wxmin + WS->curvpdata.llx;
    lly = wymin + WS->curvpdata.lly;
    urx = wxmin + WS->curvpdata.urx;
    ury = wymin + WS->curvpdata.ury;
    {
    register short i, numrects;
    typedef struct {
	short xmin;
	short ymin;
	short xmax;
	short ymax;
    } rect;
    register rect *r;

    for(i=0, numrects=0, r=(rect *)WS->rectlist;
				i < WS->numrects; i++, r++) {
	if((r->xmin > urx) || (r->ymin > ury) || (r->xmax < llx) ||
		(r->ymax < lly))
		continue;	/* scrmask and this piece don't overlap */
	if(numrects++ == 0) {
	    im_passcmd(7,FBCmasklist);
	    im_outshort(0);	/* first rect */
	} else {
	    im_outshort(1);	/* flag indicating more than one rect */
	    im_passcmd(7,FBCmasklist);
	    im_outshort(1);	/* not first rect */
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
	im_passcmd(7,FBCmasklist);
	im_outshort(0);
	im_outshort(1);		/* hack: set xmin, ymin > xmax, ymax */
	im_outshort(1);		/* if no visible pieces. */
	im_outshort(0);
	im_outshort(0);
	im_last_outshort(0);
    } else if(numrects == 1)
	im_last_outshort(0);	/* only one rect */
    else
	im_last_outshort(1);	/* more than one rect */
    }
    thread;

INTERP_LABEL(setdepth, 4); /* SETDEPTH_SIZE == 4 */
    WS->zmin = *(short *)PC++;
    WS->zmax = *(short *)PC++;
    WS->curvpdata.vcz = (WS->zmin+WS->zmax+1)<<7;
    WS->curvpdata.vsz = (WS->zmax-WS->zmin+1)<<7;
    wxmin = WS->xmin;
    wymin = WS->ymin;
    im_outshort(GEloadviewport);
    im_outlong(WS->curvpdata.vcx + (wxmin<<8));
    im_outlong(WS->curvpdata.vcy + (wymin<<8));
    im_outlong(WS->curvpdata.vsx);
    im_outlong(WS->curvpdata.vsy);
    im_outlong(WS->curvpdata.vcs);
    im_outlong(WS->curvpdata.vcz);
    im_outlong(WS->curvpdata.vss);
    im_outlong(WS->curvpdata.vsz);
    im_passcmd(5,FBCloadviewport);
    im_outshort(WS->curvpdata.llx+wxmin);
    im_outshort(WS->curvpdata.lly+wymin);
    im_outshort(WS->curvpdata.urx+wxmin);
    im_last_outshort(WS->curvpdata.ury+wymin);
    thread;
}

INTERP_LABEL(pushattributes,2);
    if (WS -> attribstatep == &WS -> attribstack[ATTRIBSTACKDEPTH])
	gl_ErrorHandler(ERR_PUSHATTR,WARNING,NULL);
    else *WS -> attribstatep++ = WS -> curatrdata;
    thread;

INTERP_LABEL(popattributes,2);
    if (WS -> attribstatep == WS -> attribstack)
	gl_ErrorHandler (ERR_POPATTR,WARNING,NULL);
    else {
	register long oldcfr,cfr,updates;

	oldcfr = WS -> curatrdata.myconfig;	/* get old config value	*/
	im_lockpipe;
	WS -> curatrdata = *--WS -> attribstatep;

	cfr = WS -> curatrdata.myconfig;	/* get new config value	*/

	/* 
	 * Detect if in between push and pops there was a change of display
	 * modes. The easiest way to do this is to test for single buffer
	 * mode since each of the update and display bits must be on in
	 * single buffer mode.
	 *  -amc
	 */

	if((oldcfr & 0x0f) == 0x0f) {		/* display mode is single */
	    if((cfr & 0x0f) == 0x0f) {		/* popped mode is single */
#ifdef DEBUG
		printf("single to single\n");
#endif
	    } else {
		cfr = oldcfr;			/* use single */
#ifdef DEBUG
		printf("pushed double switched to single\n");
#endif
		}
	} else {				/* display mode is double */
	    if((cfr & 0x0f) == 0x0f) {		/* popped mode is single */
		cfr = oldcfr;			/* stay in double */
#ifdef DEBUG
		printf("pushed single switched to double\n");
#endif
	    } else {
#ifdef DEBUG
		printf("double to double\n");
#endif
		/*
		 * test for swapbuffers between push and pop
		 */
		if ((cfr & (DISPLAYA | DISPLAYB))
		    != (oldcfr & (DISPLAYA | DISPLAYB))) {
			/*
			 * if buffers swapped between push and pop,
			 * get new update bits and swap them, but
			 * only if exactly 1 set
			 */
			updates = (UPDATEA | UPDATEB) & cfr;
			if (updates == UPDATEA)
				updates = UPDATEB;
			else if (updates == UPDATEB)
				updates = UPDATEA;

			/* clear out display and update bits	*/
			cfr &= ~(DISPLAYA | DISPLAYB | UPDATEA | UPDATEB);
			/*
			 * or in current display bits
			 *   and reconfigured update bits
			 */
			cfr |= ((DISPLAYA | DISPLAYB) & oldcfr) | updates;
		    }
	    }
	}

	if(oldcfr & (UC_DEPTHCUE<<16))
		cfr |= UC_DEPTHCUE<<16;
	else
		cfr &= ~(UC_DEPTHCUE<<16);
	WS -> curatrdata.myconfig = cfr;

#ifdef DEBUG
	printf("finally cfr is %lx\n",cfr);
#endif DEBUG
	
	im_setlinestyle(WS -> curatrdata.mylstyle);
	im_setpattern (WS -> curatrdata.mytexture);
	if (getdisplaymode() == DMRGB) {
		im_RGBcolor(WS->curatrdata.myr,WS->curatrdata.myg,
							WS->curatrdata.myb);
		im_RGBwritemask(WS->curatrdata.myrm,WS->curatrdata.mygm,
							WS->curatrdata.mybm);
	} else {
#ifdef DEBUG
		printf("writemask is %lx\n",WS -> curatrdata.mywenable);
		printf("getwritemask returns %lx\n",getwritemask());
#endif DEBUG
		im_color(WS -> curatrdata.mycolor);
		im_writemask(WS -> curatrdata.mywenable);
	}
	im_linewidth(WS -> curatrdata.mylwidth+1);
	im_lsrepeat(WS -> curatrdata.mylsrepeat+1);
	im_font(getfont());
    }
    thread;

INTERP_LABEL(pushviewport,2);
    if (WS -> vpstatep == &WS -> vpstack[VPSTACKDEPTH])
	gl_ErrorHandler (ERR_PUSHVIEWPORT,WARNING,0);
    else *WS -> vpstatep++ = WS -> curvpdata;
    thread;

INTERP_LABEL(popviewport,2);
    if (WS -> vpstatep == WS -> vpstack)
	gl_ErrorHandler (ERR_POPVIEWPORT,WARNING,0);
    else {
	WS -> curvpdata = *--WS -> vpstatep;
	WS->zmin = ((WS->curvpdata.vcz - WS->curvpdata.vsz)>>7)/2;
	WS->zmax = (((WS->curvpdata.vcz + WS->curvpdata.vsz)>>7)-2)/2;
	bra(_i_vpdoit);
    }
    thread;
}
