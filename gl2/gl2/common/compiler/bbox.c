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
#include "imdraw.h"
#include "shmem.h"
#include "uc4.h"

#define BBOX_SIZE 12
extern int i_bbox2(), i_bbox2i(), i_bbox2s();

void bbox2(xmin, ymin, x1, y1, x2, y2)
    Screencoord xmin, ymin;
    Coord x1, y1, x2, y2;
{
    if (gl_openobjhdr == 0) return;
    if (gl_checkspace(BBOX_SIZE) == 0) return;
    BEGINCOMPILE(BBOX_SIZE);
    ADDADDR(i_bbox2);
    ADDSHORT(xmin);
    ADDSHORT(ymin);
    ADDFLOAT(x1);
    ADDFLOAT(y1);
    ADDFLOAT(x2);
    ADDFLOAT(y2);
    ENDCOMPILE;
}

void bbox2i(xmin, ymin, x1, y1, x2, y2)
    Screencoord xmin, ymin;
    Icoord x1, y1, x2, y2;
{
    if (gl_openobjhdr == 0) return;
    if (gl_checkspace(BBOX_SIZE) == 0) return;
    BEGINCOMPILE(BBOX_SIZE);
    ADDADDR(i_bbox2i);
    ADDSHORT(xmin);
    ADDSHORT(ymin);
    ADDICOORD(x1);
    ADDICOORD(y1);
    ADDICOORD(x2);
    ADDICOORD(y2);
    ENDCOMPILE;
}

void bbox2s(xmin, ymin, x1, y1, x2, y2)
    Screencoord xmin, ymin;
    Scoord x1, y1, x2, y2;
{
    if (gl_openobjhdr == 0) return;
    if (gl_checkspace(BBOX_SIZE-4) == 0) return;
    BEGINCOMPILE(BBOX_SIZE-4);
    ADDADDR(i_bbox2s);
    ADDSHORT(xmin);
    ADDSHORT(ymin);
    ADDSCOORD(x1);
    ADDSCOORD(y1);
    ADDSCOORD(x2);
    ADDSCOORD(y2);
    ENDCOMPILE;
}

#include "interp.h"

INTERP_NAME(bbox2);
INTERP_NAME(bbox2i);
INTERP_NAME(bbox2s);


static bogus ()
{
    DECLARE_INTERP_REGS;
    static short xmin,ymin;

#undef im_outfloat
#undef im_last_outfloat
#define im_outfloat(x)	*(long *)GE = x
#define im_last_outfloat(x)	*(long *)LASTGE = x



#define BBOX_SETUP	\
	xmin = *(short *)PC++;	\
	ymin = *(short *)PC++;	\
	llx = *PC++;	\
	lly = *PC++;	\
	urx = *PC++;	\
	ury = *PC++;	\
	gl_checkpickfeed();			\
	im_passcmd(1,FBCfeedback);	/* lock pipe */ \
	sh->intbuf = sh->smallbuf;	\
	sh->intbuflen = (sizeof(sh->smallbuf)/sizeof(short));	\
	sh->smallbuf[0] = 0;	\
	sh->EOFpending++;	\
	sh->fastfeed = 1
#define BBOX_SETUP_S	\
	xmin = *(short *)PC++;	\
	ymin = *(short *)PC++;	\
	llx = *(short *)PC++;	\
	lly = *(short *)PC++;	\
	urx = *(short *)PC++;	\
	ury = *(short *)PC++;	\
	gl_restorepickfeed();		\
	im_passcmd(1,FBCfeedback);	/* lock pipe */ \
	sh->intbuf = sh->smallbuf;	\
	sh->intbuflen = (sizeof(sh->smallbuf)/sizeof(short));	\
	sh->smallbuf[0] = 0;	\
	sh->EOFpending++;	\
	sh->fastfeed = 1
#undef im_pclos
#define im_pclos()		{GEWAIT;im_outshort(GEclosepoly);}

INTERP_LABEL(bbox2,12); /** BBOX_SIZE == 12 */
{
    register long llx,lly,urx,ury;
    register struct shmem *sh = gl_shmemptr;

    BBOX_SETUP;
    im_rectf(llx,lly,urx,ury);
    bra(_i_bboxcheck);
}

INTERP_LABEL(bbox2s, 8); /* BBOX_SIZE-4 == 8 */
{
    register short llx,lly,urx,ury;
    register struct shmem *sh = gl_shmemptr;

    BBOX_SETUP_S;
    im_rectfs(llx,lly,urx,ury);
    bra(_i_bboxcheck);
}

INTERP_LABEL(bbox2i, 12); /* BBOX_SIZE == 12 */
{
    register long llx,lly,urx,ury;
    register struct shmem *sh = gl_shmemptr;

    BBOX_SETUP;
    im_rectfi(llx,lly,urx,ury);
}

asm("_i_bboxcheck:");
{
    register short llx,lly,urx,ury,tmpxy,skipzeez;
    register short *ptr;

#define min(a,b)	((a)<(b) ? (a) : (b))
#define max(a,b)	((a)>(b) ? (a) : (b))
    /* need to put min screen x into llx, max into urx, same for y */
    im_outlong((FBCEOF1<<16) | FBCEOF2);
    im_outshort(FBCEOF3);
    gl_WaitForEOF(0);
    im_freepipe;
    gl_shmemptr->fastfeed = 0;
    ptr = (short *)gl_wstatep;
    skipzeez = (((windowstate *)ptr)->curatrdata.myconfig &
			(UC_DEPTHCUE<<16)) ||
		((windowstate *)ptr)->myzbuffer;
    ptr = gl_shmemptr->smallbuf;
    gl_restorepickfeed();
    if(*ptr == 0) jmp(_i_retsym);
    if(((*ptr++) & 0xff) == GEmovepoly) {
	llx = urx = *ptr++;
	lly = ury = *ptr++;
	if (skipzeez) ptr += 2;
	while(((*ptr++) & 0xff) == GEdrawpoly) {
	    tmpxy = *ptr++;
	    llx = min(llx,tmpxy);
	    urx = max(urx,tmpxy);
	    tmpxy = *ptr++;
	    lly = min(lly,tmpxy);
	    ury = max(ury,tmpxy);
	    if (skipzeez) ptr += 2;
	}
    }
    else {			/* bad feedback */
	jmp(_i_retsym);
    }
    if((*(ptr-1) & 0xff) != GEclosepoly) jmp(_i_retsym);
    urx -= llx;
    ury -= lly;
    if (urx < xmin && ury < ymin) jmp(_i_retsym);
    thread;
}
}
