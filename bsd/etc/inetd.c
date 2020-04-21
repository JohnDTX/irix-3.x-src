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
static char sccsid[] = "@(#)inetd.c	5.7 (Berkeley) 8/19/86";
#endif not lint

/*
 * Inetd - Internet super-server
 *
 * This program invokes all internet services as needed.
 * connection-oriented services are invoked each time a
 * connection is made, by creating a process.  This process
 * is passed the connection as file descriptor 0 and is
 * expected to do a getpeername to find out the source host
 * and port.
 *
 * Datagram oriented services are invoked when a datagram
 * arrives; a process is created and passed a pending message
 * on file descriptor 0.  Datagram servers may either connect
 * to their peer, freeing up the original socket for inetd
 * to receive further messages on, or ``take over the socket'',
 * processing all arriving datagrams and, eventually, timing
 * out.	 The first type of server is said to be ``multi-threaded'';
 * the second type of server ``single-threaded''.
 *
 * Inetd uses a configuration file which is read at startup
 * and, possibly, at some later time in response to a hangup signal.
 * The configuration file is ``free format'' with fields given in the
 * order shown below.  Continuation lines for an entry must being with
 * a space or tab.  All fields must be present in each entry.
 *
 *	service name			must be in /etc/services
 *	socket type			stream/dgram/raw/rdm/seqpacket
 *	protocol			must be in /etc/protocols
 *	wait/nowait			single-threaded/multi-threaded
 *	user				user to run daemon as
 *	server program			full path name
 *	server program arguments	maximum of MAXARGS (5)
 *
 * Services based on RPC constitute a special case.  These services
 * need not be listed in /etc/services, because their port numbers are
 * dynamically bound by the portmapper.  The portmapper protocol specifies
 * a service with the triple (program number, version min, version max).
 * The format for RPC service entries begins as follows:
 *
 *	service name			must be ``rpc''
 *	program number			must match server's rpc protocol
 *	version[-highversion]		version(s) served
 *	socket type			stream/dgram/raw/rdm/seqpacket/rpc
 *	etc. as above
 *
 * Comment lines are indicated by a `#' in column 1.
 */
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#ifdef sgi
#include <rpc/rpc.h>		/* includes <netinet/in.h> */
#include <rpc/pmap_prot.h>
#else
#include <netinet/in.h>
#endif
#include <arpa/inet.h>

#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <netdb.h>
#include <syslog.h>
#include <pwd.h>

#define	TOOMANY		40		/* don't start more than TOOMANY */
#define	CNT_INTVL	60		/* servers in CNT_INTVL sec. */
#define	RETRYTIME	(60*10)		/* retry after bind or server fail */

#ifndef sgi
#define	SIGBLOCK	(sigmask(SIGCHLD)|sigmask(SIGHUP)|sigmask(SIGALRM))
#endif

extern	int errno;

int	reapchild(), retry();
char	*index();
char	*malloc();

int	debug = 0;
int	nsock, maxsock;
fd_set	allsock;
int	options;
int	timingout;
struct	servent *sp;

#ifdef sgi
char	RPC_SERVICE[] = "rpc";		/* special case service name */
#endif

struct	servtab {
	char	*se_service;		/* name of service */
	int	se_socktype;		/* type of socket to use */
	char	*se_proto;		/* protocol used */
	short	se_wait;		/* single threaded server */
	short	se_checked;		/* looked at during merge */
	char	*se_user;		/* user name to run as */
	struct	biltin *se_bi;		/* if built-in, description */
	char	*se_server;		/* server program */
#define MAXARGV 5
	char	*se_argv[MAXARGV+1];	/* program arguments */
	int	se_fd;			/* open descriptor */
#ifdef sgi
	union {
		struct	sockaddr_in ctrladdr;
		struct{
			u_long prog;
			u_long lowvers;
			u_long highvers;
		} rpcnum
	} se_un;
#else
	struct	sockaddr_in se_ctrladdr;/* bound address */
#endif
	int	se_count;		/* number started since se_time */
	struct	timeval se_time;	/* start of se_count */
	struct	servtab *se_next;
} *servtab;

#ifdef sgi
#define se_ctrladdr	se_un.ctrladdr
#define se_rpc		se_un.rpcnum
#endif

int echo_stream(), discard_stream(), machtime_stream();
int daytime_stream(), chargen_stream();
int echo_dg(), discard_dg(), machtime_dg(), daytime_dg(), chargen_dg();

struct biltin {
	char	*bi_service;		/* internally provided service name */
	int	bi_socktype;		/* type of socket supported */
	short	bi_fork;		/* 1 if should fork before call */
	short	bi_wait;		/* 1 if should wait for child */
	int	(*bi_fn)();		/* function which performs it */
} biltins[] = {
	/* Echo received data */
	"echo",		SOCK_STREAM,	1, 0,	echo_stream,
	"echo",		SOCK_DGRAM,	0, 0,	echo_dg,

	/* Internet /dev/null */
	"discard",	SOCK_STREAM,	1, 0,	discard_stream,
	"discard",	SOCK_DGRAM,	0, 0,	discard_dg,

	/* Return 32 bit time since 1970 */
	"time",		SOCK_STREAM,	0, 0,	machtime_stream,
	"time",		SOCK_DGRAM,	0, 0,	machtime_dg,

	/* Return human-readable time */
	"daytime",	SOCK_STREAM,	0, 0,	daytime_stream,
	"daytime",	SOCK_DGRAM,	0, 0,	daytime_dg,

	/* Familiar character generator */
	"chargen",	SOCK_STREAM,	1, 0,	chargen_stream,
	"chargen",	SOCK_DGRAM,	0, 0,	chargen_dg,
	0
};

#define NUMINT	(sizeof(intab) / sizeof(struct inent))
#ifdef sgi
char	*CONFIG = "/usr/etc/inetd.conf";
#else
char	*CONFIG = "/etc/inetd.conf";
#endif
char	**Argv;
char	*LastArg;

main(argc, argv, envp)
	int argc;
	char *argv[], *envp[];
{
	register struct servtab *sep;
	register struct passwd *pwd;
	char *cp, buf[50];
	int pid, i, dofork;
#ifdef sgi
#ifdef SVR3
extern struct passwd *getpwnam();
#endif
#else
	struct sigvec sv;
#endif

	Argv = argv;
	if (envp == 0 || *envp == 0)
		envp = argv;
	while (*envp)
		envp++;
	LastArg = envp[-1] + strlen(envp[-1]);
	argc--, argv++;
	while (argc > 0 && *argv[0] == '-') {
		for (cp = &argv[0][1]; *cp; cp++) switch (*cp) {

		case 'd':
			debug = 1;
			options |= SO_DEBUG;
			break;

		default:
			fprintf(stderr,
			    "inetd: Unknown flag -%c ignored.\n", *cp);
			break;
		}
nextopt:
		argc--, argv++;
	}
	if (argc > 0)
		CONFIG = argv[0];
#ifndef DEBUG
#ifdef sgi
#define fprintf syslog			/* this is a terrible hack */
#undef stderr				/* but it is effective */
#define stderr LOG_DEBUG
#endif
	if (fork())
		exit(0);
	{ int s;
#ifdef sgi
	for (s = getdtablesize(); --s >= 0; )
#else
	for (s = 0; s < 10; s++)
#endif
		(void) close(s);
	}
	(void) open("/", O_RDONLY);
	(void) dup2(0, 1);
	(void) dup2(0, 2);
	{ int tt = open("/dev/tty", O_RDWR);
	  if (tt > 0) {
		ioctl(tt, TIOCNOTTY, (char *)0);
		close(tt);
	  }
	}
#endif
	openlog("inetd", LOG_PID | LOG_NOWAIT, LOG_DAEMON);
#ifdef sgi
	mask_alrm();
#else
	bzero((char *)&sv, sizeof(sv));
	sv.sv_mask = SIGBLOCK;
	sv.sv_handler = retry;
	sigvec(SIGALRM, &sv, (struct sigvec *)0);
#endif
	config();
#ifdef sgi
	(void) signal(SIGHUP, config);
	(void) signal(SIGCHLD, reapchild);
	restore_alrm();
#else
	sv.sv_handler = config;
	sigvec(SIGHUP, &sv, (struct sigvec *)0);
	sv.sv_handler = reapchild;
	sigvec(SIGCHLD, &sv, (struct sigvec *)0);
#endif

	for (;;) {
	    int s, ctrl, n;
	    fd_set readable;

	    while (nsock == 0)
#ifdef sgi
		/*
		 * Better be good enough.
		 */
		pause();
#else
		sigpause(0);
#endif
	    readable = allsock;
	    if ((n = select(maxsock + 1, &readable, (fd_set *)0,
		(fd_set *)0, (struct timeval *)0)) <= 0) {
		    if (n < 0 && errno != EINTR)
			syslog(LOG_WARNING, "select: %m\n");
#ifdef sgi
		    sginap(HZ);
#else
		    sleep(1);
#endif
		    continue;
	    }
	    for (sep = servtab; n && sep; sep = sep->se_next)
	    if (FD_ISSET(sep->se_fd, &readable)) {
		n--;
		if (debug)
			fprintf(stderr, "someone wants %s\n", sep->se_service);
		if (!sep->se_wait && sep->se_socktype == SOCK_STREAM) {
			ctrl = accept(sep->se_fd, (struct sockaddr *)0,
			    (int *)0);
			if (debug)
				fprintf(stderr, "accept, ctrl %d\n", ctrl);
			if (ctrl < 0) {
				if (errno == EINTR)
					continue;
				syslog(LOG_WARNING, "accept: %m");
				continue;
			}
		} else
			ctrl = sep->se_fd;
#ifdef sgi
		(void) signal(SIGCHLD, SIG_DFL);
		(void) signal(SIGHUP, SIG_IGN);
		mask_alrm();
#else
		(void) sigblock(SIGBLOCK);
#endif
		pid = 0;
		dofork = (sep->se_bi == 0 || sep->se_bi->bi_fork);
		if (dofork) {
			if (sep->se_count++ == 0)
			    (void)gettimeofday(&sep->se_time,
				(struct timezone *)0);
			else if (sep->se_count >= TOOMANY) {
				struct timeval now;

				(void)gettimeofday(&now, (struct timezone *)0);
				if (now.tv_sec - sep->se_time.tv_sec >
				    CNT_INTVL) {
					sep->se_time = now;
					sep->se_count = 1;
#ifdef sgi
				} else if (sep->se_service != RPC_SERVICE) {
#else
				} else {
#endif
					syslog(LOG_ERR,
			"%s/%s server failing (looping), service terminated\n",
					    sep->se_service, sep->se_proto);
#ifdef sgi
					close_sep(sep);
#else
					FD_CLR(sep->se_fd, &allsock);
					(void) close(sep->se_fd);
					sep->se_fd = -1;
					sep->se_count = 0;
					nsock--;
#endif
#ifdef sgi
					(void) signal(SIGHUP, config);
					(void) signal(SIGCHLD, reapchild);
					restore_alrm();
#else
					sigsetmask(0);
#endif
					if (!timingout) {
						timingout = 1;
						alarm(RETRYTIME);
					}
					continue;
				}
			}
			pid = fork();
		}
		if (pid < 0) {
			if (!sep->se_wait && sep->se_socktype == SOCK_STREAM)
				close(ctrl);
#ifdef sgi
			(void) signal(SIGHUP, config);
			(void) signal(SIGCHLD, reapchild);
			restore_alrm();
			sginap(HZ);
#else
			sigsetmask(0);
			sleep(1);
#endif
			continue;
		}
		if (pid && sep->se_wait) {
			sep->se_wait = pid;
			FD_CLR(sep->se_fd, &allsock);
			nsock--;
		}
#ifdef sgi
		(void) signal(SIGHUP, config);
		(void) signal(SIGCHLD, reapchild);
		restore_alrm();
#else
		sigsetmask(0);
#endif
		if (pid == 0) {
#ifdef	DEBUG
			int tt;

			if (dofork && (tt = open("/dev/tty", O_RDWR)) > 0) {
				ioctl(tt, TIOCNOTTY, 0);
				close(tt);
			}
#endif
			if (dofork)
#ifdef sgi
				openlog("inetd", LOG_PID, LOG_DAEMON);
#else
				for (i = getdtablesize(); --i > 2; )
					if (i != ctrl)
						close(i);
#endif
			if (sep->se_bi)
				(*sep->se_bi->bi_fn)(ctrl, sep);
			else {
				if (debug)
					fprintf(stderr, "%d execl %s\n",
					    getpid(), sep->se_server);
				dup2(ctrl, 0);
				close(ctrl);
				dup2(0, 1);
				dup2(0, 2);
				if ((pwd = getpwnam(sep->se_user)) == NULL) {
					syslog(LOG_ERR,
						"getpwnam: %s: No such user",
						sep->se_user);
					if (sep->se_socktype != SOCK_STREAM)
						recv(0, buf, sizeof (buf), 0);
					_exit(1);
				}
				if (pwd->pw_uid) {
					(void) setgid((gid_t)pwd->pw_gid);
#ifndef sgi
					initgroups(pwd->pw_name, pwd->pw_gid);
#endif
					(void) setuid((uid_t)pwd->pw_uid);
				}
#ifdef sgi
	/* close all of the sockets opened by YP to do the getpwnam() */
				for (i = getdtablesize(); --i > 2; )
					(void)close(i);
#endif
				execv(sep->se_server, sep->se_argv);
				if (sep->se_socktype != SOCK_STREAM)
					recv(0, buf, sizeof (buf), 0);
				syslog(LOG_ERR, "execv %s: %m", sep->se_server);
				_exit(1);
			}
		}
		if (!sep->se_wait && sep->se_socktype == SOCK_STREAM)
			close(ctrl);
	    }
	}
}

reapchild()
{
	union wait status;
	int pid;
	register struct servtab *sep;

	for (;;) {
#ifdef sgi
		pid = wait(&status);
#else
		pid = wait3(&status, WNOHANG, (struct rusage *)0);
#endif
		if (pid <= 0)
			break;
		if (debug)
			fprintf(stderr, "%d reaped\n", pid);
		for (sep = servtab; sep; sep = sep->se_next)
			if (sep->se_wait == pid) {
				if (status.w_status)
					syslog(LOG_WARNING,
					    "%s: exit status 0x%x",
					    sep->se_server, status);
				if (debug)
					fprintf(stderr, "restored %s, fd %d\n",
					    sep->se_service, sep->se_fd);
				FD_SET(sep->se_fd, &allsock);
				nsock++;
				sep->se_wait = 1;
			}
#ifdef sgi
		break;			/* do not loop on system V systems */
#endif
	}
#ifdef sgi
	/*
	 * On system V systems, reset the SIGCHLD trap, but only AFTER
	 * the wait has completed.
	 */
	(void) signal(SIGCHLD, reapchild);
#endif
}

config()
{
	register struct servtab *sep, *cp, **sepp;
	struct servtab *getconfigent(), *enter();
#ifdef sgi
#ifdef SVR3
	void (*ov)();
#else
	int (*ov)();
#endif
#else
	int omask;
#endif

#ifdef sgi
	/*
	 * On System V systems, ignore SIGHUP so that back-to-back
	 * HUPs don't kill the inetd process.
	 */
	(void) signal(SIGHUP, SIG_IGN);
#endif
	if (!setconfig()) {
		syslog(LOG_ERR, "%s: %m", CONFIG);
		return;
	}
	for (sep = servtab; sep; sep = sep->se_next)
		sep->se_checked = 0;
	while (cp = getconfigent()) {
		for (sep = servtab; sep; sep = sep->se_next)
			if (strcmp(sep->se_service, cp->se_service) == 0 &&
#ifdef sgi
			    /*
			     * Must look at se_server to tell whether
			     * rpc service entry is duplicate.
			     */
			    (sep->se_service != RPC_SERVICE ||
			     strcmp(sep->se_server, cp->se_server) == 0) &&
#endif
			    strcmp(sep->se_proto, cp->se_proto) == 0)
				break;
		if (sep != 0) {
			int i;

#ifdef sgi
			ov = signal(SIGCHLD, SIG_DFL);
			mask_alrm();
#else
			omask = sigblock(SIGBLOCK);
#endif
			if (cp->se_bi == 0)
				sep->se_wait = cp->se_wait;
#define SWAP(a, b) { char *c = a; a = b; b = c; }
			if (cp->se_user)
				SWAP(sep->se_user, cp->se_user);
			if (cp->se_server)
				SWAP(sep->se_server, cp->se_server);
			for (i = 0; i < MAXARGV; i++)
				SWAP(sep->se_argv[i], cp->se_argv[i]);
#ifdef sgi
			(void) signal(SIGCHLD, ov);
			restore_alrm();
#else
			sigsetmask(omask);
#endif
			freeconfig(cp);
			if (debug)
				print_service("REDO", sep);
		} else {
			sep = enter(cp);
			if (debug)
				print_service("ADD ", sep);
		}
		sep->se_checked = 1;
#ifdef sgi
		if (sep->se_service == RPC_SERVICE) {
			rpcsetup(sep);
			continue;
		}
#endif
		sp = getservbyname(sep->se_service, sep->se_proto);
		if (sp == 0) {
			syslog(LOG_ERR, "%s/%s: unknown service",
			    sep->se_service, sep->se_proto);
			continue;
		}
		if (sp->s_port != sep->se_ctrladdr.sin_port) {
			sep->se_ctrladdr.sin_port = sp->s_port;
			if (sep->se_fd != -1)
#ifdef sgi
				close_sep(sep);
#else
				(void) close(sep->se_fd);
			sep->se_fd = -1;
#endif
		}
		if (sep->se_fd == -1)
			setup(sep);
	}
	endconfig();
	/*
	 * Purge anything not looked at above.
	 */
#ifdef sgi
	ov = signal(SIGCHLD, SIG_DFL);
	mask_alrm();
#else
	omask = sigblock(SIGBLOCK);
#endif
	sepp = &servtab;
	while (sep = *sepp) {
		if (sep->se_checked) {
			sepp = &sep->se_next;
			continue;
		}
		*sepp = sep->se_next;
		if (sep->se_fd != -1) {
#ifdef sgi
			close_sep(sep);
#else
			FD_CLR(sep->se_fd, &allsock);
			nsock--;
			(void) close(sep->se_fd);
#endif
		}
		if (debug)
			print_service("FREE", sep);
		freeconfig(sep);
		free((char *)sep);
	}
#ifdef sgi
	(void) signal(SIGCHLD, ov);
	(void) signal(SIGHUP, config);
	restore_alrm();
#else
	(void) sigsetmask(omask);
#endif
}

#ifdef sgi
/* this is a partial simulation of blocked signals */
static char nested_alrm = 0;

/* 'unmask' timers */
restore_alrm()
{
	(void) signal(SIGALRM, retry);
	if (nested_alrm) {
		nested_alrm = 0;
		retry();
	}
}

/* count timers */
delay_alrm()
{
	nested_alrm++;
	(void) signal(SIGALRM, delay_alrm);
}

/* 'mask' timers */
mask_alrm()
{
	(void) signal(SIGALRM, delay_alrm);
}
#endif

retry()
{
	register struct servtab *sep;

#ifdef sgi
	mask_alrm();
	timingout = 0;
	for (sep = servtab; sep; sep = sep->se_next) {
		if (sep->se_fd == -1) {
			if (sep->se_service == RPC_SERVICE)
				rpcsetup(sep);
			else
				setup(sep);
		}
	}
	restore_alrm();
#else
	timingout = 0;
	for (sep = servtab; sep; sep = sep->se_next)
		if (sep->se_fd == -1)
			setup(sep);
#endif
}

setup(sep)
	register struct servtab *sep;
{
	int on = 1;

	if ((sep->se_fd = socket(AF_INET, sep->se_socktype, 0)) < 0) {
		syslog(LOG_ERR, "%s/%s: socket: %m",
		    sep->se_service, sep->se_proto);
		return;
	}
#define	turnon(fd, opt) \
setsockopt(fd, SOL_SOCKET, opt, (char *)&on, sizeof (on))
	if (strcmp(sep->se_proto, "tcp") == 0 && (options & SO_DEBUG) &&
	    turnon(sep->se_fd, SO_DEBUG) < 0)
		syslog(LOG_ERR, "setsockopt (SO_DEBUG): %m");
	if (turnon(sep->se_fd, SO_REUSEADDR) < 0)
		syslog(LOG_ERR, "setsockopt (SO_REUSEADDR): %m");
#undef turnon
	if (bind(sep->se_fd, &sep->se_ctrladdr,
	    sizeof (sep->se_ctrladdr)) < 0) {
		syslog(LOG_ERR, "%s/%s: bind: %m",
		    sep->se_service, sep->se_proto);
		(void) close(sep->se_fd);
		sep->se_fd = -1;
		if (!timingout) {
			timingout = 1;
			alarm(RETRYTIME);
		}
		return;
	}
	if (sep->se_socktype == SOCK_STREAM)
		listen(sep->se_fd, 10);
	FD_SET(sep->se_fd, &allsock);
	nsock++;
	if (sep->se_fd > maxsock)
		maxsock = sep->se_fd;
}

#ifdef sgi
/* finish with a service and its socket
 */
close_sep(sep)
register struct servtab *sep;
{
	register u_long v;

	if (-1 != sep->se_fd)
		nsock--;
	FD_CLR(sep->se_fd, &allsock);
	(void) close(sep->se_fd);
	sep->se_fd = -1;
	sep->se_count = 0;
	if (sep->se_service == RPC_SERVICE) {
		for (v = sep->se_rpc.lowvers; v <= sep->se_rpc.highvers; v++)
			pmap_unset(sep->se_rpc.prog, v);
	}
}


rpcsetup(sep)
	register struct servtab *sep;
{
	register int rpcsock;

	if (sep->se_fd != -1)
		close_sep(sep);

	sep->se_fd = rpcsock =
	    getrpcsock(sep->se_rpc.prog, sep->se_rpc.lowvers,
		sep->se_rpc.highvers, sep->se_socktype,
		sep->se_proto);
	if (rpcsock < 0) {
		syslog(LOG_ERR, "rpc/%s socket creation problem: %m",
		    sep->se_proto);
		return;
	}
	FD_SET(rpcsock, &allsock);
	nsock++;
	if (rpcsock > maxsock)
		maxsock = rpcsock;
	if (debug) {
		fprintf(stderr, "registered %s (%lu/%lu-%lu) on %d\n",
		    sep->se_server, sep->se_rpc.prog,
		    sep->se_rpc.lowvers, sep->se_rpc.highvers,
		    rpcsock);
	}
}

int
getrpcsock(prognum, lowvers, highvers, socktype, proto)
	u_long prognum, lowvers, highvers;
	int socktype;
	char *proto;
{
	register int s;
	register u_long v;
	struct sockaddr_in addr;
	int len = sizeof(struct sockaddr_in);

	if ((s = socket(AF_INET, socktype, 0)) < 0) {
		return -1;
	}
	addr.sin_family = AF_INET;
	addr.sin_port = 0;
	addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(s, &addr, sizeof(addr)) < 0) {
		return -1;
	}
	if (getsockname(s, &addr, &len) != 0) {
		(void)close(s);
		return -1;
	}
	for (v = lowvers; v <= highvers; v++) {
		pmap_unset(prognum, v);
		pmap_set(prognum, v,
		    socktype == SOCK_DGRAM ? IPPROTO_UDP : IPPROTO_TCP,
		    htons(addr.sin_port));
	}
	if (socktype == SOCK_STREAM)
		listen(s, 10);
	return s;
}
#endif

struct servtab *
enter(cp)
	struct servtab *cp;
{
	register struct servtab *sep;
#ifdef sgi
#ifdef SVR3
	void (*ov)();
#else
	int (*ov)();
#endif
#else
	int omask;
#endif
	char *strdup();

	sep = (struct servtab *)malloc(sizeof (*sep));
	if (sep == (struct servtab *)0) {
		syslog(LOG_ERR, "Out of memory.");
		exit(-1);
	}
	*sep = *cp;
	sep->se_fd = -1;
#ifdef sgi
	ov = signal(SIGCHLD, SIG_DFL);
#else
	omask = sigblock(SIGBLOCK);
#endif
	sep->se_next = servtab;
	servtab = sep;
#ifdef sgi
	(void) signal(SIGCHLD, ov);
#else
	sigsetmask(omask);
#endif
	return (sep);
}

FILE	*fconfig = NULL;
struct	servtab serv;
char	line[256];
#ifdef sgi
char	*sskip(), *skip(), *nextline();
#else
char	*skip(), *nextline();
#endif

setconfig()
{

	if (fconfig != NULL) {
		fseek(fconfig, 0L, L_SET);
		return (1);
	}
	fconfig = fopen(CONFIG, "r");
	return (fconfig != NULL);
}

endconfig()
{

	if (fconfig == NULL)
		return;
	fclose(fconfig);
	fconfig = NULL;
}

struct servtab *
getconfigent()
{
	register struct servtab *sep = &serv;
#ifdef sgi
	auto char *cp;
	register char *arg;
	register int argc;
#else
	char *cp, *arg;
	int argc;
#endif

more:
	while ((cp = nextline(fconfig)) && *cp == '#')
		;
	if (cp == NULL)
		return ((struct servtab *)0);
#ifdef sgi
	/*
	 * clear the static buffer, since some fields (se_ctrladdr,
	 * for example) don't get initialized here.
	 */
	bzero((caddr_t)sep, sizeof *sep);
	arg = sskip(&cp);
	if (strcmp(arg, RPC_SERVICE) == 0) {
		sep->se_service = RPC_SERVICE;
		sep->se_rpc.prog = atoi(sskip(&cp));
		arg = sskip(&cp);
		sep->se_rpc.lowvers = atoi(arg);
		if ((arg = index(arg, '-')) == NULL) {
			sep->se_rpc.highvers = sep->se_rpc.lowvers;
		} else {
			sep->se_rpc.highvers = atoi(arg + 1);
		}
	} else {
		sep->se_service = strdup(arg);
	}
	arg = sskip(&cp);
#else
	sep->se_service = strdup(skip(&cp));
	arg = skip(&cp);
#endif
	if (strcmp(arg, "stream") == 0)
		sep->se_socktype = SOCK_STREAM;
	else if (strcmp(arg, "dgram") == 0)
		sep->se_socktype = SOCK_DGRAM;
	else if (strcmp(arg, "rdm") == 0)
		sep->se_socktype = SOCK_RDM;
	else if (strcmp(arg, "seqpacket") == 0)
		sep->se_socktype = SOCK_SEQPACKET;
	else if (strcmp(arg, "raw") == 0)
		sep->se_socktype = SOCK_RAW;
	else
		sep->se_socktype = -1;
#ifdef sgi
	sep->se_proto = strdup(sskip(&cp));
	sep->se_wait = strcmp(sskip(&cp), "wait") == 0;
	sep->se_user = strdup(sskip(&cp));
	sep->se_server = strdup(sskip(&cp));
#else
	sep->se_proto = strdup(skip(&cp));
	arg = skip(&cp);
	sep->se_wait = strcmp(arg, "wait") == 0;
	sep->se_user = strdup(skip(&cp));
	sep->se_server = strdup(skip(&cp));
#endif
	if (strcmp(sep->se_server, "internal") == 0) {
		register struct biltin *bi;

		for (bi = biltins; bi->bi_service; bi++)
			if (bi->bi_socktype == sep->se_socktype &&
			    strcmp(bi->bi_service, sep->se_service) == 0)
				break;
		if (bi->bi_service == 0) {
			syslog(LOG_ERR, "internal service %s unknown\n",
				sep->se_service);
			goto more;
		}
		sep->se_bi = bi;
		sep->se_wait = bi->bi_wait;
	} else
		sep->se_bi = NULL;
	argc = 0;
	for (arg = skip(&cp); cp; arg = skip(&cp))
		if (argc < MAXARGV)
			sep->se_argv[argc++] = strdup(arg);
	while (argc <= MAXARGV)
		sep->se_argv[argc++] = NULL;
	return (sep);
}

freeconfig(cp)
	register struct servtab *cp;
{
	int i;

	if (cp->se_service)
		free(cp->se_service);
	if (cp->se_proto)
		free(cp->se_proto);
	if (cp->se_user)
		free(cp->se_user);
	if (cp->se_server)
		free(cp->se_server);
	for (i = 0; i < MAXARGV; i++)
		if (cp->se_argv[i])
			free(cp->se_argv[i]);
}

/*
 * Safe skip - if skip returns null, log a syntax error in the
 * configuration file and exit.
 */
char *
sskip(cpp)
	char **cpp;
{
	register char *cp;

	cp = skip(cpp);
	if (cp == NULL) {
		syslog(LOG_ERR, "%s: syntax error", CONFIG);
		exit(-1);
	}
	return cp;
}

char *
skip(cpp)
	char **cpp;
{
	register char *cp = *cpp;
	char *start;

again:
	while (*cp == ' ' || *cp == '\t')
		cp++;
	if (*cp == '\0') {
#ifdef sgi
		register int c;
#else
		char c;
#endif

		c = getc(fconfig);
		ungetc(c, fconfig);
		if (c == ' ' || c == '\t')
			if (cp = nextline(fconfig))
				goto again;
		*cpp = (char *)0;
		return ((char *)0);
	}
	start = cp;
	while (*cp && *cp != ' ' && *cp != '\t')
		cp++;
	if (*cp != '\0')
		*cp++ = '\0';
	*cpp = cp;
	return (start);
}

char *
nextline(fd)
	FILE *fd;
{
	char *cp;

	if (fgets(line, sizeof (line), fd) == NULL)
		return ((char *)0);
	cp = index(line, '\n');
	if (cp)
		*cp = '\0';
	return (line);
}

char *
strdup(cp)
	char *cp;
{
	char *new;

	if (cp == NULL)
		cp = "";
	new = malloc((unsigned)(strlen(cp) + 1));
	if (new == (char *)0) {
		syslog(LOG_ERR, "Out of memory.");
		exit(-1);
	}
	strcpy(new, cp);
	return (new);
}

setproctitle(a, s)
	char *a;
	int s;
{
	int size;
	register char *cp;
	struct sockaddr_in sin;
	char buf[80];

	cp = Argv[0];
	size = sizeof(sin);
	if (getpeername(s, &sin, &size) == 0)
		sprintf(buf, "-%s [%s]", a, inet_ntoa(sin.sin_addr));
	else
		sprintf(buf, "-%s", a);
	strncpy(cp, buf, LastArg - cp);
	cp += strlen(cp);
	while (cp < LastArg)
		*cp++ = ' ';
}

/*
 * Internet services provided internally by inetd:
 */

/* ARGSUSED */
echo_stream(s, sep)		/* Echo service -- echo data back */
	int s;
	struct servtab *sep;
{
#ifdef sgi
	char buffer[1024*16];
#else
	char buffer[BUFSIZ];
#endif
	int i;

	setproctitle("echo", s);
	while ((i = read(s, buffer, sizeof(buffer))) > 0 &&
	    write(s, buffer, i) > 0)
		;
	exit(0);
}

/* ARGSUSED */
echo_dg(s, sep)			/* Echo service -- echo data back */
	int s;
	struct servtab *sep;
{
#ifdef sgi
	char buffer[1024*16];
#else
	char buffer[BUFSIZ];
#endif
	int i, size;
	struct sockaddr sa;

	size = sizeof(sa);
	if ((i = recvfrom(s, buffer, sizeof(buffer), 0, &sa, &size)) < 0)
		return;
	(void) sendto(s, buffer, i, 0, &sa, sizeof(sa));
}

/* ARGSUSED */
discard_stream(s, sep)		/* Discard service -- ignore data */
	int s;
	struct servtab *sep;
{
#ifdef sgi
	char buffer[1024*16];
	errno = 0;
#else
	char buffer[BUFSIZ];
#endif

	setproctitle("discard", s);
	while (1) {
		while (read(s, buffer, sizeof(buffer)) > 0)
			;
		if (errno != EINTR)
			break;
	}
	exit(0);
}

/* ARGSUSED */
discard_dg(s, sep)		/* Discard service -- ignore data */
	int s;
	struct servtab *sep;
{
	int i, size;
#ifdef sgi
	char buffer[1024*16];
#else
	char buffer[BUFSIZ];
#endif

	(void) read(s, buffer, sizeof(buffer));
}

#include <ctype.h>
#define LINESIZ 72
char ring[128];
char *endring;

initring()
{
	register int i;

	endring = ring;

	for (i = 0; i <= 128; ++i)
		if (isprint(i))
			*endring++ = i;
}

/* ARGSUSED */
chargen_stream(s, sep)		/* Character generator */
	int s;
	struct servtab *sep;
{
	char text[LINESIZ+2];
	register int i;
	register char *rp, *rs, *dp;

#ifdef sgi
	setproctitle("chargen", s);
#else
	setproctitle("discard", s);
#endif
	if (endring == 0)
		initring();

	for (rs = ring; ; ++rs) {
		if (rs >= endring)
			rs = ring;
		rp = rs;
		dp = text;
		i = MIN(LINESIZ, endring - rp);
		bcopy(rp, dp, i);
		dp += i;
		if ((rp += i) >= endring)
			rp = ring;
		if (i < LINESIZ) {
			i = LINESIZ - i;
			bcopy(rp, dp, i);
			dp += i;
			if ((rp += i) >= endring)
				rp = ring;
		}
		*dp++ = '\r';
		*dp++ = '\n';

		if (write(s, text, dp - text) != dp - text)
			break;
	}
	exit(0);
}

/* ARGSUSED */
chargen_dg(s, sep)		/* Character generator */
	int s;
	struct servtab *sep;
{
	char text[LINESIZ+2];
	register int i;
	register char *rp;
	static char *rs = ring;
	struct sockaddr sa;
	int size;

	if (endring == 0)
		initring();

	size = sizeof(sa);
	if (recvfrom(s, text, sizeof(text), 0, &sa, &size) < 0)
		return;
	rp = rs;
	if (rs++ >= endring)
		rs = ring;
	i = MIN(LINESIZ - 2, endring - rp);
	bcopy(rp, text, i);
	if ((rp += i) >= endring)
		rp = ring;
	if (i < LINESIZ - 2) {
		bcopy(rp, text, i);
		if ((rp += i) >= endring)
			rp = ring;
	}
	text[LINESIZ - 2] = '\r';
	text[LINESIZ - 1] = '\n';

	(void) sendto(s, text, sizeof(text), 0, &sa, sizeof(sa));
}

/*
 * Return a machine readable date and time, in the form of the
 * number of seconds since midnight, Jan 1, 1900.  Since gettimeofday
 * returns the number of seconds since midnight, Jan 1, 1970,
 * we must add 2208988800 seconds to this figure to make up for
 * some seventy years Bell Labs was asleep.
 */

long
machtime()
{
	struct timeval tv;

	if (gettimeofday(&tv, (struct timezone *)0) < 0) {
		fprintf(stderr, "Unable to get time of day\n");
		return (0L);
	}
	return (htonl((long)tv.tv_sec + 2208988800));
}

/* ARGSUSED */
machtime_stream(s, sep)
	int s;
	struct servtab *sep;
{
	long result;

	result = machtime();
	(void) write(s, (char *) &result, sizeof(result));
}

/* ARGSUSED */
machtime_dg(s, sep)
	int s;
	struct servtab *sep;
{
	long result;
	struct sockaddr sa;
	int size;

	size = sizeof(sa);
	if (recvfrom(s, (char *)&result, sizeof(result), 0, &sa, &size) < 0)
		return;
	result = machtime();
	(void) sendto(s, (char *) &result, sizeof(result), 0, &sa, sizeof(sa));
}

/* ARGSUSED */
daytime_stream(s, sep)		/* Return human-readable time of day */
	int s;
	struct servtab *sep;
{
	char buffer[256];
	time_t time(), clock;
	char *ctime();

	clock = time((time_t *) 0);

	sprintf(buffer, "%s\r", ctime(&clock));
	(void) write(s, buffer, strlen(buffer));
}

/* ARGSUSED */
daytime_dg(s, sep)		/* Return human-readable time of day */
	int s;
	struct servtab *sep;
{
	char buffer[256];
	time_t time(), clock;
	struct sockaddr sa;
	int size;
	char *ctime();

	clock = time((time_t *) 0);

	size = sizeof(sa);
	if (recvfrom(s, buffer, sizeof(buffer), 0, &sa, &size) < 0)
		return;
	sprintf(buffer, "%s\r", ctime(&clock));
	(void) sendto(s, buffer, strlen(buffer), 0, &sa, sizeof(sa));
}

/*
 * print_service:
 *	Dump relevant information to stderr
 */
print_service(action, sep)
	char *action;
	struct servtab *sep;
{
	fprintf(stderr,
	    "%s: %s proto=%s, wait=%d, user=%s builtin=%x server=%s\n",
	    action, sep->se_service, sep->se_proto,
	    sep->se_wait, sep->se_user, sep->se_bi, sep->se_server);
}
