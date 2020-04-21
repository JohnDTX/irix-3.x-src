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
/*
 * Dial box and button box support
 *
 * Written by: Tom Davis
 * Unix-ized by: Kipp Hickman
 * Generalized-ized by: Paul Haeberli
 * Edited twice by: Rocky Rhodes
 * de-crashized by Peter Broadwell and Rob Myers.
 *
 */
#include "sys/types.h"
#include "shmem.h"
#include "window.h"
#include "dials.h"

#undef	DEBUG

#define	NSWTCH		SWITCHCOUNT
#define	NDIAL		(DIALCOUNT - 2)	/* top two are light pen */

#define	DELAY(x)	{ register y; y = x; while (y--); }

static unsigned char	savech[2]; /* space for temp storage of dial info */
static short		dialcount; /* # of chars needed before cmd completes */
static short 		dialboxstate;
static short 		dialusers;

int dial_softchar();

dial_used()
{
    static char firsted = 0;

    if(!firsted) {
	firsted++;	/* prevents recusion through
			gl_initdials
			gl_initval
			setvaluator
			gl_useval
			dial_used
			... */
	gl_initdials();
	allocdialport(getic());

	mydial_putc(INITDIALBOX);		/* initialize the dial box */
	dialboxstate = 1;
    } else {
	allocdialport(getic());
    }
}

/*
 * dial_setup:
 *	- called by dial_softintr when a character comes in during
 *	  initialization
 */
dial_setup()
{
	mydial_putc(ASVAL);	/* request all dial changes */
	mydial_putc(0xff);
	mydial_putc(0xff);
	DELAY(2000);

	mydial_putc(MOMTYPE);	/* set all switches to momentary */
	mydial_putc(0xff);
	mydial_putc(0xff);
	mydial_putc(0xff);
	mydial_putc(0xff);
	DELAY(2000);

	mydial_putc(ASMOM);	/* set all switches to auto-send-momentary */
	mydial_putc(0xff);
	mydial_putc(0xff);
	mydial_putc(0xff);
	mydial_putc(0xff);
	DELAY(2000);

	setdblights(0);
	dbtext("..IRIS..");
	dialcount = 2;
}

dial_softchar(ch)
	register unsigned char ch;
{
	register unsigned short dial, swtch;
	register long dialvalue;

	if (dialboxstate == 1) {
		dialboxstate = 2;
		dial_setup();
		return;
	} 

    /* new dial value */
	if ((dialcount != 2) ||
		((ch >= DIALNBASE ) && (ch < (DIALNBASE + DIALCOUNT))) ) {
		if (dialcount) {
			savech[--dialcount] = ch;
			return;
		}
		dialcount = 2;
		dial = savech[1] - DIALNBASE;
		dialvalue = (savech[0]<<8) + ch;
		if ((dial >= 0) && (dial < NDIAL))	/* ignore junk */
		    ChangeValuator(dial + DIAL0,dialvalue);
		else
		    return;
	} else if ((MOMPRESSBASE <= ch) && 		/* switch pressed */
				(ch < MOMPRESSBASE + SWITCHCOUNT)) {
		swtch = (ch - MOMPRESSBASE)^0x18;
		if ((swtch < 0) || (swtch > NSWTCH))
			return;
		ChangeButton(swtch+SW0,1);
	} else if ((MOMRELEASEBASE <= ch) && 		/* switch released */
	    			(ch <= MOMRELEASEBASE + SWITCHCOUNT - 1)) {
		swtch = (ch - MOMRELEASEBASE)^0x18;
		if ((swtch < 0) || (swtch > NSWTCH))
			return;
		ChangeButton(swtch+SW0,0);
	}
}

/*
 * dbtext:
 *	- output some text to the dial box
 */
void dbtext(str)
	char *str;
{
	register short i;

	dial_used();
	waitfordbinittohappen();
	mydial_putc('a');
	for (i = 0; i < 8; i++) {  /* special commendation to kipp for this */
		if (*str)
			mydial_putc(*str++);
		else
			mydial_putc(' ');
	}
	DELAY(2000);
}

/*
 * setdblights:
 *	- turn off/on leds on dial box
 */
void setdblights(bits)
	register unsigned long bits;
{
	dial_used();
	waitfordbinittohappen();
	mydial_putc(LEDSON);		/* set leds */
	mydial_putc(bits & 0xff);
	mydial_putc((bits >> 8) & 0xff);
	mydial_putc((bits >> 16) & 0xff);
	mydial_putc((bits >> 24) & 0xff);
	DELAY(2000);
}

waitfordbinittohappen()
{
	register i;
	for (i = 0; i < 1000; i++) {
	    if (dialboxstate == 2) return;
	    DELAY(1000);
	}
	return;
}

gl_devport(dev,port)
int dev, port;
{
    if((port>=1) && (port<=3)) {
        if(ISDIAL(dev) || ISSW(dev)) {
	    gl_dialport = port;
        } else if(ISBPADBUT(dev) || dev == BPADX || dev == BPADY) {
	    gl_bpadport = port;
	}
    }
}

static mydial_putc(c)
int c;
{
    duxputchar(c,gl_dialport);
}

allocdialport(ic)
register struct inputchan *ic;
{
        if(ic && !ic->ic_dialused) {
   	    if(dialusers == 0) {
	        gl_setporthandler(gl_dialport,dial_softchar);
	        serial_ints(gl_dialport,1);
	    }
            ic->ic_dialused = 1;
	    dialusers++;
 	}
}

freedialport(ic)
struct inputchan *ic;
{
	if(ic->ic_dialused) {
	    ic->ic_dialused = 0;
	    if (dialusers>0)
		dialusers--;
	    if(dialusers == 0) {
	        gl_setporthandler(gl_dialport,0);
		serial_ints(gl_dialport,0);
	    }
	}
}
