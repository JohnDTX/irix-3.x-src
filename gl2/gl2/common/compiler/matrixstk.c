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
#include "immatrix.h"
#include "shmem.h"
#include "glerror.h"

ROOT_0(pushmatrix)
ROOT_0(popmatrix)

#include "interp.h"

INTERP_NAME(pushmatrix);
INTERP_NAME(popmatrix);

static bogus ()
{
    DECLARE_INTERP_REGS;

    INTERP_ROOT_0(pushmatrix);
    INTERP_ROOT_0(popmatrix);
}

/* NOTE: m should be declared as struct matdata *m	*/
#define im_fastrestorematrix(m) {register long *_z = (long *)m -> mat;	\
				im_outshort(GEloadmm);		\
				im_outlong(*_z++);		\
				im_outlong(*_z++);		\
				im_outlong(*_z++);		\
				im_outlong(*_z++);		\
				im_outlong(*_z++);		\
				im_outlong(*_z++);		\
				im_outlong(*_z++);		\
				im_outlong(*_z++);		\
				im_outlong(*_z++);		\
				im_outlong(*_z++);		\
				im_outlong(*_z++);		\
				im_outlong(*_z++);		\
				im_outlong(*_z++);		\
				im_outlong(*_z++);		\
				im_outlong(*_z++);		\
				im_outlong(*_z);}

/* note: the next two routines are called in the middle of manipulating
 * pointers in the shared memory structure.  They should not free the
 * pipe.
 */

gl_hardoverflow ()
{
    gl_checkpickfeed();

    {
    im_setup;
    register short i;
    register struct shmem *sh = gl_shmemptr;

    im_outlong(0x80000 | FBCfeedback);		/* lock pipe */
    sh->intbuf = (short *) (WS->matrixstatep - 7);
    sh->intbuflen = 8 * sizeof (struct matdata)/2;
    sh->EOFpending++;
    sh->fastfeed = 1;
    im_outlong((GEstoremm<<16) | GEpopmm);
    im_outlong((GEstoremm<<16) | GEpopmm);
    im_outlong((GEstoremm<<16) | GEpopmm);
    im_outlong((GEstoremm<<16) | GEpopmm);
    im_outlong((GEstoremm<<16) | GEpopmm);
    im_outlong((GEstoremm<<16) | GEpopmm);
    im_outlong((GEstoremm<<16) | GEpopmm);
    im_outlong((GEstoremm<<16) | FBCEOF1);
    im_outlong((FBCEOF2<<16) | FBCEOF3);
    gl_WaitForEOF(0);
    sh->fastfeed = 0;

    for (i=7; --i != -1;) {	/* 7 times through the loop	*/
	WS -> matrixstatep--;
	im_fastrestorematrix(WS -> matrixstatep);
	im_outshort(GEpushmm);
    }
    WS -> matrixstatep--;
    WS -> softstacktop += 8;	/* got 8 new matrices		*/
    im_cleanup;
    }

    gl_restorepickfeed();
}

gl_hardunderflow ()
{
    im_setup;
    register short i;
    register struct matdata *p;

    i = WS -> matrixlevel;
    if (i > 8) i = 8;
    WS -> hdwrstackbottom -= i;
    p = (struct matdata *) ((int)WS -> matrixstatep + i*64 + i*2);
    while (--i != -1) {
	im_outshort (GEpushmm);
	im_fastrestorematrix (p--);
    }
    im_cleanup;
}
