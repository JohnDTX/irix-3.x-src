/**************************************************************************
 *									  *
 * 		 Copyright (C) 1985, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/*
 *		Special treatment for primitives which return arrays
 *
 *			    Paul Haeberli - Sept 1983
 */
#include <gl.h>
#include <gl2/globals.h>
#include <gl2/glerror.h>
#include <gl2/shmem.h>
#include "term.h"
#include "hostio.h"

/*
 * Buffer for pick, select, and feedback.  Need only one since can
 * only do one of the three at a time.
 */
static short *buffer = 0;
static long buflen;

/*
** 	VVv:pick( Fsa:0 olv )
**
*/
int spl_pick()
{
	(void) reca();		/* receive bogus zero length array */
	buflen = recl();
	if(gl_fbmode) {
	    gl_ErrorHandler(ERR_INPICK, WARNING, "pick");
	    return;
	}
	if ((buffer = (short *)malloc(buflen<<1)) == 0) 
	    gl_outmem("pick");
	else {
	    switchtographics();
	    pick(buffer,buflen);
	}
}

/*
**  	OLv:endpick( FSa:retval )
**
*/
int ret_endpick()
{
    long retval;

    switchtographics();
    if (buffer == 0) {
	turndelay();
	sendL(0);
	sendSs(0,0);
    }
    else {
	retval = endpick(buffer);
	turndelay();
	sendL(retval);
	sendSs(buffer,buflen-gl_shmemptr->intbuflen);	/* OBVIOUSLY */
	free(buffer);
	buffer = 0;
    }
/*    puthostchar('\r');*/
    flushhost();
}

/*
** 	VVv:select( Fsa:0 olv )
**
*/
int spl_select()
{
	(void) reca();		/* receive bogus zero length array */
	buflen = recl();
	if(gl_fbmode) {
	    gl_ErrorHandler(ERR_INPICK, WARNING, "select");
	    return;
	}
	if ((buffer = (short *)malloc(buflen<<1)) == 0) 
	    gl_outmem("select");
	else {
	    switchtographics();
	    select(buffer,buflen);
	}
}

/*
**  	OLv:endselect( FSa:retval )
**
*/
int ret_endselect()
{
    register long retval;

    switchtographics();
    if (buffer == 0) {
	turndelay();
	sendL(0);
	sendSs(0,0);
    }
    else {
	retval = endselect(buffer);
	turndelay();
	sendL(retval);
	sendSs(buffer,buflen-gl_shmemptr->intbuflen); /* OBVIOUSLY */
	free(buffer);
	buffer = 0;
    }
/*    puthostchar('\r');*/
    flushhost();
}

/*
** 	VVv:feedback( Fsa:0 Olv )
**
*/
int spl_feedback()
{
	(void) reca();		/* receive bogus zero length array */
	buflen = recl();
	if(gl_fbmode) {
	    gl_ErrorHandler(ERR_INPICK, WARNING, "feedback");
	    return;
	}
	if ((buffer = (short *)malloc(buflen<<1)) == 0) 
	    gl_outmem("feedback");
	else {
	    switchtographics();
	    feedback(buffer,buflen);
	}
}

/*
**  	OLv:endfeedback( FSa:retval )
**
*/
int ret_endfeedback()
{
    register long retval;

    switchtographics();
    if (buffer == 0) {
	turndelay();
	sendL(0);
	sendSs(0,0);
    }
    else {
	retval = endfeedback(buffer);
	turndelay();
	sendL(retval);
	sendSs(buffer,retval);
	free(buffer);
	buffer = 0;
    }
/*    puthostchar('\r');*/
    flushhost();
}



/*
**	VVv:getmatrix( YFr:16 )
**
*/
ret_getmatrix( )
{
    float *arg1;
    int size = 16;

    arg1 = allocFa(size);
    switchtographics();
    if (arg1 == 0)
	size = 0;
    else
	getmatrix(arg1);
    turndelay();
    sendFs(arg1,size);
/*    puthostchar('\r');*/
    flushhost();
}


/*
**		OLv:readpixels( Fsv JSa:arg1 ) 
**
*/
ret_readpixels( )
{
    short arg1;
    short *arg2;
    long retval;

    arg1 = recs();
    arg2 = allocSa(arg1);
    switchtographics();
    if (arg2 == 0)
	retval = 0;
    else
	retval = readpixels( arg1, arg2 );
    turndelay();
    sendL(retval);
    sendSs(arg2,retval);
/*    puthostchar('\r');*/
    flushhost();
}


/*
**		OLv:readRGB( Fsv BBa:arg1 BBa:arg1 BBa:arg1 )
**
*/
ret_readRGB( )
{
    short arg1;
    RGBvalue *arg2;
    RGBvalue *arg3;
    RGBvalue *arg4;
    long retval;

    arg1 = recs();
    arg2 = (RGBvalue *)allocBa(arg1);
    arg3 = (RGBvalue *)allocBa(arg1);
    arg4 = (RGBvalue *)allocBa(arg1);
    switchtographics();
    if (arg2 == 0 || arg3 == 0 || arg4 == 0)
	retval = 0;
    else
	retval = readRGB( arg1, arg2, arg3, arg4 );
    turndelay();
    sendL(retval);
    sendBs(arg2,retval);
    sendBs(arg3,retval);
    sendBs(arg4,retval);
/*    puthostchar('\r');*/
    flushhost();
}

/*
**	
**   OLv:blkqread( FSa:retval Fsv )
**
**
*/
ret_blkqread( )
{
    short *arg1;
    short arg2;
    long retval;

    arg2 = recs();
    arg1 = allocSa(arg2);
    switchtographics();
    if (arg1 == 0 )
	retval = 0;
    else
	retval = blkqread(arg1,arg2);
    turndelay();
    sendL(retval);
    sendSs(arg1,retval);
/*    puthostchar('\r');*/
    flushhost();
}

/*
**	
**   VVv:getdev( Olv Fsa:arg1 FSa:arg1 )w
**
**
*/
ret_getdev( )
{
    long arg1;
    short *arg2;
    short *arg3;

    arg1 = recl();
    arg2 = recss();
    arg3 = allocSa(arg1);
    switchtographics();
    if (arg2 == 0 || arg3 == 0)
	arg1 = 0;
    else
	getdev(arg1,arg2,arg3);
    turndelay();
    sendSs(arg3,arg1);
/*    puthostchar('\r');*/
    flushhost();
}
