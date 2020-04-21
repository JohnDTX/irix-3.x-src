/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)syslog.c	5.9 (Berkeley) 5/7/86";
#endif LIBC_SCCS and not lint

/*
 * SYSLOG -- print message on log file
 *
 * This routine looks a lot like printf, except that it
 * outputs to the log file instead of the standard output.
 * Also:
 *	adds a timestamp,
 *	prints the module name in front of the message,
 *	has some other formatting types (or will sometime),
 *	adds a newline on the end of the message.
 *
 * The output of this routine is intended to be read by /etc/syslogd.
 *
 * Author: Eric Allman
 * Modified to use UNIX domain IPC by Ralph Campbell
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <signal.h>
#include <syslog.h>
#include <netdb.h>
#include <strings.h>

#ifdef	sgi
#ifdef	mips
#include <sys/param.h>
#else
#include <fcntl.h>
#define MAXHOSTNAMELEN	64
#endif
#include <netinet/in.h>

#define	DEBUG
#ifdef	DEBUG
#include <stdio.h>
#endif
#endif

#define	MAXLINE	1024			/* max message size */
#ifndef	DEBUG
#define NULL	0			/* manifest */
#endif

#define PRIMASK(p)	(1 << ((p) & LOG_PRIMASK))
#define PRIFAC(p)	(((p) & LOG_FACMASK) >> 3)
#define IMPORTANT 	LOG_ERR

static char	logname[] = "/dev/log";
static char	ctty[] = "/dev/console";

static int	LogFile = -1;		/* fd for log */
static int	LogStat	= 0;		/* status bits, set by openlog() */
static char	*LogTag = "syslog";	/* string to tag the entry with */
static int	LogMask = 0xff;		/* mask of priorities to be logged */
static int	LogFacility = LOG_USER;	/* default facility code */

#ifndef sgi
static struct sockaddr SyslogAddr;	/* AF_UNIX address of local logger */
#endif

extern	int errno, sys_nerr;
extern	char *sys_errlist[];

#ifdef sgi
syslog(pri, fmt, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9)
#else
syslog(pri, fmt, p0, p1, p2, p3, p4)
#endif
	int pri;
	char *fmt;
{
	char buf[MAXLINE + 1], outline[MAXLINE + 1];
	register char *b, *f, *o;
	register int c;
	long now;
	int pid, olderrno = errno;

	/* see if we should just throw out this message */
	if (pri <= 0 || PRIFAC(pri) >= LOG_NFACILITIES || (PRIMASK(pri) & LogMask) == 0)
		return;
	if (LogFile < 0)
		openlog(LogTag, LogStat | LOG_NDELAY, 0);

	/* set default facility if none specified */
	if ((pri & LOG_FACMASK) == 0)
		pri |= LogFacility;

	/* build the message */
	o = outline;
	sprintf(o, "<%d>", pri);
	o += strlen(o);
	time(&now);
	sprintf(o, "%.15s ", ctime(&now) + 4);
	o += strlen(o);
	if (LogTag) {
		strcpy(o, LogTag);
		o += strlen(o);
	}
	if (LogStat & LOG_PID) {
		sprintf(o, "[%d]", getpid());
		o += strlen(o);
	}
	if (LogTag) {
		strcpy(o, ": ");
		o += 2;
	}

	b = buf;
	f = fmt;
	while ((c = *f++) != '\0' && c != '\n' && b < &buf[MAXLINE]) {
		if (c != '%') {
			*b++ = c;
			continue;
		}
		if ((c = *f++) != 'm') {
			*b++ = '%';
			*b++ = c;
			continue;
		}
		if ((unsigned)olderrno > sys_nerr)
			sprintf(b, "error %d", olderrno);
		else
			strcpy(b, sys_errlist[olderrno]);
		b += strlen(b);
	}
	*b++ = '\n';
	*b = '\0';
#ifdef sgi
	sprintf(o, buf, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
#else
	sprintf(o, buf, p0, p1, p2, p3, p4);
#endif
	c = strlen(outline);
	if (c > MAXLINE)
		c = MAXLINE;

	/* output the message to the local logger */
#ifndef	sgi
	if (sendto(LogFile, outline, c, 0, &SyslogAddr, sizeof SyslogAddr) >= 0)
		return;
#else
	if (write(LogFile, outline, c) >= 0)
		return;
#endif
	if (!(LogStat & LOG_CONS) && (pri & LOG_PRIMASK) <= LOG_ERR)
		return;

	/* output the message to the console */
	pid = fork();
	if (pid == -1)
		return;
	if (pid == 0) {
		signal(SIGALRM, SIG_DFL);
#ifndef	sgi
		sigsetmask(sigblock(0) & ~sigmask(SIGALRM));
#endif
		alarm(5);
		LogFile = open(ctty, O_WRONLY);
		alarm(0);
		strcat(o, "\r");
		o = index(outline, '>') + 1;
		write(LogFile, o, c + 1 - (o - outline));
		close(LogFile);
		exit(0);
	}
	if (!(LogStat & LOG_NOWAIT))
		while ((c = wait((int *)0)) > 0 && c != pid)
			;
}

/*
 * OPENLOG -- open system log
 */

openlog(ident, logstat, logfac)
	char *ident;
	int logstat, logfac;
{
	if (ident != NULL)
		LogTag = ident;
	LogStat = logstat;
	if (logfac != 0)
		LogFacility = logfac & LOG_FACMASK;
	if (LogFile >= 0)
		return;
#ifndef	sgi
	SyslogAddr.sa_family = AF_UNIX;
	strncpy(SyslogAddr.sa_data, logname, sizeof SyslogAddr.sa_data);
	if (LogStat & LOG_NDELAY) {
		LogFile = socket(AF_UNIX, SOCK_DGRAM, 0);
		fcntl(LogFile, F_SETFD, 1);
	}
#else	/* sgi */
	/*
	 * Use a named pipe on system V systems...No network syslog's
	 * for now...
	 */
	if (LogStat & LOG_NDELAY) {
		LogFile = open(logname, O_WRONLY | O_NDELAY);
		if (LogFile < 0) {
			/*
			 * No daemon running.  Should we start one?
			 */
			LogFile = open(ctty, O_WRONLY);
		} else {
			/*
			 * Daemon is running.  Turn off NDELAY.
			 */
			fcntl(LogFile, F_SETFL, 0);
		}
		fcntl(LogFile, F_SETFD, 1);
	}
#endif	/* sgi */
}

/*
 * CLOSELOG -- close the system log
 */
closelog()
{

	(void) close(LogFile);
	LogFile = -1;
}

/*
 * SETLOGMASK -- set the log mask level
 */
setlogmask(pmask)
	int pmask;
{
	int omask;

	omask = LogMask;
	if (pmask != 0)
		LogMask = pmask;
	return (omask);
}
