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

/*
**	These routines do special things
**
**
*/

#include <errno.h>
#include <gl.h>
#include "term.h"

static void commonpreinit();
static void commonpostinit();

xginit()
{
#ifdef GL1
    if (graphinited)
	wsgexit(0);
    commonpreinit();
    ginit();
#else
    commonpreinit();
    if (zflag[1])
	gbegin();
    else
	ginit();
#endif
    commonpostinit();
}

xgexit()
{
    if (graphinited)
	wsgexit(1);
    ingraphprog = 0;
}

#ifndef GL1
xgbegin()
{
    commonpreinit();
    gbegin();
    commonpostinit();
}

xgetpor(name, len)
char name[];
long len;
{
    commonpreinit();
    getpor(name, len);
    commonpostinit();
}

xgetport(name)
char name[];
{
    xgetpor(name, strlen(name));
}

long
xwinope(name, len)
char name[];
long len;
{
    long retval;

    commonpreinit();
    retval = winope(name, len);
    commonpostinit();
    return retval;
}

long
xwinopen(name)
char name[];
{
    return xwinope(name, strlen(name));
}

#endif not GL1

xtpon()
{
#ifdef GL1
    tpon();
#else  GL2
    if (!ismex())
	tpon();
#endif GL2
    ignoretext = 0;
    tpison = 1;
    /* 
     * free the keyboard -- just in case we don't get any text 
     */
    switchtotext();
}

xtpoff()
{
#ifdef GL1
    tpon();
#else  GL2
    if (!ismex())
	tpoff();
#endif GL2
    ignoretext = !pflag;
    tpison = 0;
}

/*
**	commonpreinit - common stuff which every graphics init routine needs
**		        to do before the routine is actually called.
**
*/
static void commonpreinit()
{
    if (dflag[5]) {
        killwritehost();
    }
}


/*
**	commonpostinit - common stuff which every graphics init routine needs
**		         to do after the routine is actually called.
**
*/
static void commonpostinit()
{
    switchtographics();
    graphinited = 1;
    ingraphprog = 1;
    gcmdcnt = 0;
#ifndef GL1
    winattach();
#endif    
}

/*ARGSUSED*/
wsgexit(prterrcnt)
int prterrcnt;
{
#ifdef GL1
    extern unsigned short   gl_errcnt;

    if (gl_errcnt && prterrcnt) {
	errorm('w', "GL reported %d errors on screen", gl_errcnt);
	gl_errcnt = 0;
    }
#endif
    gexit();
    switchtotext();
    graphinited = 0;
}

void bogus()
{
}
