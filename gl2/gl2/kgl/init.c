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
#include "mouse.h"
#include "dials.h"

gl_initinput(ic)
    	register struct inputchan *ic;
{
    	register short i;
    	register buttondata *but;
    	register valuatordata *val;
    	register procbuttondata *pbut;
    	register procvaluatordata *pval;
    	register proctimerdata *ptim;

	if(!ic)
	    return;
	pbut = &ic->ic_procbuttons[0];
	for (i=0; i<BUTCOUNT; i++) {
		pbut->doqueue = 0;
		pbut->tiedevice1 = 0;
		pbut->tiedevice2 = 0;
		pbut++;
	}
	pval = &ic->ic_procvaluators[0];
	for (i=0; i<VALCOUNT; i++) {
		pval->doqueue = 0;
		pval->oldvalue = XMAXSCREEN/2;
		pval->noise = 1;
		pval++;
	}
	ptim = &ic->ic_proctimers[0];
	for (i=0; i<TIMCOUNT; i++) {
		ptim->doqueue = 0;
		ptim->noise = 10000;
		ptim++;
	}
	ic->ic_doqueue = 0;
	ic->ic_errordevice = 1;
	qreset(ic);
	gl_initbuttons();
    	gl_initmouse();
    	gl_initcursor();
	gl_initlpen();
    	kb_init();
	if(!gl_wmport)
		attachcursor(MOUSEX,MOUSEY);
}

/*
 *   	gl_initbuttons - init the button structures
 *
 */
gl_initbuttons()
{
    	register short i;
    	register buttondata *but;
	static short firsted = 0;

	if(!firsted) {
	    but = &gl_buttons[0];
	    for (i=0; i<BUTCOUNT; i++) {
		    but->state = 0;
		    but++;
	    }
	    firsted++;
	}
}

/*
 *   	gl_initmouse - Initially place the mouse in the center of the screen.
 *
 */
gl_initmouse()
{
	static short firsted = 0;

	gl_initval(MOUSEX,MOUSEMAXRAW,XMAXSCREEN,firsted);
	gl_initval(MOUSEY,MOUSEMAXRAW,YMAXSCREEN,firsted);
	firsted = 1;
}

/*
 *   	gl_initcursor - Init cursor valuator
 *
 */
gl_initcursor()
{
	static short firsted = 0;

	gl_initval(CURSORX,0,XMAXSCREEN,firsted);
	gl_initval(CURSORY,0,YMAXSCREEN,firsted);
	firsted = 1;
}

/*
 *   	gl_initdials - init the dials valuator structures
 *
 */
gl_initdials()
{
	register short i;
	static short firsted = 0;

	for(i=0; i<DIALCOUNT; i++) 
	    gl_initval(DIAL0+i,DIALMAXRAW,XMAXSCREEN,firsted);
	firsted = 1;
}

/*
 *   	gl_initlpen - init the light pen
 *
 */
gl_initlpen()
{
	static short firsted = 0;

        gl_initval(LPENX,0,XMAXSCREEN,firsted);
        gl_initval(LPENY,0,YMAXSCREEN,firsted);
	firsted = 1;
}

/*
 *   	gl_initbitpad - init the bit pad valuator structures
 *
 */
gl_initbitpad()
{
	register short i;
	static short firsted = 0;

	gl_initval(BPADX,0,32000,firsted);
	gl_initval(BPADY,0,32000,firsted);
	firsted = 1;
}

gl_initval(vnum,maxraw,maxval,firsted)
int vnum, maxraw, maxval;
int firsted;
{
	register valuatordata *val;
	register procvaluatordata *pval;

	val = &gl_valuators[vnum-VALOFFSET];
	if(!firsted) {
	    if(maxraw == 0)
		    val->absdevice = 1;
	    else
		    val->absdevice = 0;
	    val->raw = 0;
	    val->value = maxval/2;
	    val->offset = 0;
	    val->halfmaxraw = maxraw/2;
        }
	if(gl_wmport)
	    setvaluator(vnum,val->value,0,maxval);
 	else
	    setvaluator(vnum,maxval/2,0,maxval);
}
