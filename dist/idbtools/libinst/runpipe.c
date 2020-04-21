#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <signal.h>
#include "idb.h"

#define R	0
#define W	1

int	pipe_kids [NOFILE];

runpipe (argc, argv, mode)
	int		argc;
	char		*argv [];
	int		mode;
{
	char		cmd [1024], *p;
	int	 	pipefd [2], fd, pid;

	if (pipe (pipefd) < 0) {
		perror (argv [0]); return (-1);
	}
	if ((pid = fork ()) == 0) {
		if (mode == R) {
			close (1);
			close (2);
			fcntl (pipefd [W], F_DUPFD, 1);
			fcntl (pipefd [W], F_DUPFD, 2);
			close (pipefd [R]);
			close (0);
		} else {
			close (0);
			fcntl (pipefd [R], F_DUPFD, 0);
			close (pipefd [W]);
			close (1);
		}
		strcpy (cmd, argv [0]);
		for (p = argv [0] + strlen (argv [0]) - 1;
		    p >= argv [0] && *p != '/'; --p) ;
		argv [0] = p + 1;
		argv [argc] = NULL;
		execv (cmd, argv);
		perror (cmd);
		_exit (127);
	}
	if (mode == R) {
		fd = pipefd [R];
		close (pipefd [W]);
	} else {
		fd = pipefd [W]; close (pipefd [R]);
	}
	if (pid == -1) {
		close (pipefd [R]); close (pipefd [W]);
		perror (cmd);
		return (-1);
	} else {
		pipe_kids [fd] = pid;
		return (fd);
	}
}
