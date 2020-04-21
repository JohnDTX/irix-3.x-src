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

#include <sys/param.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include "term.h"
#include "hostio.h"
#include "4010.h"

static char buf[1024];		/* block mode requires 1024 byte buffer */
static int  subshellpid;	/* pid of subshell */
	
extern char *getenv();
extern char **environ;

/*
** 	writehost - take keyboard input, process escapes, and send it 
**		    to the host (and to readhost() input pipe if half-
**		    duplex).  Called "transmit" process in man page.
**
*/
writehost()
{
    register int cc;
    register char  *p, last;

    last = '\r';

    while (1) {
	if (pipeready)
	    recvpipecmd(fromreader);
	cc = read(ttyfd, p = buf, sizeof buf);
	if (cc == 0)
	    break;
	if (cc < 0) {
	    if (errno == EINTR)
		continue;
	    else {
		errorm('W',"error reading from tty");
		break;
	    }
	}
	if (!noesc) {
	    /* *** doesn't detect ...\n~ *** */
	    if (*p == escchar && (last == '\r' || last == '\n')) {
		kblamp(LAMP_LOCAL,1);
		p++;
		cc--;
		if (cc <= 0) {
		    *p = '\0';
		    if (read(ttyfd, p, 1) > 0)
			cc++;
		}
		if (cc > 0) {
		    if (escape(*p))
			cc = 0;
		    else {
			p++;
			cc--;
		    }
		}
		last = '\r';
		kblamp(LAMP_LOCAL,0);
	    }
	}

#ifndef GL1
	if (zflag[4]) {
	    if (*p == ESC) {
		kblamp(LAMP_LOCAL,1);
		p++;
		cc--;
		if (cc <= 0) {
		    *p = '\0';
		    if (read(ttyfd, p, 1) > 0)
			cc++;
		}
		if (cc > 0) {
		    if (escape4010(*p))
			cc = 0;
		    else {
			p++;
			cc--;
		    }
		}
		kblamp(LAMP_LOCAL,0);
	    }
	}
#endif not GL1

	if (cc > 0) {
	    hwrite(p, (unsigned)cc);
	    last = p[cc - 1];
	}
    }
}

#ifndef GL1
/*
**	escape4010 - Process the 4010 escapes
**
*/
escape4010(c)
char c;
{
    switch (c) {
	case 'P':
	    sendpipecmd(toreader, HARDCOPY4010, 1);
	    break;
	case 'R':
	    sendpipecmd(toreader, CLEAR4010, 1);
	    break;
	default:
	    hwrite("\033", 1); /* sends an escape */
	    hwrite(&c, 1);
    }
}
#endif not GL1

/*
** 	escape - Process escaped characters.  Return 1 if reads were done, and
** 		 caller must throw away any in its buffer.
**
*/
int
escape(c)
char    c;
{
    int     rv = 0;

 	/* ~~: ~ itself */
    if (c == escchar) {
	hwrite(&escchar, 1);
	return rv;
    }
    switch (c) {
	case '.': 	/* ~.: quit	  */
	    printf("\n\r");
	    cleanup(0);
	    break;

	case '!':	/* ~!: shell esc */
	    if (!kbdlocked) {
		restoremode(ttyfd);
		sendpipecmd(toreader, INSUBSHELL, 1);
		shell();
		sendpipecmd(toreader, INSUBSHELL, 0);
		rawmode(ttyfd);
	    }
	    else
		bellring();
	    rv = 1;
	    break;
	
	case '\177': 	/* ~DEL: reboot */
	    fprintf(stderr, "reboot local IRIS (y/n)? ");
	    fflush(stderr);
	    if (read(ttyfd, &c, 1) > 0 && (c == 'y' || c == 'Y')) {
		fprintf(stderr, "\n\r");
		system(REBOOT);
	    }
	    break;

	case '\0': 	/* ~BREAK: send BREAK */
	    sendbreakchar();
	    break;

	case '%': 
	    percentesc();
	    rv = 1;
	    break;

	default: 
	/* default - send it through */
	    hwrite(&escchar, 1);
	    hwrite(&c, 1);
	    break;

    }
    return rv;
}

/*
** 	precentesc - Process the ~% escapes
**
*/
percentesc()
{
    char    c;
    int     temp = 0;
    int	    tempcode = 0;
    static char    buf[100];
    char    *type;

    if (read(ttyfd, &c, 1) > 0) {
	switch (c) {

	    case 'D': 		/* ~%D<n> */
		c = '\0';
		read(ttyfd, &c, 1);
		switch (c) {
		    case '1':
			c -= '0';
			if (dflag[c] = !dflag[c])
			    c = -c;
			sendpipecmd(toreader, TOGGLEDFLAG, c);
			fprintf(stderr, "-d %d option %s ",
					abs(c), (c < 0) ? "on" : "off");
			break;
		    case '3':
			fflush(stderr);
			while (read(ttyfd, &buf[temp], 1) > 0 
						    && buf[temp] != '\r') {
			    if (++temp >= sizeof buf - 1)
				break;
			}
			buf[temp] = '\0';
			if (temp == 0)
			    fprintf(stderr, "logfile name is %s ", logfile);
			else {
			    strcpy(logfile, buf);
			    sendpipecmd(toreader, LOGFILENAME, (int)logfile);
			    fprintf(stderr, "logfile name set to %s ",
			    					logfile);
			}
			break;
		    default:
			bellring();
			break;
		}
		return;

	    case 'M': 		/* ~%M<n> */
		c = '\0';
		read(ttyfd, &c, 1);
		switch (c) {
		    case '0': 
			type = "30 Hz";
			goto sendcmd;
		    case '1': 
			type = "60 Hz";
			goto sendcmd;
		    case '2': 
			type = "NTSC";
			goto sendcmd;
		    case '3': 
			type = "50 Hz";
			goto sendcmd;
		    case '9': 
			type = "PAL";
		sendcmd:
#ifdef GL1
			bellring();
#else
			sendpipecmd(toreader, SETMONITOR, c - '0');
			fprintf(stderr, "monitor set to %s ", type);
#endif
			break;
		    default: 
			bellring();
			break;
		}
		return;

	    case 'P': 		/* ~%P */
		pflag = !pflag;
		sendpipecmd(toreader, TOGGLEPFLAG, pflag);
		fprintf(stderr, "-p option %s ",(pflag) ? "on" : "off");
		return;

	    case 'R': 		/* ~%R */
		sendpipecmd(toreader, RESETDISPLAY, 0);
		return;

	    case 'S': 		/* ~%S<speed> */
		while (read(ttyfd, &buf[temp], 1) > 0 && buf[temp] != '\r') {
		    if (++temp >= sizeof buf - 1)
			break;
		}
		buf[temp] = '\0';
		sscanf(buf, "%d", &temp);
		if (hosttype == SERIAL_TYPE) {
		    tempcode = convspeed(temp);
		    if (tempcode == 0)
			fprintf(stderr, "invalid speed ");
		    else {
			speedcode = tempcode;
			condline();
			fprintf(stderr, "speed set to %s ", buf);
		    }
		}
		else
		    bellring();
		return;

	    case 'T':		/* ~%T */
		sendpipecmd(toreader, TOGGLETP, 0);
		return;

	    case 'U':		/* ~%U */
		kbdlocked = 0;
		kblamp(LAMP_KBDLOCKED, 0);
		return;

	    case 'X': 		/* ~%X */
		xflag = !xflag;
		rawmode(ttyfd);
		if (hosttype == SERIAL_TYPE)
		    condline();
		sendpipecmd(toreader, TOGGLEXFLAG, xflag);
		fprintf(stderr, "-x option %s ",(xflag) ? "on" : "off");
		return;

	    case 'Z': 		/* ~%Z<n> */
		c = '\0';
		read(ttyfd, &c, 1);
		switch (c) {
		    case '1':
			c -= '0';
			if (zflag[c] = !zflag[c])
			    c = -c;
			sendpipecmd(toreader, TOGGLEZFLAG, c);
			fprintf(stderr, "-z %d option %s ",
					abs(c), (c < 0) ? "on" : "off");
			break;
		    default:
			bellring();
			break;
		}
		return;

	    default: 
		bellring();
		break;
	}
    }
}

/*
**	hwrite - write to host and to readhost()'s input pipe if half-
**		 duplex
**
*/
static
hwrite(buf, count)
char   *buf;
unsigned count;
{
    if (!kbdlocked) {
	if (write(host, buf, count) != count)
	    errorm('F',"netwrite failed");
	if (hflag)
	    write(outfile, buf, count);
    }
}

/*
**	shell - invoke a subshell
**
*/
shell()
{
    register char **cp;
    register int    i;
    char   *shell;

    for (cp = environ; *cp; cp++)
	if (strncmp(*cp, "CMDNAME=", 8) == 0)
	    *cp = cmdbuf;

    savesig(SIGINT, SIG_IGN);
    savesig(SIGQUIT, SIG_IGN);

    if ((subshellpid = fork()) == -1)
	errorm('W',"can't fork");
    else if (subshellpid == 0) {
	if ((shell = getenv("SHELL")) == 0)
	    shell = DEFAULT_SHELL;

	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	for (i = 3; i < NOFILE; i++)
	    close(i);
	printf("!\n\r");
	fflush(stdout);
	setuid(getuid());    /* so subshells on 2300 and 3010 aren't root */
	execlp(shell, shell, 0);
	errorm('F',"couldn't exec %s", shell);
    }
    else {
	while (wait((int *)0) != subshellpid)
	    continue;
    }
    restoresig(SIGINT);
    restoresig(SIGQUIT);
    printf("!\n\r");
    fflush(stdout);
}

