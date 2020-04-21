#include <sys/types.h>
#include <sys/ioctl.h>
#include <xns/Xnsioctl.h>
#include <xns/Xns.h>
#include <pwd.h>
#include <stdio.h>
#include <signal.h>

#ifdef	SYSTEMV
#include <sys/termio.h>
#endif

char buf[2048];
char hostname[32];
char debugflag;
int fd;
int errcount;
char *bootdir = "/usr/local/boot";

char	noxshflag;
char	noxxflag;
char	noxcpflag;
char	nologinflag;

char user[32];
char passwd[32];
char command[5120];
char minusname[32];
char homedir[64] = "HOME=";
char shell[64] = "SHELL=";
#ifdef SYSTEMV
char username[68] ="LOGNAME=";
#else
char username[64] ="USER=";
#endif
char *envinit[] =
	{homedir, shell, "PATH=:/usr/ucb:/usr/local/bin:/bin:/usr/bin:",
		username, 0};
extern char **environ;
struct log {
	char device[16];
	int pid;
	int flag;
};
#define NLOGINS	200
struct log logins[NLOGINS];

char Mguest[] = "Using guest account.\n\r";

/*
 * Process arguments, and start listening for connections
 */
main(argc, argv)
	int argc;
	char **argv;
{
	register int i;

	i = 1;
	while (argc > 1) {
		if (strcmp(argv[i], "-d")==0) {
			debugflag++;
			i++;
			argc--;
			continue;
		}
		if (strcmp(argv[i], "-xx")==0) {
			noxxflag++;
			i++;
			argc--;
			continue;
		}
		if (strcmp(argv[i], "-xcp")==0) {
			noxcpflag++;
			i++;
			argc--;
			continue;
		}
		if (strcmp(argv[i], "-login")==0) {
			nologinflag++;
			i++;
			argc--;
			continue;
		}
		if (strcmp(argv[i], "-xsh")==0) {
			noxshflag++;
			i++;
			argc--;
			continue;
		}
		/* must be bootdir arg */
		break;
	}
	if (argc>1)
		bootdir = argv[i];

	setup();
	/* run with some priority so that we don't drop connections */
	nice(-20);
	nice(-20);
	nice(10);		/* run at nice(-10) */
	listen();
}

/*
 * child is called via SIGCHLD, when a child process dies
 */
child()
{
	register struct log *lp;
	register pid, i;

	if (debugflag) {
		fprintf(stderr, "sig child ");
		fflush(stderr);
	}
	pid = wait((int *)0);
	if (debugflag) {
		fprintf(stderr, "\tpid %d\n", pid);
		fflush(stderr);
	}
	if (pid<=0)
		goto out;
	for(lp=logins; lp< &logins[NLOGINS]; lp++) {
		if (lp->pid == pid) {
			i = lp - logins;
			if (debugflag) {
				fprintf(stderr, "rmutmp %d\n", i);
				fflush(stderr);
			}
			if (lp->flag)
				rmutmp(i);
			if (lp->device[0]) {
				chown(lp->device, 0, 0);
				chmod(lp->device, 0666);
			}
			lp->flag = 0;
			lp->device[0] = 0;
			break;
		}
	}
out:
	signal(SIGCHLD, child);
}

setup()
{
	register x;

	if (makeproc()<0)
		exit(-1);
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGALRM, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGCHLD, child);
	/*
	 * Get out of any process groups we may have been in
	 */
	x = xnsfile();
	ioctl(x, NXSETPGRP, 0);
	close(x);

	x = geteuid();
	if (x) {
		conserr(
		    "xnsd: attempt to start by uid %d failed (must be root)\n",
		    x);
		exit(1);
	}
	gethostname(hostname, sizeof hostname);

	for(x=0; x<NDEV; x++) {
		register char *s;

		s = (char *)xnspathname(x);
		chmod(s, 0666);
	}
}

listen()
{
	int net, cc, nargs, cmd, ret;
	short socket;
	static char doneonce;

	for (;;) {
		/*
		 * Get a connection to listen on
		 */
		if ((net=xnsfile())<0) {
			if (!doneonce) {
				conserr("%s",
					"xnsd: can't open ethernet device\n");
				exit(1);
			}
			doneonce = 1;
			conserr("%s", "xnsd: can't open ethernet device\n");
			sleep(5);
			return(-1);
		}
		doneonce = 1;
		ioctl(net, NXBLOCKIO, 0);
		ioctl(net, NXIOFAST, 0);
		socket = -1;
		ioctl(net, NXSOCKET, &socket);

		/*
		 * Wait for connection
		 */
		do {
			ret = ioctl(net, NXSOCKWAIT, 0);
		} while (ret<0);
		ioctl(net, NXGETSOCK, &socket);
		netread(0, 0, 0);

		if (debugflag)
			fprintf(stderr, "socket %d\n", socket);

		docmd(net, socket);
		close(net);
	}
}


docmd(f, socket)
{
	register char *cp;
	register struct passwd *pwd;
	register struct log *lp;
	int slot, t, rootflag;
	extern char _xnsttyslot[];
	extern char _xnsfilename[];

	fd = f;
	rootflag = 0;

	slot = mkutmp("", _xnsttyslot, "");
	if (debugflag)
		fprintf(stderr, "ttyslot %d\n", slot);
	t = fork();
	lp = &logins[slot];
	if (socket==XSHSOCKET || socket==LOGINSOCKET) {
		lp->flag++;
	} else {
		if (t==0)
			signal(SIGCHLD, SIG_DFL);
	}
	lp->pid = t;
	strncpy(lp->device, _xnsfilename, sizeof (lp->device));
	if (t) {
		if (debugflag) {
			fprintf(stderr, "fork %d slot %d\n", t, slot);
			fflush(stderr);
		}
		return;
	}
	/* readjust priority to user normal */
	nice(-20);
	nice(-20);
	nice(20);

	ioctl(f, NXSETPGRP, 0);
	if (socket==LOGINSOCKET) {
#ifdef SYSTEMV
		slot = mkutmp("xnslogin", _xnsttyslot, "");
#endif
		goto doexec;
	}
	if (socket==BOOTSOCKET) {
		if (debugflag) {
			fprintf (stderr, "boot %s %s\n", hostname, bootdir);
			fflush (stderr);
		}
		goto doexec;
	}

	alarm(360);
	getstr(user, sizeof user, "username");
	getstr(passwd, sizeof passwd, "passwd");
	getstr(command, sizeof command, "command");
	if (debugflag) {
		fprintf(stderr, "user %s, passwd %s, cmd %s %s\n",
				 user, passwd, command, _xnsttyslot);
		fflush(stderr);
	}

	/*
	 * See if xx's are disabled.
	 */
	if (noxxflag && ((socket == EXECSOCKET) || (socket == XSHSOCKET))) {
		if (strncmp(command, "xcp ", 4)) {
			sprintf(buf, "%s: remote xx disallowed\r\n",
				     hostname);
			error(f, buf);
			exit(1);
		}
	}

	alarm(0);
	setpwent();
	pwd = (struct passwd *)getpwnam(user);
	if (pwd && pwd->pw_uid==0) {
		rootflag++;
		pwd = NULL;
	}

	if (pwd == NULL) {
		setpwent();
		pwd = (struct passwd *)getpwnam("guest");
		if (pwd == NULL && socket == XSHSOCKET) {
			socket = LOGINSOCKET;
#ifdef SYSTEMV
			slot = mkutmp("xnslogin", _xnsttyslot, "");
#endif
			goto doexec;
		}
		if (pwd == NULL) {
			if (rootflag) {
				error(f, "root access disallowed\r\n");
			} else {
				error(f, "no account for ");
				error(f, user);
				error(f, "\r\n");
			}
			error(f, "\n\rno guest account\n\r");
			exit(1);
		}
		strcpy(user, "guest");
	}
	endpwent();

	/*
	 if (*pwd->pw_passwd.. etc
	 */
	if (chdir(pwd->pw_dir) < 0) {
		sprintf(buf, "chdir(%s) failed on host <%s> for user %s\n\r",
			pwd->pw_dir, hostname, user);
		error(f, buf);
		exit(1);
	}

	if (*pwd->pw_shell == '\0')
		pwd->pw_shell = "/bin/sh";

	if (socket==XSHSOCKET)
		slot = mkutmp(user, _xnsttyslot, passwd);
	endutmp();
#ifdef BSD42
	initgroups(pwd->pw_name, pwd->pw_gid);
#endif
	chown(_xnsfilename, pwd->pw_uid, pwd->pw_gid);
	chmod(_xnsfilename, 0622);
	setgid(pwd->pw_gid);
	setuid(pwd->pw_uid);
	environ = envinit;
	strncat(homedir, pwd->pw_dir, sizeof(homedir)-6);
	strncat(shell, pwd->pw_shell, sizeof(shell)-7);
	strncat(username, pwd->pw_name, sizeof(username)-6);
	cp = (char *)rindex(pwd->pw_shell, '/');
	if (cp)
		cp++;
	else
		cp = pwd->pw_shell;


doexec:
	close(0); close(1); close(2);
	dup(f); dup(f); dup(f);
	close(f);
	close(3); close(4);

	if (socket==BOOTSOCKET) {
		execl("/etc/sgboot", "sgboot", "-x", hostname, bootdir);
		exit(1);
	}

	signal(SIGHUP, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGALRM, SIG_DFL);
	signal(SIGPIPE, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGCHLD, SIG_DFL);
#ifdef	SYSTEMV
	stty_sane(0);
#endif

	if (socket==LOGINSOCKET) {
		ioctl (0, NXBLOCKOFF, 0);
		if (nologinflag) {
			sprintf(buf, "%s: remote login disallowed\r\n",
				     hostname);
			write(1, buf, strlen(buf));
		} else {
			write (1, hostname, strlen(hostname));
			write (1, " ", 1);
			execl ("/bin/login", "login", 0);
		}
		exit (1);
	}
	if (socket==XSHSOCKET) {
		ioctl(0, NXBLOCKOFF, 0);
		if (noxshflag) {
			sprintf(buf, "%s: remote xsh disallowed\r\n",
				     hostname);
			write(1, buf, strlen(buf));
			exit(1);
		}
		cp = (char *)rindex(pwd->pw_shell, '/');
		if (cp)
			cp++;
		else
			cp = pwd->pw_shell;
		minusname[0] = '-';
		strncat(minusname, cp, sizeof (minusname)-2);
		if (rootflag && !strcmp(user, "guest"))
			write(1, Mguest, sizeof (Mguest));
		execl(pwd->pw_shell, minusname, 0);
	} else {
		if (strncmp(command, "xcp", 3) == 0) {
			if (noxcpflag) {
				sprintf(buf, "%s: remote xcp disallowed\r\n",
					     hostname);
				write(1, buf, strlen(buf));
				exit(1);
			}
			execl("/bin/csh", "csh", "-cf", command, 0);
		} else
			execl(pwd->pw_shell, cp, "-c", command, 0);
	}
	perror(pwd->pw_shell);
	exit(1);
}

#ifdef	SYSTEMV
/*
 * Setup the tty in a ``sane'' mode.
 */
stty_sane(f)
	int f;
{
	struct termio termio;

	(void) ioctl(f,TCGETA,&termio);
	termio.c_iflag = BRKINT|IGNPAR|ISTRIP|ICRNL|IXON;
	termio.c_oflag = OPOST|ONLCR|TAB3;
	(void) ioctl(f,TCSETA,&termio);
}
#endif

getstr(buf, cnt, err)
char *buf, *err;
{
	char c;

	do {
		if (netread(fd, &c, 1) != 1)
			exit(1);
		*buf++ = c;
		if (--cnt == 0) {
			fprintf(stderr, "%s too long\n", err);
			exit(1);
		}
	} while (c != 0);
}

/*
 * netread reads data from the xns code in 1024 byte chunks, cuz if the
 * network gives data to the user, it will give a maximum of 1024 bytes,
 * REGARDLESS OF HOW MUCH THE USER REQUESTED...That is, if the user reads
 * into a buffer holding 5 bytes, and there are 1024 waiting, the user
 * gets 1024, thus clobbering his data space...
 */
netread(fd, addr, count)
	int fd;
	char *addr;
	int count;
{
	static char buf[1024];
	static last, amount, sum;
	struct xnsio xnsio;

	sum = 0;
	if (count==0) {
		last = amount = 0;
		return(0);
	}
	if (amount==0) {
		amount = read(fd, buf, sizeof buf);
		if (debugflag)
			fprintf(stderr, "amount %d\n", amount); fflush(stderr);
		if (amount <= 0)
			return(amount);
	}
	if (amount <= count) {
		bcopy(&buf[last], addr, amount);
		sum = amount;
		last = 0;
		amount = 0;
		return(sum);
	}

	bcopy(&buf[last], addr, count);
	last += count;
	amount -= count;
	sum += count;

	return(sum);
}

/*
 * Make child process by forking twice
 * (so we don't have to wait()).
 */
makeproc()
{
	register t;

	t = fork();
	if (t) {
		(void) wait((int *)0);
		return -1;
	}
	t = fork();
	if (t) {
		if (debugflag)
			fprintf(stderr,"%d\n", t);
		exit(0);
	}
}

error(f, s)
	int f;
	char *s;
{
	write(f, s, strlen(s));
}
