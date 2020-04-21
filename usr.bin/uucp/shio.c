/* @(#)shio.c	1.3 */
#include "uucp.h"
#include <stdio.h>
#include <signal.h>


/*
 * use shell to execute command with
 * fi and fo as standard input/output
 * user name
 *	cmd 	-> command to execute
 *	fi 	-> standard input
 *	fo 	-> standard output
 *	user 	-> user name to execute process
 * return:
 *	0		-> success 
 *	non zero	-> failure  -  status from child
 */
shio(cmd, fi, fo, user)
char *cmd, *fi, *fo, *user;
{
	register int pid, ret;
	register int f;
	int status, uid;
	char path[MAXFULLNAME];
	void exit();

	if (fi == NULL)
		fi = "/dev/null";
	if (fo == NULL)
		fo = "/dev/null";

	DEBUG(3, "shio - %s\n", cmd);
	if ((pid = fork()) == 0) {
		signal(SIGINT, SIG_IGN);
		signal(SIGHUP, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		closelog();
		close(Ifn);
		close(Ofn);
		close(0);
		if (user == NULL
		|| (gninfo(user, &uid, path) != 0)
		|| setuid(uid))
			setuid(getuid());
		f = open(fi, 0);
		if (f != 0)
			exit(f);
		close(1);
		f = creat(fo, 0666);
		if (f != 1)
			exit(f);
		execle(SHELL, "sh", "-c", cmd, 0, Env);
		exit(100);
	}
	while ((ret = wait(&status)) != pid && ret != -1);
	DEBUG(3, "status %d\n", status);
	return(status);
}

#define	tst(a,b) (*mode == 'r'? (b) : (a))
#define	RDR	0
#define	WTR	1

extern FILE *fdopen();
extern int execle(), fork(), pipe(), close(), fcntl();
static int popen_pid[20];

FILE *
popen(cmd, mode)
char	*cmd, *mode;
{
	int	p[2];
	register int myside, yourside, pid;

	if(pipe(p) < 0)
		return(NULL);
	myside = tst(p[WTR], p[RDR]);
	yourside = tst(p[RDR], p[WTR]);
	if((pid = fork()) == 0) {
		/* myside and yourside reverse roles in child */
		int	stdio;

		stdio = tst(0, 1);
		(void) close(myside);
		(void) close(stdio);
		(void) fcntl(yourside, 0, stdio);
		(void) close(yourside);
		(void) execle("/bin/sh", "sh", "-c", cmd, 0, Env);
		_exit(1);
	}
	if(pid == -1)
		return(NULL);
	popen_pid[myside] = pid;
	(void) close(yourside);
	return(fdopen(myside, mode));
}

int
pclose(ptr)
FILE	*ptr;
{
	register int f, r;
	int status, (*hstat)(), (*istat)(), (*qstat)();

	f = fileno(ptr);
	(void) fclose(ptr);
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	hstat = signal(SIGHUP, SIG_IGN);
	while((r = wait(&status)) != popen_pid[f] && r != -1)
		;
	if(r == -1)
		status = -1;
	(void) signal(SIGINT, istat);
	(void) signal(SIGQUIT, qstat);
	(void) signal(SIGHUP, hstat);
	return(status);
}
