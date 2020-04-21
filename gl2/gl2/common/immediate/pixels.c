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

#include "globals.h"
#include "shmem.h"
#include "imsetup.h"
#include "uc4.h"

void writepixels(n, indices)
register short	n;
register Colorindex indices[];
{
    register short amount,i;
    im_setup;

    if (n == 1) {
	im_passcmd(2,FBCdrawpixels);
	im_outshort(*indices);
    } else while (n > 0) {
	if (n > 120) {
	    im_passcmd(120 + 1,FBCdrawpixels);
	    amount = 120/8;
	    n -= 120;
	} else {
	    amount = n;
	    im_passthru(amount + 1);
	    im_outshort(FBCdrawpixels);
	    n -= amount;
	    i = amount & 7;		/* get amount % 8	*/
	    amount -= i;		/* and output those	*/
	    while (--i != -1) im_outshort(*indices++);
	    amount >>= 3;		/* then output 8 per loop	*/
	}
	while (--amount != -1) {
	    im_outlong(*(long *)indices++);
	    im_outlong(*(long *)indices++);
	    im_outlong(*(long *)indices++);
	    im_outlong(*(long *)indices++);
	}
	i = i;			/* persuade optimizer into dbra	*/
    }
    im_freepipe;
    im_cleanup;
}


/* NOTE: Colorindex must be a short (word) for this code to work right	*/
long readpixels(n, indices)
register short n;
register Colorindex indices[];
{
    register struct shmem *sh = gl_shmemptr;
    register short i,mask;


    gl_checkpickfeed();
    {
    im_setup;

    if (n > XMAXSCREEN+1) n = XMAXSCREEN+1;
    mask = WS -> bitplanemask;
    sh->intbuf=(short *)indices;/* point feedback buffer to user array	*/
    sh->intbuflen = n;		/* words in intbuf		*/
    gl_startfeed();		/* enter slow feedback mode	*/
    im_passcmd(2,FBCreadpixels);/* instruct FBC to read pixels	*/
    sh->EOFpending++;		/* do this after locking pipe	*/
    im_outshort(n);
    gl_WaitForEOF(0);		/* wait until done		*/
    im_freepipe;
    gl_endfeed();
    im_cleanup;
    }

    gl_restorepickfeed();

    n -= sh -> intbuflen;
    for (i=n; --i != -1;)
	*indices++ &= mask;
    return (n);
}

#ifdef PM2
#ifdef CLOVER
#define im_outRGBvalue(x)	im_outshort(x)
#else
#define im_outRGBvalue(x)	*(RGBvalue *)GE = x
#endif
#endif
#ifdef IP2
#define im_outRGBvalue(x)	*(short *)GE = x
#endif

void writeRGB(n, r, g, b)
register short n;
register RGBvalue r[], g[], b[];
{
    register short amount;
    register long i;
    im_setup;

    while (n > 0) {
	amount = i = n>30 ? 30:n;
	im_passthru(i+i+i + 1);
	im_outshort(FBCdrawpixels);
	n -= amount;
	while (--amount != -1) {
	    im_outRGBvalue(*r++);
	    im_outRGBvalue(*g++);
	    im_outRGBvalue(*b++);
	}
	amount = amount;	/* persuade optimizer into dbra */
    }
    im_freepipe;
    im_cleanup;
}

long readRGB(n, r, g, b)
register short n;
register RGBvalue r[], g[], b[];
{
    struct shmem *sh = gl_shmemptr;
    register short retval, amount, a3, *p;
    short buf[256*3];

    gl_checkpickfeed();

    {
    im_setup;

    if (n > XMAXSCREEN+1) n = XMAXSCREEN+1;
    retval = 0;
    sh->intbuf=(short *)buf;	/* point feedback buffer to my array	*/
				/* this is not changed by the interrupt */
				/* routines or gl_startfeed or gl_endfeed */
    while (n > 0) {
	amount = n>256 ? 256:n;
	a3 = amount + amount + amount;
	n -= amount;
	sh->intbuflen = a3;		/* words in intbuf		*/
	
	gl_startfeed();			/* enter slow feedback mode	*/
	im_passcmd(2,FBCreadpixels);	/* instruct FBC to read pixels	*/
	sh->EOFpending++;		/* do this after locking pipe	*/
	im_outshort(amount);
	gl_WaitForEOF(0);		/* wait until done		*/
	im_freepipe;
	gl_endfeed();
	amount = (a3 - sh -> intbuflen)/3;
	retval += amount;
	p = buf;
	while (--amount != -1) {
	    *r++ = *p++;
	    *g++ = *p++;
	    *b++ = *p++;
	}
	amount = amount;
    }

    im_cleanup;
    }

    gl_restorepickfeed();

    return retval;
}
