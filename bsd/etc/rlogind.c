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
static char sccsid[] = "@(#)rlogind.c	5.11 (Berkeley) 5/23/86";
#endif not lint

/*
 * remote login server:
 *	remuser\0
 *	locuser\0
 *	terminal info\0
 *	data
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/file.h>

#include <netinet/in.h>

#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <sgtty.h>
#ifdef	sgi
#include <sys/sysmacros.h>
#include <sys/stream.h>
#endif
#include <stdio.h>
#include <netdb.h>
#include <syslog.h>
#include <strings.h>

# ifndef TIOCPKT_WINDOW
# define TIOCPKT_WINDOW 0x80
# endif TIOCPKT_WINDOW

extern	errno;
int	reapchild();
struct	passwd *getpwnam();
char	*malloc();

main(argc, argv)
	int argc;
	char **argv;
{
	int on = 1, options = 0, fromlen;
	struct sockaddr_in from;

	openlog("rlogind", LOG_PID | LOG_AUTH, LOG_AUTH);
	fromlen = sizeof (from);
	if (getpeername(0, &from, &fromlen) < 0) {
		fprintf(stderr, "%s: ", argv[0]);
		perror("getpeername");
		_exit(1);
	}
	if (setsockopt(0, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof (on)) < 0) {
		syslog(LOG_WARNING, "setsockopt (SO_KEEPALIVE): %m");
	}
	doit(0, &from);
}

int	child;
int	cleanup();
int	netf;
extern	errno;
#ifdef sgi
char stty[] = "/dev/ttyq12345";
#else
char	*line;
#endif
extern	char	*inet_ntoa();

struct winsize win = { 0, 0, 0, 0 };


doit(f, fromp)
	int f;
	struct sockaddr_in *fromp;
{
	int i, p, t, pid, on = 1;
	register struct hostent *hp;
	struct hostent hostent;
	char c;

	alarm(60);
	read(f, &c, 1);
	if (c != 0)
		exit(1);
	alarm(0);
	fromp->sin_port = ntohs((u_short)fromp->sin_port);
	hp = gethostbyaddr(&fromp->sin_addr, sizeof (struct in_addr),
		fromp->sin_family);
	if (hp == 0) {
		/*
		 * Only the name is used below.
		 */
		hp = &hostent;
		hp->h_name = inet_ntoa(fromp->sin_addr);
	}
	if (fromp->sin_family != AF_INET ||
 	    fromp->sin_port >= IPPORT_RESERVED ||
 	    fromp->sin_port < IPPORT_RESERVED/2)
		fatal(f, "Permission denied");
	write(f, "", 1);
#ifdef sgi
	{
		struct stat stb;
		p = open("/dev/ptc", O_RDWR | O_NDELAY);
		if (p < 0 || fstat(p, &stb) < 0)
			fatalperror(f, "Out of ptys", errno);
		sprintf(&stty[0], "/dev/ttyq%d", minor(stb.st_rdev));
	}
#else
	for (c = 'p'; c <= 's'; c++) {
		struct stat stb;
		line = "/dev/ptyXX";
		line[strlen("/dev/pty")] = c;
		line[strlen("/dev/ptyp")] = '0';
		if (stat(line, &stb) < 0)
			break;
		for (i = 0; i < 16; i++) {
			line[strlen("/dev/ptyp")] = "0123456789abcdef"[i];
			p = open(line, 2);
			if (p > 0)
				goto gotpty;
		}
	}
	fatal(f, "Out of ptys");
	/*NOTREACHED*/
gotpty:
#endif
	(void) ioctl(p, TIOCSWINSZ, &win);
	netf = f;
#ifndef sgi
	line[strlen("/dev/")] = 't';
#endif
#ifdef DEBUG
	{ int tt = open("/dev/tty", 2);
	  if (tt > 0) {
		ioctl(tt, TIOCNOTTY, 0);
		close(tt);
	  }
	}
#endif
#ifdef sgi
	setpgrp();
	t = open(stty, O_RDWR);
	if (t < 0)
		fatalperror(f, stty, errno);
	{ struct termio b;
		ioctl(t, TCGETA, &b);
		b.c_iflag &= ~(ISTRIP|ICRNL|INLCR|IGNCR|IXON|IXOFF|BRKINT);
		b.c_lflag &= ~(ICANON|ECHO|ISIG);
		b.c_oflag &= ~OPOST;
		b.c_oflag |= TAB3;
		b.c_cc[VMIN] = 1;
		b.c_cc[VTIME] = 1;
		b.c_line = LDISC1;
		ioctl(t, TCSETA, &b);
	}
#else
	t = open(line, 2);
	if (t < 0)
		fatalperror(f, line, errno);
	{ struct sgttyb b;
	  gtty(t, &b); b.sg_flags = RAW|ANYP; stty(t, &b);
	}
#endif
	pid = fork();
	if (pid < 0)
		fatalperror(f, "", errno);
	if (pid == 0) {
		close(f), close(p);
		dup2(t, 0), dup2(t, 1), dup2(t, 2);
#ifdef sgi
		adut();
		closelog();		/* (so syslog() can work below) */
		for (t = getdtablesize(); --t > 2; )
			(void)close(t);
		execl("/bin/login", "login", "-r", hp->h_name, 0);
		syslog(LOG_ERR,"rlogind: failed to exec /bin/login: %d",errno);
#else
		close(t);
		execl("/bin/login", "login", "-r", hp->h_name, 0);
#endif
		fatalperror(2, "/bin/login", errno);
		/*NOTREACHED*/
	}
	close(t);
	ioctl(f, FIONBIO, &on);
	ioctl(p, FIONBIO, &on);
	ioctl(p, TIOCPKT, &on);
#ifdef sgi
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
#else
	signal(SIGTSTP, SIG_IGN);
#endif
	signal(SIGCHLD, cleanup);
#ifndef sgi
	setpgrp(0, 0);
#endif
	protocol(f, p);
	cleanup();
}

char	magic[2] = { 0377, 0377 };
char	oobdata[] = {TIOCPKT_WINDOW};

/*
 * Handle a "control" request (signaled by magic being present)
 * in the data stream.  For now, we are only willing to handle
 * window size changes.
 */
control(pty, cp, n)
	int pty;
	char *cp;
	int n;
{
	struct winsize w;

	if (n < 4+sizeof (w) || cp[2] != 's' || cp[3] != 's')
		return (0);
	oobdata[0] &= ~TIOCPKT_WINDOW;	/* we know he heard */
	bcopy(cp+4, (char *)&w, sizeof(w));
	w.ws_row = ntohs(w.ws_row);
	w.ws_col = ntohs(w.ws_col);
	w.ws_xpixel = ntohs(w.ws_xpixel);
	w.ws_ypixel = ntohs(w.ws_ypixel);
	(void)ioctl(pty, TIOCSWINSZ, &w);
	return (4+sizeof (w));
}

/*
 * rlogin "protocol" machine.
 */
protocol(f, p)
	int f, p;
{
#ifdef sgi
	char pibuf[MAXBSIZE], fibuf[1024], *pbp, *fbp;
#else
	char pibuf[1024], fibuf[1024], *pbp, *fbp;
#endif
	register pcc = 0, fcc = 0;
	int cc;
	char cntl;

#ifndef sgi
	/*
	 * Must ignore SIGTTOU, otherwise we'll stop
	 * when we try and set slave pty's window shape
	 * (our controlling tty is the master pty).
	 */
	(void) signal(SIGTTOU, SIG_IGN);
#endif
	send(f, oobdata, 1, MSG_OOB);	/* indicate new rlogin */
	for (;;) {
		int ibits, obits, ebits;

		ibits = 0;
		obits = 0;
		if (fcc)
			obits |= (1<<p);
		else
			ibits |= (1<<f);
		if (pcc >= 0)
			if (pcc)
				obits |= (1<<f);
			else
				ibits |= (1<<p);
		ebits = (1<<p);
		if (select(16, &ibits, &obits, &ebits, 0) < 0) {
			if (errno == EINTR)
				continue;
			fatalperror(f, "select", errno);
		}
		if (ibits == 0 && obits == 0 && ebits == 0) {
			/* shouldn't happen... */
			sleep(5);
			continue;
		}
#define	pkcontrol(c)	((c)&(TIOCPKT_FLUSHWRITE|TIOCPKT_NOSTOP|TIOCPKT_DOSTOP))
		if (ebits & (1<<p)) {
			cc = read(p, &cntl, 1);
			if (cc == 1 && pkcontrol(cntl)) {
				cntl |= oobdata[0];
				send(f, &cntl, 1, MSG_OOB);
				if (cntl & TIOCPKT_FLUSHWRITE) {
					pcc = 0;
					ibits &= ~(1<<p);
				}
			}
		}
		if (ibits & (1<<f)) {
			fcc = read(f, fibuf, sizeof (fibuf));
			if (fcc < 0 && errno == EWOULDBLOCK)
				fcc = 0;
			else {
				register char *cp;
				int left, n;

				if (fcc <= 0)
					break;
				fbp = fibuf;

			top:
				for (cp = fibuf; cp < fibuf+fcc-1; cp++)
					if (cp[0] == magic[0] &&
					    cp[1] == magic[1]) {
						left = fcc - (cp-fibuf);
						n = control(p, cp, left);
						if (n) {
							left -= n;
							if (left > 0)
								bcopy(cp+n, cp, left);
							fcc -= n;
							goto top; /* n^2 */
						}
					}
			}
		}

		if ((obits & (1<<p)) && fcc > 0) {
			cc = write(p, fbp, fcc);
			if (cc > 0) {
				fcc -= cc;
				fbp += cc;
			}
		}

		if (ibits & (1<<p)) {
			pcc = read(p, pibuf, sizeof (pibuf));
			pbp = pibuf;
			if (pcc < 0 && errno == EWOULDBLOCK)
				pcc = 0;
			else if (pcc <= 0)
				break;
			else if (pibuf[0] == 0)
				pbp++, pcc--;
			else {
				if (pkcontrol(pibuf[0])) {
					pibuf[0] |= oobdata[0];
					send(f, &pibuf[0], 1, MSG_OOB);
				}
				pcc = 0;
			}
		}
		if ((obits & (1<<f)) && pcc > 0) {
			cc = write(f, pbp, pcc);
			if (cc < 0 && errno == EWOULDBLOCK) {
				/* also shouldn't happen */
				sleep(5);
				continue;
			}
			if (cc > 0) {
				pcc -= cc;
				pbp += cc;
			}
		}
	}
}

cleanup()
{

	rmut();
#ifndef sgi
	vhangup();		/* XXX */
#else
	kill(0, SIGHUP);	/* do not kill background processes */
#endif
	shutdown(netf, 2);
	exit(1);
}

fatal(f, msg)
	int f;
	char *msg;
{
	char buf[BUFSIZ];

	buf[0] = '\01';		/* error indicator */
	(void) sprintf(buf + 1, "rlogind: %s.\r\n", msg);
	(void) write(f, buf, strlen(buf));
	exit(1);
}

fatalperror(f, msg, errno)
	int f;
	char *msg;
	int errno;
{
	char buf[BUFSIZ];
	extern int sys_nerr;
	extern char *sys_errlist[];

	if ((unsigned)errno < sys_nerr)
		(void) sprintf(buf, "%s: %s", msg, sys_errlist[errno]);
	else
		(void) sprintf(buf, "%s: Error %d", msg, errno);
	fatal(f, buf);
}

#include <utmp.h>

#ifndef sgi
struct	utmp wtmp;
char	wtmpf[]	= "/usr/adm/wtmp";
char	utmpf[] = "/etc/utmp";
#endif
#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
#define SCMPN(a, b)	strncmp(a, b, sizeof(a))

#ifdef sgi
static struct utmp entry;

/* add ourself to utmp
 */
adut()
{
	register FILE *fp;

	SCPYN(entry.ut_user, "rlogin");
	SCPYN(entry.ut_id, &stty[(sizeof("/dev/tty") - 1)]);
	SCPYN(entry.ut_line, stty+sizeof("/dev/")-1);
	entry.ut_pid = getpid();
	entry.ut_type = LOGIN_PROCESS;
	entry.ut_time = time(0);
	setutent();
	(void)pututline(&entry);
	endutent();

/* Now attempt to add to the end of the wtmp file.  Do not create
 * if it does not already exist.  **  Note  ** This is the reason
 * "r+" is used instead of "a+".  "r+" will not create a file, while
 * "a+" will. */
	if ((fp = fopen("/etc/wtmp","r+")) != NULL) {
		fseek(fp,0L,2);
		fwrite((char*)&entry,sizeof(entry),1,fp);
		fclose(fp);
	}
}

rmut()
{
	register FILE *fp;

	SCPYN(entry.ut_user, "rlogin");
	SCPYN(entry.ut_id, &stty[(sizeof("/dev/tty") - 1)]);
	SCPYN(entry.ut_line, stty+sizeof("/dev/")-1);
	entry.ut_pid = getpid();
	entry.ut_type = DEAD_PROCESS;
	entry.ut_time = time(0);
	setutent();
	(void)pututline(&entry);
	endutent();
	if ((fp = fopen("/etc/wtmp","r+")) != NULL) {
		entry.ut_user[0] = '\0';
		fseek(fp,0L,2);
		fwrite((char*)&entry,sizeof(entry),1,fp);
		fclose(fp);
	}

	chmod(stty, 0666);
	chown(stty, 0, 0);
}
#else
rmut()
{
	register f;
	int found = 0;
	struct utmp *u, *utmp;
	int nutmp;
	struct stat statbf;

	f = open(utmpf, O_RDWR);
	if (f >= 0) {
		fstat(f, &statbf);
		utmp = (struct utmp *)malloc(statbf.st_size);
		if (!utmp)
			syslog(LOG_ERR, "utmp malloc failed");
		if (statbf.st_size && utmp) {
			nutmp = read(f, utmp, statbf.st_size);
			nutmp /= sizeof(struct utmp);

			for (u = utmp ; u < &utmp[nutmp] ; u++) {
				if (SCMPN(u->ut_line, line+5) ||
				    u->ut_name[0]==0)
					continue;
				lseek(f, ((long)u)-((long)utmp), L_SET);
				SCPYN(u->ut_name, "");
				SCPYN(u->ut_host, "");
				time(&u->ut_time);
				write(f, (char *)u, sizeof(wtmp));
				found++;
			}
		}
		close(f);
	}
	if (found) {
		f = open(wtmpf, O_WRONLY|O_APPEND);
		if (f >= 0) {
			SCPYN(wtmp.ut_line, line+5);
			SCPYN(wtmp.ut_name, "");
			SCPYN(wtmp.ut_host, "");
			time(&wtmp.ut_time);
			write(f, (char *)&wtmp, sizeof(wtmp));
			close(f);
		}
	}
	chmod(line, 0666);
	chown(line, 0, 0);
	line[strlen("/dev/")] = 'p';
	chmod(line, 0666);
	chown(line, 0, 0);
}
#endif
