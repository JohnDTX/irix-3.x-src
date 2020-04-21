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
#include "shmem.h"
#include "window.h"
#include "device.h"

#ifdef UNIX
#define NCONS		50

static connection cons[NCONS];
static connection *freecons;
#endif

ch_init()
{
#ifdef UNIX
    register int i, j;
    register struct inputchan *ic;

    ic = &inchan[0];
    for(i=0; i<NINCHANS; i++, ic++) {
	for(j=0; j<OUTCOUNT; j++)
	    ic->ic_outcon[j] = 0; 
	ic->ic_consleep = 0;
    }
    freecons = 0;
    for(i=0; i<NCONS; i++)
	freecon(&cons[i]);
#endif
}

static freecon(con)
connection *con;
{
#ifdef UNIX
    con->next = freecons;
    freecons = con;
#endif
}

static connection *newcon()
{
#ifdef UNIX
    register connection *con;

    if(freecons) {
	con = freecons;
	freecons = freecons->next;
	return con;
    } else
	return 0;
#endif
}

ch_signal(ic,ch,val)
register struct inputchan *ic;
register int ch, val;
{
#ifdef UNIX
    register connection *con;

    con = ic->ic_outcon[ch-OUTOFFSET];
    ic->ic_consleep = 0;
    while(con) {
        gr_qenter(con->ic,con->dev,val);
        con = con->next;
	ic->ic_consleep++;
    }
    while(ic->ic_consleep)
	gr_sleep(ic->ic_oshandle, ic);
#endif
}

ch_reply(ic)
register struct inputchan *ic;
{
#ifdef UNIX
    if(ic->ic_consleep) {
	if((--ic->ic_consleep) == 0) 
	   gr_wakeup(ic->ic_oshandle, ic);
    }
#endif
}

ch_modify(outicno,outport,inicno,inport,make)
register unsigned int outicno, inicno;
register int outport, inport;
int make;
{
#ifdef UNIX
    register struct inputchan *outic, *inic;

    if(make == 2) {
	ch_init();
	return;
    }
    if(!ISOUTPUT(outport))
   	return 0;
    if(!ISINPUT(inport))
   	return 0;
    if(outicno>=NINCHANS)
   	return 0;
    if(inicno>=NINCHANS)
   	return 0;
    outic = &inchan[outicno];
    inic = &inchan[inicno];
    if(make)
	return ch_make(outic,outport,inic,inport);
    else
	return ch_break(outic,outport,inic,inport);
#endif
}

static ch_make(outic,outport,inic,inport)
register struct inputchan *outic, *inic;
register int outport, inport;
{
#ifdef UNIX
    register connection *con;

    
    ch_break(outic,outport,inic,inport);
    con = newcon();
    if(con) {
        con->ic = inic;
        con->dev = inport;
        con->next = outic->ic_outcon[outport-OUTOFFSET];
        outic->ic_outcon[outport-OUTOFFSET] = con;
	return 1;
    } else
	return 0;
#endif
}

static ch_break(outic,outport,inic,inport)
register struct inputchan *outic, *inic;
register int outport, inport;
{
#ifdef UNIX
    register connection *con, *tcon;

    con = outic->ic_outcon[outport-OUTOFFSET];
    if(con && (con->ic == inic) && (con->dev == inport)) {
        outic->ic_outcon[outport-OUTOFFSET] = con->next;
	freecon(con);
    	return 1;
    } else {
	while(con && con->next) {
	    if((con->next->ic == inic) && (con->next->dev == inport)) {
		tcon = con->next;	
		con->next = tcon->next;
		freecon(tcon);
		return 1;
	    } else
		con = con->next;
	}
    }
    return 0;
#endif
}
