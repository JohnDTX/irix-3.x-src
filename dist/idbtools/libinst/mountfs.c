#include <sys/types.h>
#include <sys/stat.h>
#include "idb.h"
#include "inst.h"

extern char	usrdev [];
extern char	rootdev [];

mountfs (dev, path)
	char		*dev;
	char		*path;
{
	char		buff [Strsize], *argv [32];
	int		r, argc;
	Memset		*mset;

	argv [0] = "/etc/fsstat";
	argv [1] = dev;
	if (run (2, argv, Silent) == (1 << 8)) {
		argv [0] = "/etc/fsck";
		argv [1] = "-y";
		argv [2] = dev;
		fprintf (stderr, "checking file system %s; please wait.\n",
			dev);
		run (3, argv, Silent);
	}
	argv [0] = "/etc/mount";
	argv [1] = dev;
	argv [2] = path;
	if (vline (3, argv, buff) < 0) return (-1);
	if ((argc = words (buff, argv, mset = idb_newset ())) > 0 &&
	    (argc < 4 || strcmp (argv [2], "already") != 0)) {
	 	r = -1;
	} else {
		r = 0;
	}
	idb_dispose (mset);
	return (r);
}
