/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)syslogd.c	5.6 (Berkeley) 9/18/85";
#endif not lint

#define COMPAT		/* include 4.3 Alpha compatibility */

/*
 *  syslogd -- log system messages
 *
 * This program implements a system log. It takes a series of lines.
 * Each line may have a priority, signified as "<n>" as
 * the first characters of the line.  If this is
 * not present, a default priority is used.
 *
 * To kill syslogd, send a signal 15 (terminate).  A signal 1 (hup) will
 * cause it to reread its configuration file.
 *
 * Defined Constants:
 *
 * MAXLINE -- the maximimum line length that can be handled.
 * NLOGS   -- the maximum number of simultaneous log files.
 * DEFUPRI -- the default priority for user messages
 * DEFSPRI -- the default priority for kernel messages
 *
 * Author: Eric Allman
 * extensive changes by Ralph Campbell
 */

#define	NLOGS		20		/* max number of log files */
#define	MAXLINE		1024		/* maximum line length */
#define DEFUPRI		(LOG_USER|LOG_NOTICE)
#define DEFSPRI		(LOG_KERN|LOG_CRIT)

#include <syslog.h>
#include <errno.h>
#include <stdio.h>
#ifndef	sgi
#include <utmp.h>
#endif
#include <ctype.h>
#include <signal.h>
#include <sysexits.h>
#ifdef sgi
#include <string.h>
#else
#include <strings.h>
#endif

#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/msgbuf.h>
#include <sys/uio.h>
#ifndef sgi
#include <sys/un.h>
#endif
#include <sys/time.h>
#include <sys/resource.h>

#include <netinet/in.h>
#include <netdb.h>

#ifdef	sgi
#define	strcpyn(a, b, n)	strncpy(a, b, n)

/* include this after types.h */
#include <utmp.h>

#define	LOGPIPE		"/dev/log"
int	fpipe;
int	catchpipe();
#endif

char	*LogName = "/dev/log";
char	*ConfFile = "/etc/syslog.conf";
char	*PidFile = "/etc/syslog.pid";
char	ctty[] = "/dev/console";

#define FDMASK(fd)	(1 << (fd))

#define	dprintf		if (Debug) printf

#define UNAMESZ		8	/* length of a login name */
#define MAXUNAMES	20	/* maximum number of user names */
#define MAXFNAME	200	/* max file pathname length */

#define NOPRI		0x10	/* the "no priority" priority */

/*
 * Flags to logmsg().
 */

#define IGN_CONS	0x001	/* don't print on console */
#define SYNC_FILE	0x002	/* do fsync on file after printing */
#define NOCOPY		0x004	/* don't suppress duplicate messages */
#define ADDDATE		0x008	/* add a date to the message */

/*
 * This structure represents the files that will have log
 * copies printed.
 */

struct filed {
	short	f_type;			/* entry type, see below */
	short	f_file;			/* file descriptor */
	u_char	f_pmask[LOG_NFACILITIES];	/* priority mask */
	union {
		char	f_uname[MAXUNAMES][UNAMESZ+1];
		struct
		{
			char	f_hname[MAXHOSTNAMELEN+1];
			struct sockaddr_in	f_addr;
		}	f_forw;		/* forwarding address */
		char	f_fname[MAXFNAME];
	}	f_un;
};

/* values for f_type */
#define F_UNUSED	0		/* unused entry */
#define F_FILE		1		/* regular file */
#define F_TTY		2		/* terminal */
#define F_CONSOLE	3		/* console terminal */
#define F_FORW		4		/* remote machine */
#define F_USERS		5		/* list of users */
#define F_WALL		6		/* everyone logged on */

char	*TypeNames[7] = {
	"UNUSED",	"FILE",		"TTY",		"CONSOLE",
	"FORW",		"USERS",	"WALL"
};

struct filed	Files[NLOGS];

int	Debug;			/* debug flag */
char	LocalHostName[MAXHOSTNAMELEN+1];	/* our hostname */
int	InetInuse = 0;		/* non-zero if INET sockets are being used */
int	LogPort;		/* port number for INET connections */
char	PrevLine[MAXLINE + 1];	/* copy of last line to supress repeats */
char	PrevHost[MAXHOSTNAMELEN+1];		/* previous host */
int	PrevFlags;
int	PrevPri;
int	PrevCount = 0;		/* number of times seen */
int	Initialized = 0;	/* set when we have initialized ourselves */

extern	int errno, sys_nerr;
extern	char *sys_errlist[];
extern	char *ctime(), *index();

#ifdef COMPAT
int	CompatMode = 0;		/* run in compatibility mode */
int	CompatCodes[32] = {
		LOG_USER|LOG_ALERT,	/* 0 -- undefined */
			/* kernel priorities */
		LOG_KERN|LOG_EMERG,	/* KERN_EMERG */
		LOG_KERN|LOG_ALERT,	/* KERN_ALERT */
		LOG_KERN|LOG_CRIT,	/* KERN_ERR */
		LOG_KERN|LOG_ERR,	/* KERN_FAIL */
		LOG_KERN|LOG_WARNING,	/* KERN_RECOV */
		LOG_KERN|LOG_INFO,	/* KERN_INFO */
			/* user abnormal conditions priorities */
		LOG_USER|LOG_EMERG,	/* LOG_EMERG */
		LOG_USER|LOG_ALERT,	/* LOG_ALERT */
		LOG_USER|LOG_CRIT,	/* LOG_CRIT */
		LOG_USER|LOG_ERR,	/* LOG_ERR */
		LOG_USER|LOG_ERR,	/* LOG_ERR */
		LOG_USER|LOG_WARNING,	/* LOG_WARNING */
			/* user priorities */
		LOG_USER|LOG_ALERT,	/* LOG_SALERT */
		LOG_AUTH|LOG_NOTICE,	/* LOG_SECURITY */
		LOG_USER|LOG_INFO,	/* LOG_FIXED */
		LOG_MAIL|LOG_ERR,	/* LOG_MAIL */
		LOG_DAEMON|LOG_ERR,	/* LOG_REJECT */
		LOG_USER|LOG_NOTICE,	/* LOG_NOTICE */
			/* user information priorities */
		LOG_USER|LOG_INFO,	/* LOG_INFO */
		LOG_LOCAL1|LOG_INFO,	/* LOG_INFO1 */
		LOG_LOCAL2|LOG_INFO,	/* LOG_INFO2 */
		LOG_LOCAL3|LOG_INFO,	/* LOG_INFO3 */
		LOG_LOCAL4|LOG_INFO,	/* LOG_INFO4 */
		LOG_LOCAL5|LOG_INFO,	/* LOG_INFO5 */
			/* user debug/local priorities */
		LOG_USER|LOG_DEBUG,	/* LOG_DEBUG */
		LOG_LOCAL1|LOG_DEBUG,	/* LOG_LOCAL1 */
		LOG_LOCAL2|LOG_DEBUG,	/* LOG_LOCAL2 */
		LOG_LOCAL3|LOG_DEBUG,	/* LOG_LOCAL3 */
		LOG_LOCAL4|LOG_DEBUG,	/* LOG_LOCAL4 */
		LOG_LOCAL5|LOG_DEBUG,	/* LOG_LOCAL5 */
		LOG_LOCAL6|LOG_DEBUG,	/* LOG_LOCAL6 */
};
#endif COMPAT

int	console_fd;

main(argc, argv)
	int argc;
	char **argv;
{
	register int i;
	register char *p;
	int funix, finet, inetm, fklog, klogm, len;
#ifndef sgi
	struct sockaddr_un sun, fromunix;
#endif
	struct sockaddr_in sin, frominet;
	FILE *fp;
	char line[MSG_BSIZE + 1];
	extern int die(), domark(), reapchild();

	while (--argc > 0) {
		p = *++argv;
		if (p[0] != '-')
			usage();
		switch (p[1]) {
		case 'f':		/* configuration file */
			if (p[2] != '\0')
				ConfFile = &p[2];
			break;

		case 'd':		/* debug */
			Debug++;
			break;

#ifdef COMPAT
		case 'C':		/* run in compat mode */
			CompatMode++;
			break;
#endif COMPAT

		case 'p':		/* path */
			if (p[2] != '\0')
				LogName = &p[2];
			break;

		default:
			usage();
		}
	}

	if (!Debug) {
		if (fork())
			exit(0);
		for (i = 0; i < 10; i++)
			(void) close(i);
		(void) open("/", 0);
		(void) dup2(0, 1);
		(void) dup2(0, 2);
		untty();
	}
#ifndef	sgi
	else
		setlinebuf(stdout);
#endif

	(void) signal(SIGTERM, die);
#if defined(sgi) && defined(SVR3)
	if (Debug) {			/* work around mips compiler */
		(void) signal(SIGINT, die);
		(void) signal(SIGQUIT, die);
	} else {
		(void) signal(SIGINT, SIG_IGN);
		(void) signal(SIGQUIT, SIG_IGN);
	}
#else
	(void) signal(SIGINT, Debug ? die : SIG_IGN);
	(void) signal(SIGQUIT, Debug ? die : SIG_IGN);
#endif
	(void) signal(SIGCHLD, reapchild);
	(void) unlink(LogName);

#ifndef	sgi
	sun.sun_family = AF_UNIX;
	(void) strncpy(sun.sun_path, LogName, sizeof sun.sun_path);
	(void) gethostname(LocalHostName, sizeof LocalHostName);
	funix = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (funix < 0 || bind(funix, (struct sockaddr *) &sun,
	    sizeof(sun.sun_family)+strlen(sun.sun_path)) < 0 ||
	    chmod(LogName, 0666) < 0) {
		(void) sprintf(line, "cannot create %s", LogName);
		logerror(line);
		dprintf("cannot create %s (%d)\n", LogName, errno);
		die(0);
	}
#endif
#ifdef	notdef
	finet = socket(AF_INET, SOCK_DGRAM, 0);
	if (finet >= 0) {
		struct servent *sp;

		sp = getservbyname("syslog", "udp");
		if (sp == NULL) {
			errno = 0;
			logerror("syslog/udp: unknown service");
			die(0);
		}
		sin.sin_family = AF_INET;
		sin.sin_port = LogPort = sp->s_port;
		if (bind(finet, &sin, sizeof(sin)) < 0) {
			logerror("bind");
			if (!Debug)
				die(0);
		} else {
			inetm = FDMASK(finet);
			InetInuse = 1;
		}
	}
#endif
#ifdef	sgi
	(void) gethostname(LocalHostName, sizeof LocalHostName);
	(void) unlink(LOGPIPE);
	if (mknod(LOGPIPE, S_IFIFO|0666, 0) < 0) {
		logerror("mknod of /dev/log");
		die(0);
	}
	signal(SIGPIPE, catchpipe);
	openpipe();
#endif
#ifndef	sgi
	if ((fklog = open("/dev/klog", O_RDONLY)) >= 0)
		klogm = FDMASK(fklog);
	else {
		dprintf("can't open /dev/klog (%d)\n", errno);
		klogm = 0;
	}
#endif

	/* tuck my process id away */
	fp = fopen(PidFile, "w");
	if (fp != NULL) {
		fprintf(fp, "%d\n", getpid());
		(void) fclose(fp);
	}

	dprintf("off & running....\n");

	init();
	(void) signal(SIGHUP, init);

	for (;;) {
		int nfds, readfds = FDMASK(funix) | inetm | klogm;

		errno = 0;
		dprintf("readfds = %#x\n", readfds, funix, finet, fklog);
#ifndef	sgi
		nfds = select(20, (fd_set *) &readfds, (fd_set *) NULL,
				  (fd_set *) NULL, (struct timeval *) NULL);
		dprintf("got a message (%d, %#x)\n", nfds, readfds);
		if (nfds == 0)
			continue;
		if (nfds < 0) {
			if (errno != EINTR)
				logerror("select");
			continue;
		}
		if (readfds & klogm) {
			i = read(fklog, line, sizeof(line) - 1);
			if (i > 0) {
				line[i] = '\0';
				printsys(line);
			} else if (i < 0 && errno != EINTR) {
				logerror("klog");
				fklog = -1;
				klogm = 0;
			}
		}
		if (readfds & FDMASK(funix)) {
			len = sizeof fromunix;
			i = recvfrom(funix, line, MAXLINE, 0,
				     (struct sockaddr *) &fromunix, &len);
			if (i > 0) {
				line[i] = '\0';
				printline(LocalHostName, line);
			} else if (i < 0 && errno != EINTR)
				logerror("recvfrom unix");
		}
		if (readfds & inetm) {
			len = sizeof frominet;
			i = recvfrom(finet, line, MAXLINE, 0, &frominet, &len);
			if (i > 0) {
				extern char *cvthname();

				line[i] = '\0';
				printline(cvthname(&frominet), line);
			} else if (i < 0 && errno != EINTR)
				logerror("recvfrom inet");
		} 
#else
		i = read(fpipe, line, sizeof(line) - 1);
		if (i < 0 && errno != EINTR) {
			close(fpipe);
			openpipe();
		} else
		if (i > 0) {
			line[i] = '\0';
			printline(LocalHostName, line);
		}
		dprintf("got a message\n");
#endif
	}
}

usage()
{
	fprintf(stderr, "usage: syslogd [-d] [-ppath] [-fconffile]\n");
	exit(1);
}

untty()
{
	int i;

	if (!Debug) {
		i = open("/dev/tty", O_RDWR);
		if (i >= 0) {
			(void) ioctl(i, (int) TIOCNOTTY, (char *)0);
			(void) close(i);
		}
	}
}

/*
 * Take a raw input line, decode the message, and print the message
 * on the appropriate log files.
 */

printline(hname, msg)
	char *hname;
	char *msg;
{
	register char *p, *q;
	register int c;
	char line[MAXLINE + 1];
	int pri;

	p = msg;
#ifdef sgi
repeat:
#endif
	/* test for special codes */
	pri = DEFUPRI;
	if (*p == '<') {
		pri = 0;
		while (isdigit(*++p))
			pri = 10 * pri + (*p - '0');
		if (*p == '>')
			++p;
		if (pri <= 0 || pri >= (LOG_NFACILITIES << 3))
			pri = DEFUPRI;
	}

	/* don't allow users to log kernel messages */
	if ((pri & LOG_PRIMASK) == LOG_KERN)
		pri |= LOG_USER;

	q = line;
#ifdef COMPAT
	if (CompatMode) {
		register char *lp = index(p, ':');

		if (lp && lp[1] == ' ' && lp[17] == '-' && lp[18] == '-') {
			/*
			 * Old format message
			 */
			dprintf("mapping <%d> to <%d>\n", pri, CompatCodes[pri]);
			pri = CompatCodes[pri];
			(void) strncpy(q, lp + 2, 15);
			q += 15;
			*q++ = ' ';
			(void) strncpy(q, p, lp - p + 1);
			q += lp - p + 1;
			p = lp + 19;
		}
	}
#endif COMPAT

	while ((c = *p++ & 0177) != '\0' && c != '\n' &&
	    q < &line[sizeof(line) - 1]) {
		if (iscntrl(c)) {
			*q++ = '^';
			*q++ = c ^ 0100;
		} else
			*q++ = c;
	}
	*q = '\0';

	logmsg(pri, line, hname, 0);

#ifdef sgi
	/*
	 * Named pipes don't preserve record boundaries, so more than
	 * one message can be retrieved in a single read.
	 * Skip duplicate line feeds and go print the next message
	 */
	if (c == '\n') {
		while (*p == '\n') ++p;
		if (*p != '\0')
			goto repeat;
	}
#endif
}

#ifndef sgi
/*
 * Take a raw input line from /dev/klog, split and format similar to syslog().
 */

printsys(msg)
	char *msg;
{
	register char *p, *q;
	register int c;
	char line[MAXLINE + 1];
	int pri, flags;
	char *lp;
	time_t now;

	(void) time(&now);
#if defined(sgi) && defined(SVR3)
	(void) sprintf(line, "%.15s unix: ", ctime(&now) + 4);
#else
	(void) sprintf(line, "%.15s vmunix: ", ctime(&now) + 4);
#endif
	lp = line + strlen(line);
	for (p = msg; *p != '\0'; ) {
		flags = SYNC_FILE;	/* fsync file after write */
		pri = DEFSPRI;
		if (*p == '<') {
			pri = 0;
			while (isdigit(*++p))
				pri = 10 * pri + (*p - '0');
			if (*p == '>')
				++p;
			if (pri <= 0 || pri >= (LOG_NFACILITIES << 3))
				pri = DEFSPRI;
#ifdef COMPAT
			else if (CompatMode) {
				dprintf("mapping <%d> to <%d>\n", pri, CompatCodes[pri]);
				pri = CompatCodes[pri];
			}
#endif COMPAT
		} else {
			/* kernel printf's come out on console */
			flags |= IGN_CONS;
		}
		q = lp;
		while (*p != '\0' && (c = *p++) != '\n' &&
		    q < &line[MAXLINE])
			*q++ = c;
		*q = '\0';
		logmsg(pri, line, LocalHostName, flags);
	}
}
#endif !sgi

/*
 * Log a message to the appropriate log files, users, etc. based on
 * the priority.
 */

logmsg(pri, msg, from, flags)
	int pri;
	char *msg, *from;
	int flags;
{
	char line[MAXLINE + 1];
	register struct filed *f;
	register int l;
	int fac, prilev;
	struct iovec iov[6];
	register struct iovec *v = iov;
	int omask;
#if defined(sgi) && defined(SVR3)
	void (*oa)(), (*oh)();
#else
	int (*oa)(), (*oh)();
#endif

	dprintf("logmsg: pri %o, flags %x, from %s, msg %s\n", pri, flags, from, msg);

#ifndef	sgi
	omask = sigblock(sigmask(SIGALRM)|sigmask(SIGHUP));
#else
	/*
	 * Not quite right...
	 */
	oa = signal(SIGALRM, SIG_IGN);
	oh = signal(SIGHUP, SIG_IGN);
#endif

	/*
	 * Check to see if msg looks non-standard.
	 */
	if (!(flags & ADDDATE) && (strlen(msg) < 16 ||
	    msg[3] != ' ' || msg[6] != ' ' ||
	    msg[9] != ':' || msg[12] != ':' || msg[15] != ' '))
		flags |= ADDDATE;

	if ((flags & NOCOPY) == 0) {
		if (flags & ADDDATE)
			flushmsg();
		else if (!strcmp(msg + 16, PrevLine + 16)) {
			/* we found a match, update the time */
			(void) strncpy(PrevLine, msg, 15);
			PrevCount++;
#ifndef	sgi
			(void) sigsetmask(omask);
#else
			(void) signal(SIGALRM, oa);
			(void) signal(SIGHUP, oh);
#endif
			return;
		} else {
			/* new line, save it */
			flushmsg();
			(void) strcpy(PrevLine, msg);
			(void) strcpy(PrevHost, from);
			PrevFlags = flags;
			PrevPri = pri;
		}
	}

	if (flags & ADDDATE) {
		time_t now;

		(void) time(&now);
		v->iov_base = ctime(&now) + 4;
	} else
		v->iov_base = msg;
	v->iov_len = 15;
	v++;
	v->iov_base = " ";
	v->iov_len = 1;
	v++;
	v->iov_base = from;
	v->iov_len = strlen(v->iov_base);
	v++;
	v->iov_base = " ";
	v->iov_len = 1;
	v++;
	if (flags & ADDDATE)
		v->iov_base = msg;
	else
		v->iov_base = msg + 16;
	v->iov_len = strlen(v->iov_base);
	v++;

	/* extract facility and priority level */
	fac = (pri & LOG_FACMASK) >> 3;
	prilev = pri & LOG_PRIMASK;

	/* log the message to the particular outputs */
	if (!Initialized) {
		int cfd = open(ctty, O_WRONLY);

		if (cfd >= 0) {
			v->iov_base = "\r\n";
			v->iov_len = 2;
			(void) writev(cfd, iov, 6);
			(void) close(cfd);
		}
		untty();
		return;
	}
	for (f = Files; f < &Files[NLOGS]; f++) {
		/* skip messages that are incorrect priority */
		if (f->f_pmask[fac] < prilev || f->f_pmask[fac] == NOPRI)
			continue;

		dprintf("Logging to %s", TypeNames[f->f_type]);
		switch (f->f_type) {
		case F_UNUSED:
			dprintf("\n");
			break;

		case F_FORW:
			dprintf(" %s\n", f->f_un.f_forw.f_hname);
			(void) sprintf(line, "<%d>%.15s %s", pri, v[0].iov_base,
				v[4].iov_base);
			l = strlen(line);
			if (l > MAXLINE)
				l = MAXLINE;
			if (sendto(f->f_file, line, l, 0,
			    &f->f_un.f_forw.f_addr,
			    sizeof f->f_un.f_forw.f_addr) != l) {
				int e = errno;
				(void) close(f->f_file);
				f->f_type = F_UNUSED;
				errno = e;
				logerror("sendto");
			}
			break;

		case F_CONSOLE:
			if (flags & IGN_CONS) {
				dprintf(" (ignored)\n");
				break;
			}

		case F_TTY:
		case F_FILE:
			dprintf(" %s\n", f->f_un.f_fname);
			if (f->f_type != F_FILE) {
				v->iov_base = "\r\n";
				v->iov_len = 2;
			} else {
				v->iov_base = "\n";
				v->iov_len = 1;
			}
			if (writev(f->f_file, iov, 6) < 0) {
				int e = errno;
				(void) close(f->f_file);
				/*
				 * Check for EBADF on TTY's due to vhangup() XXX
				 */
				if (e == EBADF && f->f_type != F_FILE) {
#ifdef sgi
					f->f_file = open(f->f_un.f_fname, O_WRONLY|O_APPEND|O_CREAT);
#else
					f->f_file = open(f->f_un.f_fname, O_WRONLY|O_APPEND);
#endif
					if (f->f_file < 0) {
						f->f_type = F_UNUSED;
						logerror(f->f_un.f_fname);
					}
				} else {
					f->f_type = F_UNUSED;
					errno = e;
					logerror(f->f_un.f_fname);
				}
			}
#ifndef	sgi
			else if (flags & SYNC_FILE)
				(void) fsync(f->f_file);
#endif
			break;

		case F_USERS:
		case F_WALL:
			dprintf("\n");
			v->iov_base = "\r\n";
			v->iov_len = 2;
			wallmsg(f, iov);
			break;
		}
	}

#ifndef	sgi
	(void) sigsetmask(omask);
#else
	(void) signal(SIGALRM, oa);
	(void) signal(SIGHUP, oh);
#endif
}


/*
 *  WALLMSG -- Write a message to the world at large
 *
 *	Write the specified message to either the entire
 *	world, or a list of approved users.
 */

wallmsg(f, iov)
	register struct filed *f;
	struct iovec *iov;
{
	register char *p;
	register int i;
	int ttyf, len;
	FILE *uf;
	static int reenter = 0;
	struct utmp ut;
	time_t now;
	char greetings[200];

	if (reenter++)
		return;

	/* open the user login file */
	if ((uf = fopen("/etc/utmp", "r")) == NULL) {
		logerror("/etc/utmp");
		reenter = 0;
		return;
	}

	(void) time(&now);
	(void) sprintf(greetings,
	    "\r\n\7Message from syslogd@%s at %.24s ...\r\n",
		iov[2].iov_base, ctime(&now));
	len = strlen(greetings);

	/* scan the user login file */
	while (fread((char *) &ut, sizeof ut, 1, uf) == 1) {
		/* is this slot used? */
		if (ut.ut_name[0] == '\0')
			continue;

		/* should we send the message to this user? */
		if (f->f_type == F_USERS) {
			for (i = 0; i < MAXUNAMES; i++) {
				if (!f->f_un.f_uname[i][0]) {
					i = MAXUNAMES;
					break;
				}
				if (strncmp(f->f_un.f_uname[i], ut.ut_name,
				    UNAMESZ) == 0)
					break;
			}
			if (i >= MAXUNAMES)
				continue;
		}

		/* compute the device name */
		p = "/dev/12345678";
		strcpyn(&p[5], ut.ut_line, UNAMESZ);

		/*
		 * Might as well fork instead of using nonblocking I/O
		 * and doing notty().
		 */
		if (fork() == 0) {
			if (f->f_type == F_WALL) {
				iov[0].iov_base = greetings;
				iov[0].iov_len = len;
			}
			(void) signal(SIGALRM, SIG_DFL);
			(void) alarm(30);
			/* open the terminal */
			ttyf = open(p, O_WRONLY);
			if (ttyf >= 0)
				(void) writev(ttyf, iov, 6);
			exit(0);
		}
		/* avoid having them all pile up at once */
		sleep(1);
	}
	/* close the user login file */
	(void) fclose(uf);
	reenter = 0;
}

reapchild()
{
	union wait status;

#ifndef	sgi
	while (wait3(&status, WNOHANG, (struct rusage *) NULL) > 0)
		;
#else
	(void) wait(&status);
#endif
}

/*
 * Return a printable representation of a host address.
 */
char *
cvthname(f)
	struct sockaddr_in *f;
{
	struct hostent *hp;
	extern char *inet_ntoa();

	dprintf("cvthname(%s)\n", inet_ntoa(f->sin_addr));

	if (f->sin_family != AF_INET) {
		dprintf("Malformed from address\n");
		return ("???");
	}
	hp = gethostbyaddr(&f->sin_addr, sizeof(struct in_addr), f->sin_family);
	if (hp == 0) {
		dprintf("Host name for your address (%s) unknown\n",
			inet_ntoa(f->sin_addr));
		return (inet_ntoa(f->sin_addr));
	}
	return (hp->h_name);
}

flushmsg()
{
	if (PrevCount == 0)
		return;
	if (PrevCount > 1)
		(void) sprintf(PrevLine+16, "last message repeated %d times", PrevCount);
	PrevCount = 0;
	logmsg(PrevPri, PrevLine, PrevHost, PrevFlags|NOCOPY);
	PrevLine[0] = '\0';
}

/*
 * Print syslogd errors some place.
 */
logerror(type)
	char *type;
{
	char buf[100];

	if (errno == 0)
		(void) sprintf(buf, "syslogd: %s", type);
	else if ((unsigned) errno > sys_nerr)
		(void) sprintf(buf, "syslogd: %s: error %d", type, errno);
	else
		(void) sprintf(buf, "syslogd: %s: %s", type, sys_errlist[errno]);
	errno = 0;
	dprintf("%s\n", buf);
	logmsg(LOG_DAEMON|LOG_ERR, buf, LocalHostName, ADDDATE);
}

die(sig)
{
	char buf[100];

	dprintf("syslogd: going down on signal %d\n", sig);
	flushmsg();
	(void) sprintf(buf, "going down on signal %d", sig);
	logerror(buf);
	(void) unlink(LogName);
	exit(0);
}

/*
 *  INIT -- Initialize syslogd from configuration table
 */

init()
{
	register int i;
	register FILE *cf;
	register struct filed *f;
	register char *p;
	char cline[BUFSIZ];

#ifdef sgi
	(void) signal(SIGHUP, SIG_IGN);
#endif
	dprintf("init\n");

	/* flush any pending output */
	flushmsg();

	/*
	 *  Close all open log files.
	 */
	for (f = Files; f < &Files[NLOGS]; f++) {
		if (f->f_type == F_FILE || f->f_type == F_TTY)
			(void) close(f->f_file);
		f->f_type = F_UNUSED;
	}

	/* open the configuration file */
	if ((cf = fopen(ConfFile, "r")) == NULL) {
		dprintf("cannot open %s\n", ConfFile);
		cfline("*.ERR\t/dev/console", &Files[0]);
		cfline("*.PANIC\t*", &Files[1]);
#ifdef sgi
		(void) signal(SIGHUP, init);
#endif
		return;
	}

	/*
	 *  Foreach line in the conf table, open that file.
	 */
	f = Files;
	while (fgets(cline, sizeof cline, cf) != NULL && f < &Files[NLOGS]) {
		/* check for end-of-section */
		if (cline[0] == '\n' || cline[0] == '#')
			continue;

		/* strip off newline character */
		p = index(cline, '\n');
		if (p)
			*p = '\0';

		cfline(cline, f++);
	}

	/* close the configuration file */
	(void) fclose(cf);

	Initialized = 1;

	if (Debug) {
		for (f = Files; f < &Files[NLOGS]; f++) {
			for (i = 0; i < LOG_NFACILITIES; i++)
				if (f->f_pmask[i] == NOPRI)
					printf("X ");
				else
					printf("%d ", f->f_pmask[i]);
			printf("%s: ", TypeNames[f->f_type]);
			switch (f->f_type) {
			case F_FILE:
			case F_TTY:
			case F_CONSOLE:
				printf("%s", f->f_un.f_fname);
				break;

			case F_FORW:
				printf("%s", f->f_un.f_forw.f_hname);
				break;

			case F_USERS:
				for (i = 0; i < MAXUNAMES && *f->f_un.f_uname[i]; i++)
					printf("%s, ", f->f_un.f_uname[i]);
				break;
			}
			printf("\n");
		}
	}

	logmsg(LOG_DAEMON|LOG_INFO, "syslogd: restart", LocalHostName, ADDDATE);
	dprintf("syslogd: restarted\n");
#ifdef sgi
	(void) signal(SIGHUP, init);
#endif
}

/*
 * Crack a configuration file line
 */

struct code {
	char	*c_name;
	int	c_val;
};

struct code	PriNames[] = {
	"panic",	LOG_EMERG,
	"emerg",	LOG_EMERG,
	"alert",	LOG_ALERT,
	"crit",		LOG_CRIT,
	"err",		LOG_ERR,
	"error",	LOG_ERR,
	"warn",		LOG_WARNING,
	"warning",	LOG_WARNING,
	"notice",	LOG_NOTICE,
	"info",		LOG_INFO,
	"debug",	LOG_DEBUG,
	NULL,		-1
};

struct code	FacNames[] = {
	"kern",		LOG_KERN,
	"user",		LOG_USER,
	"mail",		LOG_MAIL,
	"auth",		LOG_AUTH,
	"security",	LOG_AUTH,
	"local0",	LOG_LOCAL0,
	"local1",	LOG_LOCAL1,
	"local2",	LOG_LOCAL2,
	"local3",	LOG_LOCAL3,
	"local4",	LOG_LOCAL4,
	"local5",	LOG_LOCAL5,
	"local6",	LOG_LOCAL6,
	"local7",	LOG_LOCAL7,
	NULL,		-1
};

cfline(line, f)
	char *line;
	register struct filed *f;
{
	register char *p;
	register char *q;
	register int i;
	char *bp;
	int pri;
	struct hostent *hp;
	char buf[MAXLINE];

	dprintf("cfline(%s)\n", line);

	/* clear out file entry */
	bzero((char *) f, sizeof *f);
	for (i = 0; i < LOG_NFACILITIES; i++)
		f->f_pmask[i] = NOPRI;

	/* scan through the list of selectors */
	for (p = line; *p && *p != '\t';) {

		/* find the end of this facility name list */
		for (q = p; *q && *q != '\t' && *q++ != '.'; )
			continue;

		/* collect priority name */
		for (bp = buf; *q && !index("\t,;", *q); )
			*bp++ = *q++;
		*bp = '\0';

		/* skip cruft */
		while (index(", ;", *q))
			q++;

		/* decode priority name */
		pri = decode(buf, PriNames);
		if (pri < 0) {
			char xbuf[200];

			(void) sprintf(xbuf, "unknown priority name \"%s\"", buf);
			logerror(xbuf);
			return;
		}

		/* scan facilities */
		while (*p && !index("\t.;", *p)) {
			int i;

			for (bp = buf; *p && !index("\t,;.", *p); )
				*bp++ = *p++;
			*bp = '\0';
			if (*buf == '*')
				for (i = 0; i < LOG_NFACILITIES; i++)
					f->f_pmask[i] = pri;
			else {
				i = decode(buf, FacNames);
				if (i < 0) {
					char xbuf[200];

					(void) sprintf(xbuf, "unknown facility name \"%s\"", buf);
					logerror(xbuf);
					return;
				}
				f->f_pmask[i >> 3] = pri;
			}
			while (*p == ',' || *p == ' ')
				p++;
		}

		p = q;
	}

	/* skip to action part */
	while (*p == '\t')
		p++;

	switch (*p)
	{
	case '@':
		if (!InetInuse)
			break;
		(void) strcpy(f->f_un.f_forw.f_hname, ++p);
		hp = gethostbyname(p);
		if (hp == NULL) {
			char buf[100];

			(void) sprintf(buf, "unknown host %s", p);
			errno = 0;
			logerror(buf);
			break;
		}
		bzero((char *) &f->f_un.f_forw.f_addr,
			 sizeof f->f_un.f_forw.f_addr);
		f->f_un.f_forw.f_addr.sin_family = AF_INET;
		f->f_un.f_forw.f_addr.sin_port = LogPort;
		bcopy(hp->h_addr, (char *) &f->f_un.f_forw.f_addr.sin_addr, hp->h_length);
		f->f_file = socket(AF_INET, SOCK_DGRAM, 0);
		if (f->f_file < 0) {
			logerror("socket");
			break;
		}
		f->f_type = F_FORW;
		break;

	case '/':
		(void) strcpy(f->f_un.f_fname, p);
#ifdef sgi
		if ((f->f_file = open(p, O_WRONLY|O_APPEND|O_CREAT)) < 0) {
#else
		if ((f->f_file = open(p, O_WRONLY|O_APPEND)) < 0) {
#endif
			logerror(p);
			break;
		}
		if (isatty(f->f_file)) {
			f->f_type = F_TTY;
			untty();
		}
		else
			f->f_type = F_FILE;
		if (strcmp(p, ctty) == 0)
			f->f_type = F_CONSOLE;
		break;

	case '*':
		f->f_type = F_WALL;
		break;

	default:
		for (i = 0; i < MAXUNAMES && *p; i++) {
			for (q = p; *q && *q != ','; )
				q++;
			(void) strncpy(f->f_un.f_uname[i], p, UNAMESZ);
			if ((q - p) > UNAMESZ)
				f->f_un.f_uname[i][UNAMESZ] = '\0';
			else
				f->f_un.f_uname[i][q - p] = '\0';
			while (*q == ',' || *q == ' ')
				q++;
			p = q;
		}
		f->f_type = F_USERS;
		break;
	}
}


/*
 *  Decode a symbolic name to a numeric value
 */

decode(name, codetab)
	char *name;
	struct code *codetab;
{
	register struct code *c;
	register char *p;
	char buf[40];

	if (isdigit(*name))
		return (atoi(name));

	(void) strcpy(buf, name);
	for (p = buf; *p; p++)
		if (isupper(*p))
			*p = tolower(*p);
	for (c = codetab; c->c_name; c++)
		if (!strcmp(buf, c->c_name))
			return (c->c_val);

	return (-1);
}

#ifdef	sgi
int
writev(fd, iov, niov)
	int fd;
	struct iovec *iov;
	int niov;
{
	char buf[16384];
	register struct iovec *iv;
	register int len;
	register int nb;
	register int r;
	register int i;
	register char *cp;

	/*
	 * Try to write data out atomically by copying it to a single
	 * buffer.  If we can't, then write the data out in as few
	 * pieces as possible.
	 */
	nb = 0;
	len = 0;
	iv = iov;
	cp = buf;
	for (i = niov; --i >= 0; iv++) {
top:
		if (len + iv->iov_len > sizeof(buf)) {
			if (len) {
				r = write(fd, buf, len);
				if (r < 0)
					return (r);
				nb += r;
				len = 0;
				goto top;
			} else {
				/*
				 * Can't fit this iov in the buffer.
				 * Just write it out...
				 */
				r = write(fd, iv->iov_base, iv->iov_len);
				if (r < 0)
					return (r);
				nb += r;
				continue;
			}
		}
		bcopy(iv->iov_base, cp, iv->iov_len);
		len += iv->iov_len;
		cp += iv->iov_len;
	}
	if (len) {
		r = write(fd, buf, len);
		if (r < 0)
			return (r);
		nb += r;
	}
	return (nb);
}
#endif

#ifdef	sgi
openpipe()
{
	fpipe = open("/dev/log", O_RDWR);
	if (fpipe < 0) {
		logerror("open of /dev/log");
		die(0);
	}
}

catchpipe()
{
	dprintf("SIGPIPE\n");
	close(fpipe);
	openpipe();
	signal(SIGPIPE, catchpipe);
}
#endif
