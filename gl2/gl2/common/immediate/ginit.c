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
#include "imsetup.h"
#include "glerror.h"
#include "get.h"

void ginit()
{
    gl_doginit(1);
}

void gbegin()
{
    gl_doginit(0);
}

gl_doginit(flag)
short flag;
{
    if(!gl_ginited) {
	gl_initglobals();
	gl_ginited++;
    }
    if(gl_initobj)		/* set by makeobj */
	(gl_initobj)();
    gl_initallstackstuff();
    gl_dogreset(flag);
    swapinterval(1);
    gl_WaitForEOF(1);
    {
	im_setup;

	im_freepipe;
    }
}

void greset()
{
    gl_dogreset(1);
}

gl_dogreset(flag)
short flag;
{
    register long i,maxcolor;
    im_setup;

    if(gl_openobjhdr && gl_closeobj)    /* set by makeobj */
	(gl_closeobj)();
    if(flag) blankscreen(TRUE);
    gl_defreset ();	/* reset all the attribute tables	*/
    singlebuffer();
    onemap();
    gconfig();
    switch(getdisplaymode()) {
        case DMSINGLE:
	    writemask((1 << getplanes())-1);
	    break;
        case DMDOUBLE:
	    /*
	    gconfig doesn't do quite whats needed here.
	    writemask will be masked down on 12 bit-plane systems under mex
	    if some other program is running doublebufferd... (CDC BUG #3),
	    so need to make it full fledged in stored state with second line,
	    first line needed for side effect of realy setting the hardware.
	    ;;peter
	    */
	    writemask((1<<(2*(1+getplanes())))-1);
	    WS->curatrdata.mywenable = (1<<(2*(1+getplanes())))-1;
	    break;
	default:	/* RGB, gconfig does fine with this */
	    break;
    }
    gl_fbmode = 0;
    gl_picking = 0;
    gl_pickselect = 0;
    maxcolor = (1 << getplanes());
    if (maxcolor > 4096) maxcolor = 4096;
    if(flag) {
	gl_initallevents();	/* turn off blinking */
	mapcolor(BLACK,0,0,0);
	mapcolor(RED,255,0,0);
	mapcolor(GREEN,0,255,0);
	mapcolor(YELLOW,255,255,0);
	mapcolor(BLUE,0,0,255);
	mapcolor(MAGENTA,255,0,255);
	mapcolor(CYAN,0,255,255);
	mapcolor(WHITE,255,255,255);
    }
    setcursor(0,1,0xfff);	/* peter, as per scr 1814 */
    setlinestyle(0);
    linewidth(1);
    lsrepeat(1);
    lsbackup(0);
    resetls(1);
    setpattern(0);
    font(0);
    viewport(0,XMAXSCREEN,0,YMAXSCREEN);
    ortho2(-0.5, (float)XMAXSCREEN + 0.5, -0.5, (float)YMAXSCREEN + 0.5);
    depthcue(FALSE);
    zbuffer(FALSE);
    backface(FALSE);
    picksize(10,10);
    setdepth(0,XMAXSCREEN);		/* keep your !*!%#**ing hands off this
					 * code!
					 */
    shaderange(0,7,0,1023);
    gl_initinput();
    curson();
    blankscreen(FALSE);
}

#ifndef	CLOVER
void gflush() 
{
}

/* 
 * put these in so RGL programs which call them don't have to be changed
 * when compiled to run locally -- CSK 3/3/86
 */
Boolean setfastcom()
{
    return (TRUE);
}

Boolean setslowcom()
{
    return (TRUE);
}
#endif

/* reconfigure the iris and set the writemask to all planes	*/
void gconfig ()
{
    gl_resolvecolor ();
    if (getdisplaymode())
	writemask((1 << getplanes())-1);
    else
	RGBwritemask(0xff, 0xff, 0xff);
}

/* set the matrix to the identity	*/
gl_IdentifyMatrix (matrix)
register float *matrix;
{
    *matrix++ = 1.0;	/* row 1	*/
    *matrix++ = 0.0;
    *matrix++ = 0.0;
    *matrix++ = 0.0;
    *matrix++ = 0.0;	/* row 2	*/
    *matrix++ = 1.0;
    *matrix++ = 0.0;
    *matrix++ = 0.0;
    *matrix++ = 0.0;	/* row 3	*/
    *matrix++ = 0.0;
    *matrix++ = 1.0;
    *matrix++ = 0.0;
    *matrix++ = 0.0;	/* row 4	*/
    *matrix++ = 0.0;
    *matrix++ = 0.0;
    *matrix++ = 1.0;
}

long GL_OUT_OF_MEMORY = 0;

gl_outmem(str)
char *str;
{
    GL_OUT_OF_MEMORY = 1;
    gl_ErrorHandler(ERR_OUTMEM,WARNING,str);
}
