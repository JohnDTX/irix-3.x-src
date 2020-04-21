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
#include "sys/types.h"
#include "shmem.h"
#include "window.h"
#include "glerror.h"

void qdevice( dnum )
short dnum;
{
	gl_moddevice( dnum, 1, "qdevice: %d" );
}

void unqdevice( dnum )
short dnum;
{
	gl_moddevice( dnum, 0, "unqdevice: %d" );
}

gl_moddevice( dnum, val, str )
register short dnum, val;
char *str;
{
    register struct inputchan *ic = getic();

    if(!ic)
	return;
    gl_useval(dnum);
    gl_usebut(dnum);
    if(dnum == WINCLOSE)
	gl_modbit(ic,val,DQ_WINCLOSE);
    else if (ISBUTTON(dnum))
	ic->ic_procbuttons[dnum].doqueue = val;
    else if (ISVALUATOR(dnum)) {
	register procvaluatordata *pval;

	pval = &ic->ic_procvaluators[dnum-VALOFFSET];
	if(val && !pval->doqueue)
	    pval->oldvalue = gl_valuators[dnum-VALOFFSET].value;
	pval->doqueue = val;
    } else if (ISTIMER(dnum)) {
	modtimer(dnum,val);
    } else if(dnum == KEYBD)
	gl_modbit(ic,val,DQ_KEYBOARD);
    else if(dnum == RAWKEYBD)
	gl_modbit(ic,val,DQ_RAWKEYBOARD);
    else if(dnum == GERROR)
	gl_modbit(ic,val,DQ_ERRORS);
    else if(dnum == VALMARK)
	gl_modbit(ic,val,DQ_VALMARK);
    else if(dnum == REDRAW)
	gl_modbit(ic,val,DQ_REDRAW);
    else if(dnum == MODECHANGE)
	gl_modbit(ic,val,DQ_MODECHANGE);
    else if(dnum == INPUTCHANGE)
	gl_modbit(ic,val,DQ_INPUTCHANGE);
    else if(dnum == PIECECHANGE)
	gl_modbit(ic,val,DQ_PIECECHANGE);
    else
	gl_ErrorHandler(ERR_BADDEVICE, WARNING, str, dnum);
}

isqueued(ic,dnum)
register struct inputchan *ic;
short dnum;
{
    if(dnum == WINCLOSE)
	return (ic->ic_doqueue & DQ_WINCLOSE);
    else if (ISBUTTON(dnum))
	return ic->ic_procbuttons[dnum].doqueue;
    else if (ISVALUATOR(dnum)) 
	return ic->ic_procvaluators[dnum-VALOFFSET].doqueue;
    else if (ISTIMER(dnum)) 
	return ic->ic_proctimers[dnum-TIMOFFSET].doqueue;
    else if(dnum == KEYBD)
	return (ic->ic_doqueue & DQ_KEYBOARD);
    else if(dnum == RAWKEYBD)
	return (ic->ic_doqueue & DQ_RAWKEYBOARD);
    else if(dnum == GERROR)
	return (ic->ic_doqueue & DQ_ERRORS);
    else if(dnum == VALMARK)
	return (ic->ic_doqueue & DQ_VALMARK);
    else if(dnum == REDRAW)
	return (ic->ic_doqueue & DQ_REDRAW);
    else if(dnum == MODECHANGE)
	return (ic->ic_doqueue & DQ_MODECHANGE);
    else if(dnum == INPUTCHANGE)
	return (ic->ic_doqueue & DQ_INPUTCHANGE);
    else if(dnum == PIECECHANGE)
	return (ic->ic_doqueue & DQ_PIECECHANGE);
    else
	gl_ErrorHandler(ERR_BADDEVICE, WARNING, "isqueued %d", dnum);
	return 0;
}

gl_modbit(ic,val,devbit)
struct inputchan *ic;
long val, devbit;
{
    if(val)
	ic->ic_doqueue |= devbit;
    else
	ic->ic_doqueue &= ~devbit;
}

gl_useval(vnum)
register short vnum;
{
    if(ISDIAL(vnum))
	dial_used();
    else if(vnum == BPADX || vnum == BPADY) {
	bitpad_used();
    }
}

gl_usebut(bnum)
register short bnum;
{
    if(ISSW(bnum))
	dial_used();
    else if(ISBPADBUT(bnum))
	bitpad_used();
}

long getbutton(bnum)
register short bnum;
{
    register struct inputchan *ic = getic();

    if(!ic)
	return;
    gl_usebut(bnum);	
    if (ISBUTTON(bnum))  
	return gl_buttons[bnum].state;
    else {
	gl_ErrorHandler(ERR_BADBUTTON, WARNING,"getbutton: %d",bnum);
	return -1;
    }
}

/*
 *	valuator device control
 *
 */

long getvaluator(vnum)
register short vnum;
{
	register short value, temp;

	gl_useval(vnum);
	if (ISVALUATOR(vnum)) 
	    return gl_valuators[vnum-VALOFFSET].value;
	else if(ISTIMER(vnum))
	    return gl_framecount&0xffff;
	else { 
	    gl_ErrorHandler(ERR_BADVALUATOR, WARNING, "getvaluator: %d",vnum);
	    return 0;
	}
}

void setvaluator(vnum, newvalue, minval, maxval)
register short vnum;
short newvalue, minval, maxval;
{
	register valuatordata *val;
	register long oraw;
	register short pri;

	gl_useval(vnum);
	if (!ISVALUATOR(vnum)) {
		gl_ErrorHandler(ERR_BADVALUATOR, WARNING, 
						"setvaluator: %d",vnum);
		return;
	}
	if(minval>maxval) 	/* this should be an error!! */
		return;
	if(newvalue<minval)  
		newvalue = minval;
	if(newvalue>maxval)  
		newvalue = maxval;
	val = &gl_valuators[vnum-VALOFFSET];
	pri = spl6();
	val->minval = minval;
	val->maxval = maxval;
	if(val->value != newvalue) {
		val->offset += (val->value-newvalue);
		val->value = newvalue;
		ResetCursor(vnum,val); 
	}
	splx(pri);
	DoQueueValuators();
}


void signalerror(errno)
long errno;
{
	gl_ErrorHandler(-errno, WARNING, "signalerror");
}

void redirecterrors(dev)	/* hacked for now -- fix me */
long dev;
{
    register struct inputchan *ic = getic();

    if(!ic)
	return;
    ic->ic_errordevice = dev;
}

void attachcursor(valx, valy)
register short valx, valy;
{
    gl_useval(valx);
    gl_useval(valy);
    if ( (!ISVALUATOR(valx)) || (!ISVALUATOR(valy)) ) {
	gl_ErrorHandler(ERR_BADVALUATOR, WARNING, 
				"attachcursor: %d or %d", valx,valy);
	return;
    }
    gl_cursorxvaluator = valx;
    gl_cursoryvaluator = valy;
    ResetCursor(valx,&gl_valuators[valx-VALOFFSET]);
    ResetCursor(valy,&gl_valuators[valy-VALOFFSET]);
}

void getcursorpos(x, y)
short *x, *y;
{
    *x = gl_cursorx;
    *y = gl_cursory;
}

void noise(dnum, delta)
register short dnum;
register short delta;
{
    register struct inputchan *ic = getic();
    register short activetimer;

    if(!ic)
	return;
    gl_useval(dnum);
    if (ISVALUATOR(dnum)) {
	ic->ic_procvaluators[dnum-VALOFFSET].noise = (delta<=0) ? 1:delta;
	DoQueueValuators();
    } else if (ISTIMER(dnum)) {
	activetimer = isqueued(ic,dnum);
	if(activetimer)
	    unqdevice(dnum);	
	ic->ic_proctimers[dnum-TIMOFFSET].noise = (delta <= 0) ? 1:delta;
	if(activetimer)
	    qdevice(dnum);	
    } else
	gl_ErrorHandler(ERR_BADDEVICE, WARNING,"noise: %d",dnum);
}

void tie(bnum, vnum1, vnum2)
register short bnum, vnum1, vnum2;
{
    register struct inputchan *ic = getic();

    if(!ic)
	return;
    gl_useval(vnum1);
    gl_useval(vnum2);
    gl_usebut(bnum);
    if ((vnum1 != 0) && !ISVALUATOR(vnum1) && !ISTIMER(vnum1)) {
	gl_ErrorHandler(ERR_BADVALUATOR, WARNING, "tie: %d",vnum1);
	return;
    }
    if ((vnum2 != 0) && !ISVALUATOR(vnum2) && !ISTIMER(vnum2)) {
	gl_ErrorHandler(ERR_BADVALUATOR, WARNING, "tie: %d",vnum2);
	return;
    }
    if (ISBUTTON(bnum)) {
	register procbuttondata *pbut;

	pbut = &ic->ic_procbuttons[bnum];
	pbut->tiedevice1 = vnum1;
	pbut->tiedevice2 = vnum2;
    } else {
	gl_ErrorHandler(ERR_BADBUTTON, WARNING,"tie: %d",bnum);
	return;
    }
}

gl_setcuroffset(x,y)
short x, y;
{
    gl_cursorxorgin = x;
    gl_cursoryorgin = y;
}

gl_getdev(n,buf)
register int n;
register short *buf;
{
    register int i;
    register short pri;

    pri = spl6();
    for(i=0; i<n; i++) {
        if(ISVALUATOR(*buf)) 
	    *buf = getvaluator(*buf);
        else 
	    *buf = getbutton(*buf);
   	buf++;
    }
    splx(pri);
}
