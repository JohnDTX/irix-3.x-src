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

void getgpos(x,y,z,w)
Coord	*x, *y, *z, *w;
/* Get the current graphics position.  Harder than it sounds.  The GE
   clippers are the ones that store this information, but to get it back from
   them, they must be configured as matrix multipliers and then told to pop
   their stack once.  The current graphics position is then obtained by doing
   a getmatrix() and finding it as the first column of the returned matrix.
   Because of the use of the "stripping pass-thru" command in this routine,
   only systems with REV-D GE chips will execute this routine properly.
 */
{
    register struct shmem 	*sh = gl_shmemptr;
    register short		i;
    register float		*from;
    short			buf[33];
    im_setup;

    /* Reconfigure the pipeline with the first four clippers posing
       as matrix multipliers.
     */

    gl_getgposconfig();
	
    im_outlong(0x80000 | FBCfeedback);	/* lock pipe */
    sh->intbuf = sh->smallbuf;
    sh->intbuflen = 33;
    sh->EOFpending++;
    sh->fastfeed = 1;

    /* Stripping passthru commands to give popmm and storemm commands
       to the first clipper chip.
     */

    im_outshort(0x0439);
    im_outshort(0x0339);
    im_outshort(0x0239);
    im_outshort(0x0139);

    /* Get the "matrix" and keep the first column. */

    im_outshort(GEpopmm);
    im_outshort(GEstoremm);
    im_outlong((FBCEOF1<<16) | FBCEOF2);
    im_outshort(FBCEOF3);
    gl_WaitForEOF(0);
    sh->fastfeed = 0;

    from = (float *)&(sh->smallbuf[1]);
    *x = *from++;
    *y = *from++;
    *z = *from++;
    *w = *from;

    /* Reconfigure the system normally. */
    gl_normalconfig();
    im_freepipe;
    im_cleanup;
}
