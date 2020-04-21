/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char *sccsid = "@(#)popen.c	5.2 (Berkeley) 6/21/85";
#endif not lint

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#ifdef	USG
#include <fcntl.h>
#endif

#define	tst(a,b)	(*mode == 'r'? (b) : (a))
#define	RDR	0
#define	WTR	1
static	int	popen_pid[20];

#ifndef VMUNIX
#define vfork	fork
#endif VMUNIX
#ifndef	SIGRETRO
#define	sigchild()
#endif

#ifndef	VMUNIX
/*
 * invok shell to execute command waiting for
 * command to terminate
 *	s	-> command string
 * return:
 *	status	-> command exit status
 */
system(s)
char *s;
{
	register int pid, w;
	int status;
#ifdef SVR3
	void (*istat)(), (*qstat)();
#else
	int (*istat)(), (*qstat)();
#endif

	/*
	 * Spawn the shell to execute command, however,
	 * since the mail command runs setgid mode
	 * reset the effective group id to the real
	 * group id so that the command does not
	 * acquire any special privileges
	 */
	if ((pid = fork()) == 0) {
		setuid(getuid());
		setgid(getgid());
		execl("/bin/sh", "sh", "-c", s, NULL);
		_exit(127);
	}

	/*
	 * Parent temporarily ignores signals so it 
	 * will remain around for command to finish
	 */
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	while ((w = wait(&status)) != pid && (w >= 0))
		;
	if (w < 0)
		status = -1;
	signal(SIGINT, istat);
	signal(SIGQUIT, qstat);
	return (status);
}
#endif

FILE *
popen(cmd,mode)
char	*cmd;
char	*mode;
{
	int p[2];
	register myside, hisside, pid;

	if(pipe(p) < 0)
		return NULL;
	myside = tst(p[WTR], p[RDR]);
	hisside = tst(p[RDR], p[WTR]);
	if((pid = vfork()) == 0) {
		/* myside and hisside reverse roles in child */
		sigchild();
		close(myside);
#ifndef	USG
		dup2(hisside, tst(0, 1));
#else
		{
			int	stdio;
			stdio = tst(0, 1);
			close(stdio);
			(void) fcntl(hisside, F_DUPFD, stdio);
			(void) setuid(getuid());
			(void) setgid(getgid());
		}
#endif
		close(hisside);
		execl("/bin/csh", "sh", "-c", cmd, 0);
		_exit(1);
	}
	if(pid == -1)
		return NULL;
	popen_pid[myside] = pid;
	close(hisside);
	return(fdopen(myside, mode));
}

pclose(ptr)
FILE *ptr;
{
	register f, r;
	int status, omask;
	extern int errno;
#ifdef SVR3
	void (*istat)(), (*qstat)(), (*hstat)();
#else
	int (*istat)(), (*qstat)(), (*hstat)();
#endif

	f = fileno(ptr);
	fclose(ptr);
# ifdef VMUNIX
	omask = sigblock(sigmask(SIGINT)|sigmask(SIGQUIT)|sigmask(SIGHUP));
# else
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	hstat = signal(SIGHUP, SIG_IGN);
# endif	VMUNIX
	while((r = wait(&status)) != popen_pid[f] && r != -1 && errno != EINTR)
		;
	if(r == -1)
		status = -1;
# ifdef VMUNIX
	sigsetmask(omask);
# else
	(void) signal(SIGINT, istat);
	(void) signal(SIGQUIT, qstat);
	(void) signal(SIGHUP, hstat);
# endif	VMUNIX
	return(status);
}
