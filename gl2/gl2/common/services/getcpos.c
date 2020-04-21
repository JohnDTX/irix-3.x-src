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

void getcpos(x, y)
Screencoord *x, *y;
{
    register windowstate *ws = gl_wstatep;
    register struct shmem *sh = gl_shmemptr;
    register short	buf[3];

    gl_checkpickfeed();
    {
    im_setup;

    sh->intbuf = buf;
    sh->intbuflen = 3;
    gl_startfeed();
    im_lockpipe;
    sh->EOFpending++;
    im_passcmd(1,FBCreadcharposn);
    gl_WaitForEOF(0);
    im_freepipe;
    gl_endfeed();
    im_cleanup;
    }

    gl_restorepickfeed();

    if(buf[2] == -1) {		/* good char posn	*/
	*x = buf[0];
	*y = buf[1];
    } else {			/* invalid char posn	*/
	*x = -1;
	*y = -1;
    }
}
