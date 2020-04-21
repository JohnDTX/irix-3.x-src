#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <signal.h>
#include "idb.h"

extern int	pipe_kids [NOFILE];

waitpipe (fd)
	int		fd;
{
	int	 	pid, w, status;
	void		(*istat) (), (*qstat) ();

	if (fd < 0 || fd >= NOFILE) return (-1);
	if (pipe_kids [fd] == 0) return (-1);
#ifdef notdef
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	while ((w = wait (&status)) != pipe_kids [fd] && w != -1) {
		;
	}
	signal (SIGINT, istat);
	signal (SIGQUIT, qstat);
	pipe_kids [fd] = 0;
	return ((w == -1 || status != 0) ? -1 : 0);
#else
	return (0);
#endif
}
