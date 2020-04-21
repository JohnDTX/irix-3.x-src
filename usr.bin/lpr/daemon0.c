/*	@(#)daemon0.c	1.2	*/
/*	3.0 SID #	1.3	*/
/*
 *  daemon0.c -- dem_setup() and logerr() routines for spider and
 *			and dataphone dpd and fget daemons.
 */

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<pwd.h>
#include	<signal.h>

#define	WRMODE	2

#define	FCLOSE(F)	if(F != NULL){ fclose(F);  F = NULL;}
#define	DAEMUID	1

extern char **environ;
char	*endenv;
char	*DAEMNAM;
char	*argv1;
struct	stat	statbuf;
struct passwd *getpwuid();
char	*ctime();
long	time();

unlock()
{
	signal(SIGTERM, SIG_IGN);
	dem_dis();
	logerr("Daemon killed.");
	unlink(lock);
	exit(1);
}


dem_setup(argc, argv)
int	argc;
char	**argv;
{
	int i;
	int of;

	endenv = (char *)NULL;	/*clean out old environment.*/
	environ = &endenv;
	DAEMNAM = argv[0];
	setuid(DAEMUID);
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, unlock);
#ifndef	DEBUG
	signal(SIGQUIT, SIG_IGN);
/*
 * Close all files, open /dev/null as 0, 1, 2
 * to assure standard environment
 */
	freopen("/dev/null", "r", stdin);
	freopen("/dev/null", "w", stdout);
	freopen("/dev/null", "w", stderr);
	for (of=3; of<_NFILE; of++)
		close(of);
#endif
	if ((of=creat(lock, 0)) < 0)
		exit(0);
	if(fstat(of, &statbuf) < 0 || statbuf.st_mode != 0100000){
		logerr("Bad lock file %s.", lock);
		exit(1);
	}
	close(of);
#ifndef	DEBUG
	if (i = fork()){
		if(i == -1){
			logerr("Unable to fork.");
			unlink(lock);
			exit(1);
		}
		exit(0);
	}
#endif
	chdir(dpd);
	if (argc == 2) {
		argv1 = argv[1];
		tlimit = 0;
		i = 0;
		while ( argv[1][i] ) tlimit = 10*tlimit +  argv[1][i++] - '0' ;
	}
#if LPD == 0
	if(i = config()){
		logerr("Bad return from config: %d", i);
		unlink(lock);
		exit(2);
	}
#endif
}


#if LPD == 0

int	nlog	= 0;

/* VARARGS */
logerr(s, a1, a2, a3, a4)
char	*s;
int	a1, a2, a3, a4;
{
	long static tb;
	register i;
	FILE *f;
	struct passwd *pwp;

#if DEBUG >= 3
	if (tlimit == 0)
		abort();
#endif
#ifdef	DEBUG
	fprintf(stderr, s, a1, a2, a3, a4); putc('\n', stderr); /* PRINTF */
#endif
	if(access(error, WRMODE) != 0)
		return;
	if((f = fopen(error, "a")) == NULL)
		return;
	time(&tb);
	fprintf(f, "%.19s:%s:", ctime(&tb), dname);
	fprintf(f, s, a1, a2, a3, a4);
#if FGET == 0
	i = snsum/1000;
	if(i <= 0){
		i = snsum;
		fprintf(f, "  %3d bytes", i);
	}
	else
		fprintf(f, " %3dk bytes", i);
#endif
	if(!nlog++ && ((pwp = getpwuid(getuid())) != NULL)){
		putc(' ', f);  putc(' ', f);
		i = 0;
		while(pwp->pw_name[i])
			putc(pwp->pw_name[i++], f);
	}
	putc('\n', f);
	fclose(f);
}

#endif
