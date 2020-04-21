
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

#include	"gl.h"
#include 	"globals.h"
#include	"shmem.h"
#include	"grioctl.h"
#include	"glipc.h"

static struct portreq defaultportreq = {
    40,  30,				/* minsizex,y */
    XMAXSCREEN+1, YMAXSCREEN+1,		/* maxsizex,y */
    0, 	 0,   0,			/* keepaspect, aspectx,y */
    0,   0,   0,			/* prefsize, prefsizex,y */
    0,   0,   0,			/* preforigin, originx,y */
    1,   1,   0,    0,			/* x,yunit, x,yunitfudge */
    0,   				/* imakebackground 	 */
    0,   				/* imakemap 		 */
};
struct portreq gl_initportreq;
static short defaultputinbg = 1;
short gl_putinbackground = 1;

gl_initmsg()
{
    struct sendrec *srec;
    short v;

    gl_hintinit();
    /* send port request to window manager (if one is running) */
    srec = (struct sendrec *)gl_shmemptr->smallbuf; 
    srec->msg = PORTREQ;
    srec->len = sizeof(struct portreq);
    gl_initportreq.gfnum = gl_shmemptr->gfnum;
    gl_initportreq.pid = getpid();
    bcopy(&gl_initportreq, srec->data, sizeof(struct portreq));
    grioctl(GR_SEND, -1);
    gl_hintinited = 0;
}

gl_hintinit()
{
    if (!gl_hintinited) {
	gl_initportreq = defaultportreq;	/* reset to default */
	gl_putinbackground = defaultputinbg;
	gl_hintinited++;
    }
}

gl_alwaysforeground()		/* for wsiris's convenience */
{
    defaultputinbg = 0;
}

