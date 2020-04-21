/*
**	These routines do special things
**
**
*/
#include <sys/types.h>
#include <stdio.h>
#include <gl.h>
#include <device.h>
#include "term.h"

static void initcommon();

void xginit()
{
    if (zflag[1])
	gbegin();
    else {
	prefsize(1023,767);
	prefposition(0,1023,0,767);
	foreground();
	getport("2");
    }
    initcommon();
}


void xgreset()
{
	greset();
}


void xgexit()
{

    if (graphinited)
	wsgexit(1);
    ingraphprog = 0;
}


void xcharstr(ptr)
u_char *ptr;
{
	charstr(ptr);
}


/*
**	initcommon - common stuff which every graphics init routine needs
**		     to do
**
*/
static void initcommon()
{
    switchtographics();
    graphinited = 1;
    ingraphprog = 1;
    gcmdcnt = 0;
    winattach();
}
 
/*
**	These routines do special things
**
**
*/



xgexit1()
{
    if (graphinited)
	wsgexit(1);
    ingraphprog = 0;
}

wsgexit(prterrcnt)
int prterrcnt;
{
    gexit();
    switchtotext();
    graphinited = 0;
}

xgbegin()
{
    gbegin();
    initcommon();
}

xgetpor(name, len)
char name[];
long len;
{
    getpor(name, len);
    initcommon();
}

xgetport(name)
char name[];
{
    xgetpor(name, strlen(name));
    initcommon();
}

long
xwinope(name, len)
char name[];
long len;
{
    long retval;

    retval = winope(name, len);
    initcommon();
    return retval;
}

long
xwinopen(name)
char name[];
{
    return xwinope(name, strlen(name));
}

xtpon()
{
    if (!ismex())
	tpon();
    ignoretext = 0;
    tpison = 1;
    /* 
     * free the keyboard -- just in case we don't get any text 
     */
    switchtotext();
}

xtpoff()
{
    if (!ismex())
	tpoff();
    ignoretext = !pflag;
    tpison = 0;
}


void bogus() {}
void iftpsetup() {}
void iftpse() {}
void gdownload() {}
void lastone() {}
