/* @(#)xqt.c	1.3 */
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/usr.bin/uucp/RCS/xqt.c,v 1.1 89/03/27 18:31:11 root Exp $";
/*
 * $Log:	xqt.c,v $
 * Revision 1.1  89/03/27  18:31:11  root
 * Initial check-in for 3.7
 * 
 * Revision 1.1  87/09/23  17:05:37  vjs
 * Initial revision
 * 
 * Revision 1.3  85/02/07  22:10:40  bob
 * Fixed to close file descriptors 3 to 19 before execint.
 * 
 * Revision 1.2  85/02/07  21:35:50  bob
 * Fixed 8 char sys name bugs
 * 
 */
#include "uucp.h"
#include <signal.h>

void exit();

/*
 * start up uucico for rmtname
 * return:
 *	none
 */
xuucico(rmtname)
char *rmtname;
{

	/*
	 * start uucico for rmtname system
	 */
	if (fork() == 0) {
		euucico(rmtname);
	}
	return;
}
euucico(rmtname)
char	*rmtname;
{
	char opt[100];
	int i;

	close(0);
	close(1);
	close(2);
	open("/dev/null", 0);
	open("/dev/null", 1);
	open("/dev/null", 1);
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	closelog();
	if (rmtname[0] != '\0')
		sprintf(opt, "-s%.*s", SYSNSIZE, rmtname);
	else
		opt[0] = '\0';
	for (i=3; i<20; i++)
		close(i);
	execle(UUCICO, "UUCICO", "-r1", opt, 0, Env);
	exit(100);
}


/*
 * start up uuxqt
 * return:
 *	none
 */
xuuxqt()
{
	int i;

	/*
	 * start uuxqt
	 */
	if (fork() == 0) {
		close(0);
		close(1);
		close(2);
		open("/dev/null", 2);
		open("/dev/null", 2);
		open("/dev/null", 2);
		signal(SIGINT, SIG_IGN);
		signal(SIGHUP, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		closelog();
		for (i=3; i<20; i++)
			close(i);
		execle(UUXQT, "UUXQT",  0, Env);
		exit(100);
	}
	return;
}
xuucp(str)
char *str;
{
	char text[300];
	int i;
	unsigned sleep();

	/*
	 * start uucp
	 */
	if (fork() == 0) {
		close(0);
		close(1);
		close(2);
		open("/dev/null", 0);
		open("/dev/null", 1);
		open("/dev/null", 1);
		signal(SIGINT, SIG_IGN);
		signal(SIGHUP, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		closelog();
		sprintf(text, "%s -r %s", UUCP, str);
		for (i=3; i<20; i++)
			close(i);
		execle(SHELL, "sh", "-c", text, 0, Env);
		exit(100);
	}
	sleep(15);
	return;
}
