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

void gexit () 
{ 
    gl_initinput(); 
    gl_grfree(); 
    gl_ginited = 0;
}

#include "shmem.h"
#include "imsetup.h"
#include "uc4.h"

short FBCdump[2060];

void gl_dumpFBC ()
{
    register i;

    gl_checkpickfeed();
    {
	im_setup;

	gl_shmemptr->intbuf = FBCdump;		/* this must be FIXED?? */
	gl_shmemptr->intbuflen = 2060;
	gl_startfeed();

	im_passcmd(1,FBCsaveregs);
	for (i=0; i<2048; i+=16) {
		im_passcmd(2,FBCdumpram);
		im_outshort((short)i);
	}
	im_freepipe;
	gl_endfeed();
	im_cleanup;
    }
    gl_restorepickfeed();
}


void gl_prflags ()
{
    GEflags = GERUNMODE;
    printf("\n   FBCflags = %x",FBCflags);
    printf("\n   GEflags  = %x",GEflags);
    printf("\n   UCflags  = %x",*UCRAddr);
}
