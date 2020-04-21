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
static char sccsid[] = "@(#)rsh.c	5.4 (Berkeley) 8/28/85";
#endif not lint

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>

#include <netinet/in.h>

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <pwd.h>
#include <netdb.h>

#ifdef sgi
#ifdef SVR3
#define RLOGIN "/usr/bsd/rlogin"	/* look for rlogin(1) here */
#else
#define RLOGIN "/usr/bin/rlogin"	/* look for rlogin(1) here */
#endif
#endif

/*
 * rsh - remote shell
 */
/* VARARGS */
int	error();
#if defined(sgi) && defined(mips)
char	*index(), *rindex(), *malloc(), *getpass(), *strcpy();
#else
char	*index(), *rindex(), *malloc(), *getpass(), *sprintf(), *strcpy();
#endif

struct	passwd *getpwuid();

int	errno;
int	options;
int	rfd2;
int	sendsig();

#define	mask(s)	(1 << ((s) - 1))

main(argc, argv0)
	int argc;
	char **argv0;
{
	int rem, pid;
	char *host, *cp, **ap, buf[BUFSIZ], *args, **argv = argv0, *user = 0;
	register int cc;
	int asrsh = 0;
	struct passwd *pwd;
	int readfrom, ready;
	int one = 1;
	struct servent *sp;
	int omask;

#ifdef sgi				/* work around sh bug */
	for (cc = 3; cc < getdtablesize(); cc++)	/* which starts lots */
		(void)close(cc);			/* of open files */
#endif
	host = rindex(argv[0], '/');
	if (host)
		host++;
	else
		host = argv[0];
	argv++, --argc;
	if (!strcmp(host, "rsh")) {
		host = *argv++, --argc;
		asrsh = 1;
	}
another:
	if (argc > 0 && !strcmp(*argv, "-l")) {
		argv++, argc--;
		if (argc > 0)
			user = *argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-n")) {
		argv++, argc--;
		(void) close(0);
		(void) open("/dev/null", 0);
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-d")) {
		argv++, argc--;
		options |= SO_DEBUG;
		goto another;
	}
	/*
	 * Ignore the -L, -w, -e and -8 flags to allow aliases with rlogin
	 * to work
	 *
	 * There must be a better way to do this! -jmb
	 */
	if (argc > 0 && !strncmp(*argv, "-L", 2)) {
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strncmp(*argv, "-w", 2)) {
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strncmp(*argv, "-e", 2)) {
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strncmp(*argv, "-8", 2)) {
		argv++, argc--;
		goto another;
	}
	if (host == 0)
		goto usage;
	if (argv[0] == 0) {
		if (asrsh)
			*argv0 = "rlogin";
#ifdef sgi
		execv(RLOGIN, argv0);
		perror(RLOGIN);
#else
		execv("/usr/ucb/rlogin", argv0);
		perror("/usr/ucb/rlogin");
#endif
		exit(1);
	}
	pwd = getpwuid(getuid());
	if (pwd == 0) {
		fprintf(stderr, "who are you?\n");
		exit(1);
	}
	cc = 0;
	for (ap = argv; *ap; ap++)
		cc += strlen(*ap) + 1;
	cp = args = malloc(cc);
	for (ap = argv; *ap; ap++) {
		(void) strcpy(cp, *ap);
		while (*cp)
			cp++;
		if (ap[1])
			*cp++ = ' ';
	}
	sp = getservbyname("shell", "tcp");
	if (sp == 0) {
		fprintf(stderr, "rsh: shell/tcp: unknown service\n");
		exit(1);
	}
        rem = rcmd(&host, sp->s_port, pwd->pw_name,
	    user ? user : pwd->pw_name, args, &rfd2);
        if (rem < 0)
                exit(1);
	if (rfd2 < 0) {
		fprintf(stderr, "rsh: can't establish stderr\n");
		exit(2);
	}
	if (options & SO_DEBUG) {
		if (setsockopt(rem, SOL_SOCKET, SO_DEBUG, &one, sizeof (one)) < 0)
			perror("setsockopt (stdin)");
		if (setsockopt(rfd2, SOL_SOCKET, SO_DEBUG, &one, sizeof (one)) < 0)
			perror("setsockopt (stderr)");
	}
	(void) setuid(getuid());
#ifndef sgi
	omask = sigblock(mask(SIGINT)|mask(SIGQUIT)|mask(SIGTERM));
#endif sgi
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, sendsig);
	if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
		signal(SIGQUIT, sendsig);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, sendsig);
#ifdef sgi
	if (signal(SIGPIPE, SIG_IGN) != SIG_IGN)
		signal(SIGPIPE, sendsig);
#endif sgi
        pid = fork();
        if (pid < 0) {
		perror("fork");
                exit(1);
        }
	ioctl(rfd2, FIONBIO, &one);
	ioctl(rem, FIONBIO, &one);
        if (pid == 0) {
		char *bp; int rembits, wc;
	reread:
		errno = 0;
		cc = read(0, buf, sizeof buf);
#ifdef sgi
		if (cc <= 0) {
			if (cc == 0)
				goto done;
			if (errno == EINTR || errno == EAGAIN)
				goto reread;
		}
#else
		if (cc <= 0)
			goto done;
#endif
		bp = buf;
	rewrite:
		rembits = 1<<rem;
		if (select(16, 0, &rembits, 0, 0) < 0) {
			if (errno != EINTR) {
				perror("select");
				exit(1);
			}
			goto rewrite;
		}
		if ((rembits & (1<<rem)) == 0)
			goto rewrite;
		wc = write(rem, bp, cc);
		if (wc < 0) {
			if (errno == EWOULDBLOCK)
				goto rewrite;
			goto done;
		}
		cc -= wc; bp += wc;
		if (cc == 0)
			goto reread;
		goto rewrite;
	done:
  		(void) shutdown(rem, 1);
		exit(0);
	}
#ifdef sgi
	(void)signal(SIGINT, SIG_IGN);
	(void)signal(SIGQUIT, SIG_IGN);
	(void)signal(SIGTERM, SIG_IGN);
#else
	sigsetmask(omask);
#endif
	readfrom = (1<<rfd2) | (1<<rem);
	do {
		ready = readfrom;
		if (select(16, &ready, 0, 0, 0) < 0) {
			if (errno != EINTR) {
				perror("select");
				exit(1);
			}
			continue;
		}
		if (ready & (1<<rfd2)) {
			errno = 0;
			cc = read(rfd2, buf, sizeof buf);
			if (cc <= 0) {
				if (errno != EWOULDBLOCK)
					readfrom &= ~(1<<rfd2);
			} else
				(void) write(2, buf, cc);
		}
		if (ready & (1<<rem)) {
			errno = 0;
			cc = read(rem, buf, sizeof buf);
			if (cc <= 0) {
				if (errno != EWOULDBLOCK)
					readfrom &= ~(1<<rem);
			} else
				(void) write(1, buf, cc);
		}
        } while (readfrom);
        (void) kill(pid, SIGKILL);
	exit(0);
usage:
	fprintf(stderr,
	    "usage: rsh host [ -l login ] [ -n ] command\n");
	exit(1);
}

#ifdef sgi
sendsig(signo)
int signo;
{
	char sigchar = signo;

 	(void)signal(signo, sendsig); 
 	(void) write(rfd2, &sigchar, 1);
}
#else
sendsig(signo)
	char signo;
{

	(void) write(rfd2, &signo, 1);
}
#endif
