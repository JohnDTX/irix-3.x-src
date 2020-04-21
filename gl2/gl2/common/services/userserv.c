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
#include "glerror.h"
#include "uc4.h"

gl_WaitForRetrace()
{
    if(!gl_fbmode) {
	im_setup;
	register struct shmem *sh = gl_shmemptr;

	gl_WaitForEOF(1);
	im_freepipe;
	gsync();
    } else
	gl_ErrorHandler(ERR_INPICK, WARNING, "gl_WaitForRetrace");
}

gl_WaitForEOF(sendcmd)
short sendcmd;
{
    register struct shmem *sh = gl_shmemptr;
    if(sendcmd) {
	im_setup;

	im_passthru(1);		/* lock pipe before incrementing EOFpending */
	sh->EOFpending++;
	im_outshort(FBCeof);
	im_cleanup;
    }
    while (sh->EOFpending & EOFPENDINGBITS)
	;		/* wait for EOF to reach FBC  */
}


void gsync()	/* buzz until retrace seen */
{
    register struct shmem *sh = gl_shmemptr;

    if(!((*UCRAddr) & UCR_VERTICAL)) {
	sh->EOFpending |= VERTPENDINGBIT;
	while (sh->EOFpending & VERTPENDINGBIT) 
		;
    }
}
