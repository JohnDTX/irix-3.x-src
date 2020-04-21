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
#include "glerror.h"
#include "shmem.h"
#include "TheMacro.h"
#include "imsetup.h"

#define im_feedback(buffer,words) {		\
	gl_shmemptr->intbuf = buffer;		\
	gl_shmemptr->intbuflen=words;		\
	gl_startfeed();				\
	im_outlong(0x80000 | FBCfeedback);	\
	gl_shmemptr->EOFpending++;}

#define im_endfeedback() {im_outlong((FBCEOF1<<16) | FBCEOF2);	\
			im_outshort(FBCEOF3);			\
			gl_WaitForEOF(0);			\
			im_freepipe;				\
			gl_endfeed();}

static long gl_fbsavewords;
static long gl_fbcount;
static short *gl_fbbuf; 

ROOT_1S(passthrough)

void feedback(buff,words)
short	*buff;
long	words;
{
    if(!gl_fbmode) {
	im_setup;

	gl_fbmode = 1;
	gl_fbbuf = buff;
	gl_fbsavewords = words;
	im_feedback(buff,words);
	im_cleanup;
    } else 
	gl_ErrorHandler(ERR_INPICK, WARNING, "feedback");
}

gl_suspendfeedback()
{
    im_setup;
    gl_fbmode = 0;
    im_endfeedback();
    im_cleanup;
    gl_fbcount = (gl_fbsavewords - gl_shmemptr->intbuflen);
}

gl_resumefeedback()
{
    im_setup;
    gl_fbmode = 1;
    im_feedback(&gl_fbbuf[gl_fbcount],gl_fbsavewords-gl_fbcount);
    im_cleanup;
}

long endfeedback(buff)
short	*buff;
{

    if(gl_fbmode) {
	im_setup;

	gl_fbmode = 0;
	im_endfeedback();
	im_cleanup;
	return (gl_fbsavewords - gl_shmemptr->intbuflen);
    } else 
	gl_ErrorHandler(ERR_NOTINPICK, WARNING, "endfeedback");
}

#include "interp.h"

INTERP_NAME(passthrough);

static bogus ()
{
    DECLARE_INTERP_REGS;

    INTERP_ROOT_1S(passthrough);
}
