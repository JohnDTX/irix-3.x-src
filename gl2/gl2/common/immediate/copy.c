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
#include "imsetup.h"
#include "imattrib.h"
#include "uc4.h"

void rectcopy(xsrc, ysrc, xlim, ylim, xdst, ydst)
	Screencoord xsrc, ysrc;
	register Screencoord xlim;
	Screencoord ylim, xdst, ydst;
{
    register i,j,incr;
    im_setup;

    xsrc += gl_wstatep->xmin;
    ysrc += gl_wstatep->ymin;
    xlim += gl_wstatep->xmin;
    ylim += gl_wstatep->ymin;
    xdst += gl_wstatep->xmin;
    ydst += gl_wstatep->ymin;

    if(xsrc>xlim) {			/* order xsrc and xlim */
	i = xsrc;
	xsrc = xlim; 
	xlim = i;
    }
    if (ysrc == ylim) {			/* single-height span */
	im_passcmd(6,FBCcopyscreen);
	im_outshort(xsrc);
	im_outshort(ysrc);
	im_outshort(xdst);
	im_outshort(ydst);
	im_outshort(xlim);
	goto pexit;
    }
    if(ysrc>ylim) {			/* order ysrc and ylim */
	i = ysrc;
	ysrc = ylim; 
	ylim = i;
    }
    if (ysrc < ydst) { 		/* start at opp. end of rect if necessary */
	i = ylim - ysrc;
	ylim = ysrc;
	j = ydst+i;	
	i = ysrc+i;	
	incr = -1;
    } else { 
	j = ydst;
	i = ysrc;
	incr = 1;
    }
    ylim += incr;		/* magic -- tweak end adr to be 1 past */
    while(i!=ylim) {
	im_passcmd(6,FBCcopyscreen);
	im_outshort(xsrc);
	im_outshort(i);
	im_outshort(xdst);
	im_outshort(j);
	im_outshort(xlim);
	i+=incr;
	j+=incr;
    }
pexit:
    im_freepipe;
    im_cleanup;
}

