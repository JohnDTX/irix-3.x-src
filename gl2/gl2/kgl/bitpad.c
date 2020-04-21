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

/*
 * Handler for characters from summagraphics compatible digitizing pad.
 *
 */
static short state;
static short synced;
static short havecoord;
static short x, y;
static short ox, oy, obut;
static short b0, b1, b2, b3;
static short bpadusers;

#define BUTTONBITS	0x40

int bitpad_softchar();

/*
 *	bitpad_used -
 *		called any time a digitizer device number is used.	
 */
bitpad_used()
{
    static char firsted;
    register struct inputchan *ic;

    if(!firsted) {
	firsted++;
	gl_initbitpad();
    }
    allocbitpadport(getic());
}

/*
 *	bitpad_softchar -
 *		called indirectly from the hardware serial interrupt.
 *		This uses ChangeValuator and ChangeButton to let
 *		the input system know that values have changed.
 */
bitpad_softchar( onechar )
register unsigned char onechar;
{
    register short but;

    if(synced) {
	switch(state) {
	    case 0: x = onechar & 0x3f;
		    state++;
		    break;
	    case 1: x |= (onechar & 0x3f)<<6;
		    state++;
		    break;
	    case 2: y = onechar & 0x3f;
		    state++;
		    break;
	    case 3: y |= (onechar & 0x3f)<<6;
		    state++;
		    break;
	    case 4: but = (onechar & 0x3f)>>2;
		    if( (onechar & BUTTONBITS) == 0) {
  			synced = 0;
			break;
		    }
		    if(x != ox) {
			ox = x;
			ChangeValuator(BPADX,x);	
		    }
		    if(y != oy) {
			oy = y;
			ChangeValuator(BPADY,y);	
		    }
		    if(but != obut) {
			obut = but;
			if((but&1) != b0) {
			    b0 = but&1 ;
			    ChangeButton(BPAD0,b0);
			}
		 	but >>= 1;
			if((but&1) != b1) {
			    b1 = but&1 ;
			    ChangeButton(BPAD1,b1);
			}
		 	but >>= 1;
			if((but&1) != b2) {
			    b2 = but&1;
			    ChangeButton(BPAD2,b2);
			}
		 	but >>= 1;
			if((but&1) != b3) {
			    b3 = but&1;
			    ChangeButton(BPAD3,b3);
			}
		    }
		    state = 0;
		    break;
	}
    } else {
	if(onechar & BUTTONBITS)
	    synced = 1;
	state = 0;
    }
} 

allocbitpadport(ic)
register struct inputchan *ic;
{
        if(ic && !ic->ic_bpadused) {
   	    if(bpadusers == 0) {
	        gl_setporthandler(gl_bpadport,bitpad_softchar);
	        serial_ints(gl_bpadport,1);
	    }
            ic->ic_bpadused = 1;
	    bpadusers++;
	}
}

freebitpadport(ic)
struct inputchan *ic;
{
	if(ic->ic_bpadused) {
	    ic->ic_bpadused = 0;
	    if (bpadusers>0)
		bpadusers--;
	    if(bpadusers == 0) {
	        gl_setporthandler(gl_bpadport,0);
		serial_ints(gl_bpadport,0);
	    }
	}
}
