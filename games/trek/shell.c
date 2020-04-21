static char ID[] = "@(#)shell.c	1.1";
/* $Source: /d2/3.7/src/games/trek/RCS/shell.c,v $ */
static	char	*Sccsid = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 15:47:05 $ */

#include "trek.h"
/**
 **	call the shell
 **/

shell()
{
	int		i;
	register int	pid;
	register int	oldsig, oldqit;
	char		*sh;
	char		*getenv();

	if (!(pid = fork()))
	{
		setuid(getuid());
		nice(0);
		signal(SIGINT, 0);
		execl("/bin/sh", "-", 0);
		sh = getenv("SHELL");
		if (!sh || !*sh)
			sh = "/bin/sh";
		execl(sh,"Cap't Kirk",0);
		printf("cannot execute %s",sh);
		syserr();
	}
	oldsig=signal(2,1); oldqit=signal(3,1);
	while (wait(&i) != pid) ;
	printf("trek!\n");
	signal(2,oldsig); signal(3,oldqit);
}
