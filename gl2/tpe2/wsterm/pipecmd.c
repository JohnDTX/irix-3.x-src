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

#include <errno.h>
#include <signal.h>
#include "term.h"

/*
**	Implement IPC between readhost() and writehost() by using
**	two pipes.  Commands are written into a pipe and the other
**	process signaled with SIGUSR1.  The receiving process reads 
**	the commands from the pipe and executes them.  A command consists
**	of a command token and a single argument.
*/


/*
** 	setpipeready - set flag that there is a command awaiting in the
**		       pipe.  This is the routine called upon receipt
**		       of SIGUSR1.
**
*/
setpipeready()
{
    signal(SIGUSR1, setpipeready);
    pipeready++;
}

/*
**	sendpipecmd - send a command.  It is written to the pipe and then
**		      the receiving process signalled.
**
*/
sendpipecmd(fd, cmd, data)
int     fd, cmd, data;
{
    int     buf[2];

    buf[0] = cmd;
    buf[1] = (cmd == LOGFILENAME) ? strlen((char *)data)+1 : data;
    if (pipereaderpid == 0) {
        errorm('w', "pipereaderpid = 0");
	return;
    }
    if (write(fd, (char *)buf, sizeof buf) != sizeof buf) {
	errorm('W', "write to pipe %d failed", fd);
	return;
    }
    /* problem with this write not being atomic with first one ?? */
    if (cmd == LOGFILENAME) {
	if (write(fd, (char *)data, buf[1]) != buf[1]) {
	    errorm('W', "write to pipe %d failed", fd);
	    return;
	}
    }
    kill(pipereaderpid, SIGUSR1);
}

/*
** 	recvpipecmd - receive a command.  Commands are read from the pipe
**		      and executed until the pipe is empty.  Both readhost()
**		      and writehost() use this routine to receive commands.
*/
recvpipecmd(fd)
int     fd;
{
    register int i;
    int     buf[2];
    char    *fname;

    while (i = read(fd, (char *)buf, sizeof buf)) {
	if (i < 0) {
	    if (errno == EINTR)
		continue;
	    else {
		errorm('W', "read from pipe %d failed", fd);
		return;
	    }
	}
	if (i != sizeof buf) {
	    errorm('W', "read from pipe %d too small: %d", fd, i);
	    continue;
	}
	switch (buf[0]) {
	    /* commands to writer */

	    case SWITCHCONTEXT: 
		context = buf[1];
		kbdlocked = (context == GRAPHICS);
		break;


	    /* commands to reader */

#ifndef GL1
	    case SETMONITOR: 
		setmonitor(buf[1]);
		break;
#endif

	    case TOGGLETP:
		if (tpison)
		    xtpoff();
		else {
		    xtpon();
		    putscreenchar('\0');	/* force textport to appear */
		    flushscreen();
		}
		break;

	    case TOGGLEPFLAG: 
		pflag = buf[1];
		ignoretext = !pflag;
		break;

	    case TOGGLEXFLAG: 
		xflag = buf[1];
		break;

	    case RESETDISPLAY: 
#ifndef GL1
		noport();
#endif
		ginit();
#ifndef GL1
		if (ismex())		/* if mex is running, restore c-map */
		    rstcmap();
#endif
		xtpon();
		putscreenchar('\0');	/* force textport to appear */
		flushscreen();
		graphinited = 1;
		break;

	    case INSUBSHELL: 
		insubshell = buf[1];
		/* flush logfile output in case we want to look at it */
		if (lf && insubshell)
		    fflush(lf);
		break;

#ifndef GL1
	    case CLEAR4010:
		clearall();
		homeAlphCurs();
		break;

	    case HARDCOPY4010:
		if (fname = tempnam(SPOOL,PREFIX)) {
		    capture(fname, 0);
		    free(fname);
		    system(COLORD);
		}
		else
		    errorm('W',"can't create temporary name");
		break;
#endif not GL1

	    case TERMINATE:
		cleanup(0);
		break;

	    case TOGGLEDFLAG: 
		i = abs(buf[1]);
		dflag[i] = buf[1] < 0;
		switch (i) {
		    case 1:
			if (dflag[1])
			    openlogfile("a");
			else
			    closelogfile();
			break;
		    case 5:
			/*
			 * This would work, but can't seem to be able to
			 * get writehost() to recognize ~%D4 when both it
			 * and the debugger are reading the keyboard.
			 */
			if (dflag[5]) {
			    killwritehost();
			    fprintf(stderr," killed transmit process\n");
			}
			break;
		}
		break;

	    case TOGGLEZFLAG: 
		i = abs(buf[1]);
		zflag[i] = buf[1] < 0;
		break;

	    case LOGFILENAME:
		while ((i = read(fd, logfile, buf[1])) == 0)
		    continue;
		if (i < 0) {
		    if (errno == EINTR)
			continue;
		    else {
			errorm('W', "read from pipe %d failed", fd);
			return;
		    }
		}
		else if (i != buf[1])
		    errorm('W', "read from pipe %d too small: %d", fd, i);
		openlogfile("a");
		break;

	    default: 
		errorm('w', "unknown pipe cmd = %d data = %d\n\r",
			buf[0], buf[1]);
		break;
	}
    }
    pipeready = 0;
}
