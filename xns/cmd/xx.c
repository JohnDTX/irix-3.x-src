/*
 * xx.c --
 */
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <xns/Xns.h>
#include <xns/Xnsioctl.h>
#include <errno.h>
#include <setjmp.h>

#ifdef SYSTEM5
# define bcopy(s, t, n)	blt(t, s, n)
#include <termio.h>
#include <string.h>
#endif

#ifdef UNIX4_2
#include <sgtty.h>
struct ltchars defltc;
#include <strings.h>
#endif

extern char *rindex(), *index();

#ifndef CTRL
#define CTRL(ch)	('ch' & 037)
#endif

extern int cleanup();


short _super = 0;
char *logname;
char *CalledAs = "xx";
char *hostp;
int xloginmode;
int full_login;		/* full "login" */
int tflag = 0;
int rflag = 0;
int xflag = 1;
char cmdbuf[256];

int inrawmode;
int net;
int ttyfd;
int child;
char deadchild;

char buf[1024];


char Musage[] = "\
\tusage: xlogin [-rtx] host\n\
\tor:    xx [-rtx] host [cmd]\n\
\tor:    HOST [-rtx] cmd\n";

main(argc, argv)
	int argc; char **argv;
{
	register char *cmd, *p;

	/* determine how called (xlogin, xx, etc) */
	if (--argc<0)
		errwarn(Musage);
	CalledAs = *argv++;

	if ((p = rindex(CalledAs, '/'))!=0)
		CalledAs = p+1;

	/* get flags */
	while (argc > 0 && *(p = *argv)=='-') {
		argc--; argv++; p++;

		while (*p!=0)
		switch (*p++) {

		case 'r':
			rflag = !rflag;
			break;

		case 't':
			tflag = !tflag;
			break;

		case 'x':
			xflag = !xflag;
			break;

		default:
			errexit("unknown flag %c\n", p[-1]);
			break;
		}
	}

	/* get host arg */
	hostp = 0;
	if (strcmp(CalledAs, "xlogin")==0) {
		xloginmode++;
	} else
	if (strcmp(CalledAs, "xx")==0) {
	} else {
		hostp = CalledAs;
	}

	if (hostp == 0) {
		if (--argc<0)
			errexit(Musage);
		hostp = *argv++;
	}

	/* split off optional :USER */
	logname = 0;
	if ((p = index(hostp, '.')) != 0
	 || (p = index(hostp, ':')) != 0) {
		*p = 0;
		logname = p+1;
	}

	if (!_super)
	if (logname != 0 && !xloginmode)
		errexit("only xlogin accepts :USER\n");

	sprintf(cmdbuf, "CMDNAME=%s:%s", CalledAs, hostp);
#ifdef debug
	fprintf(stderr, "CMDNAME=%s:%s\n", CalledAs, hostp);
#endif

	cmd = *argv;

	/* make connection and copy to / from */
	full_login = 0;
	if (xloginmode) {
		if (argc>0)
			errexit(Musage);
		net = xnsconnect(hostp, LOGINSOCKET);
#ifdef debug
		fprintf(stderr, "net= %d after xnsconnect(%s, LOGINSOCKET)\n",
				net, hostp);
#endif
		full_login ++;
	} else
	if (argc<=0) {
		net = xsh(hostp);
#ifdef debug
		fprintf(stderr, "net= %d after xsh(%s)\n", net, hostp);
#endif
		logname = 0;
		full_login ++;
	} else {
		/* (UGH) concatenate remaining args to form cmd string */
		argc--; argv++;
		while (--argc>=0) {
			p = *argv++; p--;
			while (*p == 0)
				*p-- = ' ';
		}
		net = xcmd(hostp, cmd);
#ifdef debug
		fprintf(stderr, "net= %d after xcmd(%s, %s)\n",
				net, hostp, cmd);
#endif
		logname = 0;
	}

	if (net<0)
		scerrexit("can't connect to %s\n", hostp);

	setcatch(SIGHUP, cleanup);
	setcatch(SIGTERM, cleanup);
	signal(SIGCHLD, cleanup);

	child = fork();
	if (child < 0)
		scerrexit("Can't fork\n");

	if (child == 0) {

		if (full_login) {
			signal(SIGINT, SIG_IGN);
			signal(SIGQUIT, SIG_IGN);
		}
		readnet();
		close(net);

		_exit(0);	/* should generate SIGCHLD */
	}

	if (full_login)
		readtty();
	else
		readinput();

	xnseof(net);
	close(net);
	while (!deadchild)
		pidwait(child);

	_exit(0);
}

readinput()
{
	register int cc;

	while ((cc=read(0, buf, sizeof buf)) > 0)
		write(net, buf, cc);
}


readtty()
{
	extern int errno;

	register int cc, i;
	register char *p, last;

#ifdef UNIX4_2
	ioctl(0, TIOCGLTC, (char *)&defltc);
#endif

	inrawmode = isatty(ttyfd);
	grab_tty();

	if (logname != 0) {
		while (*logname != 0)
			write(net, logname++, 1);
		write(net, "\r", 1);
	}

	last = '\r';

	while (!deadchild) {
		cc = read(ttyfd, p = buf, sizeof buf);
		if (cc == 0 || cc < 0 && errno != EINTR)
			break;

		if (!tflag) {
			if (*p=='~' && (last=='\r'||last=='\n')) {
				p++; cc--;
				if (cc<=0) {
					*p = 000;
					if (read(ttyfd, p, 1) > 0)
						cc++;
				}
				escape(*p);
				p++; cc--;
				last = '\r';
			}
		}

		if (!xflag) {
			for (i = 0; i < cc; i++) {
				last = p[i];
				if (last == CTRL(S) || last == CTRL(Q)) {
# ifdef SYSTEM5
					ioctl(ttyfd, TCXONC, last==CTRL(Q));
					cc--;
					bcopy(p+i+1, p+i, cc-i);
					i--;
# endif SYSTEM5
					last = '\r';
				}
			}
		}

		if (cc > 0) {
			write(net, p, cc);
			last = p[cc-1];
		}
	}

	relse_tty();
}

escape(c)
	char c;
{	
	switch (c) {
# ifdef SYSTEM5
		  /* toggle xflag */
	case 'x':
		xflag = !xflag;
		fprintf(stderr, "local xon-xoff %s\n\r", xflag?"OFF":"ON");
		return;
# endif SYSTEM5
		  /* ~.: quit	  */
	case '.':
		fprintf(stderr, "\n\r") , fflush(stderr);
		cleanup();
		return;
		  /* ~~: ~ itself */
	case '~':
		write(net, "~", 1);
		return;
		  /* ~!: shell esc */
	case '!':
		shell();
		return;
	}

#ifdef UNIX4_2
	if (c==defltc.t_suspc || c==defltc.t_dsuspc) {
		fprintf(stderr, "\n\r") , fflush(stderr);

		pushsig(SIGCHLD, SIG_IGN);
		relse_tty();

		if (buf[1]==defltc.t_suspc)
			kill(0, SIGTSTP);
		else
			kill(getpid(), SIGTSTP);

		popsig(SIGCHLD);
		grab_tty();
		return;
	}
#endif UNIX4_2

	/* default - send it through */
	write(net, "~", 1);
	write(net, &c, 1);
}

shell()
{
	extern char **environ;
	extern char *getenv();

	register char **cp;
	char *shell;
	register int shellpid;

	for(cp = environ; *cp; cp++)
		if(strncmp(*cp, "CMDNAME=", 8) == 0)
			*cp = cmdbuf;

	pushsig(SIGINT, SIG_IGN);
	pushsig(SIGQUIT, SIG_IGN);
	pushsig(SIGCHLD, SIG_DFL);
	relse_tty();

	if ((shellpid = fork()) < 0)
		scerrwarn("can't fork\n");

	if (shellpid == 0) {
		register int i;

		if ((shell = getenv("SHELL")) == 0)
			shell = "sh";

		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		for(i = 3; i < 20; i++)
			close(i);
		fprintf(stderr, "!\n\r") , fflush(stderr);
		execlp(shell, shell, 0);
		_exit(1);
	}

	pidwait(shellpid);

	popsig(SIGINT);
	popsig(SIGQUIT);
	popsig(SIGCHLD);
	grab_tty();

	fprintf(stderr, "!!\n\r") , fflush(stderr);
}

readnet()
{
	register int cc;

	while ((cc = read(net, buf, sizeof buf)) > 0)
		write(1, buf, cc);
}


grab_tty()
{
	if (inrawmode)
		rawmode(ttyfd);
}

relse_tty()
{
	if (inrawmode)
		restoremode(ttyfd);
}

pidwait(wpid)
	register int wpid;
{
	register int w;

	for (;;) {
		w = wait(0);
		if (w < 0 || w == child)
			deadchild++;
		if (w == wpid)
			break;
	}
}

setcatch(signo, action)
	int signo;
	int action;
{
	register int osig;

	if ((osig = (int)signal(signo, SIG_IGN)) == (int)SIG_DFL)
		signal(signo, action);
	else
		signal(signo, osig);
}

int osigs[NSIG+1];
pushsig(signo, action)
	int signo;
	int action;
{
	osigs[signo] = (int)signal(signo, action);
}

popsig(signo)
	int signo;
{
	signal(signo, osigs[signo]);
}

errexit(a)
    struct { int x[5]; } a;
{
    errwarn(a);
    xcleanup(1);
}

errwarn(a)
    struct { int x[5]; } a;
{
    fprintf(stderr, "%s: ", CalledAs);
    fprintf(stderr, a);
    fflush(stderr);
}

scerrwarn(a)
    struct { int x[5]; } a;
{
    extern int sys_nerr;
    extern char *sys_errlist[];
    extern int errno;
    register int xerrno;

    xerrno = errno;
    fprintf(stderr, "%s: ", CalledAs);
    if ((unsigned)xerrno < sys_nerr)
	fprintf(stderr, "%s -- ", sys_errlist[xerrno]);
    else
	fprintf(stderr, "Error %d -- ", xerrno);
    fprintf(stderr, a);
    fflush(stderr);
    errno = xerrno;
    return -1;
}

scerrexit(a)
    struct { int x[5]; } a;
{
    scerrwarn(a);
    xcleanup(1);
}


cleanup()
{
    xcleanup(0);
}

xcleanup(xstat)
    int xstat;
{
    if( child > 0 )
	kill(child, SIGKILL);
    relse_tty();
    _exit(xstat);
}
