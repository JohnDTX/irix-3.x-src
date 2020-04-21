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

/* this is peter's trick to actually define the globals */
#define extern	/* nothin */

#include 	"globals.h"
#include 	"splinegl.h"
#include	"shmem.h"
#include	"device.h"
#include	"grioctl.h"
#include	"gr.h"
#include	"get.h"

char gl_version[] = GLVERSION;		/* a variable so nm can find it etc. */

gl_initglobals()
{
    gl_xtableglobs();	/* hack to get xsin globals included */
    gl_openobjhdr = 0;
    gl_openobj = -1;
    gl_fbmode = 0;
    gl_picking = 0;
    gl_pickselect = 0;
    gl_basis_list = 0;
    gl_memory_list = 0;
    gl_objchunksize = INITOBJCHUNKSIZE;
    gl_currentshade = 0;
    gl_objsizefrozen = 0;
    gl_personal_buffer_mode = DMSINGLE;
    if (gl_gralloc() == -1) {		/* no graphics today */
	printf("error: out of graph ports!!\n");
	exit(1);
    }
    if (strcmp(gl_version, gl_shmemptr->smallbuf)) {
	printf("gl: bad version, gl version is '%s', kernel is '%s'\n",
		    gl_version, gl_shmemptr->smallbuf);
	exit(1);
    }
    gl_getcharoffsets(gl_defont_offsets);
    gl_initmsg();
}
