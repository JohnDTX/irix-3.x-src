#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <signal.h>
#include "idb.h"

runline (cmd, volume)
	char		*cmd;
	int		volume;
{
	char		*argv [64];
	int		argc, r;
	Memset		*mset;

	mset = idb_newset ();
	if ((argc = words (cmd, argv, mset)) <= 0) {
		r = -1;
	} else {
		r = run (argc, argv, volume);
	}
	idb_dispose (mset);
	return (r);
}
