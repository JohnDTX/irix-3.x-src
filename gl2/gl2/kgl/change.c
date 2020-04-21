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
#include "kb.h"
#include "device.h"

/*
 *	ChangeValuator - 
 *		called by input device interrupt routines to register
 *		a new value for a specific valuator.  This modifies
 *		the system valuator structure and calls ResetCursor
 *		to update the cursor position on the screen.
 *
 */
ChangeValuator(vnum, val)	/* no checking val # - we trust caller !! */
register short vnum;
register short val;
{
	register valuatordata *vd = &gl_valuators[vnum-VALOFFSET];
	register long dval, stream;

	kunblanklater();
	if (vd->absdevice == 0) {	/* relative device */
		dval = val - vd->raw;	/* difference should never be zero! */
		vd->raw = val;
		stream = vd->value + vd->offset;
		stream += dval;
		if (dval > vd->halfmaxraw) 
			stream -= (vd->halfmaxraw<<1);
		else if (dval < -vd->halfmaxraw) 
			stream += (vd->halfmaxraw<<1);
		if ((stream - vd->offset) < vd->minval) 
			vd->offset = stream - vd->minval;
		if ((stream - vd->offset) > vd->maxval) 
			vd->offset = stream - vd->maxval;
		if(vd->value != (stream - vd->offset)) {
			vd->value = stream - vd->offset;
			ResetCursor(vnum,vd);
		}
	} else {	/* absolute device (like lightpen, tablet) */
		vd->value = val;
		if (val < vd->minval)
			vd->value = vd->minval;
		if (val > vd->maxval)
			vd->value = vd->maxval;
		ResetCursor(vnum,vd);
	}
}

/*
 *	ResetCursor -
 *		a valuator has a new value - if this valuator is
 *		attached to the cursor, update the cursor postion
 *		globals, and set the cursor moved flag.
 *
 */
ResetCursor(vnum,vd)
register short vnum;
register valuatordata *vd;
{
	if(vnum == gl_cursorxvaluator) { 
		gl_cursorx = vd->value;
		gl_valuators[CURSORX-VALOFFSET].value = vd->value;
	} else if(vnum == gl_cursoryvaluator) {
		gl_cursory = vd->value;
		gl_valuators[CURSORY-VALOFFSET].value = vd->value;
	}
	updateMousePosition(gl_cursorx, gl_cursory);
}

/*
 *	DoQueueValuators -
 *		this is called every few frames from the retrace interrupt
 *		handler. This runs through all the valuators that the
 *		current process has queued, if the value has changed,  
 *		a valuator entry is entered into the queue.  This forces
 *		all the values to be sampled at the same time.
 *
 */
DoQueueValuators()
{
	register short diff;
	register valuatordata *vd = &gl_valuators[0];
	register procvaluatordata *pvd;
	register short i;
	register short didsomething = 0;
	short pri;

	if(!gl_curric)
		return;
	pvd = &gl_curric->ic_procvaluators[0];
	for(i=0; i<VALCOUNT; i++) {
		if (pvd->doqueue) {
			pri = spl6();
			diff = vd->value - pvd->oldvalue;
			if ( (diff >= pvd->noise) || (diff <= -pvd->noise) ) {
				inter_qenter(i+VALOFFSET, vd->value);
				pvd->oldvalue = vd->value;
				didsomething++;
			}
			splx(pri);
		}
		vd++;
		pvd++;
	}
	if(didsomething && (gl_curric->ic_doqueue & DQ_VALMARK))
		inter_qenter(VALMARK,gl_framecount);
}

/*
 *	ChangeButton -
 *		A button has changed state.  If the button is reserved
 *		then use the input channel of that process.  If the button
 *		is queued, enter a value in the queue unless the SETUP key
 *		is down.  If valuators are tied to this button, also enter
 *		the tied valuator values in the queue.
 *
 *		Special stuff has to be done if there is a window manager 
 *		running, and the right mouse button goes down outside
 *		the current input window. In this case input focus is 
 *		moved to the window manager.
 */
ChangeButton(bnum, state)	/* no check on button no - trusted caller */
register short bnum;
register short state;
{
    register buttondata *bd;
    register procbuttondata *pbd = (procbuttondata *)0;
    register struct inputchan *ic = gl_curric;
    static struct inputchan *downic;
    int pri;

    kunblanklater();
    pri = spl6();
    state = (state != 0) ? 1 : 0;
    gl_buttons[bnum].state = state;

    /*
     * Pass MENUBUTTON through to window manager, IFF
     *	(a) the window manager is running
     *	(b) the current input process isn't getting the MENUBUTTON
     *	(c) or, the button down stroke is outside the current input processes
     *	    window
     */
    if(bnum == MENUBUTTON && gl_wmport) {
	if (!ic || !ic->ic_procbuttons[MENUBUTTON].doqueue
	       				|| (state&&cursoroutside()) ) {
	    ic = gl_wmport;
	}
    }

    /*
     * When we see a downstroke on the MENUBUTTON, we record it so that we can
     * give the upstroke to the same input channel.
     */
    if (bnum == MENUBUTTON) {
	if (state)
		downic = ic;
	else {
		/*
		 * Up stroke on MENUBUTTON.  Pass up event to the same ic
		 * that we used for the down stroke.
		 */
		ic = downic;
	}
    }

    if(gl_buttons[bnum].reserved)
	ic = gl_buttons[bnum].reserved;
    if(ic)
	pbd = &ic->ic_procbuttons[bnum];

/* if button is queue'd add it to the queue unless SETUP key is down */
    if (pbd && ic->ic_shmemptr && pbd->doqueue && 
			(!gl_buttons[SETUPKEY].state || bnum == SETUPKEY)) {
	gr_qenter(ic,bnum, state);
	if (pbd->tiedevice1)
	    tieqenter(ic,pbd->tiedevice1);
	if (pbd->tiedevice2)
	    tieqenter(ic,pbd->tiedevice2);
	splx(pri);
	return 1;	/* gl used the button */
    } else {
	splx(pri);
	return 0;	/* gl didn't use the button */
    }
}

/*
 *	tieqenter -
 *		used by ChangeButton to queue valuators tied to a button
 *
 */
tieqenter(ic,vnum)
register struct inputchan *ic;
short vnum;
{
    if(ISTIMER(vnum))
        gr_qenter(ic,vnum,gl_framecount);
    else
        gr_qenter(ic,vnum,gl_valuators[vnum-VALOFFSET].value);
}

/*
 * 	txinputchannel - 
 *		attach the keyboard to the given textport
 *
 */
txinputchannel(no)
    short no;
{
    gl_textportno = no;
}

/*
 * 	gfinputchannel - 
 *		attach the keyboard to the given graphport.  If the
 *		input focus has changed, and the user wants INPUTCHANGE
 *		events, then queue INPUTCHANGE events.
 *
 */
gfinputchannel(no)
    short no;
{
    register struct gfport *gf;

    if(gl_curric && (gl_curric->ic_gf->gf_no != no) &&
			(gl_curric->ic_doqueue & DQ_INPUTCHANGE))
	gr_qenter(gl_curric,INPUTCHANGE,0);
    if((no >= 0) && ((gf = &gfport[no])->gf_ic)) {
	gl_curric = gf->gf_ic;	/* current input gfport */
	if(gl_curric->ic_doqueue & DQ_INPUTCHANGE)
	    gr_qenter(gf->gf_ic,INPUTCHANGE,no);
    } else
	gl_curric = 0;
}

cursoroutside()
{
    register struct gfport *gf;
    register struct piece *p;
    register cx = gl_cursorx;
    register cy = gl_cursory;

    if(cx<2 || cy<2)
	return 1;
    if(cx>(XMAXSCREEN-2) || cy>(YMAXSCREEN-2))
	return 1;
    for(gf = gl_curric->ic_gf; gf; gf = gf->gf_next) {
	p = gf->gf_piecelist;
	while(p) {
	    if(cx>=p->p_xmin && cx<=p->p_xmax &&
	       cy>=p->p_ymin && cy<=p->p_ymax ) 
		return 0;
	    p = p->p_next;
	}
    }
    return 1;
}
