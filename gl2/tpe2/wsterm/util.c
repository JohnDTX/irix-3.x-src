/**************************************************************************
 *									  *
 * 		 Copyright (C) 1985, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

#include <termio.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "term.h"
#ifdef GL1
#include <sys/types.h>
#include <gl1/kbd.h>
#include <gl1/grioctl.h>
#endif
#ifdef GL2
#include <machine/cpureg.h>
#include <gl2/grioctl.h>
#endif

#define BELL		    7

static FUNPTR osigs[NSIG + 1];

extern char *malloc();

/*
**	convspeed - encode a baud rate into a termio.h define.  Return
**		    zero if illegal.
**
*/
convspeed(speed)
int     speed;
{
    int speedcode;

    switch (speed) {
	case 75: 
	    speedcode = B75;
	    break;
	case 110: 
	    speedcode = B110;
	    break;
	case 134: 
	    speedcode = B134;
	    break;
	case 150: 
	    speedcode = B150;
	    break;
	case 300: 
	    speedcode = B300;
	    break;
	case 600: 
	    speedcode = B600;
	    break;
	case 1200: 
	    speedcode = B1200;
	    break;
	case 2400: 
	    speedcode = B2400;
	    break;
	case 4800: 
	    speedcode = B4800;
	    break;
	case 9600: 
	    speedcode = B9600;
	    break;
	case 19200: 
	    speedcode = EXTA;
	    break;
	default: 
	    speedcode = 0;
	    break;
    }
    return speedcode;
}


/*
**	savesig - save the current action of a signal
**
*/
savesig(signo, action)
int     signo;
FUNPTR  action;
{
    osigs[signo] = signal(signo, action);
}

/*
**	restoresig - restore the previous action of a signal
**
*/
restoresig(signo)
int     signo;
{
    signal(signo, osigs[signo]);
}

/*
**	putcexpc - a putc() which expands control character into "^char"
**
*/
putexpc(c, lf)
FILE *lf;
register    c;
{
    if (c >= 0x80) {
	/* use \ to indicate bit 7 on */
	putc('\\', lf);
	c &= 0x7f;
    }
    if (c < ' ' && c != '\n') {
	putc('^', lf);
	putc(c + '@', lf);
    }
    else if (c == 0177)
	fprintf(lf, "^?");
    else
	putc(c, lf);
}

/* 
** 	concat - concats two strings, mallocing space for them.  Returns
** 	NULL if malloc returns NULL.
**
*/
char *
concat(s1, s2)
register char  *s1, *s2;
{
    register char  *rv;

    if (s1 == NULL)
	return NULL;
    if (s2 == NULL)
	s2 = "";
    if (rv = malloc(strlen(s1) + strlen(s2) + 1)) {
	strcpy(rv, s1);
	strcat(rv, s2);
    }
    return rv;
}

/*
**	bellring - ring the bell
**
*/
bellring()
{
    putscreenchar(BELL);
    flushscreen();
}

/*
** 	kblamp - manipulate the keyboard lamps.  A prior ginit() is not
** 		 required to call this routine.
*/
kblamp(which, state)
int     which, state;
{
#ifdef GL1
    struct kbd  kbd;

    grioctl(GR_GETKBD, &kbd);
    if (state)
	kbd.k_ledmask |= which;
    else
	kbd.k_ledmask &= ~which;
    grioctl(GR_SETKBD, &kbd);

#else not GL1

    if (state)
	lampon(which);
    else
	lampoff(which);

#endif not GL1
}

/*
**	getdefaultspeed - read PM2 configuration switches and return
**			  serial speed
**
*/
int getdefaultspeed()
{
#ifdef GL1
    return 9600;
#else  GL2

    int switches;
    short bits;
#ifdef PM2
    static int HSpeeds[] = { 300, 1200, 19200, 9600 };
#else  IP2
    static int HSpeeds[] = { 9600, 300, 1200, 19200 };
#endif IP2

#ifdef PM2   /* for now, as this grioctl doesn't work with IP2 */
    if ((switches = grioctl(GR_GETCONFSW,0)) < 0)
	return 9600;
#endif IP2

#ifdef PM2
    return HSpeeds[HOSTSPEED(switches)];
#else  IP2
/*     	  uncomment when config switch griotcl works
 *  bits = (short)switches;
 *  return HSpeeds[ ((struct swregbits *)&bits)->sw_consspd ];
 */
    return 9600;
#endif IP2
#endif GL2
}


/*
**	goodipaddr - returns 1 if address has 3 dots, otherwise 0
**
*/
int goodipaddr(s)
register char *s;
{
    register int i;

    for (i = 0; i < 3; i++) {
	s = strchr(s,'.');
	if (s == NULL)
	    return 0;
	s++;
    }
    return 1;
}
