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

#include <stdio.h>
#include <ctype.h>
#include "term.h"
#include "hostio.h"
#include "rpc.h"
#include "4010.h"	

#ifdef GL1
#define STACKLONGS 276
#else GL2
#define STACKLONGS 0x3000	    /* reduce back to 276 when rcapture
				     * and colorwrite routines are changed
				     * to not allocate local arrays on
				     * the stack  SCR #1388
				     */
#endif GL2

#define MARKER	   0xBADBADAD	    /* if this is overwritten, then some
				     * GL routine's stack has exceeded
				     * the bounds of args[]
				     */

static long 	args[STACKLONGS] = { MARKER };   
				    /* Must be big enough to accomodate the
				     * maximum stack growth of any GL routine.
				     * As of 3.3.1, capture() is the routine
				     * which causes the most stack growth.
				     */

static char 	*cwa_temp;
static char 	*cwa_newsp;

unsigned 	curcmd;		    /* used in irisio.c to lookup cmd name */
extern long 	a_retval;	    /* from irisio.c */

extern int 	bogus();
#ifdef GL1
extern int  	gl_intcmd();
#endif

/*
**	readhost - read text and graphics from the host (or from pipe 1
**		   if in half-duplex).  Text is written to the textport
**		   and graphics commands executed.  Called "receive"
**		   process in man page.
**
*/
readhost()
{
    char    c;
    register int    scrtouched;

    if (dflag[1])
	openlogfile("w");
    /* 
     * OK to do this here as no other process does graphics
     */
    irisinit();

    flushscreen();

    scrtouched = 0;

    while (haveconnection) {
	if (pipeready)
	    recvpipecmd(fromwriter);
	c = gethostchar();
	if (c == TESC) {   /* graphic escape */
	    if (scrtouched) {
		flushscreen();
		scrtouched = 0;
	    }
	    doprimitive();
	}

#ifndef GL1
	else if (zflag[4]) {
	    if (c == GS /* 4010 escape character */) {
		if (scrtouched) {
		    flushscreen();
		    scrtouched = 0;
		}
		graphmode4010();
	    } else {
		alphamode4010(c);
	    }
	}
#endif not GL1

	else if (!ignoretext) {
	    switchtotext();
	    putscreenchar(c);
	    if (c == '\n' && hosttype == SERIAL_TYPE && ingraphprog) {
		c = '\r';
		putscreenchar(c);
	    /* this shouldn''t be necessary, but it is */
	    }
	    if (lf)
		putexpc(c, lf);
	    scrtouched = 1;
	}
	if (scrtouched && rc == 0) {
	    flushscreen();
	    scrtouched = 0;
	}
    }
    fprintf(stderr, "Connection closed.\n\r");
    flushscreen();

    /*
     * if there is a subshell active, wait for it to terminate
     */
    if (pipeready)
	recvpipecmd(fromwriter);
    while (insubshell) {
	pause();
	if (pipeready)
	    recvpipecmd(fromwriter);
    }
}

/*
** 	netintput - This process gets started when in half-duplex mode or
**		    when using IEEE-488.  It reads from the host
**		    connection and writes to readhost()'s input pipe.
**
*/
netinput()
{
    unsigned count;
    char   buf[RWBUFSIZE];
#if 0
int fd;
fd = open("serial.log",0401,0666);
if (fd < 0)
    errorm('F',"couldn't open serial.log");
#endif

    /* 
     * Don't read again when count is 0 -- a kill(0,SIGHUP) is generated
     * by the network driver.
     */

    while ((count = read(host, buf, RWBUFSIZE)) > 0) {
	write(outfile, buf, count);
#if 0
write(fd, buf, count);
#endif
}	
    if (count < 0)
	errorm('W',"netinput error");
    cleanup(0);
}

/*
** 	i488input - This process gets started when using IEEE-488.
**		    It reads from the IEEE-488 line and writes to
**		    readhost()'s input pipe.
**
*/
i488input()
{
    unsigned count;
    char   buf[RWBUFSIZE];
#if 0
int fd;
fd = open("488.log",0401,0666);
if (fd < 0)
    errorm('F',"couldn't open 488.log");
#endif

    while ((count = read(grhost_r, buf, RWBUFSIZE)) > 0) {
	write(outfile, buf, count);
#if 0
write(fd, buf, count);
#endif
}	
    if (count < 0)
	errorm('W',"i488input error");
    cleanup(0);
}

/*
**	irisinit - initialize the graphics.
**
*/
static 
irisinit()
{
#ifndef GL1
    gl_alwaysforeground();
#endif

    if (zflag[2] && !graphinited) {
#ifdef GL1
	ginit();
#else GL2
	gbegin();		/* leave color map alone */
#endif GL2
	graphinited = 1;
    }
    context = TEXT;
#ifndef GL1
    if (zflag[4])
	init4010();
#endif not GL1
}

/*
**	doprimitive - get a graphics command code and call the primitive
**
*/
static 
doprimitive()
{
    register unsigned cmd;
    register dispatchEntry *prim;
    register char *newsp;

    curcmd = cmd = recgcmd();
    if (lf) {
	char   *cp;
	switchtographics();
	fprintf(lf, "%d: ", gcmdcnt++);
	cp = getcmdname(cmd);
	if (!isdigit(*cp) && strcmp(cp,"bogus"))
	    fprintf(lf, "%s ( ", cp);
	else
	    fprintf(lf, "gcmd=%d ( ", cmd);
	fflush(lf);
    }

    if (cmd >= maxcom) {
	errorm('w', "command code out of range: %d", cmd);
	return;
    }
    prim = &dispatch[cmd];
    newsp = (char *)(&args[STACKLONGS - 1]) - (prim->framesize);
    if (receivef(prim->format, newsp) >= 0) {
	if (lf)
	    logrecv(prim->format, newsp);
	switchtographics();
#ifdef GL1
	if (prim->func == 0) {
	    newsp -= 4;
	    *((long *) newsp) = prim->token << 2;
	    newsp -= 4;
	    *((long *) newsp) = prim->arg;
	    cwa_newsp = newsp;

	    asm("movl sp, _cwa_temp");/* sorry kipp and bruce */
	    asm("movl _cwa_newsp, sp");
	    a_retval = gl_intcmd();
	    asm("movl _cwa_temp, sp");
	    gflush();
	}
	else
#endif GL1

	if (prim->func == bogus) {
	    errorm('w', "bogus command %d - using the right remote library?",
			cmd);
	}
	else {
	    cwa_newsp = newsp;

	    asm("movl sp, _cwa_temp");/* sorry kipp and bruce */
	    asm("movl _cwa_newsp, sp");

	    a_retval = (prim->func) ();
	    asm("movl _cwa_temp, sp");
	    if (args[0] != MARKER) {
		errorm('f', 
		       "%s exceeded stack depth limit - increase STACKLONGS",
		       getcmdname());
	    }


#ifdef GL1
	    /* 
	     * first couple of cmds aren''t really graphics, e.g setfastcom
	     */
	    if (graphinited)
		gflush();
	    senddata(prim->format);
#else  not GL1
	    if (prim->returnsdata)
		senddata(prim->format);
#endif not GL1
	}
	if (lf)
	    logsend(prim->format);
    }	     
    else if (lf) {
	fprintf(lf, "receivef error )\n");
	fflush(lf);
    }

    freearrays();
}

/*
** switchtotext - make context be text
**
*/
switchtotext()
{
    if (context == TEXT)
	return;

    kblamp(LAMP_KBDLOCKED, 0);
    context = TEXT;
    if (writehostpid > 0)
	sendpipecmd(towriter, SWITCHCONTEXT, context);
    if (lf)
	fprintf(lf, "<TEXT>\n");
}

/*
** switchtotext - make context be graphics
**
*/
switchtographics()
{
    if (context == GRAPHICS)
	return;

    kblamp(LAMP_KBDLOCKED, 1);
    context = GRAPHICS;
    if (writehostpid > 0)
	sendpipecmd(towriter, SWITCHCONTEXT, context);
    if (lf)
	fprintf(lf, "<GRAPHICS>\n");
}

