static char *sccsid = "@(#)update.c	4.2 (Berkeley) 10/16/80";
/* $Source: /d2/3.7/src/etc/RCS/update.c,v $ */
static	char	*Sccsid = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 15:38:50 $ */
/*
 * Update the file system every 30 seconds.
 * For cache benefit, open certain system directories.
 */

#include <signal.h>
#include <stdio.h>

char *fillst[] = {
	"/bin",
	"/lib",
	"/usr",
	"/usr/bin",
	"/usr/lib",
	0,
};

main()
{
	char	**f;

	signal(SIGHUP,SIG_IGN);
	signal(SIGINT,SIG_IGN);
	signal(SIGQUIT,SIG_IGN);
	signal(SIGTERM,SIG_IGN);
	if (fork())
		exit(0);
	close(0);
	close(1);
	close(2);
	for (f = fillst; *f; f++)
		open(*f,0);
	for (;;) {
		sync();
		sleep(30);
	}
}
