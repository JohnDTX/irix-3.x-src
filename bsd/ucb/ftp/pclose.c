/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)pclose.c	1.2 (Berkeley) 3/7/86";
#endif not lint

#include <stdio.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/wait.h>

#define	tst(a,b)	(*mode == 'r'? (b) : (a))
#define	RDR	0
#define	WTR	1

extern	char *malloc();

static	int *popen_pid;
static	int nfiles;

FILE *
mypopen(cmd,mode)
	char *cmd;
	char *mode;
{
	int p[2];
	int myside, hisside, pid;

	if (nfiles <= 0)
		nfiles = getdtablesize();
	if (popen_pid == NULL) {
		popen_pid = (int *)malloc((unsigned) nfiles * sizeof *popen_pid);
		if (popen_pid == NULL)
			return (NULL);
		for (pid = 0; pid < nfiles; pid++)
			popen_pid[pid] = -1;
	}
	if (pipe(p) < 0)
		return (NULL);
	myside = tst(p[WTR], p[RDR]);
	hisside = tst(p[RDR], p[WTR]);
#ifdef sgi
	if ((pid = fork()) == 0) {
#else
	if ((pid = vfork()) == 0) {
#endif
		/* myside and hisside reverse roles in child */
		(void) close(myside);
		if (hisside != tst(0, 1)) {
			(void) dup2(hisside, tst(0, 1));
			(void) close(hisside);
		}
		execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
		_exit(127);
	}
	if (pid == -1) {
		(void) close(myside);
		(void) close(hisside);
		return (NULL);
	}
	popen_pid[myside] = pid;
	(void) close(hisside);
	return (fdopen(myside, mode));
}

pabort()
{
	extern int mflag;

	mflag = 0;
}

mypclose(ptr)
	FILE *ptr;
{
#ifdef sgi
#ifdef SVR3
	int child, pid, pabort();
	void (*omask1)(), (*omask2)(), (*istat)();
#else
	int child, pid, (*omask1)(), (*omask2)(), pabort(), (*istat)();
#endif
#else
	int child, pid, omask, pabort(), (*istat)();
#endif
	union wait status;

	child = popen_pid[fileno(ptr)];
	popen_pid[fileno(ptr)] = -1;
	(void) fclose(ptr);
	if (child == -1)
		return (-1);
	istat = signal(SIGINT, pabort);
#ifdef sgi
	omask1 = signal(SIGQUIT, SIG_IGN);
	omask2 = signal(SIGHUP, SIG_IGN);
#else
	omask = sigblock(sigmask(SIGQUIT)|sigmask(SIGHUP));
#endif
	while ((pid = wait(&status)) != child && pid != -1)
		;
#ifdef sgi
	(void)signal(SIGQUIT, omask1);
	(void)signal(SIGHUP, omask2);
#else
	(void) sigsetmask(omask);
#endif
	(void) signal(SIGINT, istat);
	return (pid == -1 ? -1 : 0);
}
