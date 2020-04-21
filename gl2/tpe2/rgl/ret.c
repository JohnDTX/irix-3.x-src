/*
 *		Special treatment for primitives which return arrays
 *
 *			    Paul Haeberli - Sept 1983
 */
#include "gl.h"
#include "globals.h"
#include "glerror.h"
#include "rpc.h"
#include "term.h"
#include "Vio.h"
#include "hostio.h"
#include "shmem.h"

short *pickbuf, *selectbuf, *feedbackbuf;
static long buflen;

/*
** 	VVv:pick( Fsa:0 olv )
**
*/
int spl_pick()
{
	reca();		/* receive bogus zero length array */
	buflen = recl();
	if(gl_fbmode) {
	    gl_ErrorHandler(ERR_INPICK, WARNING, "pick");
	    return;
	}
	if((pickbuf = (short *)malloc(buflen<<1)) == 0) 
	    gl_outmem("pick");
	else {
	    switchtographics();
	    pick(pickbuf,buflen);
	}
}

/*
**  	OLv:endpick( FSa:retval )
**
*/
int ret_endpick()
{
    long retval;

    if (pickbuf == 0) 
	return;
    switchtographics();
    retval = endpick(pickbuf);
    turndelay();
    sendL(retval);
    sendSs(pickbuf,buflen-gl_shmemptr->intbuflen);	/* OBVIOUSLY */
    free(pickbuf);
    pickbuf = 0;
    puthostchar('\r');
    flushhost();
}

/*
** 	VVv:select( Fsa:0 olv )
**
*/
int spl_select()
{
	reca();		/* receive bogus zero length array */
	buflen = recl();
	if(gl_fbmode) {
	    gl_ErrorHandler(ERR_INPICK, WARNING, "select");
	    return;
	}
	if((selectbuf = (short *)malloc(buflen<<1)) == 0) 
	    gl_outmem("select");
	else {
	    switchtographics();
	    select(selectbuf,buflen);
	}
}

/*
**  	OLv:endselect( FSa:retval )
**
*/
int ret_endselect()
{
    register long retval;

    if (selectbuf == 0) 
	return;
    switchtographics();
    retval = endselect(selectbuf);
    turndelay();
    sendL(retval);
    sendSs(selectbuf,buflen-gl_shmemptr->intbuflen); 	/* OBVIOUSLY */
    free(selectbuf);
    selectbuf = 0;
    puthostchar('\r');
    flushhost();
}

/*
** 	VVv:feedback( Fsa:0 Olv )
**
*/
int spl_feedback()
{
	reca();		/* receive bogus zero length array */
	buflen = recl();
	if(gl_fbmode) {
	    gl_ErrorHandler(ERR_INPICK, WARNING, "feedback");
	    return;
	}
	if((feedbackbuf = (short *)malloc(buflen<<1)) == 0) 
	    gl_outmem("feedback");
	else {
	    switchtographics();
	    feedback(feedbackbuf,buflen);
	}
}

/*
**  	OLv:endfeedback( FSa:retval )
**
*/
int ret_endfeedback()
{
    register long retval;

    if (feedbackbuf == 0) 
	return;
    switchtographics();
    retval = endfeedback(feedbackbuf);
    turndelay();
    sendL(retval);
    sendSs(feedbackbuf,retval);
    free(feedbackbuf);
    feedbackbuf = 0;
    puthostchar('\r');
    flushhost();
}



/*
**	VVv:getmatrix( YFr:16 )
**
*/
ret_getmatrix( )
{
    float *arg1;

    arg1 = allocFa(16);
    if (arg1 == 0)
	return;
    switchtographics();
    getmatrix(arg1);
    turndelay();
    sendFs(arg1,16);
    puthostchar('\r');
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
    if (arg2 == 0)
	return;
    switchtographics();
    retval = readpixels( arg1, arg2 );
    turndelay();
    sendL(retval);
    sendSs(arg2,retval);
    puthostchar('\r');
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
    if (arg2 == 0 || arg3 == 0 || arg4 == 0)
	return;
    switchtographics();
    retval = readRGB( arg1, arg2, arg3, arg4 );
    turndelay();
    sendL(retval);
    sendBs(arg2,retval);
    sendBs(arg3,retval);
    sendBs(arg4,retval);
    puthostchar('\r');
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
    if (arg1 == 0 )
	return;
    switchtographics();
    retval = blkqread(arg1,arg2);
    turndelay();
    sendL(retval);
    sendSs(arg1,retval);
    puthostchar('\r');
    flushhost();
}
