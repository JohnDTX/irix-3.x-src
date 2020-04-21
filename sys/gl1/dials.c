/*
 * Dial box and button box support
 *
 * Written by: Tom Davis
 * Unix-ized by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/sys/gl1/RCS/dials.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:28:14 $
 */

#include "../h/param.h"
#include "machine/cpureg.h"
#include "../gl1/kgl.h"
#include "../gl1/shmem.h"
#include "../gl1/dials.h"
#include "../gl1/device.h"

#undef	DEBUG

#define	NSWTCH		SWITCHCOUNT

long	dialstream[DIALCOUNT];
long	oldrawdial[DIALCOUNT];
long	oldcookeddial[DIALCOUNT];
long	dialoffset[DIALCOUNT];

u_char	savech[2];		/* space for temporary storage of dial info */
short	dialcount;		/* # of chars needed before cmd completes */

/*
 * dial_init:
 *	- initialize the dials and buttons
 */
dial_init()
{
	register short i;
	extern int (*softintr[])();
	int dial_softintr();

	softintr[3] = dial_softintr;
	dial_ints(1);			/* turn ON dial box interrupts */
	dial_putc(INITDIALBOX);		/* initialize the dial box */
	gl_dialboxinuse = -1;
	sleep((caddr_t)&gl_dialboxinuse, PWAIT);
	for (i = 0; i < DIALCOUNT ; i++)
		dialstream[i] = oldrawdial[i] = oldcookeddial[i] =
			dialoffset[i] = 0;
}

/*
 * dial_setup:
 *	- called by dial_translate when a character comes in during
 *	  initialization
 */
dial_setup()
{
	dial_putc(ASVAL);	/* request all dial changes */
	dial_putc(0xff);
	dial_putc(0xff);
	DELAY(2000);

	dial_putc(MOMTYPE);	/* set all switches to momentary */
	dial_putc(0xff);
	dial_putc(0xff);
	dial_putc(0xff);
	dial_putc(0xff);
	DELAY(2000);

	dial_putc(ASMOM);	/* set all switches to auto-send-momentary */
	dial_putc(0xff);
	dial_putc(0xff);
	dial_putc(0xff);
	dial_putc(0xff);
	DELAY(2000);

	dial_leds(0);
	dial_text("..DUCKY.");
	dialcount = 2;
}

/*
 * dial_reset:
 *	- called by the setvaluator grioctl to update a dials position
 *	  according to the users new values
 */
dial_reset(dialnum, initialvalue)
	register short dialnum;
	int initialvalue;
{
	dialnum += VALOFFSET - DIAL0;
	dialoffset[dialnum] = dialstream[dialnum] - initialvalue;
}

/*
 * dial_softintr:
 *	- called by the duart interrupt routine to translate a character
 *	  from the dial box into some sort of dial event
 */
dial_softintr(ch)
	register u_char ch;
{
	register struct shmem *sh = (struct shmem *)SHMEM_VBASE;
	register struct valuatordata *vp;
	register ushort dial, swtch;
	register long dialvalue, ddial;
	short noise;

	if (gl_dialboxinuse == -1) {
		gl_dialboxinuse = 1;
		dial_setup();
		wakeup((caddr_t)&gl_dialboxinuse);
		return;
	}
    /* new dial value */
	if ((dialcount != 2) ||
	    (DIALNBASE <= ch) && (ch < DIALNBASE + DIALCOUNT)) {
		if (dialcount) {
			savech[--dialcount] = ch;
			return;
		}
		dialcount = 2;
		dial = savech[1] - DIALNBASE;
		if ((dial < 0) || (dial > DIALCOUNT))	/* ignore junk */
			return;
		dialvalue = (savech[0]<<8) + ch;
		ddial = dialvalue - oldrawdial[dial];
		dialstream[dial] += ddial;
		if (ddial > 32768)
			dialstream[dial] -= 65536;
		else
		if (ddial < -32768)
			dialstream[dial] += 65536;
		oldrawdial[dial] = dialvalue;

	    /* clasiffy dial thingy into lightpen or not */
		if (dial == LIGHTPENX)
			vp = &sh->valuators[LPENX - VALOFFSET];
		else
		if (dial == LIGHTPENY)
			vp = &sh->valuators[LPENY - VALOFFSET];
		else
			vp = &sh->valuators[dial + DIAL0 - VALOFFSET];

		if ((dialstream[dial] - dialoffset[dial]) < vp->minvalue)
			dialoffset[dial] = dialstream[dial] - vp->minvalue;
		if ((dialstream[dial] - dialoffset[dial]) > vp->maxvalue)
			dialoffset[dial] = dialstream[dial] - vp->maxvalue;
		vp->value = dialvalue = dialstream[dial] - dialoffset[dial];
		if (vp->queue) {
			noise = vp->noise;
			if (noise <= dialvalue - oldcookeddial[dial] ||
			    noise <= oldcookeddial[dial] - dialvalue) {
				if (dial == LIGHTPENX)
					qenter(LPENX, dialvalue);
				else
				if (dial == LIGHTPENY)
					qenter(LPENY, dialvalue);
				else
					qenter(dial + DIAL0, dialvalue);
				oldcookeddial[dial] = dialvalue;
			}
		} else
			oldcookeddial[dial] = dialvalue;
		/* ResetCursor(); */
	} else

    /* switch pressed */
	if ((MOMPRESSBASE <= ch) && (ch < MOMPRESSBASE + SWITCHCOUNT)) {
		/* the next line takes care of a byte reversal problem: */
		swtch = (ch - MOMPRESSBASE)^0x18;
		if ((swtch < 0) || (swtch > NSWTCH))
			return;
		changebutton(swtch + SW0, 1);
	} else

    /* switch released */
	if ((MOMRELEASEBASE <= ch) && (ch < MOMRELEASEBASE + SWITCHCOUNT)) {
		/* the next line takes care of a byte reversal problem: */
		swtch = (ch - MOMRELEASEBASE)^0x18;
		if ((swtch < 0) || (swtch > NSWTCH))
			return;
		changebutton(swtch+SW0, 0);
	} else

    /* light pen button pressed */
	if (ch == '+')
		changebutton(LPENBUT, 1);
	else

    /* light pen button released */
	if (ch == '-')
		changebutton(LPENBUT, 0);
}

/*
 * changebutton:
 *	- change the state of a button
 */
changebutton(but, state)
	short but, state;
{
	register struct shmem *sh = (struct shmem *)SHMEM_VBASE;
	register short tie;

	sh->Buttons[but].state = state;
	if (sh->Buttons[but].queue) {
		qenter(but, state);
		if (tie = sh->Buttons[but].tiedevice1)
			qenter(tie, sh->valuators[tie-VALOFFSET].value);
		if (tie = sh->Buttons[but].tiedevice2)
			qenter(tie, sh->valuators[tie-VALOFFSET].value);
	}
}

/*
 * dial_text:
 *	- output some text to the dial box
 */
dial_text(str)
	char *str;
{
	register short i;

	dial_putc('a');
	for (i = 0; i < 8; i++) {
		if (*str)
			dial_putc(*str++);
		else
			dial_putc(' ');
	}
	DELAY(2000);
}

/*
 * dial_leds:
 *	- turn off/on leds on dial box
 */
dial_leds(bits)
	register u_long bits;
{
	dial_putc(LEDSON);		/* set leds */
	dial_putc(bits);
	dial_putc(bits >> 8);
	dial_putc(bits >> 16);
	dial_putc(bits >> 24);
	DELAY(2000);
}
