/**************************************************************************
 *									  *
 * 		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *									  *
 **************************************************************************/
#include "sys/types.h"
#include "shmem.h"
#include "window.h"

/*
 * Input queue routines.  The queue is managed as a circular
 * buffer of shorts.  The entries come in pairs, where the first item
 * in a pair is the device type and the second is the value.
 * If the queue overflows, then the new information is simply lost.
 *
 */

/* 
 * inter_qenter:
 *	- adds a type-value pair to the queue, ignoring overruns.  
 *	- callable from device interupt routines (usually through
 *		ChangeValuator and ChangeButton).
 */
inter_qenter(type, value)
short type, value;
{
    gr_qenter(gl_curric,type,value);
}

/* 
 * qenter:
 *	- adds a type-value pair to the queue, ignoring overruns.  
 *	- called from grioctl only.
 */
void
qenter(type, value)
short type, value;
{
    gr_qenter(getic(),type,value);
}

/* 
 * 	gr_qenter - adds a type-value pair to the queue, ignoring overruns.  
 *
 */
gr_qenter(ic,type,value)
register struct inputchan *ic;
short type, value;
{
    register short wasempty;
    register short pri;
    register queueentry *nextin;

    if(!ic || !ic->ic_shmemptr)
	return;
    if(type == 0)
        return;
    pri = spl6();
    nextin = ic->ic_queuein+1;
    if (nextin >= &ic->ic_queue[BUFFER_SIZE])
	nextin = ic->ic_queue;

/* check for full queue */
    if (nextin == ic->ic_queueout) {
	    splx(pri);
	    return;
    }
    ic->ic_queuein->type = type;
    ic->ic_queuein->value = value;
    wasempty = (ic->ic_queuein == ic->ic_queueout);
    ic->ic_queuein = nextin;
    if (wasempty) {
	addqevent(ic);
	setqtop(ic);
	gr_signalqueue(ic->ic_oshandle);
    }
    splx(pri);
    return;
}

/*
 * Return non-zero if the queue is empty, zero otherwise
 */
int
gr_isqempty(ic)
    register struct inputchan *ic;
{
    int s;

    s = spl6();
    if (!ic || !ic->ic_shmemptr) {
	splx(s);
	return (0);
    }
    if (ic->ic_queuein == ic->ic_queueout) {
	splx(s);
	return (1);
    }
    splx(s);
    return (0);
}

/*
 * 	qreset - empties the queue. gl_queue ensures a valid gl_queue.
 *
 */
void
qreset(ic)
    register struct inputchan *ic;
{ 	
    register short pri;

    if (!ic)
	return;
    pri = spl6();
    ic->ic_queuein = ic->ic_queueout = ic->ic_queue;	
    setqtop(ic);
    splx(pri);
}

/*
 * inter_qread:
 *	- we know someting is on the queue. get it for a blocked
 *	  graphics process.
 */
unsigned long
inter_qread(value,ic)
    register short *value;
    register struct inputchan *ic;
{
    register unsigned short returnvalue;
    register short pri;

    pri = spl6();
    returnvalue = ic->ic_queueout->type;
    *value = ic->ic_queueout->value;
    if (++ic->ic_queueout>=&ic->ic_queue[BUFFER_SIZE])
	ic->ic_queueout = ic->ic_queue;
    setqtop(ic);
    splx(pri);
    return returnvalue;
}

/*
 *  	qread - if there is something in the queue, then we  
 *		return the type of that entry.  The value of the entry is 
 *		returned in the referenced variable.
 */
long
qread(value)
    register short *value;
{
    register struct inputchan *ic = getic();
    register unsigned short returnvalue;
    register short pri;

    if(!ic)
	return;
    pri = spl6();
    if(ic->ic_queuein == ic->ic_queueout) {
	splx(pri);
	return 0;			/* signal kernel that queue is empty */
    }
    returnvalue = ic->ic_queueout->type;
    *value = ic->ic_queueout->value;
    if (++ic->ic_queueout>=&ic->ic_queue[BUFFER_SIZE])
	ic->ic_queueout = ic->ic_queue;
    setqtop(ic);
    splx(pri);
    return returnvalue;
}

void
gl_anyqenter(port,type, value)
unsigned int port;
short type, value;
{
    if(port < NINCHANS)
        gr_qenter(&inchan[port],type,value);
}
