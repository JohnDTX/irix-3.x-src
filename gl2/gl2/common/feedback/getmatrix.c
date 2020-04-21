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

void getmatrix(fmat)
register Matrix fmat;
{
    register struct shmem *sh;

#ifdef UNDEF
    if (gl_checkpick("getmatrix"))	/* getmatrix illegal in pick mode */
	return;
#endif

    gl_checkpickfeed();

    {
	im_setup;

	im_outlong(0x80000 | FBCfeedback);	/* lock pipe */
	sh = gl_shmemptr;
	sh->intbuf = sh->smallbuf;
	sh->intbuflen = 33;
	sh->EOFpending++;
	sh->fastfeed = 1;
	im_outshort(GEstoremm);
	im_outlong((FBCEOF1<<16) | FBCEOF2);
	im_outshort(FBCEOF3);
	gl_WaitForEOF(0);
	im_freepipe;
	sh->fastfeed = 0;
	im_cleanup;
    }
    {
	register float *p;
	p = (float *)(sh->smallbuf + 1);
	fmat[0][0] = *p++;
	fmat[1][0] = *p++;
	fmat[2][0] = *p++;
	fmat[3][0] = *p++;
	fmat[0][1] = *p++;
	fmat[1][1] = *p++;
	fmat[2][1] = *p++;
	fmat[3][1] = *p++;
	fmat[0][2] = *p++;
	fmat[1][2] = *p++;
	fmat[2][2] = *p++;
	fmat[3][2] = *p++;
	fmat[0][3] = *p++;
	fmat[1][3] = *p++;
	fmat[2][3] = *p++;
	fmat[3][3] = *p++;
    }

    gl_restorepickfeed();
}
