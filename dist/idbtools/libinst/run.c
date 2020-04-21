#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <signal.h>
#include "idb.h"
#include "inst.h"

run (argc, argv, volume)
	int		argc;
	char		*argv [];
	int		volume;
{
	char		cmd [1024], *p;
	int	 	pid, w, status;
	void		(*istat) (), (*qstat) ();

	if ((pid = fork ()) == 0) {
		strcpy (cmd, argv [0]);
		for (p = argv [0] - 1; p >= argv [0] && *p != '/'; --p) ;
		argv [0] = p + 1;
		argv [argc] = NULL;
		if (volume == Silent) {
			close (1);
			close (2);
			open ("/dev/null", 1);
			open ("/dev/null", 1);
		}
		execv (cmd, argv);
		perror (cmd);
		_exit (127);
	}
	if (pid == -1) {
		perror (cmd);
		return (-1);
	}
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	while ((w = wait (&status)) != pid && w != -1) ;
	signal (SIGINT, istat);
	signal (SIGQUIT, qstat);
	return ((w == -1) ? w : status);
}
